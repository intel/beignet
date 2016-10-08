#include <cstdint>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"
#include "utest_file_map.hpp"

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

extern "C"
{
#include <X11/Xlibint.h>
#include <X11/Xlib.h>
#include <xf86drm.h>
#include <intel_bufmgr.h>
#include <drm.h>
#include <drm_sarea.h>
#include <X11/Xmd.h>
#include <X11/Xregion.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
}

typedef cl_mem (OCLCREATEIMAGEFROMLIBVAINTEL)(cl_context, const cl_libva_image *, cl_int *);
OCLCREATEIMAGEFROMLIBVAINTEL *oclCreateImageFromLibvaIntel = NULL;

// part of following code is copy from beignet/src/x11/
typedef struct {
    CARD8   reqType;
    CARD8   dri2Reqtype;
    CARD16  length B16;
    CARD32  window B32;
    CARD32  magic B32;
} xDRI2AuthenticateReq;
#define sz_xDRI2AuthenticateReq   12

typedef struct {
    BYTE    type;   /* X_Reply */
    BYTE    pad1;
    CARD16  sequenceNumber B16;
    CARD32  length B32;
    CARD32  authenticated B32;
    CARD32  pad2 B32;
    CARD32  pad3 B32;
    CARD32  pad4 B32;
    CARD32  pad5 B32;
    CARD32  pad6 B32;
} xDRI2AuthenticateReply;
#define sz_xDRI2AuthenticateReply	32

#define X_DRI2Authenticate		2

static char va_dri2ExtensionName[] = "DRI2";
static XExtensionInfo _va_dri2_info_data;
static XExtensionInfo *va_dri2Info = &_va_dri2_info_data;
static XEXT_GENERATE_CLOSE_DISPLAY (VA_DRI2CloseDisplay, va_dri2Info)
static /* const */ XExtensionHooks va_dri2ExtensionHooks = {
    NULL,				/* create_gc */
    NULL,				/* copy_gc */
    NULL,				/* flush_gc */
    NULL,				/* free_gc */
    NULL,				/* create_font */
    NULL,				/* free_font */
    VA_DRI2CloseDisplay,		/* close_display */
    NULL,				/* wire_to_event */
    NULL,				/* event_to_wire */
    NULL,				/* error */
    NULL,				/* error_string */
};

static XEXT_GENERATE_FIND_DISPLAY (DRI2FindDisplay, va_dri2Info,
				   va_dri2ExtensionName,
				   &va_dri2ExtensionHooks,
				   0, NULL)

static Bool VA_DRI2Authenticate(Display *dpy, XID window, drm_magic_t magic)
{
    XExtDisplayInfo *info = DRI2FindDisplay(dpy);
    xDRI2AuthenticateReq *req;
    xDRI2AuthenticateReply rep;

    XextCheckExtension (dpy, info, va_dri2ExtensionName, False);

    LockDisplay(dpy);
    GetReq(DRI2Authenticate, req);
    req->reqType = info->codes->major_opcode;
    req->dri2Reqtype = X_DRI2Authenticate;
    req->window = window;
    req->magic = magic;

    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }

    UnlockDisplay(dpy);
    SyncHandle();

    return rep.authenticated;
}


void runtime_climage_from_boname(void)
{
  const int w = 1024;
  const int h = 256;
  const int hStart = 128;
  const int offset = hStart * w;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("runtime_climage_from_boname");

  int fd = open("/dev/dri/card0", O_RDWR);
  OCL_ASSERT(fd>0);

  drm_magic_t magic;
  drmGetMagic(fd, &magic);

  Display* dpy = XOpenDisplay(NULL);
  if (dpy == NULL) {
    fprintf(stderr, " Can't open Display, skipping.\n");
    return; 
  }
  XID root = RootWindow(dpy, DefaultScreen(dpy));

  Bool auth = VA_DRI2Authenticate(dpy, root, magic);
  OCL_ASSERT(auth);

  drm_intel_bufmgr* bufmgr = drm_intel_bufmgr_gem_init(fd, 1024);
  OCL_ASSERT(bufmgr != NULL);

  drm_intel_bo * bo = drm_intel_bo_alloc(bufmgr, "runtime_climage_from_boname", w*h, 0);
  OCL_ASSERT(bo != NULL);

  drm_intel_bo_map(bo, 0);
  unsigned char* addr = (unsigned char*)bo->virt;
  memset(addr, 0xCD, w*h);
  drm_intel_bo_unmap(bo);

  unsigned int boName = 0;
  drm_intel_bo_flink(bo, &boName);

  cl_image_format fmt;
  fmt.image_channel_order = CL_R;
  fmt.image_channel_data_type = CL_UNORM_INT8;

  cl_libva_image imageParam;
  imageParam.fmt = fmt;
  imageParam.bo_name = boName;
  imageParam.offset = offset;
  imageParam.width = w;
  imageParam.height = h - hStart;
  imageParam.row_pitch = w;

#ifdef CL_VERSION_1_2
  oclCreateImageFromLibvaIntel = (OCLCREATEIMAGEFROMLIBVAINTEL *)clGetExtensionFunctionAddressForPlatform(platform, "clCreateImageFromLibvaIntel");
#else
  oclCreateImageFromLibvaIntel = (OCLCREATEIMAGEFROMLIBVAINTEL *)clGetExtensionFunctionAddress("clCreateImageFromLibvaIntel");
#endif
  if(!oclCreateImageFromLibvaIntel){
    fprintf(stderr, "Failed to get extension clCreateImageFromLibvaIntel\n");
    OCL_ASSERT(0);
  }
  cl_mem dst = oclCreateImageFromLibvaIntel(ctx, &imageParam, NULL);

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &dst);
  globals[0] = w;
  globals[1] = h-hStart;
  locals[0] = 16;
  locals[1] = 16;
  OCL_NDRANGE(2);

  OCL_FINISH();

  drm_intel_bo_map(bo, 0);
  addr = (unsigned char*)bo->virt;
  for (int i = 0; i < hStart; ++i) {
    for (int j = 0; j < w; ++j) {
      OCL_ASSERT(addr[j+i*w]==0xCD);
    }
  }
  for (int i = hStart; i < h; ++i) {
    for (int j = 0; j < w; ++j) {
      OCL_ASSERT(addr[j+i*w]==(unsigned char)(0.34*255+0.5));
    }
  }
  drm_intel_bo_unmap(bo);


  // Run the kernel for the seconde time
  OCL_SET_ARG(0, sizeof(cl_mem), &dst);
  globals[0] = w;
  globals[1] = h-hStart;
  locals[0] = 16;
  locals[1] = 16;
  OCL_NDRANGE(2);

  OCL_FINISH();

  drm_intel_bo_map(bo, 0);
  addr = (unsigned char*)bo->virt;
  for (int i = 0; i < hStart; ++i) {
    for (int j = 0; j < w; ++j) {
      OCL_ASSERT(addr[j+i*w]==0xCD);
    }
  }
  for (int i = hStart; i < h; ++i) {
    for (int j = 0; j < w; ++j) {
      OCL_ASSERT(addr[j+i*w]==(unsigned char)(0.34*255+0.5));
    }
  }
  drm_intel_bo_unmap(bo);

  clReleaseMemObject(dst);
  drm_intel_bo_unreference(bo);
  drm_intel_bufmgr_destroy(bufmgr);
  XCloseDisplay(dpy);
  close(fd);
}

MAKE_UTEST_FROM_FUNCTION(runtime_climage_from_boname);

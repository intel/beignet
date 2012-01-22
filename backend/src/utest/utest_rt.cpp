/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "renderer/renderer_obj.hpp"
#include "renderer/renderer.hpp"
#include "rt/bvh2_traverser.hpp"
#include "rt/bvh2.hpp"
#include "rt/rt_triangle.hpp"
#include "rt/rt_camera.hpp"
#include "models/obj.hpp"
#include "game/camera.hpp"
#include "image/stb_image.hpp"

#include "sys/alloc.hpp"
#include "sys/tasking.hpp"
#include "sys/tasking_utility.hpp"
#include "sys/logging.hpp"
#include "sys/default_path.hpp"

#include "utest/utest.hpp"

#include <cstring>
#include <GL/freeglut.h>
#include <cstdio>
#include <iostream>

namespace pf
{
  static Ref<Intersector> intersector  = NULL;
  static const int CAMW = 1024, CAMH = 1024;

  static const FileName objName("f000.obj");
  //static const FileName objName("arabic_city_II.obj");
  //static const FileName objName("conference.obj");
  //static const FileName objName("sibenik.obj");
  //static const FileName objName("sponza.obj");

  static RTTriangle *ObjComputeTriangle(const Obj &obj) {
    RTTriangle *tris = GBE_NEW_ARRAY(RTTriangle, obj.triNum);
    for (size_t i = 0; i < obj.triNum; ++i) {
      const vec3f &v0 = obj.vert[obj.tri[i].v[0]].p; 
      const vec3f &v1 = obj.vert[obj.tri[i].v[1]].p; 
      const vec3f &v2 = obj.vert[obj.tri[i].v[2]].p; 
      tris[i] = RTTriangle(v0,v1,v2);
    }
    return tris;
  }

  /*! Task set that computes a frame buffer with ray tracing */
  template <bool singleRay>
  class TaskRayTrace : public TaskSet
  {
  public:
    INLINE TaskRayTrace(const Intersector &intersector,
                        const RTCamera &cam,
                        const uint32 *c,
                        uint32 *rgba,
                        uint32 w, uint32 jobNum) :
      TaskSet(jobNum, "TaskRayTrace"),
      intersector(intersector), cam(cam), c(c), rgba(rgba),
      w(w), h(jobNum * RayPacket::height) {}

    virtual void run(size_t jobID)
    {
      if (singleRay) {
        RTCameraRayGen gen;
        cam.createGenerator(gen, w, h);
        for (uint32 row = 0; row < RayPacket::height; ++row) {
          const uint32 y = row + jobID * RayPacket::height;
          for (uint32 x = 0; x < w; ++x) {
            Ray ray;
            Hit hit;
            gen.generate(ray, x, y);
            intersector.traverse(ray, hit);
            rgba[x + y*w] = hit ? c[hit.id0] : 0u;
          }
        }
      } else {
        RTCameraPacketGen gen;
        cam.createGenerator(gen, w, h);
        const uint32 y = jobID * RayPacket::height;
        for (uint32 x = 0; x < w; x += RayPacket::width) {
          RayPacket pckt;
          PacketHit hit;
          gen.generate(pckt, x, y);
          intersector.traverse(pckt, hit);
          const int32 *IDs = (const int32 *) &hit.id0[0][0];
          uint32 curr = 0;
          for (uint32 j = 0; j < pckt.height; ++j) {
            for (uint32 i = 0; i < pckt.width; ++i, ++curr) {
              const uint32 offset = x + i + (y + j) * w;
              rgba[offset] = IDs[curr] != -1 ? c[IDs[curr]] : 0u;
            }
          }
        }
      }
    }

    const Intersector &intersector; //!< To traverse the scene
    const RTCamera &cam;            //!< Parameterize the view
    const uint32 *c;                //!< One color per triangle
    uint32 *rgba;                   //!< Frame buffer
    uint32 w, h;                    //!< Frame buffer dimensions
  };

  /*! Ray trace the loaded scene */
  template <bool singleRay>
  static void rayTrace(int w, int h, const uint32 *c) {
    FPSCamera fpsCam;
    const RTCamera cam(fpsCam.org, fpsCam.up, fpsCam.view, fpsCam.fov, fpsCam.ratio);
    uint32 *rgba = GBE_NEW_ARRAY(uint32, w * h);
    std::memset(rgba, 0, sizeof(uint32) * w * h);
    GBE_COMPILER_READ_WRITE_BARRIER;
    const double t = getSeconds();
    Task *rayTask = GBE_NEW(TaskRayTrace<singleRay>, *intersector,
                           cam, c, rgba, w, h/RayPacket::height);
    Task *returnToMain = GBE_NEW(TaskInterruptMain);
    rayTask->starts(returnToMain);
    rayTask->scheduled();
    returnToMain->scheduled();
    TaskingSystemEnter();
    const double dt = getSeconds() - t;
    GBE_MSG_V(dt * 1000. << " msec - " << CAMW * CAMH / dt << " rays/s");
    if (singleRay)
      stbi_write_bmp("single.bmp", w, h, 4, rgba);
    else
      stbi_write_bmp("packet.bmp", w, h, 4, rgba);
    GBE_DELETE_ARRAY(rgba);
  }

  static void RTStart(void)
  {
    Obj obj;
    size_t path = 0;
    for (path = 0; path < defaultPathNum; ++path)
      if (obj.load(FileName(defaultPath[path]) + objName)) {
        GBE_MSG_V("Obj: " << objName << " loaded from " << defaultPath[path]);
        break;
      }
    if (path == defaultPathNum)
      GBE_WARNING_V("Obj: " << objName << " not found");

    // Build the BVH
    RTTriangle *tris = ObjComputeTriangle(obj);
    Ref<BVH2<RTTriangle>> bvh = GBE_NEW(BVH2<RTTriangle>);
    buildBVH2(tris, obj.triNum, *bvh);
    GBE_DELETE_ARRAY(tris);

    // Now we have an intersector on the triangle soup
    intersector = GBE_NEW(BVH2Traverser<RTTriangle>, bvh);

    // Compute some triangle color
    uint32 *c = GBE_NEW_ARRAY(uint32, bvh->primNum);
    for (uint32 i = 0; i < bvh->primNum; ++i) {
      c[i] = rand();
      c[i] |= 0xff000000;
    }

    // Ray trace now
    GBE_MSG_V("Packet ray tracing");
    for (int i = 0; i < 16; ++i) rayTrace<false>(CAMW, CAMH, c);
    GBE_MSG_V("Single ray tracing");
    for (int i = 0; i < 16; ++i) rayTrace<true>(CAMW, CAMH, c);
    GBE_DELETE_ARRAY(c);
  }

  static void RTEnd(void) { intersector = NULL; }
}

void utest_rt(void)
{
  using namespace pf;
  RTStart();
  RTEnd();
}

UTEST_REGISTER(utest_rt);

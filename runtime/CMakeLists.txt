configure_file("OCLConfig.h.in" "OCLConfig.h")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${DRM_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	)

set(OPENCL_SRC
	cl_alloc.c
	cl_api_command_queue.c
	cl_api_context.c
	cl_api_device_id.c
	cl_api_event.c
	cl_api_kernel.c
	cl_api_mem.c
	cl_api_platform_id.c
	cl_api_program.c
	cl_api_sampler.c
	cl_base_object.c
	cl_command_queue.c
	cl_compiler.c
	cl_context.c
	cl_device_id.c
	cl_enqueue.c
	cl_event.c
	cl_extensions.c
	cl_image.c
	cl_kernel.c
	cl_mem.c
	cl_platform_id.c
	cl_program.c
	cl_sampler.c
	cl_utils.c
	)

if (OCLIcd_FOUND)
	set(OPENCL_SRC ${OPENCL_SRC} cl_khr_icd.c)
	SET(CMAKE_CXX_FLAGS "-DHAS_OCLIcd ${CMAKE_CXX_FLAGS}")
	SET(CMAKE_C_FLAGS "-DHAS_OCLIcd ${CMAKE_C_FLAGS}")
endif (OCLIcd_FOUND)

set(GIT_SHA1 "git_sha1.h")
add_custom_target(${GIT_SHA1} ALL
	COMMAND chmod +x ${CMAKE_CURRENT_SOURCE_DIR}/git_sha1.sh
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/git_sha1.sh ${CMAKE_CURRENT_SOURCE_DIR} ${GIT_SHA1}
	)

SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bsymbolic,--allow-shlib-undefined")

add_subdirectory(gen)

add_library(cl SHARED ${OPENCL_SRC})
ADD_DEPENDENCIES(cl ${GIT_SHA1})

link_directories(${LLVM_LIBRARY_DIR} ${DRM_LIBDIR} ${OPENGL_LIBDIR} ${EGL_LIBDIR})

target_link_libraries(
	cl
	rt
	cl_gen
	m
	${X11_LIBRARIES}
	${ELF_LIB_LIBRARIES}
	${XEXT_LIBRARIES}
	${XFIXES_LIBRARIES}
	${DRM_INTEL_LIBRARIES}
	${ELF_LIB_LIBRARIES}
	${DRM_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${CMAKE_DL_LIBS}
	${OPENGL_LIBRARIES}
	${EGL_LIBRARIES})
install (TARGETS cl LIBRARY DESTINATION ${BEIGNET_INSTALL_DIR})

PROJECT(LIBOCL)
SET (OCL_OBJECT_DIR ${LIBOCL_BINARY_DIR}/${BEIGNET_INSTALL_DIR})
SET (OCL_HEADER_FILES ${OCL_OBJECT_DIR}/include/ocl_defines.h)
SET (OCL_SOURCE_FILES "")

ADD_CUSTOM_COMMAND(OUTPUT ${OCL_OBJECT_DIR}/include/ocl_defines.h
    COMMAND mkdir -p ${OCL_OBJECT_DIR}/include/
    #   COMMAND echo "cat ${LIBOCL_SOURCE_DIR}/tmpl/ocl_defines.tmpl.h \\> ${LIBOCL_BINARY_DIR}/include/ocl_defines.h"
    COMMAND cat ${LIBOCL_SOURCE_DIR}/tmpl/ocl_defines.tmpl.h > ${OCL_OBJECT_DIR}/include/ocl_defines.h
    #   COMMAND echo "cat ${LIBOCL_SOURCE_DIR}/../ocl_common_defines.h \\>\\> ${LIBOCL_BINARY_DIR}/include/ocl_defines.h"
    COMMAND cat ${LIBOCL_SOURCE_DIR}/../ocl_common_defines.h >> ${OCL_OBJECT_DIR}/include/ocl_defines.h
    DEPENDS ${LIBOCL_SOURCE_DIR}/tmpl/ocl_defines.tmpl.h ${LIBOCL_SOURCE_DIR}/../ocl_common_defines.h
    COMMENT "Generate the header: ${LIBOCL_BINARY_DIR}/include/ocl_defines.h"
    )

#other module just copy.
MACRO(COPY_THE_HEADER _mod)
    # Use the python script to generate the header files.
    STRING(REGEX REPLACE "\(o.*\)" "${OCL_OBJECT_DIR}/include/\\1.h" output_name ${_mod})
    STRING(REGEX REPLACE "\(o.*\)" "${LIBOCL_SOURCE_DIR}/include/\\1.h" orgin_name ${_mod})
    SET(OCL_HEADER_FILES ${OCL_HEADER_FILES} ${output_name})
    IF(orgin_name STREQUAL output_name)
    ELSE(orgin_name STREQUAL output_name)
	ADD_CUSTOM_COMMAND(OUTPUT ${output_name}
	    COMMAND mkdir -p ${OCL_OBJECT_DIR}/include/
	    #COMMAND echo "cp ${orgin_name} ${output_name}"
	    COMMAND cp ${orgin_name} ${output_name}
	    DEPENDS ${orgin_name}
	    COMMENT "Copy the header: ${output_name}"
	    )
    ENDIF(orgin_name STREQUAL output_name)
ENDMACRO(COPY_THE_HEADER)
MACRO(COPY_THE_SOURCE _mod)
    # Use the python script to generate the header files.
    STRING(REGEX REPLACE "\(o.*\)" "${LIBOCL_BINARY_DIR}/src/\\1.cl" output_name ${_mod})
    STRING(REGEX REPLACE "\(o.*\)" "${LIBOCL_SOURCE_DIR}/src/\\1.cl" orgin_name ${_mod})
    SET(OCL_SOURCE_FILES ${OCL_SOURCE_FILES} ${output_name})
    IF(orgin_name STREQUAL output_name)
    ELSE(orgin_name STREQUAL output_name)
	ADD_CUSTOM_COMMAND(OUTPUT ${output_name}
	    COMMAND mkdir -p ${LIBOCL_BINARY_DIR}/src/
	    #COMMAND echo "cp ${orgin_name} ${output_name}"
	    COMMAND cp ${orgin_name} ${output_name}
	    DEPENDS ${orgin_name}
	    COMMENT "Copy the source: ${output_name}"
	    )
    ENDIF(orgin_name STREQUAL output_name)
ENDMACRO(COPY_THE_SOURCE)

SET (OCL_COPY_HEADERS ocl ocl_types ocl_float ocl_printf)
FOREACH(M ${OCL_COPY_HEADERS})
    COPY_THE_HEADER(${M})
ENDFOREACH(M) 

SET (OCL_COPY_MODULES ocl_workitem ocl_atom ocl_async ocl_sync ocl_misc ocl_vload ocl_geometric ocl_image)
FOREACH(M ${OCL_COPY_MODULES})
    COPY_THE_HEADER(${M})
    COPY_THE_SOURCE(${M})
ENDFOREACH(M) 


MACRO(GENERATE_HEADER_PY _mod)
    STRING(REGEX REPLACE "\(o.*\)" "${OCL_OBJECT_DIR}/include/\\1.h" output_name ${_mod})
    STRING(REGEX REPLACE "\(o.*\)" "${LIBOCL_SOURCE_DIR}/tmpl/\\1.tmpl.h" tmpl_name ${_mod})
    STRING(REGEX REPLACE "\(o.*\)" "${LIBOCL_SOURCE_DIR}/script/\\1.def" def_name ${_mod})
    SET(OCL_HEADER_FILES ${OCL_HEADER_FILES} ${output_name})
    ADD_CUSTOM_COMMAND(OUTPUT ${output_name}
	COMMAND mkdir -p ${OCL_OBJECT_DIR}/include/
	#COMMAND echo "cat ${tmpl_name} \\> ${output_name}"
	COMMAND cat ${tmpl_name} > ${output_name}
	#COMMAND echo "${LIBOCL_SOURCE_DIR}/script/gen_vector.py ${def_name} ${output_name} 1"
	COMMAND ${PYTHON_EXECUTABLE} ${LIBOCL_SOURCE_DIR}/script/gen_vector.py ${def_name} ${output_name} 1
	#COMMAND echo "echo \\#endif \\>\\> ${output_name}"
	COMMAND echo "\\#endif" >> ${output_name}
	DEPENDS ${tmpl_name} ${def_name} ${LIBOCL_SOURCE_DIR}/script/gen_vector.py
	COMMENT "Generate the header by python: ${output_name}"
	)
ENDMACRO(GENERATE_HEADER_PY)
MACRO(GENERATE_SOURCE_PY _mod)
    STRING(REGEX REPLACE "\(o.*\)" "${LIBOCL_BINARY_DIR}/src/\\1.cl" output_name ${_mod})
    STRING(REGEX REPLACE "\(o.*\)" "${LIBOCL_SOURCE_DIR}/tmpl/\\1.tmpl.cl" tmpl_name ${_mod})
    STRING(REGEX REPLACE "\(o.*\)" "${LIBOCL_SOURCE_DIR}/script/\\1.def" def_name ${_mod})
    SET(OCL_SOURCE_FILES ${OCL_SOURCE_FILES} ${output_name})
    ADD_CUSTOM_COMMAND(OUTPUT ${output_name}
	COMMAND mkdir -p ${LIBOCL_BINARY_DIR}/src/
	COMMAND cat ${tmpl_name} > ${output_name}
	COMMAND ${PYTHON_EXECUTABLE} ${LIBOCL_SOURCE_DIR}/script/gen_vector.py ${def_name} ${output_name} 0
	DEPENDS ${tmpl_name} ${def_name} ${LIBOCL_SOURCE_DIR}/script/gen_vector.py
	COMMENT "Generate the source by python: ${output_name}"
	)
ENDMACRO(GENERATE_SOURCE_PY)

SET (OCL_PY_GENERATED_MODULES ocl_common ocl_relational ocl_integer ocl_math)
FOREACH(M ${OCL_PY_GENERATED_MODULES})
    GENERATE_HEADER_PY(${M})
    GENERATE_SOURCE_PY(${M})
ENDFOREACH(M) 


MACRO(GENERATE_HEADER_BASH _mod)
    # Use the python script to generate the header files.
    STRING(REGEX REPLACE "\(o.*\)" "${OCL_OBJECT_DIR}/include/\\1.h" output_name ${_mod})
    STRING(REGEX REPLACE "\(o.*\)" "${LIBOCL_SOURCE_DIR}/script/\\1.sh" sh_name ${_mod})
    SET(OCL_HEADER_FILES ${OCL_HEADER_FILES} ${output_name})
    ADD_CUSTOM_COMMAND(OUTPUT ${output_name}
	COMMAND mkdir -p ${OCL_OBJECT_DIR}/include/
	COMMAND ${sh_name} -p > ${output_name}
	DEPENDS ${sh_name}
	COMMENT "Generate the header by script: ${output_name}"
	)
ENDMACRO(GENERATE_HEADER_BASH)
MACRO(GENERATE_SOURCE_BASH _mod)
    # Use the python script to generate the header files.
    STRING(REGEX REPLACE "\(o.*\)" "${LIBOCL_BINARY_DIR}/src/\\1.cl" output_name ${_mod})
    STRING(REGEX REPLACE "\(o.*\)" "${LIBOCL_SOURCE_DIR}/script/\\1.sh" def_name ${_mod})
    SET(OCL_SOURCE_FILES ${OCL_SOURCE_FILES} ${output_name})
    ADD_CUSTOM_COMMAND(OUTPUT ${output_name}
	COMMAND mkdir -p ${LIBOCL_BINARY_DIR}/src/
	COMMAND ${sh_name} > ${output_name}
	DEPENDS ${sh_name}
	COMMENT "Generate the source by script: ${output_name}"
	)
ENDMACRO(GENERATE_SOURCE_BASH)

SET (OCL_BASH_GENERATED_MODULES ocl_as ocl_convert)
FOREACH(M ${OCL_BASH_GENERATED_MODULES})
    GENERATE_HEADER_BASH(${M})
    GENERATE_SOURCE_BASH(${M})
ENDFOREACH(M) 


SET (CLANG_OCL_FLAGS -fno-builtin -ffp-contract=off -cl-kernel-arg-info -DGEN7_SAMPLER_CLAMP_BORDER_WORKAROUND "-cl-std=CL1.2")
MACRO(ADD_CL_TO_BC_TARGET _file)
    # CMake seems can not add pattern rule, use MACRO to replace.
    STRING(REGEX REPLACE "${LIBOCL_BINARY_DIR}/src/\(o.*\)\\.cl" "${OCL_OBJECT_DIR}/\\1.bc" output_name ${_file})
    ADD_CUSTOM_COMMAND(OUTPUT ${output_name}
	COMMAND mkdir -p ${OCL_OBJECT_DIR}/
	#COMMAND echo ${LLVM_INSTALL_DIR}clang -cc1 ${CLANG_OCL_FLAGS} -I ${LIBOCL_BINARY_DIR}/include/ -emit-llvm-bc -triple spir -o ${output_name} -x cl ${_file}
	COMMAND ${LLVM_INSTALL_DIR}clang -cc1 ${CLANG_OCL_FLAGS} -I ${OCL_OBJECT_DIR}/include/ -emit-llvm-bc -triple spir -o ${output_name} -x cl ${_file}
	DEPENDS ${_file} ${OCL_HEADER_FILES}
	COMMENT "Compiling ${_file}"
	)
ENDMACRO(ADD_CL_TO_BC_TARGET)


FOREACH(f ${OCL_SOURCE_FILES})
    ADD_CL_TO_BC_TARGET(${f})
ENDFOREACH(f) 

FOREACH(f ${OCL_SOURCE_FILES})
    STRING(REGEX REPLACE "${LIBOCL_BINARY_DIR}/src/\(o.*\)\\.cl" "${OCL_OBJECT_DIR}/\\1.bc" bc_name ${f})
    SET(OCL_BC_FILES ${OCL_BC_FILES} ${bc_name})
ENDFOREACH(f) 


# handle the ll files
MACRO(COPY_THE_LL _mod)
    # Use the python script to generate the header files.
    STRING(REGEX REPLACE "\(o.*\)" "${LIBOCL_BINARY_DIR}/src/\\1.ll" output_name ${_mod})
    STRING(REGEX REPLACE "\(o.*\)" "${LIBOCL_SOURCE_DIR}/src/\\1.ll" orgin_name ${_mod})
    IF(orgin_name STREQUAL output_name)
    ELSE(orgin_name STREQUAL output_name)
	ADD_CUSTOM_COMMAND(OUTPUT ${output_name}
	    COMMAND mkdir -p ${LIBOCL_BINARY_DIR}/src/
	    #COMMAND echo "cp ${orgin_name} ${output_name}"
	    COMMAND cp ${orgin_name} ${output_name}
	    DEPENDS ${orgin_name}
	    COMMENT "Copy the LL file: ${output_name}"
	    )
    ENDIF(orgin_name STREQUAL output_name)
ENDMACRO(COPY_THE_LL)
MACRO(ADD_LL_TO_BC_TARGET M)
    STRING(REGEX REPLACE "\(o.*\)" "${OCL_OBJECT_DIR}/\\1.bc" output_name ${M})
    STRING(REGEX REPLACE "\(o.*\)" "${LIBOCL_BINARY_DIR}/src/\\1.ll" srcll_name ${M})
    ADD_CUSTOM_COMMAND(OUTPUT ${output_name}
	COMMAND mkdir -p ${OCL_OBJECT_DIR}/
	#COMMAND echo ${LLVM_INSTALL_DIR}llvm-as -o ${output_name} ${srcll_name}
	COMMAND ${LLVM_INSTALL_DIR}llvm-as -o ${output_name} ${srcll_name}
	DEPENDS ${srcll_name}
	COMMENT "Compiling ${srcll_name}"
	)
ENDMACRO(ADD_LL_TO_BC_TARGET)

SET (OCL_LL_MODULES ocl_barrier ocl_memcpy ocl_memset)
FOREACH(f ${OCL_LL_MODULES})
    COPY_THE_LL(${f})
    ADD_LL_TO_BC_TARGET(${f})
    STRING(REGEX REPLACE "\(o.*\)" "${OCL_OBJECT_DIR}/\\1.bc" bc_name ${f})
    SET(OCL_BC_FILES ${OCL_BC_FILES} ${bc_name})
ENDFOREACH(f) 


ADD_CUSTOM_COMMAND(OUTPUT ${OCL_OBJECT_DIR}/beignet.bc
    COMMAND mkdir -p ${LIBOCL_BINARY_DIR}/lib/
    #COMMAND echo llvm-link -o ${LIBOCL_BINARY_DIR}/lib/beignet.bc ${OCL_BC_FILES}
    COMMAND ${LLVM_INSTALL_DIR}llvm-link -o ${OCL_OBJECT_DIR}/beignet.bc ${OCL_BC_FILES}
    DEPENDS ${OCL_BC_FILES}
    COMMENT "Generate the bitcode file: ${OCL_OBJECT_DIR}/beignet.bc"
    )

ADD_CUSTOM_COMMAND(OUTPUT ${OCL_OBJECT_DIR}/beignet.local.pch
    COMMAND mkdir -p ${OCL_OBJECT_DIR}
    COMMAND ${LLVM_INSTALL_DIR}clang -cc1 ${CLANG_OCL_FLAGS} -triple spir -I ${OCL_OBJECT_DIR}/include/ -emit-pch -x cl ${OCL_OBJECT_DIR}/include/ocl.h -o ${OCL_OBJECT_DIR}/beignet.local.pch
    DEPENDS ${OCL_HEADER_FILES}
    COMMENT "Generate the pch file: ${OCL_OBJECT_DIR}/beignet.local.pch"
    )

ADD_CUSTOM_COMMAND(OUTPUT ${OCL_OBJECT_DIR}/beignet.pch
    COMMAND mkdir -p ${OCL_OBJECT_DIR}
    COMMAND ${LLVM_INSTALL_DIR}clang -cc1 ${CLANG_OCL_FLAGS} -triple spir -I ${OCL_OBJECT_DIR}/include/ --relocatable-pch -emit-pch -isysroot ${LIBOCL_BINARY_DIR} -x cl ${OCL_OBJECT_DIR}/include/ocl.h -o ${OCL_OBJECT_DIR}/beignet.pch
    DEPENDS ${OCL_HEADER_FILES}
    COMMENT "Generate the pch file: ${OCL_OBJECT_DIR}/beignet.pch"
    )


add_custom_target(beignet_bitcode ALL DEPENDS ${OCL_OBJECT_DIR}/beignet.bc ${OCL_OBJECT_DIR}/beignet.pch ${OCL_OBJECT_DIR}/beignet.local.pch)
SET (OCL_OBJECT_DIR ${OCL_OBJECT_DIR} PARENT_SCOPE)
SET (OCL_HEADER_FILES ${OCL_HEADER_FILES} PARENT_SCOPE)

# ########################################################################
# Copyright 2013 Advanced Micro Devices, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ########################################################################


set( Common.Headers
  common.h
  ../include/clQMC/clQMC.h
  ../include/clQMC/latticerule.h
  )
set( Common.Source  common.c )

# Self-contained example
set( SelfContained.Source   selfcontained.c )
set( SelfContained.Files    ${SelfContained.Source} )

# Docs Tutorial
set( DocsTutorial1.Source   DocsTutorial/example1.c
                            DocsTutorial/common.c
                            ${Common.Source} )
set( DocsTutorial1.Headers  DocsTutorial/common.h
                            ${Common.Headers} )
set( DocsTutorial1.Files    ${DocsTutorial1.Source}
                            ${DocsTutorial1.Headers}
                            DocsTutorial/example1_kernel.cl )

set( DocsTutorial2.Source   DocsTutorial/example2.c
                            DocsTutorial/common.c
                            ${Common.Source} )
set( DocsTutorial2.Headers  DocsTutorial/common.h
                            ${Common.Headers} )
set( DocsTutorial2.Files    ${DocsTutorial2.Source}
                            ${DocsTutorial2.Headers}
                            DocsTutorial/example2_kernel.cl )

set( DocsTutorial3.Source   DocsTutorial/example3.c
                            DocsTutorial/common.c
                            ${Common.Source} )
set( DocsTutorial3.Headers  DocsTutorial/common.h
                            ${Common.Headers} )
set( DocsTutorial3.Files    ${DocsTutorial3.Source}
                            ${DocsTutorial3.Headers}
                            DocsTutorial/example3_kernel.cl )

set( DocsTutorial4.Source   DocsTutorial/example4.c
                            DocsTutorial/common.c
                            ${Common.Source} )
set( DocsTutorial4.Headers  DocsTutorial/common.h
                            ${Common.Headers} )
set( DocsTutorial4.Files    ${DocsTutorial4.Source}
                            ${DocsTutorial4.Headers}
                            DocsTutorial/example4_kernel.cl )


set( DL_LIB "" )
if( WIN32 )
        add_definitions( "/D_CONSOLE" )
elseif( APPLE )
        set( CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ ${CMAKE_CXX_FLAGS}" )
else( )
        # To use the dlopen() and dlclose() functions, we should link with libdl
        set( DL_LIB "-ldl" )
endif( )

if( CMAKE_COMPILER_IS_GNUCC )
    set( MATH_LIB "-lm" )
endif()

# Include standard OpenCL headers
include_directories( ${OPENCL_INCLUDE_DIRS} ${PROJECT_BINARY_DIR}/include ../include )


add_executable(        SelfContained ${SelfContained.Files} )
include_directories(   SelfContained ${OPENCL_INCLUDE_DIRS} ${PROJECT_BINARY_DIR}/include ../include )
target_link_libraries( SelfContained clQMC ${OPENCL_LIBRARIES} ${DL_LIB} ${MATH_LIB} )
set_target_properties( SelfContained PROPERTIES VERSION ${CLQMC_VERSION} )
set_target_properties( SelfContained PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/staging" )

# CPack configuration; include the executable into the package
install( TARGETS SelfContained
        RUNTIME DESTINATION bin${SUFFIX_BIN}
        LIBRARY DESTINATION lib${SUFFIX_LIB}
        ARCHIVE DESTINATION lib${SUFFIX_LIB}/import
        )


add_executable(        DocsTutorial2 ${DocsTutorial2.Files} )
include_directories(   DocsTutorial2 ${OPENCL_INCLUDE_DIRS} ${PROJECT_BINARY_DIR}/include ../include )
target_link_libraries( DocsTutorial2 clQMC ${OPENCL_LIBRARIES} ${DL_LIB} ${MATH_LIB} )
set_target_properties( DocsTutorial2 PROPERTIES VERSION ${CLQMC_VERSION} )
set_target_properties( DocsTutorial2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/staging" )
install( FILES "DocsTutorial/example2_kernel.cl" DESTINATION "./client/DocsTutorial" )

# CPack configuration; include the executable into the package
install( TARGETS DocsTutorial2
        RUNTIME DESTINATION bin${SUFFIX_BIN}
        LIBRARY DESTINATION lib${SUFFIX_LIB}
        ARCHIVE DESTINATION lib${SUFFIX_LIB}/import
        )

if (clRNG_VERSION)

    install( FILES "DocsTutorial/common.clh" DESTINATION "./include/clQMC/DocsTutorial" )

    add_executable(        DocsTutorial1 ${DocsTutorial1.Files} )
    include_directories(   DocsTutorial1 ${clRNG_INCLUDE_DIRS} ${OPENCL_INCLUDE_DIRS} ${PROJECT_BINARY_DIR}/include ../include )
    target_link_libraries( DocsTutorial1 clQMC clRNG ${OPENCL_LIBRARIES} ${DL_LIB} ${MATH_LIB} )
    set_target_properties( DocsTutorial1 PROPERTIES VERSION ${CLQMC_VERSION} )
    set_target_properties( DocsTutorial1 PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/staging" )
    install( FILES "DocsTutorial/example1_kernel.cl" DESTINATION "./client/DocsTutorial" )

    add_executable(        DocsTutorial3 ${DocsTutorial3.Files} )
    include_directories(   DocsTutorial3 ${clRNG_INCLUDE_DIRS} ${OPENCL_INCLUDE_DIRS} ${PROJECT_BINARY_DIR}/include ../include )
    target_link_libraries( DocsTutorial3 clQMC clRNG ${OPENCL_LIBRARIES} ${DL_LIB} ${MATH_LIB} )
    set_target_properties( DocsTutorial3 PROPERTIES VERSION ${CLQMC_VERSION} )
    set_target_properties( DocsTutorial3 PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/staging" )
    install( FILES "DocsTutorial/example3_kernel.cl" DESTINATION "./client/DocsTutorial" )

    add_executable(        DocsTutorial4 ${DocsTutorial4.Files} )
    include_directories(   DocsTutorial4 ${clRNG_INCLUDE_DIRS} ${OPENCL_INCLUDE_DIRS} ${PROJECT_BINARY_DIR}/include ../include )
    target_link_libraries( DocsTutorial4 clQMC clRNG ${OPENCL_LIBRARIES} ${DL_LIB} ${MATH_LIB} )
    set_target_properties( DocsTutorial4 PROPERTIES VERSION ${CLQMC_VERSION} )
    set_target_properties( DocsTutorial4 PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/staging" )
    install( FILES "DocsTutorial/example4_kernel.cl" DESTINATION "./client/DocsTutorial" )

    install( TARGETS DocsTutorial1 DocsTutorial3 DocsTutorial4
        RUNTIME DESTINATION bin${SUFFIX_BIN}
        LIBRARY DESTINATION lib${SUFFIX_LIB}
        ARCHIVE DESTINATION lib${SUFFIX_LIB}/import
        )

else()

  message("not building client examples that require clRNG")

endif()

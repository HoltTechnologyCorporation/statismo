message("-- External project - ITK")

if (${VTK_SUPPORT})
  if(NOT ${USE_SYSTEM_VTK})
    set(_itk_deps VTK ${_itk_deps})
  endif()
  set(_itkoptions ${_itkoptions} -DVTK_DIR:PATH=${VTK_DIR} -DModule_ITKVtkGlue:BOOL=ON)
endif()

if (USE_SYSTEM_HDF5)
  set(_itkoptions ${_itkoptions} -DHDF5_DIR:PATH=${HDF5_DIR})
endif()

if (USE_SYSTEM_EIGEN)
  set(_itkoptions ${_itkoptions} -DEIGEN3_INCLUDE_DIR:PATH=${EIGEN3_INCLUDE_DIR} -DEigen3_DIR:PATH=${Eigen3_DIR})
endif()

ExternalProject_Add(ITK
  DEPENDS ${_itk_deps}
  GIT_REPOSITORY https://github.com/InsightSoftwareConsortium/ITK.git
  GIT_TAG v5.0.1 # If you modify this, update the ITK_DIR at the end of the file
  SOURCE_DIR ITK
  BINARY_DIR ITK-build
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  CMAKE_GENERATOR ${_ep_cmake_gen}
  CMAKE_ARGS
    ${_ep_common_args}
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DBUILD_TESTING:BOOL=OFF
    -DCMAKE_BUILD_TYPE:STRING=${BUILD_TYPE}
    -DITK_BUILD_DEFAULT_MODULES:BOOL=ON
    -DModule_ITKReview:BOOL=ON
    -DITK_LEGACY_REMOVE:BOOL=ON
    -DCMAKE_INSTALL_PREFIX:PATH=${INSTALL_DEPENDENCIES_DIR}
    -DITK_USE_SYSTEM_HDF5:BOOL=${USE_SYSTEM_HDF5}
    -DITK_USE_SYSTEM_EIGEN:BOOL=${USE_SYSTEM_EIGEN}
    ${_itkoptions}
    ${ITK_EXTRA_OPTIONS}
)

# This is passed to Statismo so it is able to find it in priority
set(ITK_DIR ${INSTALL_DEPENDENCIES_DIR}/lib/cmake/ITK-5.0/)

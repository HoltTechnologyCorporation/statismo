message("External project - ITK")

set(ITK_DEPENDENCIES)
set(_vtkoptions)

if (${VTK_SUPPORT})
  if(${USE_SYSTEM_VTK})
    set(_vtkoptions -DVTK_DIR:PATH=${VTK_DIR})
  else()
    set(ITK_DEPENDENCIES VTK ${ITK_DEPENDENCIES})
  endif()
  set(_vtkoptions ${_vtkoptions} -DModule_ITKVtkGlue:BOOL=ON)
endif()

ExternalProject_Add(ITK
  DEPENDS ${ITK_DEPENDENCIES}
  GIT_REPOSITORY https://github.com/InsightSoftwareConsortium/ITK.git
  GIT_TAG v5.0.1
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
    -DITK_USE_SYSTEM_HDF5:BOOL=OFF
    -DITK_USE_SYSTEM_EIGEN:BOOL=OFF
    ${_vtkoptions}
    ${ITK_EXTRA_OPTIONS}
)

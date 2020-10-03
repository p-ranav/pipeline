cmake_minimum_required(VERSION 3.8)

if(DEFINED PROJECT_NAME)
  set(PIPELINE_SUBPROJECT ON)
endif()

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.12")
  project(pipeline VERSION 1.0.0 LANGUAGES CXX
    HOMEPAGE_URL "https://github.com/p-ranav/pipeline"
    DESCRIPTION "Pipelines for Modern C++")
elseif(CMAKE_VERSION VERSION_GREATER_EQUAL "3.9")
  project(pipeline VERSION 1.0.0 LANGUAGES CXX
    DESCRIPTION "Pipelines for Modern C++")
else()
  project(pipeline VERSION 1.0.0 LANGUAGES CXX)
endif()

if(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    conan_basic_setup()
endif()

option(PIPELINE_BUILD_TESTS "Build pipeline tests + enable CTest")
option(PIPELINE_SAMPLES "Build pipeline samples")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

find_package(Threads REQUIRED)

add_library(pipeline INTERFACE)
add_library(pipeline::pipeline ALIAS pipeline)

target_compile_features(pipeline INTERFACE cxx_std_17)
target_include_directories(pipeline INTERFACE
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)
target_link_libraries(pipeline INTERFACE Threads::Threads)

if(PIPELINE_SAMPLES)
  add_subdirectory(samples)
endif()

if(NOT PIPELINE_SUBPROJECT)
  configure_package_config_file(pipelineConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/pipelineConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pipeline)
  write_basic_package_version_file(pipelineConfigVersion.cmake
                                   COMPATIBILITY AnyNewerVersion)

  configure_file(pipeline.pc.in pipeline.pc @ONLY)

  install(TARGETS pipeline EXPORT pipelineTargets)
  install(EXPORT pipelineTargets
          FILE pipelineTargets.cmake
          NAMESPACE pipeline::
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pipeline)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pipelineConfig.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/pipelineConfigVersion.cmake
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pipeline)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pipeline.pc
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
  install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/pipeline
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
          USE_SOURCE_PERMISSIONS
          PATTERN "*.hpp")
  install(FILES LICENSE LICENSE.termcolor
          DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/licenses/pipeline)

  if(EXISTS "${PROJECT_SOURCE_DIR}/.gitignore")
    # Simple glob to regex conversion (.gitignore => CPACK_SOURCE_IGNORE_FILES)
    file(READ ".gitignore" DOT_GITIGNORE)
    string(REPLACE ";" "RANDOMSEQUENCE" DOT_GITIGNORE "${DOT_GITIGNORE}")
    string(REPLACE "\n" ";" DOT_GITIGNORE "${DOT_GITIGNORE}")
    string(REPLACE "RANDOMSEQUENCE" "\\;" DOT_GITIGNORE "${DOT_GITIGNORE}")
    foreach(IGNORE_LINE ${DOT_GITIGNORE})
      if(NOT IGNORE_LINE OR IGNORE_LINE MATCHES "^#")
        continue()
      endif()
      string(REPLACE "\\" "\\\\" IGNORE_LINE "${IGNORE_LINE}")
      string(REPLACE "." "\\\\." IGNORE_LINE "${IGNORE_LINE}")
      string(REPLACE "*" ".*" IGNORE_LINE "${IGNORE_LINE}")
      string(REPLACE "+" "\\\\+" IGNORE_LINE "${IGNORE_LINE}")
      list(APPEND CPACK_SOURCE_IGNORE_FILES "${IGNORE_LINE}")
    endforeach()
  endif()

  # extra ignored files
  list(APPEND CPACK_SOURCE_IGNORE_FILES
    .editorconfig
    .git
    .gitignore
    .travis.yml
    .appveyor.yml
  )
  set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")
  set(CPACK_GENERATOR "TGZ;TXZ")
  set(CPACK_SOURCE_GENERATOR "TGZ;TXZ")
  include(CPack)
endif()

cmake_minimum_required(VERSION 3.9)
set(CMAKE_VERBOSE_MAKEFILE ON)
if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

## Global properties
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

get_property(global_libs GLOBAL PROPERTY GLOBAL_LIBS_LIST)
if(NOT global_libs)
  define_property(GLOBAL PROPERTY GLOBAL_LIBS_LIST
      BRIEF_DOCS "Global list of libs"
      FULL_DOCS "Global list of libs")
  set_property(GLOBAL PROPERTY GLOBAL_LIBS_LIST "")
endif()
#在子模块中可通过以下语句加入其它库SUB_LIB
# set_property(GLOBAL APPEND PROPERTY GLOBAL_LIBS_LIST SUB_LIB)

get_property(global_include GLOBAL PROPERTY GLOBAL_INCLUDE_LIST)
if(NOT global_include)
  define_property(GLOBAL PROPERTY GLOBAL_INCLUDE_LIST
      BRIEF_DOCS "Global list of includes"
      FULL_DOCS "Global list of includes")
  set_property(GLOBAL PROPERTY GLOBAL_INCLUDE_LIST "")
endif()
# set_property(GLOBAL APPEND PROPERTY GLOBAL_INCLUDE_LIST include)


## Project name
set (PROJECT_NAME S4A)
project(${PROJECT_NAME})
message(STATUS "PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")

set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for executables")
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers")

## Thirdparty
add_subdirectory("thirdparty")
add_definitions(-DCPPHTTPLIB_ZLIB_SUPPORT=on)

## CMake module path
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

## Compiler features
set(CMAKE_WARN_AS_ERROR ON)
if(MSVC)
  include(SetCompilerFeatures)
endif()
include(SetCompilerWarnings)
include(SetPlatformFeatures)
include(SystemInformation)

## git SHA
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC S4GIT_SHA1)
add_definitions("-DS4GIT_SHA1=\"${S4GIT_SHA1}\"")

add_definitions(-D__STDC_CONSTANT_MACROS)#ffmepg or some libs require this
add_definitions(-D__STDC_FORMAT_MACROS)
if (WIN32)
  add_definitions(-DWIN32_LEAN_AND_MEAN) #winsock2.h windows.h 
  add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)  #windows
endif()
message(STATUS "CMAKE_C_COMPILER=${CMAKE_C_COMPILER} CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")

if(NOT MSVC)
  message(STATUS "CC_VERSION=")
  EXECUTE_PROCESS( COMMAND ${CMAKE_C_COMPILER} -dumpversion CC_VERSION )
  message(STATUS "CXX_VERSION=")
  EXECUTE_PROCESS( COMMAND ${CMAKE_CXX_COMPILER} -dumpversion CXX_VERSION )
endif()


find_package(OpenSSL REQUIRED)
message(STATUS "OpenSSL version: ${OPENSSL_VERSION} ${OPENSSL_INCLUDE_DIR} ${OPENSSL_LIBRARIES}")


## Link libraries
list(APPEND LINKLIBS ${OPENSSL_LIBRARIES})
if(WIN32)
  list(APPEND LINKLIBS ${CRYPT_LIBRARIES})
  list(APPEND LINKLIBS ${WINSOCK_LIBRARIES})
endif()

# find_package(tcmalloc)
# if (tcmalloc_FOUND)
#   message(STATUS "Found tcmalloc: ${tcmalloc_LIBRARY}")
#   list(APPEND LINKLIBS ${tcmalloc_LIBRARY})
#   if (MSVC)
#     add_definitions(-DHAS_TCMALLOC)
#     add_link_options(/INCLUDE:"__tcmalloc")
#   endif(MSVC)
# else()
#   message(WARNING "*******************************************************************\n*** tcmalloc not found: Performance will decrease dramatically! ***\n*******************************************************************")
# endif(tcmalloc_FOUND)

if(NOT MSVC)
  list(APPEND LINKLIBS stdc++fs)
endif()

## System directories
get_property(dirs DIRECTORY ${SYSTEM} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "SYSTEM dir has: '${dir}'")
endforeach()

## Library
file(GLOB_RECURSE LIB_HEADER_FILES "include/*.h")
file(GLOB_RECURSE LIB_INLINE_FILES "include/*.inl")
file(GLOB_RECURSE LIB_SOURCE_FILES "source/*.cpp")

set_source_files_properties(${LIB_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}")


add_library(${PROJECT_NAME} STATIC  ${LIB_HEADER_FILES} ${LIB_INLINE_FILES} ${LIB_SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include;${CMAKE_CURRENT_BINARY_DIR};")


## collect GLOBAL_INCLUDE_LIST
get_property(global_include GLOBAL PROPERTY GLOBAL_INCLUDE_LIST)
message(STATUS "GLOBAL_INCLUDE_LIST = ${global_include}")
target_include_directories(${PROJECT_NAME} PUBLIC "${global_include}")

## collect GLOBAL_LIBS_LIST
get_property(global_libs GLOBAL PROPERTY GLOBAL_LIBS_LIST)
message(STATUS "GLOBAL_LIBS_LIST = ${global_libs}")
target_link_libraries(${PROJECT_NAME} ${LINKLIBS} ${global_libs})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER libraries)

function(assign_source_group arg)
  # message(STATUS "ARGN = ${ARGN}")
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        string(REPLACE "${ARGV0}/" "" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${CMAKE_CURRENT_SOURCE_DIR}/${_source_rel}")
        # message(STATUS "${_source_path_msvc} / ${CMAKE_CURRENT_SOURCE_DIR}/${_source_rel}")
    endforeach()
endfunction(assign_source_group)
file(GLOB_RECURSE source_list RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "include/*.h")
assign_source_group("include" ${source_list})
file(GLOB_RECURSE source_list RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "source/*.cpp")
assign_source_group("source" ${source_list})



list(APPEND INSTALL_TARGETS ${PROJECT_NAME})
list(APPEND LINKLIBS ${PROJECT_NAME})
message(STATUS "LINKLIBS = ${LINKLIBS}")

# # E:\work\s4\build\S4_autogen\include_Debug\ui_sqlviewer.h
# # E:\work\s4\build\S4_autogen\include_Debug\ui_dbconnectdialog.h
# # E:\work\s4\build\ui_dbconnectdialog.h
# foreach(ui_h ${ui_headers})
#   STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" ui_name ${ui_h})
#   message(STATUS "ui_h = ${ui_h}")
#   message(STATUS "ui_name = ${ui_name}")

#   add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD        # Adds a post-build event to MyTest
#       COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
#           "${ui_h}"      # <--this is in-file
#           "${PROJECT_SOURCE_DIR}/build/S4_autogen/include_Debug/${ui_name}.h")                 # <--this is out-file path
# endforeach()

## Additional module components: benchmarks, examples, plugins, tests, tools and install
if(NOT AS_MODULE)

  # Application
  file(GLOB APP_HEADER_FILES "application/*.h")
  file(GLOB APP_INLINE_FILES "application/*.inl")
  file(GLOB APP_SOURCE_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/application" "application/*.cpp")
  foreach(APP_SOURCE_FILE ${APP_SOURCE_FILES})
    string(REGEX REPLACE "(.*)\\.cpp" "\\1" APP_NAME ${APP_SOURCE_FILE})
    set(APP_TARGET "${PROJECT_NAME}-${APP_NAME}")

    set_source_files_properties(application/${APP_SOURCE_FILE} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}")
    add_executable(${APP_TARGET} ${APP_HEADER_FILES} ${APP_INLINE_FILES} application/${APP_SOURCE_FILE})
    target_link_libraries(${APP_TARGET} ${LINKLIBS})
    set_target_properties(${APP_TARGET} PROPERTIES FOLDER application)
    target_include_directories(${APP_TARGET} PUBLIC "${global_include};")

    list(APPEND INSTALL_TARGETS ${APP_TARGET})
    # list(APPEND INSTALL_TARGETS_PDB ${APP_TARGET})
  endforeach()

  # Test
  file(GLOB TEST_HEADER_FILES "test/*.h")
  file(GLOB TEST_INLINE_FILES "test/*.inl")
  file(GLOB TEST_SOURCE_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/test" "test/*.cpp")
  foreach(TEST_SOURCE_FILE ${TEST_SOURCE_FILES})
    string(REGEX REPLACE "(.*)\\.cpp" "\\1" TEST_NAME ${TEST_SOURCE_FILE})
    set(TEST_TARGET "${PROJECT_NAME}-${TEST_NAME}")

    set_source_files_properties(test/${TEST_SOURCE_FILE} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}")
    add_executable(${TEST_TARGET} ${TEST_HEADER_FILES} ${TEST_INLINE_FILES} test/${TEST_SOURCE_FILE})
    target_link_libraries(${TEST_TARGET} ${LINKLIBS})
    set_target_properties(${TEST_TARGET} PROPERTIES FOLDER test)
    target_include_directories(${TEST_TARGET} PUBLIC "${global_include};")

    # list(APPEND INSTALL_TARGETS ${TEST_TARGET})
    # list(APPEND INSTALL_TARGETS_PDB ${TEST_TARGET})
  endforeach()
endif()
  
# Install
install(TARGETS ${INSTALL_TARGETS}
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" 
)

set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)

include(InstallRequiredSystemLibraries)

install(
    PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
    DESTINATION "${INSTALL_BIN_DIR}"
)

install(
    DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    DESTINATION
    "${CMAKE_INSTALL_PREFIX}"
    FILES_MATCHING
    PATTERN
    "*.h*"
)


#============================================================================
# uninstall target
#============================================================================
message(STATUS "INSTALL_LIB_DIR = ${INSTALL_LIB_DIR}")
message(STATUS "INSTALL_BIN_DIR = ${INSTALL_BIN_DIR}")
message(STATUS "INSTALL_INC_DIR = ${INSTALL_INC_DIR}")

if(NOT TARGET uninstall)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()



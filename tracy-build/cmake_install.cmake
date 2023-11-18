# Install script for directory: C:/Users/Jakub/pytracy/tracy

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/Tracy")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/Jakub/pytracy/tracy-build/Debug/TracyClient.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/Jakub/pytracy/tracy-build/Release/TracyClient.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/Jakub/pytracy/tracy-build/MinSizeRel/TracyClient.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/Jakub/pytracy/tracy-build/RelWithDebInfo/TracyClient.lib")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/tracy" TYPE FILE FILES
    "C:/Users/Jakub/pytracy/tracy/public/tracy/TracyC.h"
    "C:/Users/Jakub/pytracy/tracy/public/tracy/Tracy.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/tracy/TracyD3D11.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/tracy/TracyD3D12.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/tracy/TracyLua.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/tracy/TracyOpenCL.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/tracy/TracyOpenGL.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/tracy/TracyVulkan.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/client" TYPE FILE FILES
    "C:/Users/Jakub/pytracy/tracy/public/client/tracy_concurrentqueue.h"
    "C:/Users/Jakub/pytracy/tracy/public/client/tracy_rpmalloc.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/client/tracy_SPSCQueue.h"
    "C:/Users/Jakub/pytracy/tracy/public/client/TracyArmCpuTable.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/client/TracyCallstack.h"
    "C:/Users/Jakub/pytracy/tracy/public/client/TracyCallstack.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/client/TracyCpuid.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/client/TracyDebug.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/client/TracyDxt1.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/client/TracyFastVector.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/client/TracyLock.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/client/TracyProfiler.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/client/TracyRingBuffer.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/client/TracyScoped.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/client/TracyStringHelpers.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/client/TracySysPower.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/client/TracySysTime.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/client/TracySysTrace.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/client/TracyThread.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/common" TYPE FILE FILES
    "C:/Users/Jakub/pytracy/tracy/public/common/tracy_lz4.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/common/tracy_lz4hc.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/common/TracyAlign.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/common/TracyAlloc.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/common/TracyApi.h"
    "C:/Users/Jakub/pytracy/tracy/public/common/TracyColor.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/common/TracyForceInline.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/common/TracyMutex.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/common/TracyProtocol.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/common/TracyQueue.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/common/TracySocket.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/common/TracyStackFrames.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/common/TracySystem.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/common/TracyUwp.hpp"
    "C:/Users/Jakub/pytracy/tracy/public/common/TracyYield.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/Tracy/TracyTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/Tracy/TracyTargets.cmake"
         "C:/Users/Jakub/pytracy/tracy-build/CMakeFiles/Export/7430802ac276f58e70c46cf34d169c6f/TracyTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/Tracy/TracyTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/Tracy/TracyTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/Tracy" TYPE FILE FILES "C:/Users/Jakub/pytracy/tracy-build/CMakeFiles/Export/7430802ac276f58e70c46cf34d169c6f/TracyTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/Tracy" TYPE FILE FILES "C:/Users/Jakub/pytracy/tracy-build/CMakeFiles/Export/7430802ac276f58e70c46cf34d169c6f/TracyTargets-debug.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/Tracy" TYPE FILE FILES "C:/Users/Jakub/pytracy/tracy-build/CMakeFiles/Export/7430802ac276f58e70c46cf34d169c6f/TracyTargets-minsizerel.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/Tracy" TYPE FILE FILES "C:/Users/Jakub/pytracy/tracy-build/CMakeFiles/Export/7430802ac276f58e70c46cf34d169c6f/TracyTargets-relwithdebinfo.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/Tracy" TYPE FILE FILES "C:/Users/Jakub/pytracy/tracy-build/CMakeFiles/Export/7430802ac276f58e70c46cf34d169c6f/TracyTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/Tracy" TYPE FILE FILES "C:/Users/Jakub/pytracy/tracy-build/TracyConfig.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/Users/Jakub/pytracy/tracy-build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")

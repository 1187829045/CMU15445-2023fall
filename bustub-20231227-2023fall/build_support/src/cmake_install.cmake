# Install script for directory: /home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
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

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/src/binder/cmake_install.cmake")
  include("/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/src/buffer/cmake_install.cmake")
  include("/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/src/catalog/cmake_install.cmake")
  include("/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/src/common/cmake_install.cmake")
  include("/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/src/concurrency/cmake_install.cmake")
  include("/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/src/container/cmake_install.cmake")
  include("/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/src/execution/cmake_install.cmake")
  include("/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/src/recovery/cmake_install.cmake")
  include("/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/src/storage/cmake_install.cmake")
  include("/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/src/type/cmake_install.cmake")
  include("/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/src/planner/cmake_install.cmake")
  include("/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/src/primer/cmake_install.cmake")
  include("/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/src/optimizer/cmake_install.cmake")

endif()

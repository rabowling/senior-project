cmake_minimum_required(VERSION 3.8)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
endif()

# Name of the project
project(SeniorProject)

# Use glob to get the list of all source files.
file(GLOB_RECURSE SOURCES "src/*.cpp" "ext/**.cpp" "ext/**.c")

# We don't really need to include header and resource files to build, but it's
# nice to have them show up in IDEs.
file(GLOB_RECURSE HEADERS "src/*.h" "ext/**.h")
include_directories(SYSTEM "ext/glad/include" "ext/tiny_obj_loader" "ext/stb_image" "ext/inih")
if (WIN32)
	include_directories(SYSTEM "ext/getopt" "ext/dirent")
endif()


# OS specific options and libraries
# Enable all pedantic warnings.
if (WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /w34716 /MT")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -pedantic")
endif()

# Add required frameworks for PhysX.
set(PHYSX_DIR "$ENV{PHYSX_DIR}")
if (PHYSX_DIR)
    add_definitions(-DNDEBUG)
    include_directories(SYSTEM "${PHYSX_DIR}/physx/include" "${PHYSX_DIR}/pxshared/include")
    if (APPLE)
        link_directories("${PHYSX_DIR}/physx/bin/mac.x86_64/release")
    elseif (UNIX)
        link_directories("${PHYSX_DIR}/physx/bin/linux.clang/release")
	elseif (WIN32)
		link_directories("${PHYSX_DIR}/physx/bin/win.x86_64.vc142.mt/release")
    endif()
else()
    message(FATAL_ERROR "PhysX not found")
endif()

# Add required frameworks for GLFW.
set(GLFW_DIR "$ENV{GLFW_DIR}")
if (GLFW_DIR)
    message(STATUS "GLFW path: ${GLFW_DIR}")
    option(GLFW_BUILD_EXAMPLES "GLFW_BUILD_EXAMPLES" OFF)
    option(GLFW_BUILD_TESTS "GLFW_BUILD_TESTS" OFF)
    option(GLFW_BUILD_DOCS "GLFW_BUILD_DOCS" OFF)
    if(CMAKE_BUILD_TYPE MATCHES Release)
        add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/release)
    else()
        add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/debug)
    endif()
    include_directories(SYSTEM ${GLFW_DIR}/include)
else()
    message(FATAL_ERROR "GLFW not found")
endif()

# Add GLM
# Get the GLM environment variable. Since GLM is a header-only library, we
# just need to add it to the include directory.
set(GLM_INCLUDE_DIR "$ENV{GLM_INCLUDE_DIR}")
if (GLM_INCLUDE_DIR)
    include_directories(SYSTEM ${GLM_INCLUDE_DIR})
    message(STATUS "GLM include path: ${GLM_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "GLM not found")
endif()

# Add option to build for distribution
option(BUILD_DISTRIBUTE "Build for distribution" OFF)
if (BUILD_DISTRIBUTE)
    add_compile_definitions(BUILD_DISTRIBUTE)
endif()

# Other compile definitions
if (WIN32)
	add_compile_definitions(PX_PHYSX_STATIC_LIB _USE_MATH_DEFINES)
endif()

# Set the executable.
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${CMAKE_PROJECT_NAME} PhysXCharacterKinematic_static_64 PhysX_static_64 PhysXCommon_static_64 PhysXFoundation_static_64 PhysXExtensions_static_64 PhysXCooking_static_64 PhysXPvdSDK_static_64 PhysXVehicle_static_64)
target_link_libraries(${CMAKE_PROJECT_NAME} glfw ${GLFW_LIBRARIES})

# Set up OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${CMAKE_PROJECT_NAME} OpenMP::OpenMP_CXX)
endif()

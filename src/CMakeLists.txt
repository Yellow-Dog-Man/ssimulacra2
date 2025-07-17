cmake_minimum_required(VERSION 3.21)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(SSIMULACRA2 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Check if we're using vcpkg
if(DEFINED CMAKE_TOOLCHAIN_FILE AND CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg")
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
    set(USING_VCPKG TRUE)
else()
    set(USING_VCPKG FALSE)
endif()

# Find Highway library
if(USING_VCPKG)
    find_package(hwy CONFIG QUIET)
    if(hwy_FOUND)
        message(STATUS "✓ Found Highway via vcpkg")
        set(HWY_LIBRARIES hwy::hwy)
        set(HWY_FOUND TRUE)
        # Create alias for compatibility with lib/CMakeLists.txt
        if(NOT TARGET hwy)
            add_library(hwy ALIAS hwy::hwy)
        endif()
    endif()
endif()

if(NOT HWY_FOUND)
    find_package(HWY 0.15.0 QUIET)
    if(HWY_FOUND)
        message(STATUS "✓ Found Highway via system")
        set(HWY_LIBRARIES hwy)
    endif()
endif()

if(NOT HWY_FOUND)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/highway/CMakeLists.txt")
        message(STATUS "Using bundled Highway")
        set(HWY_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(HWY_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
        add_subdirectory(third_party/highway)
        set(HWY_LIBRARIES hwy)
        set(HWY_FOUND TRUE)
    else()
        message(FATAL_ERROR "Highway library not found")
    endif()
endif()

# Find LCMS2 library
if(USING_VCPKG)
    find_package(lcms2 CONFIG QUIET)
    if(lcms2_FOUND)
        message(STATUS "✓ Found LCMS2 via vcpkg")
        set(LCMS2_LIBRARIES lcms2)
        set(LCMS2_FOUND TRUE)
    endif()
endif()

if(NOT LCMS2_FOUND)
    find_package(LCMS2 2.13 QUIET)
    if(LCMS2_FOUND)
        message(STATUS "✓ Found LCMS2 via system")
    endif()
endif()

if(NOT LCMS2_FOUND)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/lcms/src")
        message(STATUS "Using bundled LCMS2")
        add_subdirectory(third_party/lcms)
        set(LCMS2_LIBRARIES lcms2)
        set(LCMS2_FOUND TRUE)
    else()
        message(FATAL_ERROR "LCMS2 library not found")
    endif()
endif()

# Find image format libraries
if(USING_VCPKG)
    # JPEG
    find_package(libjpeg-turbo CONFIG QUIET)
    if(libjpeg-turbo_FOUND)
        message(STATUS "✓ Found JPEG via vcpkg (libjpeg-turbo)")
        set(JPEG_LIBRARIES libjpeg-turbo::turbojpeg)
        set(JPEG_FOUND TRUE)
    endif()

    # PNG
    find_package(PNG CONFIG QUIET)
    if(PNG_FOUND)
        message(STATUS "✓ Found PNG via vcpkg")
        set(PNG_LIBRARIES PNG::PNG)
    endif()

    # ZLIB
    find_package(ZLIB CONFIG QUIET)
    if(ZLIB_FOUND)
        message(STATUS "✓ Found ZLIB via vcpkg")
        set(ZLIB_LIBRARIES ZLIB::ZLIB)
    endif()

    # GIF
    find_package(unofficial-giflib CONFIG QUIET)
    if(unofficial-giflib_FOUND)
        message(STATUS "✓ Found GIF via vcpkg")
        set(GIF_LIBRARIES unofficial::giflib::giflib)
        set(GIF_FOUND TRUE)
    endif()
endif()

# Fallback to system packages if vcpkg didn't work
if(NOT JPEG_FOUND)
    find_package(JPEG QUIET)
    if(JPEG_FOUND)
        message(STATUS "✓ Found JPEG via system")
    endif()
endif()

if(NOT PNG_FOUND)
    find_package(PNG QUIET)
    if(PNG_FOUND)
        message(STATUS "✓ Found PNG via system")
    endif()
endif()

if(NOT ZLIB_FOUND)
    find_package(ZLIB QUIET)
    if(ZLIB_FOUND)
        message(STATUS "✓ Found ZLIB via system")
    endif()
endif()

if(NOT GIF_FOUND)
    find_package(GIF QUIET)
    if(GIF_FOUND)
        message(STATUS "✓ Found GIF via system")
    endif()
endif()

# Build the JXL library components
add_subdirectory(lib)

# Collect all image format libraries
set(IMAGE_LIBRARIES "")
if(JPEG_FOUND)
    list(APPEND IMAGE_LIBRARIES ${JPEG_LIBRARIES})
endif()
if(PNG_FOUND)
    list(APPEND IMAGE_LIBRARIES ${PNG_LIBRARIES})
endif()
if(ZLIB_FOUND)
    list(APPEND IMAGE_LIBRARIES ${ZLIB_LIBRARIES})
endif()
if(GIF_FOUND)
    list(APPEND IMAGE_LIBRARIES ${GIF_LIBRARIES})
endif()

# Create the SSIMULACRA2 shared library
add_library(ssimulacra2_lib SHARED
    ssimulacra2.cc
    ssimulacra2_c_api.cc
)

target_include_directories(ssimulacra2_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

target_link_libraries(ssimulacra2_lib 
    jxl-static 
    jxl_extras-static 
    ${HWY_LIBRARIES} 
    ${LCMS2_LIBRARIES}
    ${IMAGE_LIBRARIES}
)

# Set library properties
set_target_properties(ssimulacra2_lib PROPERTIES
    OUTPUT_NAME "ssimulacra2"
    VERSION "2.1.0"
    SOVERSION "2"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# Define export macro for Windows DLL
target_compile_definitions(ssimulacra2_lib PRIVATE SSIMULACRA2_EXPORTS)

# Create the executable
add_executable(ssimulacra2_exe
    ssimulacra2_main.cc
    ssimulacra2.cc
)

target_include_directories(ssimulacra2_exe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

target_link_libraries(ssimulacra2_exe 
    jxl-static 
    jxl_extras-static 
    ${HWY_LIBRARIES} 
    ${LCMS2_LIBRARIES}
    ${IMAGE_LIBRARIES}
)

set_target_properties(ssimulacra2_exe PROPERTIES
    OUTPUT_NAME "ssimulacra2"
)

# Copy runtime DLLs on Windows
if(WIN32)
    add_custom_command(TARGET ssimulacra2_lib POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:ssimulacra2_lib> $<TARGET_RUNTIME_DLLS:ssimulacra2_lib>
        COMMAND_EXPAND_LISTS
    )
    
    add_custom_command(TARGET ssimulacra2_exe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:ssimulacra2_exe> $<TARGET_RUNTIME_DLLS:ssimulacra2_exe>
        COMMAND_EXPAND_LISTS
    )
endif()

# Print build summary
message(STATUS "")
message(STATUS "SSIMULACRA2 Build Configuration:")
message(STATUS "  Highway: ${HWY_FOUND}")
message(STATUS "  LCMS2: ${LCMS2_FOUND}")
message(STATUS "  JPEG: ${JPEG_FOUND}")
message(STATUS "  PNG: ${PNG_FOUND}")
message(STATUS "  ZLIB: ${ZLIB_FOUND}")
message(STATUS "  GIF: ${GIF_FOUND}")
message(STATUS "")
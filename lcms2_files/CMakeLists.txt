# Simple CMakeLists.txt for Little-CMS (LCMS2)
# This is a minimal build configuration for LCMS2 to work with SSIMULACRA2

cmake_minimum_required(VERSION 3.10)
project(lcms2 C)

# Set version
set(LCMS2_VERSION_MAJOR 2)
set(LCMS2_VERSION_MINOR 14)
set(LCMS2_VERSION_PATCH 0)

# Include directories
include_directories(include)

# Source files - include all .c files from src directory
set(LCMS2_SOURCES
    src/cmsalpha.c
    src/cmscam02.c
    src/cmscgats.c
    src/cmscnvrt.c
    src/cmserr.c
    src/cmsgamma.c
    src/cmsgmt.c
    src/cmshalf.c
    src/cmsintrp.c
    src/cmsio0.c
    src/cmsio1.c
    src/cmslut.c
    src/cmsmd5.c
    src/cmsmtrx.c
    src/cmsnamed.c
    src/cmsopt.c
    src/cmspack.c
    src/cmspcs.c
    src/cmsplugin.c
    src/cmsps2.c
    src/cmssamp.c
    src/cmssm.c
    src/cmstypes.c
    src/cmsvirt.c
    src/cmswtpnt.c
    src/cmsxform.c
)

# Create the library
add_library(lcms2 STATIC ${LCMS2_SOURCES})

# Set properties
set_target_properties(lcms2 PROPERTIES
    C_STANDARD 99
    POSITION_INDEPENDENT_CODE ON
)

# Define preprocessor macros
target_compile_definitions(lcms2 PRIVATE
    CMS_STATIC
)

# Export include directory for other targets
target_include_directories(lcms2 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Create alias for compatibility
add_library(LCMS2::LCMS2 ALIAS lcms2)

# Set variables for parent project
set(LCMS2_FOUND TRUE PARENT_SCOPE)
set(LCMS2_LIBRARIES lcms2 PARENT_SCOPE)
set(LCMS2_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
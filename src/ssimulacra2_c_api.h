// Copyright (c) Jon Sneyers, Cloudinary. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef SSIMULACRA2_C_API_H_
#define SSIMULACRA2_C_API_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Export macros for Windows DLL
#ifdef _WIN32
  #ifdef SSIMULACRA2_EXPORTS
    #define SSIMULACRA2_API __declspec(dllexport)
  #else
    #define SSIMULACRA2_API __declspec(dllimport)
  #endif
#else
  #define SSIMULACRA2_API __attribute__((visibility("default")))
#endif

// Error codes
typedef enum {
    SSIMULACRA2_OK = 0,
    SSIMULACRA2_ERROR_INVALID_INPUT = -1,
    SSIMULACRA2_ERROR_FILE_NOT_FOUND = -2,
    SSIMULACRA2_ERROR_UNSUPPORTED_FORMAT = -3,
    SSIMULACRA2_ERROR_SIZE_MISMATCH = -4,
    SSIMULACRA2_ERROR_TOO_SMALL = -5,
    SSIMULACRA2_ERROR_OUT_OF_MEMORY = -6,
    SSIMULACRA2_ERROR_CORRUPT_DATA = -7,
    SSIMULACRA2_ERROR_EMPTY_DATA = -8,
    SSIMULACRA2_ERROR_DECODE_FAILED = -9,
    SSIMULACRA2_ERROR_UNKNOWN = -99
} ssimulacra2_result;

// Compute SSIMULACRA2 score from file paths
// Returns the score (range -inf to 100) on success, or negative error code on failure
SSIMULACRA2_API double ssimulacra2_compute_from_files(
    const char* original_path,
    const char* distorted_path,
    ssimulacra2_result* result
);

// Compute SSIMULACRA2 score from file paths with alpha blending background
// bg_intensity: background intensity for alpha blending (0.0 to 1.0)
SSIMULACRA2_API double ssimulacra2_compute_from_files_with_background(
    const char* original_path,
    const char* distorted_path,
    float bg_intensity,
    ssimulacra2_result* result
);

// Compute SSIMULACRA2 score from memory buffers (PNG/JPEG data)
SSIMULACRA2_API double ssimulacra2_compute_from_memory(
    const unsigned char* original_data,
    size_t original_size,
    const unsigned char* distorted_data,
    size_t distorted_size,
    ssimulacra2_result* result
);

// Compute SSIMULACRA2 score from memory buffers with alpha blending background
SSIMULACRA2_API double ssimulacra2_compute_from_memory_with_background(
    const unsigned char* original_data,
    size_t original_size,
    const unsigned char* distorted_data,
    size_t distorted_size,
    float bg_intensity,
    ssimulacra2_result* result
);

// Get error message for result code
SSIMULACRA2_API const char* ssimulacra2_get_error_message(ssimulacra2_result result);

// Get version string
SSIMULACRA2_API const char* ssimulacra2_get_version(void);


#ifdef __cplusplus
}
#endif

#endif // SSIMULACRA2_C_API_H_
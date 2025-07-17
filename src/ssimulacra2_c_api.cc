// Copyright (c) Jon Sneyers, Cloudinary. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "ssimulacra2_c_api.h"
#include "ssimulacra2.h"

#include <stdio.h>
#include <string.h>
#include <memory>
#include <sstream>
#include <iomanip>

#include "lib/extras/codec.h"
#include "lib/jxl/color_management.h"
#include "lib/jxl/enc_color_management.h"

namespace {

ssimulacra2_result LoadImageFromFile(const char* path, jxl::CodecInOut* io) {
    if (!path || !io) {
        return SSIMULACRA2_ERROR_INVALID_INPUT;
    }

    if (!SetFromFile(path, jxl::extras::ColorHints(), io)) {
        return SSIMULACRA2_ERROR_FILE_NOT_FOUND;
    }

    if (io->xsize() < 8 || io->ysize() < 8) {
        return SSIMULACRA2_ERROR_TOO_SMALL;
    }

    return SSIMULACRA2_OK;
}

ssimulacra2_result LoadImageFromMemory(const uint8_t* data, size_t size, jxl::CodecInOut* io) {
    if (!data || size == 0 || !io) {
        return SSIMULACRA2_ERROR_INVALID_INPUT;
    }

    if (size == 0) {
        return SSIMULACRA2_ERROR_EMPTY_DATA;
    }

    try {
        jxl::Span<const uint8_t> span(data, size);
        if (!SetFromBytes(span, jxl::extras::ColorHints(), io)) {
            return SSIMULACRA2_ERROR_DECODE_FAILED;
        }

        if (io->xsize() < 8 || io->ysize() < 8) {
            return SSIMULACRA2_ERROR_TOO_SMALL;
        }

        return SSIMULACRA2_OK;

    } catch (const std::exception& e) {
        return SSIMULACRA2_ERROR_DECODE_FAILED;
    } catch (...) {
        return SSIMULACRA2_ERROR_UNKNOWN;
    }
}

double ComputeScore(const jxl::CodecInOut& io1, const jxl::CodecInOut& io2, float bg_intensity) {
    if (!io1.Main().HasAlpha()) {
        Msssim msssim = ComputeSSIMULACRA2(io1.Main(), io2.Main());
        return msssim.Score();
    } else {
        // For alpha transparency: blend against dark and bright backgrounds
        // and return the worst of both scores
        Msssim msssim0 = ComputeSSIMULACRA2(io1.Main(), io2.Main(), 0.1f);
        Msssim msssim1 = ComputeSSIMULACRA2(io1.Main(), io2.Main(), 0.9f);
        return std::min(msssim0.Score(), msssim1.Score());
    }
}

} // namespace

extern "C" {

double ssimulacra2_compute_from_files(
    const char* original_path,
    const char* distorted_path,
    ssimulacra2_result* result) {

    if (!original_path || !distorted_path) {
        if (result) *result = SSIMULACRA2_ERROR_INVALID_INPUT;
        return -1.0;
    }

    try {
        jxl::CodecInOut io1, io2;

        ssimulacra2_result load_result = LoadImageFromFile(original_path, &io1);
        if (load_result != SSIMULACRA2_OK) {
            if (result) *result = load_result;
            return -1.0;
        }

        load_result = LoadImageFromFile(distorted_path, &io2);
        if (load_result != SSIMULACRA2_OK) {
            if (result) *result = load_result;
            return -1.0;
        }

        if (io1.xsize() != io2.xsize() || io1.ysize() != io2.ysize()) {
            if (result) *result = SSIMULACRA2_ERROR_SIZE_MISMATCH;
            return -1.0;
        }

        double score = ComputeScore(io1, io2, 0.5f);
        if (result) *result = SSIMULACRA2_OK;
        return score;

    } catch (...) {
        if (result) *result = SSIMULACRA2_ERROR_UNKNOWN;
        return -1.0;
    }
}

double ssimulacra2_compute_from_files_with_background(
    const char* original_path,
    const char* distorted_path,
    float bg_intensity,
    ssimulacra2_result* result) {

    if (!original_path || !distorted_path || bg_intensity < 0.0f || bg_intensity > 1.0f) {
        if (result) *result = SSIMULACRA2_ERROR_INVALID_INPUT;
        return -1.0;
    }

    try {
        jxl::CodecInOut io1, io2;

        ssimulacra2_result load_result = LoadImageFromFile(original_path, &io1);
        if (load_result != SSIMULACRA2_OK) {
            if (result) *result = load_result;
            return -1.0;
        }

        load_result = LoadImageFromFile(distorted_path, &io2);
        if (load_result != SSIMULACRA2_OK) {
            if (result) *result = load_result;
            return -1.0;
        }

        if (io1.xsize() != io2.xsize() || io1.ysize() != io2.ysize()) {
            if (result) *result = SSIMULACRA2_ERROR_SIZE_MISMATCH;
            return -1.0;
        }

        Msssim msssim = ComputeSSIMULACRA2(io1.Main(), io2.Main(), bg_intensity);
        double score = msssim.Score();

        if (result) *result = SSIMULACRA2_OK;
        return score;

    } catch (...) {
        if (result) *result = SSIMULACRA2_ERROR_UNKNOWN;
        return -1.0;
    }
}

double ssimulacra2_compute_from_memory(
    const uint8_t* original_data,
    size_t original_size,
    const uint8_t* distorted_data,
    size_t distorted_size,
    ssimulacra2_result* result) {

    if (!original_data || !distorted_data || original_size == 0 || distorted_size == 0) {
        if (result) *result = SSIMULACRA2_ERROR_INVALID_INPUT;
        return -1.0;
    }

    try {
        jxl::CodecInOut io1, io2;

        ssimulacra2_result load_result = LoadImageFromMemory(original_data, original_size, &io1);
        if (load_result != SSIMULACRA2_OK) {
            if (result) *result = load_result;
            return -1.0;
        }

        load_result = LoadImageFromMemory(distorted_data, distorted_size, &io2);
        if (load_result != SSIMULACRA2_OK) {
            if (result) *result = load_result;
            return -1.0;
        }

        if (io1.xsize() != io2.xsize() || io1.ysize() != io2.ysize()) {
            if (result) *result = SSIMULACRA2_ERROR_SIZE_MISMATCH;
            return -1.0;
        }

        double score = ComputeScore(io1, io2, 0.5f);
        if (result) *result = SSIMULACRA2_OK;
        return score;

    } catch (...) {
        if (result) *result = SSIMULACRA2_ERROR_UNKNOWN;
        return -1.0;
    }
}

double ssimulacra2_compute_from_memory_with_background(
    const uint8_t* original_data,
    size_t original_size,
    const uint8_t* distorted_data,
    size_t distorted_size,
    float bg_intensity,
    ssimulacra2_result* result) {

    if (!original_data || !distorted_data || original_size == 0 || distorted_size == 0 ||
        bg_intensity < 0.0f || bg_intensity > 1.0f) {
        if (result) *result = SSIMULACRA2_ERROR_INVALID_INPUT;
        return -1.0;
    }

    try {
        jxl::CodecInOut io1, io2;

        ssimulacra2_result load_result = LoadImageFromMemory(original_data, original_size, &io1);
        if (load_result != SSIMULACRA2_OK) {
            if (result) *result = load_result;
            return -1.0;
        }

        load_result = LoadImageFromMemory(distorted_data, distorted_size, &io2);
        if (load_result != SSIMULACRA2_OK) {
            if (result) *result = load_result;
            return -1.0;
        }

        if (io1.xsize() != io2.xsize() || io1.ysize() != io2.ysize()) {
            if (result) *result = SSIMULACRA2_ERROR_SIZE_MISMATCH;
            return -1.0;
        }

        Msssim msssim = ComputeSSIMULACRA2(io1.Main(), io2.Main(), bg_intensity);
        double score = msssim.Score();

        if (result) *result = SSIMULACRA2_OK;
        return score;

    } catch (...) {
        if (result) *result = SSIMULACRA2_ERROR_UNKNOWN;
        return -1.0;
    }
}

const char* ssimulacra2_get_error_message(ssimulacra2_result result) {
    switch (result) {
        case SSIMULACRA2_OK:
            return "Success";
        case SSIMULACRA2_ERROR_INVALID_INPUT:
            return "Invalid input parameters";
        case SSIMULACRA2_ERROR_FILE_NOT_FOUND:
            return "File not found or could not be loaded";
        case SSIMULACRA2_ERROR_UNSUPPORTED_FORMAT:
            return "Unsupported image format";
        case SSIMULACRA2_ERROR_SIZE_MISMATCH:
            return "Image size mismatch";
        case SSIMULACRA2_ERROR_TOO_SMALL:
            return "Image too small (minimum 8x8 pixels)";
        case SSIMULACRA2_ERROR_OUT_OF_MEMORY:
            return "Out of memory";
        case SSIMULACRA2_ERROR_CORRUPT_DATA:
            return "Corrupt or invalid image data";
        case SSIMULACRA2_ERROR_EMPTY_DATA:
            return "Empty data buffer";
        case SSIMULACRA2_ERROR_DECODE_FAILED:
            return "Failed to decode image data";
        case SSIMULACRA2_ERROR_UNKNOWN:
            return "Unknown error";
        default:
            return "Invalid error code";
    }
}

const char* ssimulacra2_get_version(void) {
    return "SSIMULACRA 2.1";
}

} // extern "C"
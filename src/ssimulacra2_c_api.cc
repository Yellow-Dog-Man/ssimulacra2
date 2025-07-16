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

// Thread-local storage for detailed error information
thread_local std::string g_last_error_details;

void SetLastErrorDetails(const std::string& details) {
    g_last_error_details = details;
}

std::string AnalyzeImageHeader(const uint8_t* data, size_t size) {
    std::ostringstream analysis;
    
    if (!data || size == 0) {
        return "Data is null or empty";
    }
    
    analysis << "Data size: " << size << " bytes\n";
    
    // Show first 16 bytes as hex
    analysis << "Header bytes (hex): ";
    size_t header_size = std::min(size, size_t(16));
    for (size_t i = 0; i < header_size; ++i) {
        analysis << std::hex << std::setfill('0') << std::setw(2) << (int)data[i] << " ";
    }
    analysis << "\n";
    
    // Detect common image formats
    if (size >= 8) {
        // PNG signature
        if (data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47 &&
            data[4] == 0x0D && data[5] == 0x0A && data[6] == 0x1A && data[7] == 0x0A) {
            analysis << "Format detected: PNG\n";
        }
        // JPEG signature
        else if (data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF) {
            analysis << "Format detected: JPEG\n";
        }
        // GIF signature
        else if (size >= 6 && 
                 ((data[0] == 'G' && data[1] == 'I' && data[2] == 'F' && data[3] == '8' && data[4] == '7' && data[5] == 'a') ||
                  (data[0] == 'G' && data[1] == 'I' && data[2] == 'F' && data[3] == '8' && data[4] == '9' && data[5] == 'a'))) {
            analysis << "Format detected: GIF\n";
        }
        // BMP signature
        else if (data[0] == 'B' && data[1] == 'M') {
            analysis << "Format detected: BMP\n";
        }
        // TIFF signatures
        else if ((data[0] == 'I' && data[1] == 'I' && data[2] == 0x2A && data[3] == 0x00) ||
                 (data[0] == 'M' && data[1] == 'M' && data[2] == 0x00 && data[3] == 0x2A)) {
            analysis << "Format detected: TIFF\n";
        }
        // WebP signature
        else if (size >= 12 && data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F' &&
                 data[8] == 'W' && data[9] == 'E' && data[10] == 'B' && data[11] == 'P') {
            analysis << "Format detected: WebP\n";
        }
        else {
            analysis << "Format detected: Unknown/Unsupported\n";
            analysis << "Note: Supported formats are PNG, JPEG, GIF, BMP, TIFF, WebP\n";
        }
    } else {
        analysis << "Format detected: Data too small to determine format\n";
    }
    
    return analysis.str();
}

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
        SetLastErrorDetails("Invalid input: data is null, size is zero, or io is null");
        return SSIMULACRA2_ERROR_INVALID_INPUT;
    }

    if (size == 0) {
        SetLastErrorDetails("Empty data buffer provided");
        return SSIMULACRA2_ERROR_EMPTY_DATA;
    }

    // Analyze the image data for diagnostics
    std::string analysis = AnalyzeImageHeader(data, size);
    
    try {
        jxl::Span<const uint8_t> span(data, size);
        if (!SetFromBytes(span, jxl::extras::ColorHints(), io)) {
            std::ostringstream error_details;
            error_details << "Failed to decode image data.\n";
            error_details << "Image analysis:\n" << analysis;
            error_details << "\nPossible causes:\n";
            error_details << "- Corrupted image data\n";
            error_details << "- Unsupported image format variant\n";
            error_details << "- Incomplete image data\n";
            error_details << "- Missing required image libraries\n";
            SetLastErrorDetails(error_details.str());
            return SSIMULACRA2_ERROR_DECODE_FAILED;
        }

        if (io->xsize() < 8 || io->ysize() < 8) {
            std::ostringstream error_details;
            error_details << "Image too small: " << io->xsize() << "x" << io->ysize() << " pixels\n";
            error_details << "Minimum required size: 8x8 pixels\n";
            error_details << "Image analysis:\n" << analysis;
            SetLastErrorDetails(error_details.str());
            return SSIMULACRA2_ERROR_TOO_SMALL;
        }

        // Success - clear any previous error details
        SetLastErrorDetails("");
        return SSIMULACRA2_OK;
        
    } catch (const std::exception& e) {
        std::ostringstream error_details;
        error_details << "Exception during image loading: " << e.what() << "\n";
        error_details << "Image analysis:\n" << analysis;
        SetLastErrorDetails(error_details.str());
        return SSIMULACRA2_ERROR_DECODE_FAILED;
    } catch (...) {
        std::ostringstream error_details;
        error_details << "Unknown exception during image loading\n";
        error_details << "Image analysis:\n" << analysis;
        SetLastErrorDetails(error_details.str());
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

const char* ssimulacra2_get_last_error_details(void) {
    return g_last_error_details.c_str();
}

const char* ssimulacra2_analyze_image_data(
    const unsigned char* data,
    size_t size) {
    
    if (!data || size == 0) {
        return "Error: Data is null or empty";
    }
    
    // Use a static string to return the analysis (not thread-safe but simple)
    static std::string analysis_result;
    analysis_result = AnalyzeImageHeader(data, size);
    return analysis_result.c_str();
}

} // extern "C"
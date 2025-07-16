using System;
using System.Runtime.InteropServices;

namespace Ssimulacra2
{
    /// <summary>
    /// Error codes returned by SSIMULACRA2 functions
    /// </summary>
    public enum Ssimulacra2Result
    {
        Ok = 0,
        InvalidInput = -1,
        FileNotFound = -2,
        UnsupportedFormat = -3,
        SizeMismatch = -4,
        TooSmall = -5,
        OutOfMemory = -6,
        CorruptData = -7,
        EmptyData = -8,
        DecodeFailed = -9,
        Unknown = -99
    }

    /// <summary>
    /// C# wrapper for SSIMULACRA2 library
    /// </summary>
    public static class Ssimulacra2Native
    {
        private const string LibraryName = "ssimulacra2";

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern double ssimulacra2_compute_from_files(
            [MarshalAs(UnmanagedType.LPStr)] string originalPath,
            [MarshalAs(UnmanagedType.LPStr)] string distortedPath,
            out Ssimulacra2Result result);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern double ssimulacra2_compute_from_files_with_background(
            [MarshalAs(UnmanagedType.LPStr)] string originalPath,
            [MarshalAs(UnmanagedType.LPStr)] string distortedPath,
            float bgIntensity,
            out Ssimulacra2Result result);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern double ssimulacra2_compute_from_memory(
            IntPtr originalData,
            UIntPtr originalSize,
            IntPtr distortedData,
            UIntPtr distortedSize,
            out Ssimulacra2Result result);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern double ssimulacra2_compute_from_memory_with_background(
            IntPtr originalData,
            UIntPtr originalSize,
            IntPtr distortedData,
            UIntPtr distortedSize,
            float bgIntensity,
            out Ssimulacra2Result result);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr ssimulacra2_get_error_message(Ssimulacra2Result result);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr ssimulacra2_get_version();

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr ssimulacra2_get_last_error_details();

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr ssimulacra2_analyze_image_data(
            IntPtr data,
            UIntPtr size);

        /// <summary>
        /// Compute SSIMULACRA2 score between two image files
        /// </summary>
        /// <param name="originalPath">Path to the original image</param>
        /// <param name="distortedPath">Path to the distorted image</param>
        /// <returns>SSIMULACRA2 score (range -inf to 100)</returns>
        /// <exception cref="Ssimulacra2Exception">Thrown when computation fails</exception>
        public static double ComputeFromFiles(string originalPath, string distortedPath)
        {
            double score = ssimulacra2_compute_from_files(originalPath, distortedPath, out Ssimulacra2Result result);

            if (result != Ssimulacra2Result.Ok)
            {
                string errorMessage = GetErrorMessage(result);
                throw new Ssimulacra2Exception($"Failed to compute SSIMULACRA2 score: {errorMessage}", result);
            }

            return score;
        }

        /// <summary>
        /// Compute SSIMULACRA2 score between two image files with custom background intensity for alpha blending
        /// </summary>
        /// <param name="originalPath">Path to the original image</param>
        /// <param name="distortedPath">Path to the distorted image</param>
        /// <param name="backgroundIntensity">Background intensity for alpha blending (0.0 to 1.0)</param>
        /// <returns>SSIMULACRA2 score (range -inf to 100)</returns>
        /// <exception cref="Ssimulacra2Exception">Thrown when computation fails</exception>
        public static double ComputeFromFiles(string originalPath, string distortedPath, float backgroundIntensity)
        {
            double score = ssimulacra2_compute_from_files_with_background(originalPath, distortedPath, backgroundIntensity, out Ssimulacra2Result result);

            if (result != Ssimulacra2Result.Ok)
            {
                string errorMessage = GetErrorMessage(result);
                throw new Ssimulacra2Exception($"Failed to compute SSIMULACRA2 score: {errorMessage}", result);
            }

            return score;
        }

        /// <summary>
        /// Compute SSIMULACRA2 score between two images from memory buffers
        /// </summary>
        /// <param name="originalData">Original image data (PNG/JPEG bytes)</param>
        /// <param name="distortedData">Distorted image data (PNG/JPEG bytes)</param>
        /// <returns>SSIMULACRA2 score (range -inf to 100)</returns>
        /// <exception cref="Ssimulacra2Exception">Thrown when computation fails</exception>
        public static double ComputeFromMemory(byte[] originalData, byte[] distortedData)
        {
            if (originalData == null || distortedData == null)
                throw new ArgumentNullException("Image data cannot be null");

            GCHandle originalHandle = GCHandle.Alloc(originalData, GCHandleType.Pinned);
            GCHandle distortedHandle = GCHandle.Alloc(distortedData, GCHandleType.Pinned);

            try
            {
                IntPtr originalPtr = originalHandle.AddrOfPinnedObject();
                IntPtr distortedPtr = distortedHandle.AddrOfPinnedObject();

                double score = ssimulacra2_compute_from_memory(
                    originalPtr, (UIntPtr)originalData.Length,
                    distortedPtr, (UIntPtr)distortedData.Length,
                    out Ssimulacra2Result result);

                if (result != Ssimulacra2Result.Ok)
                {
                    string errorMessage = GetErrorMessage(result);
                    throw new Ssimulacra2Exception($"Failed to compute SSIMULACRA2 score: {errorMessage}", result);
                }

                return score;
            }
            finally
            {
                originalHandle.Free();
                distortedHandle.Free();
            }
        }

        /// <summary>
        /// Compute SSIMULACRA2 score between two images from memory buffers with custom background intensity
        /// </summary>
        /// <param name="originalData">Original image data (PNG/JPEG bytes)</param>
        /// <param name="distortedData">Distorted image data (PNG/JPEG bytes)</param>
        /// <param name="backgroundIntensity">Background intensity for alpha blending (0.0 to 1.0)</param>
        /// <returns>SSIMULACRA2 score (range -inf to 100)</returns>
        /// <exception cref="Ssimulacra2Exception">Thrown when computation fails</exception>
        public static double ComputeFromMemory(byte[] originalData, byte[] distortedData, float backgroundIntensity)
        {
            if (originalData == null || distortedData == null)
                throw new ArgumentNullException("Image data cannot be null");

            GCHandle originalHandle = GCHandle.Alloc(originalData, GCHandleType.Pinned);
            GCHandle distortedHandle = GCHandle.Alloc(distortedData, GCHandleType.Pinned);

            try
            {
                IntPtr originalPtr = originalHandle.AddrOfPinnedObject();
                IntPtr distortedPtr = distortedHandle.AddrOfPinnedObject();

                double score = ssimulacra2_compute_from_memory_with_background(
                    originalPtr, (UIntPtr)originalData.Length,
                    distortedPtr, (UIntPtr)distortedData.Length,
                    backgroundIntensity,
                    out Ssimulacra2Result result);

                if (result != Ssimulacra2Result.Ok)
                {
                    string errorMessage = GetErrorMessage(result);
                    throw new Ssimulacra2Exception($"Failed to compute SSIMULACRA2 score: {errorMessage}", result);
                }

                return score;
            }
            finally
            {
                originalHandle.Free();
                distortedHandle.Free();
            }
        }

        /// <summary>
        /// Get error message for a result code
        /// </summary>
        /// <param name="result">Result code</param>
        /// <returns>Error message string</returns>
        public static string GetErrorMessage(Ssimulacra2Result result)
        {
            IntPtr ptr = ssimulacra2_get_error_message(result);
            return Marshal.PtrToStringAnsi(ptr) ?? "Unknown error";
        }

        /// <summary>
        /// Get library version string
        /// </summary>
        /// <returns>Version string</returns>
        public static string GetVersion()
        {
            IntPtr ptr = ssimulacra2_get_version();
            return Marshal.PtrToStringAnsi(ptr) ?? "Unknown version";
        }

        /// <summary>
        /// Get detailed diagnostic information about the last error
        /// </summary>
        /// <returns>Detailed error information string</returns>
        public static string GetLastErrorDetails()
        {
            IntPtr ptr = ssimulacra2_get_last_error_details();
            return Marshal.PtrToStringAnsi(ptr) ?? "";
        }

        /// <summary>
        /// Analyze image data and return diagnostic information
        /// </summary>
        /// <param name="imageData">Image data to analyze</param>
        /// <returns>Diagnostic information about the image data</returns>
        public static string AnalyzeImageData(byte[] imageData)
        {
            if (imageData == null)
                return "Error: Image data is null";

            GCHandle handle = GCHandle.Alloc(imageData, GCHandleType.Pinned);
            try
            {
                IntPtr dataPtr = handle.AddrOfPinnedObject();
                IntPtr ptr = ssimulacra2_analyze_image_data(dataPtr, (UIntPtr)imageData.Length);
                return Marshal.PtrToStringAnsi(ptr) ?? "Error: Failed to analyze image data";
            }
            finally
            {
                handle.Free();
            }
        }
    }

    /// <summary>
    /// Exception thrown by SSIMULACRA2 operations
    /// </summary>
    public class Ssimulacra2Exception : Exception
    {
        public Ssimulacra2Result ResultCode { get; }

        public Ssimulacra2Exception(string message, Ssimulacra2Result resultCode) : base(message)
        {
            ResultCode = resultCode;
        }

        public Ssimulacra2Exception(string message, Ssimulacra2Result resultCode, Exception innerException) 
            : base(message, innerException)
        {
            ResultCode = resultCode;
        }
    }
}
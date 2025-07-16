using System;
using System.IO;
using Ssimulacra2;

class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine($"SSIMULACRA2 C# Example - {Ssimulacra2Native.GetVersion()}");
        Console.WriteLine();

        if (args.Length < 2)
        {
            Console.WriteLine("Usage: dotnet run <original_image> <distorted_image> [background_intensity]");
            Console.WriteLine();
            Console.WriteLine("Examples:");
            Console.WriteLine("  dotnet run original.png distorted.png");
            Console.WriteLine("  dotnet run original.png distorted.png 0.5");
            Console.WriteLine();
            Console.WriteLine("Background intensity is optional (0.0 to 1.0, default 0.5)");
            Console.WriteLine("Used for alpha blending when images have transparency");
            return;
        }

        string originalPath = args[0];
        string distortedPath = args[1];
        float backgroundIntensity = 0.5f;

        if (args.Length > 2)
        {
            if (!float.TryParse(args[2], out backgroundIntensity) || 
                backgroundIntensity < 0.0f || backgroundIntensity > 1.0f)
            {
                Console.WriteLine("Error: Background intensity must be a number between 0.0 and 1.0");
                return;
            }
        }

        try
        {
            // Check if files exist
            if (!File.Exists(originalPath))
            {
                Console.WriteLine($"Error: Original image file not found: {originalPath}");
                return;
            }

            if (!File.Exists(distortedPath))
            {
                Console.WriteLine($"Error: Distorted image file not found: {distortedPath}");
                return;
            }

            Console.WriteLine($"Computing SSIMULACRA2 score...");
            Console.WriteLine($"Original: {originalPath}");
            Console.WriteLine($"Distorted: {distortedPath}");
            
            if (args.Length > 2)
            {
                Console.WriteLine($"Background intensity: {backgroundIntensity}");
            }
            
            Console.WriteLine();

            // First, let's analyze the image data for diagnostics
            Console.WriteLine("=== IMAGE ANALYSIS ===");
            byte[] originalData = File.ReadAllBytes(originalPath);
            byte[] distortedData = File.ReadAllBytes(distortedPath);
            
            Console.WriteLine("Original image analysis:");
            Console.WriteLine(Ssimulacra2Native.AnalyzeImageData(originalData));
            Console.WriteLine();
            
            Console.WriteLine("Distorted image analysis:");
            Console.WriteLine(Ssimulacra2Native.AnalyzeImageData(distortedData));
            Console.WriteLine();

            // Compute the score
            double score;
            if (args.Length > 2)
            {
                score = Ssimulacra2Native.ComputeFromFiles(originalPath, distortedPath, backgroundIntensity);
            }
            else
            {
                score = Ssimulacra2Native.ComputeFromFiles(originalPath, distortedPath);
            }

            Console.WriteLine($"SSIMULACRA2 Score: {score:F8}");
            Console.WriteLine();

            // Provide quality interpretation
            Console.WriteLine("Quality interpretation:");
            if (score < 0)
            {
                Console.WriteLine("  Extremely low quality, very strong distortion");
            }
            else if (score < 10)
            {
                Console.WriteLine("  Very low quality");
            }
            else if (score < 30)
            {
                Console.WriteLine("  Low quality");
            }
            else if (score < 50)
            {
                Console.WriteLine("  Medium quality");
            }
            else if (score < 70)
            {
                Console.WriteLine("  High quality");
            }
            else if (score < 80)
            {
                Console.WriteLine("  Very high quality");
            }
            else if (score < 85)
            {
                Console.WriteLine("  Excellent quality");
            }
            else if (score < 90)
            {
                Console.WriteLine("  Visually lossless");
            }
            else
            {
                Console.WriteLine("  Near mathematically lossless");
            }

            // Example of using memory-based API with enhanced error reporting
            Console.WriteLine();
            Console.WriteLine("=== TESTING MEMORY-BASED API ===");
            
            try
            {
                double memoryScore = Ssimulacra2Native.ComputeFromMemory(originalData, distortedData);
                Console.WriteLine($"Score from memory: {memoryScore:F8}");
                
                if (Math.Abs(score - memoryScore) < 1e-6)
                {
                    Console.WriteLine("✓ File and memory APIs produce identical results");
                }
                else
                {
                    Console.WriteLine("⚠ File and memory APIs produce different results (unexpected)");
                }
            }
            catch (Ssimulacra2Exception memEx)
            {
                Console.WriteLine($"❌ Memory API failed: {memEx.Message}");
                Console.WriteLine($"Error Code: {memEx.ResultCode}");
                
                // Get detailed diagnostic information
                string details = Ssimulacra2Native.GetLastErrorDetails();
                if (!string.IsNullOrEmpty(details))
                {
                    Console.WriteLine();
                    Console.WriteLine("=== DETAILED ERROR DIAGNOSTICS ===");
                    Console.WriteLine(details);
                }
            }
        }
        catch (Ssimulacra2Exception ex)
        {
            Console.WriteLine($"❌ SSIMULACRA2 Error: {ex.Message}");
            Console.WriteLine($"Error Code: {ex.ResultCode}");
            
            // Get detailed diagnostic information
            string details = Ssimulacra2Native.GetLastErrorDetails();
            if (!string.IsNullOrEmpty(details))
            {
                Console.WriteLine();
                Console.WriteLine("=== DETAILED ERROR DIAGNOSTICS ===");
                Console.WriteLine(details);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"❌ Unexpected error: {ex.Message}");
            Console.WriteLine($"Stack trace: {ex.StackTrace}");
        }
    }
}
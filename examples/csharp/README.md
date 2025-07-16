# SSIMULACRA2 C# Example

This example demonstrates how to use the SSIMULACRA2 shared library from C#.

## Prerequisites

- .NET 6.0 or later
- Built SSIMULACRA2 shared library (`ssimulacra2.dll` on Windows)

## Usage

### Basic Usage
```bash
dotnet run original.png distorted.png
```

### With Custom Background Intensity
```bash
dotnet run original.png distorted.png 0.8
```

## API Overview

The C# wrapper provides these main functions:

### File-based Comparison
```csharp
// Basic comparison
double score = Ssimulacra2Native.ComputeFromFiles("original.png", "distorted.png");

// With custom background for alpha blending
double score = Ssimulacra2Native.ComputeFromFiles("original.png", "distorted.png", 0.5f);
```

### Memory-based Comparison
```csharp
byte[] originalData = File.ReadAllBytes("original.png");
byte[] distortedData = File.ReadAllBytes("distorted.png");

// Basic comparison
double score = Ssimulacra2Native.ComputeFromMemory(originalData, distortedData);

// With custom background
double score = Ssimulacra2Native.ComputeFromMemory(originalData, distortedData, 0.5f);
```

### Error Handling
```csharp
try {
    double score = Ssimulacra2Native.ComputeFromFiles("original.png", "distorted.png");
    Console.WriteLine($"Score: {score:F8}");
} catch (Ssimulacra2Exception ex) {
    Console.WriteLine($"Error: {ex.Message}");
    Console.WriteLine($"Error Code: {ex.ResultCode}");
}
```

## Score Interpretation

- **100**: Mathematically lossless
- **90+**: Visually lossless  
- **85+**: Excellent quality
- **80+**: Very high quality
- **70+**: High quality
- **50+**: Medium quality
- **30+**: Low quality
- **10+**: Very low quality
- **<0**: Extremely low quality

## Troubleshooting

### "Unable to load DLL 'ssimulacra2'"
Make sure `ssimulacra2.dll` is in the same directory as your executable, or in your system PATH.

### "The specified module could not be found"
This usually means missing dependencies. Make sure all required DLLs (like `hwy.dll`) are available.

### Image Loading Errors
- Ensure images are in supported formats (PNG, JPEG, etc.)
- Check that both images have the same dimensions
- Verify images are at least 8x8 pixels
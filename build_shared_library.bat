@echo off
REM Build script for Windows to create SSIMULACRA2 shared library

echo Building SSIMULACRA2 shared library for Windows...

REM Create build directory
if not exist build mkdir build
cd build

REM Try to find vcpkg toolchain file
set "VCPKG_TOOLCHAIN="
if defined VCPKG_ROOT (
    set "VCPKG_TOOLCHAIN=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
    echo Found vcpkg at: %VCPKG_ROOT%
) else (
    REM Try common vcpkg locations
    if exist "C:\vcpkg\scripts\buildsystems\vcpkg.cmake" (
        set "VCPKG_TOOLCHAIN=C:\vcpkg\scripts\buildsystems\vcpkg.cmake"
        set "VCPKG_ROOT=C:\vcpkg"
        echo Found vcpkg at: C:\vcpkg
    ) else if exist "C:\tools\vcpkg\scripts\buildsystems\vcpkg.cmake" (
        set "VCPKG_TOOLCHAIN=C:\tools\vcpkg\scripts\buildsystems\vcpkg.cmake"
        set "VCPKG_ROOT=C:\tools\vcpkg"
        echo Found vcpkg at: C:\tools\vcpkg
    ) else if exist "%USERPROFILE%\vcpkg\scripts\buildsystems\vcpkg.cmake" (
        set "VCPKG_TOOLCHAIN=%USERPROFILE%\vcpkg\scripts\buildsystems\vcpkg.cmake"
        set "VCPKG_ROOT=%USERPROFILE%\vcpkg"
        echo Found vcpkg at: %USERPROFILE%\vcpkg
    ) else if exist "D:\Documents\GitHub\ResoniteDevelopment\vcpkg\scripts\buildsystems\vcpkg.cmake" (
        set "VCPKG_TOOLCHAIN=D:\Documents\GitHub\ResoniteDevelopment\vcpkg\scripts\buildsystems\vcpkg.cmake"
        set "VCPKG_ROOT=D:\Documents\GitHub\ResoniteDevelopment\vcpkg"
        echo Found vcpkg at: D:\Documents\GitHub\ResoniteDevelopment\vcpkg
    ) else (
        echo Warning: vcpkg not found. Image format libraries may not be available.
        echo To use vcpkg, set VCPKG_ROOT environment variable or install vcpkg to C:\vcpkg
        echo.
        echo You can install vcpkg by running:
        echo   git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
        echo   C:\vcpkg\bootstrap-vcpkg.bat
        echo   C:\vcpkg\vcpkg integrate install
    )
)

REM Configure with CMake
if defined VCPKG_TOOLCHAIN (
    echo Using vcpkg toolchain: %VCPKG_TOOLCHAIN%
    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DSSIMULACRA2_BUILD_EXECUTABLE=ON -DCMAKE_TOOLCHAIN_FILE="%VCPKG_TOOLCHAIN%" ../src
) else (
    echo Building without vcpkg integration
    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DSSIMULACRA2_BUILD_EXECUTABLE=ON ../src
)

REM Build the project
cmake --build . --config Release

echo.
echo Build complete!
echo.
echo Shared library: build/Release/ssimulacra2.dll
echo Executable: build/Release/ssimulacra2.exe
echo.
echo To test the C# example:
echo   cd examples/csharp
echo   dotnet run path/to/original.png path/to/distorted.png

pause
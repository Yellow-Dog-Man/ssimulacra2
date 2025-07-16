@echo off
REM Setup script for Windows to install dependencies for SSIMULACRA2

echo Setting up dependencies for SSIMULACRA2...
echo.

REM Check if vcpkg is available
where vcpkg >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo Found vcpkg, installing dependencies...
    vcpkg install highway lcms2 libjpeg-turbo libpng zlib giflib
    if %ERRORLEVEL% == 0 (
        echo Dependencies installed successfully with vcpkg!
        echo.
        echo You can now run: build_shared_library.bat
        goto :end
    ) else (
        echo vcpkg installation failed, trying git submodules...
    )
) else (
    echo vcpkg not found, using git submodules...
)

REM Create third_party directory
if not exist src\third_party mkdir src\third_party

REM Add Highway as submodule
echo Adding Highway library as git submodule...
git submodule add https://github.com/google/highway.git src/third_party/highway
if %ERRORLEVEL% neq 0 (
    echo Warning: Failed to add Highway submodule. It might already exist.
)

REM Add LCMS2 as submodule  
echo Adding LCMS2 library as git submodule...
git submodule add https://github.com/mm2/Little-CMS.git src/third_party/lcms
if %ERRORLEVEL% neq 0 (
    echo Warning: Failed to add LCMS2 submodule. It might already exist.
)

REM Initialize and update submodules
echo Initializing and updating submodules...
git submodule update --init --recursive

echo.
echo Dependencies setup complete!
echo.
echo You can now run: build_shared_library.bat

:end
pause
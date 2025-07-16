@echo off
REM Script to set up vcpkg and install required packages for SSIMULACRA2

echo Setting up vcpkg for SSIMULACRA2...
echo.

REM Check if vcpkg is already available
where vcpkg >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo ✓ vcpkg is already available in PATH
    goto :install_packages
)

REM Check if VCPKG_ROOT is set
if defined VCPKG_ROOT (
    if exist "%VCPKG_ROOT%\vcpkg.exe" (
        echo ✓ Found vcpkg at: %VCPKG_ROOT%
        set "PATH=%VCPKG_ROOT%;%PATH%"
        goto :install_packages
    )
)

REM Check common vcpkg locations
if exist "C:\vcpkg\vcpkg.exe" (
    echo ✓ Found vcpkg at: C:\vcpkg
    set "VCPKG_ROOT=C:\vcpkg"
    set "PATH=C:\vcpkg;%PATH%"
    goto :install_packages
) else if exist "C:\tools\vcpkg\vcpkg.exe" (
    echo ✓ Found vcpkg at: C:\tools\vcpkg
    set "VCPKG_ROOT=C:\tools\vcpkg"
    set "PATH=C:\tools\vcpkg;%PATH%"
    goto :install_packages
) else if exist "%USERPROFILE%\vcpkg\vcpkg.exe" (
    echo ✓ Found vcpkg at: %USERPROFILE%\vcpkg
    set "VCPKG_ROOT=%USERPROFILE%\vcpkg"
    set "PATH=%USERPROFILE%\vcpkg;%PATH%"
    goto :install_packages
)

REM vcpkg not found, offer to install it
echo vcpkg not found. Would you like to install it?
echo.
echo This will:
echo 1. Clone vcpkg to C:\vcpkg
echo 2. Bootstrap vcpkg
echo 3. Integrate with Visual Studio
echo 4. Install required packages
echo.
set /p "choice=Install vcpkg? (y/n): "
if /i "%choice%" neq "y" (
    echo Skipping vcpkg installation.
    echo You can install it manually later with:
    echo   git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
    echo   C:\vcpkg\bootstrap-vcpkg.bat
    echo   C:\vcpkg\vcpkg integrate install
    pause
    exit /b 0
)

echo.
echo Installing vcpkg...
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
if %ERRORLEVEL% neq 0 (
    echo Failed to clone vcpkg repository
    pause
    exit /b 1
)

echo Bootstrapping vcpkg...
C:\vcpkg\bootstrap-vcpkg.bat
if %ERRORLEVEL% neq 0 (
    echo Failed to bootstrap vcpkg
    pause
    exit /b 1
)

echo Integrating vcpkg with Visual Studio...
C:\vcpkg\vcpkg integrate install

set "VCPKG_ROOT=C:\vcpkg"
set "PATH=C:\vcpkg;%PATH%"

:install_packages
echo.
echo Installing required packages...
echo This may take several minutes...
echo.

REM Install packages one by one with progress
echo [1/6] Installing highway...
vcpkg install highway:x64-windows
if %ERRORLEVEL% neq 0 (
    echo Warning: Failed to install highway
)

echo [2/6] Installing lcms2...
vcpkg install lcms2:x64-windows
if %ERRORLEVEL% neq 0 (
    echo Warning: Failed to install lcms2
)

echo [3/6] Installing libjpeg-turbo...
vcpkg install libjpeg-turbo:x64-windows
if %ERRORLEVEL% neq 0 (
    echo Warning: Failed to install libjpeg-turbo
)

echo [4/6] Installing libpng...
vcpkg install libpng:x64-windows
if %ERRORLEVEL% neq 0 (
    echo Warning: Failed to install libpng
)

echo [5/6] Installing zlib...
vcpkg install zlib:x64-windows
if %ERRORLEVEL% neq 0 (
    echo Warning: Failed to install zlib
)

echo [6/6] Installing giflib...
vcpkg install giflib:x64-windows
if %ERRORLEVEL% neq 0 (
    echo Warning: Failed to install giflib
)

echo.
echo ✅ Package installation complete!
echo.
echo vcpkg is now set up with the following packages:
vcpkg list | findstr /i "highway lcms2 libjpeg libpng zlib giflib"
echo.
echo You can now build SSIMULACRA2 with image format support:
echo   build_shared_library.bat
echo.
echo Note: Make sure to set VCPKG_ROOT environment variable permanently:
echo   setx VCPKG_ROOT "%VCPKG_ROOT%"

pause
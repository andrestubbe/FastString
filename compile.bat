@echo off
setlocal EnableDelayedExpansion

echo ===========================================
echo FastString v1.0 - SIMD-Accelerated Build
echo ===========================================
echo.

:: Check for Java
if not defined JAVA_HOME (
    if exist "C:\Program Files\Java\jdk-25" (
        set "JAVA_HOME=C:\Program Files\Java\jdk-25"
    ) else if exist "C:\Program Files\Java\jdk-17" (
        set "JAVA_HOME=C:\Program Files\Java\jdk-17"
    ) else if exist "C:\Program Files\Eclipse Adoptium\jdk-17-hotspot" (
        set "JAVA_HOME=C:\Program Files\Eclipse Adoptium\jdk-17-hotspot"
    )
)

if not defined JAVA_HOME (
    echo ERROR: JAVA_HOME not set!
    echo Please set JAVA_HOME to your JDK installation path
    pause
    exit /b 1
)

if not exist "%JAVA_HOME%\include\jni.h" (
    echo ERROR: Cannot find jni.h in %JAVA_HOME%\include
    echo Please check your Java installation
    pause
    exit /b 1
)

echo Using JAVA_HOME: %JAVA_HOME%

:: Use vswhere to find Visual Studio
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "%VSWHERE%" (
    echo ERROR: vswhere.exe not found!
    echo Visual Studio Installer might be missing.
    pause
    exit /b 1
)

:: Find VS installation path
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_INSTALL=%%i"
)

if not defined VS_INSTALL (
    echo ERROR: Visual Studio with C++ tools not found!
    echo.
    pause
    exit /b 1
)

echo Found Visual Studio at: %VS_INSTALL%

:: Setup VS environment
set "VCVARS=%VS_INSTALL%\VC\Auxiliary\Build\vcvars64.bat"

echo Setting up Visual Studio environment...
call "%VCVARS%"
if errorlevel 1 (
    echo ERROR: Failed to setup VS environment
    pause
    exit /b 1
)

:: Create build directory
if not exist build mkdir build

:: Compile
echo.
echo Compiling FastString v1.0 with AVX2/SSE4.2 SIMD support...
echo =====================================================
cl /LD /Fe:build\faststring.dll ^
    native\faststring.cpp ^
    /I"%JAVA_HOME%\include" ^
    /I"%JAVA_HOME%\include\win32" ^
    /EHsc /std:c++17 /O2 /W3 /arch:AVX2

:: Check result
if %errorlevel% neq 0 (
    echo.
    echo =====================================================
    echo COMPILATION FAILED
    echo =====================================================
    echo Check errors above
    pause
    exit /b 1
)

:: Copy to resources
echo.
echo Copying DLL to resources...
if not exist "src\main\resources\native" mkdir "src\main\resources\native"
copy build\faststring.dll src\main\resources\native\faststring.dll

:: Success
echo.
echo =====================================================
echo COMPILATION SUCCESSFUL!
echo =====================================================
echo.
echo FastString v1.0 DLL created with:
echo - AVX2 256-bit SIMD operations
echo - SSE4.2 fallback
echo - Zero-copy substring support
echo - 10-100x faster than Java String
echo.
echo You can now build with Maven:
echo   mvn clean package
echo.
echo Run benchmark:
echo   mvn exec:java
echo.

pause

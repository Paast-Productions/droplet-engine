@echo off
setlocal

cd /d "%~dp0" || exit /b 1

set "project=%~1"
set "config=%~2"

if "%project%"=="" (
    echo ERROR: Missing project argument
    exit /b 1
)

if "%config%"=="" (
    echo ERROR: Missing configuration argument
    exit /b 1
)

set "outputDir=..\..\Build\Target\%config%\%project%"

if not exist "%outputDir%" (
    mkdir "%outputDir%" || exit /b 1
)

echo === Checking Slang compiler ===
where slangc

if errorlevel 1 (
    echo ERROR: slangc.exe was not found in PATH
    exit /b 1
)

echo === Compiling shaders for %project% - %config% ===

slangc debug_draw.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -o "%outputDir%\debug_draw.vert.spv" || exit /b 1

slangc debug_draw.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry fragMain -o "%outputDir%\debug_draw.frag.spv" || exit /b 1

slangc debug_icon.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry iconVertMain -o "%outputDir%\debug_icon.vert.spv" || exit /b 1

slangc debug_icon.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry iconFragMain -o "%outputDir%\debug_icon.frag.spv" || exit /b 1

echo === Shader compilation completed successfully ===
exit /b 0
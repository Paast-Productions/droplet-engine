@echo off
cd /d "%~dp0"
slangc debug_draw.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -o ../../../../Generated/debug_draw.vert.spv || exit /b 1
slangc debug_draw.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry fragMain -o ../../../../Generated/debug_draw.frag.spv || exit /b 1
slangc debug_icon.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry iconVertMain -o ../../../../Generated/debug_icon.vert.spv || exit /b 1
slangc debug_icon.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry iconFragMain -o ../../../../Generated/debug_icon.frag.spv || exit /b 1
slangc shader.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o ../../../../Generated/slang.spv || exit /b 1
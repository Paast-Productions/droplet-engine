@echo off
cd /d "%~dp0"
mkdir ..\..\Build\Target\Editor
slangc debug_draw.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -o ..\..\Build\Target\Editor\debug_draw.vert.spv || exit /b 1
slangc debug_draw.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry fragMain -o ..\..\Build\Target\Editor\debug_draw.frag.spv || exit /b 1
slangc debug_icon.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry iconVertMain -o ..\..\Build\Target\Editor\debug_icon.vert.spv || exit /b 1
slangc debug_icon.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry iconFragMain -o ..\..\Build\Target\Editor\debug_icon.frag.spv || exit /b 1
slangc slug_test.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry slugVertMain -o ..\..\Build\Target\Editor\slug_test.vert.spv || exit /b 1
slangc slug_test.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry slugFragMain -o ..\..\Build\Target\Editor\slug_test.frag.spv || exit /b 1
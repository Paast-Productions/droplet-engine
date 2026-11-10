@echo off
cd /d "%~dp0"
mkdir ..\..\Build\Target\debug\Game
mkdir ..\..\Build\Target\release\Game

slangc debug_draw.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -o ..\..\Build\Target\debug\Game\debug_draw.vert.spv || exit /b 1
slangc debug_draw.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry fragMain -o ..\..\Build\Target\debug\Game\debug_draw.frag.spv || exit /b 1
slangc debug_icon.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry iconVertMain -o ..\..\Build\Target\debug\Game\debug_icon.vert.spv || exit /b 1
slangc debug_icon.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry iconFragMain -o ..\..\Build\Target\debug\Game\debug_icon.frag.spv || exit /b 1

slangc debug_draw.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -o ..\..\Build\Target\release\Game\debug_draw.vert.spv || exit /b 1
slangc debug_draw.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry fragMain -o ..\..\Build\Target\release\Game\debug_draw.frag.spv || exit /b 1
slangc debug_icon.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry iconVertMain -o ..\..\Build\Target\release\Game\debug_icon.vert.spv || exit /b 1
slangc debug_icon.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry iconFragMain -o ..\..\Build\Target\release\Game\debug_icon.frag.spv || exit /b 1



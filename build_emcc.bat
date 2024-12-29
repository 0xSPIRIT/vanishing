@echo off

emcc -o ./web/index.html ^
    src/main.cpp src/pl_mpeg.cpp -Wall c:/dev/raylib-5.0_webassembly/lib/libraylib.a ^
    -w ^
    -I. -I c:/dev/raylib-5.0_webassembly/include/ ^
    -L. ^
    -s USE_GLFW=3 ^
    -s USE_WEBGL2=1 ^
    -s FULL_ES3=1 ^
    -sMAX_WEBGL_VERSION=2 ^
    -s ASYNCIFY ^
    --shell-file shell_minimal.html ^
    --preload-file data ^
    -s TOTAL_STACK=256MB ^
    -s INITIAL_MEMORY=512MB ^
    -s ALLOW_MEMORY_GROWTH ^
    -s ASSERTIONS ^
    -DPLATFORM_WEB ^
    -g ^
    -DRELEASE

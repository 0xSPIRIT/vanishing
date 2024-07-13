#define _CRT_SECURE_NO_WARNINGS

#define RL_CULL_DISTANCE_NEAR 0.1
#define RL_CULL_DISTANCE_FAR  1000

#include <raylib.h>
#include <raymath.h>
#define RLIGHTS_IMPLEMENTATION
#include <rlights.h>
#define RCAMERA_IMPLEMENTATION
#include <rcamera.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define GLSL_VERSION 330

const int default_width = 1280;
const int default_height = 720;

int render_width = default_width;
int render_height = default_height;

enum Game_Mode {
    GAME_MODE_INVALID,
    GAME_MODE_INTRO,
    GAME_MODE_ATARI,
    GAME_MODE_3D
};
Game_Mode game_mode = GAME_MODE_INVALID;

Font global_font;

void set_game_mode(Game_Mode mode);

#include "text.cpp"

#include "3d.cpp"
#include "atari.cpp"
#include "intro.cpp"

Game_3D game_3d;
Game_Intro game_intro;
Game_Atari game_atari;

void toggle_fullscreen() {
    if (!IsWindowFullscreen()) {
        int monitor = GetCurrentMonitor();
        SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        ToggleFullscreen();
    } else {
        ToggleFullscreen();
        SetWindowSize(default_width, default_height);
    }
}

void set_game_mode(Game_Mode mode) {
    game_mode = mode;

    switch (mode) {
        case GAME_MODE_INTRO: game_intro_init(&game_intro); break;
        case GAME_MODE_3D:    game_3d_init(&game_3d);       break;
        case GAME_MODE_ATARI: game_atari_init(&game_atari); break;
    }
}

int main() {
    SetTraceLogLevel(LOG_ERROR);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);

    InitWindow(default_width, default_height, "Vanishing");
    SetTargetFPS(60);

    global_font = LoadFontEx("FRABK.TTF", 48, 0, 0);
    assert(IsFontReady(global_font));

    DisableCursor();

    set_game_mode(GAME_MODE_INTRO);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F11))
            toggle_fullscreen();

        switch (game_mode) {
            case GAME_MODE_INTRO: game_intro_run(&game_intro); break;
            case GAME_MODE_3D:    game_3d_run(&game_3d);       break;
            case GAME_MODE_ATARI: game_atari_run(&game_atari); break;
            case GAME_MODE_INVALID: assert(false);
        }
    }

    CloseWindow();
    return 0;
}

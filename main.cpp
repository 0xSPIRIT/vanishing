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
#include <ctype.h>
#include <math.h>
#include <time.h>

#define GLSL_VERSION 330

#ifdef RELEASE
  #define MainFunction int WinMain
  #define NDEBUG
#else
  #define MainFunction int main
#endif

const int default_width  = 192*5;
const int default_height = 144*5;

int render_width = default_width;
int render_height = default_height;

enum Game_Mode {
    GAME_MODE_INVALID,
    GAME_MODE_INTRO,
    GAME_MODE_ATARI,
    GAME_MODE_3D
};
Game_Mode game_mode = GAME_MODE_INVALID;

int chapter = 3;

Font global_font, comic_sans, bitmap_font;

enum Keyboard_Focus {
    NO_KEYBOARD_FOCUS, // we're free to move around
    KEYBOARD_FOCUS_TEXTBOX, // a textbox is currently opened.
};

void set_game_mode(Game_Mode mode);

Rectangle get_screen_rectangle() {
    Rectangle result;

    float desired_aspect_ratio = 4.f/3.f;

    int screen_width = GetRenderWidth();
    int screen_height = (int) ((float)screen_width / desired_aspect_ratio);
    if (screen_height > GetRenderHeight()) {
        screen_height = GetRenderHeight();
        screen_width = (int)(screen_height * desired_aspect_ratio);
    }

    result.x = GetRenderWidth()  / 2 - screen_width  / 2;
    result.y = GetRenderHeight() / 2 - screen_height / 2;
    result.width  = screen_width;
    result.height = screen_height;

    return result;
}

Vector2 get_mouse() {
    Vector2 result   = GetMousePosition();
    Rectangle screen = get_screen_rectangle();

    float scale_x = screen.width / (float)render_width;
    float scale_y = screen.height / (float)render_height;

    result.x -= screen.x;
    result.y -= screen.y;
    result.x /= scale_x;
    result.y /= scale_y;

    return result;
}

#include "util.cpp"

#include "keys.cpp"
#include "text.cpp"

#include "3d.cpp"

// The chapter_n.cpp files are included in atari.cpp.
#include "chapter_1.h"
#include "chapter_2.h"
#include "chapter_3.h"

#include "atari.cpp"
#include "intro.cpp"

Game_3D    game_3d;
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

MainFunction() {
    srand(time(0));
    set_global_system_timer_frequency();

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);

    InitWindow(default_width, default_height, "Video Game");
    SetTargetFPS(60);

    SetExitKey(0);

    global_font = LoadFontEx("frabk.ttf", 32, 0, 0);
    comic_sans  = LoadFontEx("comic.ttf", 16, 0, 0);

    assert(IsFontReady(global_font));

    //DisableCursor();

    set_game_mode(GAME_MODE_INTRO);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F11))
            toggle_fullscreen();

        if ((IsKeyPressed(KEY_F4) && IsKeyDown(KEY_LEFT_ALT)) || IsKeyPressed(KEY_ESCAPE)) {
            exit(0);
        }

        switch (game_mode) {
            case GAME_MODE_INTRO:   game_intro_run(&game_intro); break;
            case GAME_MODE_3D:      game_3d_run(&game_3d);       break;
            case GAME_MODE_ATARI:   game_atari_run(&game_atari); break;
            case GAME_MODE_INVALID: assert(false);
        }
    }

    CloseWindow();
    return 0;
}

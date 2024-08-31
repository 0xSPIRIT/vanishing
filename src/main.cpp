#define _CRT_SECURE_NO_WARNINGS

#define RES_DIR "./"

#define RL_CULL_DISTANCE_NEAR 0.1
#define RL_CULL_DISTANCE_FAR  1000

#include <raylib.h>
#include <rlgl.h>
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

#include <iostream>

#define GLSL_VERSION 330

#ifdef RELEASE
  #define MainFunction int WinMain
  #define NDEBUG
#else
  #define MainFunction int main
#endif

#define DIM_ATARI_WIDTH  192
#define DIM_ATARI_HEIGHT 160

#define DIM_3D_WIDTH  (320)
#define DIM_3D_HEIGHT (240)

#define PLAYER_SPEED_3D 3

#define FOV_DEFAULT 65

const int default_width  = 192*6;
const int default_height = 144*6;

int render_width = default_width;
int render_height = default_height;

enum Game_Mode {
    GAME_MODE_INVALID,
    GAME_MODE_INTRO,
    GAME_MODE_ATARI,
};
Game_Mode game_mode = GAME_MODE_INVALID;

int chapter = 5;

Font global_font, atari_font, comic_sans, italics_font, bold_font, bold_2_font, mono_font, bold_font_big;

bool toggled_fullscreen_past_second = false;
float fullscreen_timer = 0;
bool fullscreen = false;

enum Keyboard_Focus {
    NO_KEYBOARD_FOCUS,      // we're free to move around
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

    float scale_x = screen.width  / (float)render_width;
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

// The chapter_n.cpp files are included in game.cpp.
#include "chapter_1.h"
#include "chapter_2.h"
#include "chapter_3.h"
#include "chapter_4.h"

#include "game.cpp"
#include "intro.cpp"

Game_Intro game_intro;
Game       game_atari;

void toggle_fullscreen() {
    toggled_fullscreen_past_second = true;

    fullscreen_timer = 0.25;
    fullscreen = !fullscreen;

    ToggleBorderlessWindowed();
}

void set_game_mode(Game_Mode mode) {
    game_mode = mode;

    switch (mode) {
        case GAME_MODE_INTRO: game_intro_init(&game_intro); break;
        case GAME_MODE_ATARI: game_atari_init(&game_atari); break;
    }
}

MainFunction() {
    srand(time(0));
    set_global_system_timer_frequency();

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

    InitWindow(default_width, default_height, "Video Game");
    SetTargetFPS(60);

    if (fullscreen)
        toggle_fullscreen();

    SetExitKey(0);

    atari_font    = LoadFont(RES_DIR "art/romulus.png");
    global_font   = LoadFontEx(RES_DIR "art/frabk.ttf", 32, 0, 0);
    comic_sans    = LoadFontEx(RES_DIR "art/comic.ttf", 16, 0, 0);
    italics_font  = LoadFontEx(RES_DIR "art/cambriaz.ttf", 16, 0, 0);
    bold_font     = LoadFontEx(RES_DIR "art/cambriab.ttf", 16, 0, 0);
    bold_font_big = LoadFontEx(RES_DIR "art/cambriab.ttf", 32, 0, 0);
    bold_2_font   = LoadFontEx(RES_DIR "art/BOOKOSB.TTF", 32, 0, 0);
    mono_font     = LoadFontEx(RES_DIR "art/cour.ttf", 8, 0, 0);

    //DisableCursor();

    set_game_mode(GAME_MODE_ATARI);

    while (!WindowShouldClose()) {
        fullscreen_timer -= GetFrameTime();

        if (fullscreen_timer <= 0)
            toggled_fullscreen_past_second = false;

        if (IsKeyPressed(KEY_F11))
            toggle_fullscreen();

        if (IsKeyPressed(KEY_F10)) {
            Image image   = {};

            image.data    = rlReadScreenPixels(GetRenderWidth(), GetRenderHeight());
            image.width   = GetRenderWidth();
            image.height  = GetRenderHeight();
            image.mipmaps = 1;
            image.format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

            ExportImage(image, "screenshots/screenshot.png");
        }

        if (fullscreen && !IsWindowFocused()) {
            MinimizeWindow();
        }

        if ((IsKeyPressed(KEY_F4) && IsKeyDown(KEY_LEFT_ALT)) || IsKeyPressed(KEY_ESCAPE)) {
            exit(0);
        }

        switch (game_mode) {
            case GAME_MODE_INTRO:   game_intro_run(&game_intro); break;
            case GAME_MODE_ATARI:   game_atari_run(&game_atari); break;
            case GAME_MODE_INVALID: assert(false);
        }
    }

    CloseWindow();
    return 0;
}
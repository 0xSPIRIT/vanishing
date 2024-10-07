#define _CRT_SECURE_NO_WARNINGS

#ifdef RELEASE
  #define RES_DIR "data/"
#else
  #define RES_DIR "./"
#endif

#define RL_CULL_DISTANCE_NEAR 0.1
#define RL_CULL_DISTANCE_FAR  1000

#include <raylib.h>

#define GLSL_VERSION 330
#include <glad/glad.h>

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

#ifdef RELEASE
  #define MainFunction int WinMain
  #define NDEBUG
#else
  #define MainFunction int main
  #define DEBUG
#endif

#define DIM_ATARI_WIDTH  192
#define DIM_ATARI_HEIGHT 160

#define DIM_3D_WIDTH  (320)
#define DIM_3D_HEIGHT (240)

#define PLAYER_SPEED_3D 3

#define FOV_DEFAULT 65

const int default_width  = 192*5;
const int default_height = 144*5;

int render_width = default_width;
int render_height = default_height;

enum Game_Mode {
    GAME_MODE_INVALID,
    GAME_MODE_TITLESCREEN,
    GAME_MODE_INTRO,
    GAME_MODE_ATARI,
};
Game_Mode game_mode = GAME_MODE_INVALID;

int chapter = 1;

Font global_font, atari_font, comic_sans, italics_font, bold_font, bold_2_font, mono_font, bold_font_big, atari_small_font, titlescreen_font, titlescreen_minor_font;

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

    // When alt-tabbing in fullscreen this becomes 0.
    // If we don't do this we get a division by 0 somewhere
    // and the camera position and target turns into a bunch
    // of nans.
    if (screen_width == 0 || screen_height == 0) {
        screen_width = default_width;
        screen_height = default_height;
    }

    result.x = GetRenderWidth()  / 2 - screen_width  / 2;
    result.y = GetRenderHeight() / 2 - screen_height / 2;
    result.width  = screen_width;
    result.height = screen_height;

    return result;
}

#include "util.cpp"

#include "keys.cpp"
#include "text.cpp"

#include "post_processing.cpp"

// The chapter_n.cpp files are included in game.cpp.
#include "chapter_1.h"
#include "chapter_2.h"
#include "chapter_3.h"
#include "chapter_4.h"

#include "game.cpp"
#include "intro.cpp"

Game_Intro  game_intro;
Game        game_atari;

#include "titlescreen.cpp"
Titlescreen game_titlescreen;

void toggle_fullscreen() {
    toggled_fullscreen_past_second = true;

    fullscreen_timer = 0.25;
    fullscreen = !fullscreen;

    ToggleBorderlessWindowed();
}

void set_game_mode(Game_Mode mode) {
    game_mode = mode;
}

void initialize_game_mode(Game_Mode mode) {
    switch (mode) {
        case GAME_MODE_INTRO: game_intro_init(&game_intro); break;
        case GAME_MODE_ATARI: game_init(&game_atari); break;
        case GAME_MODE_TITLESCREEN: titlescreen_init(&game_titlescreen); break;
    }
}

MainFunction() {
    if (__argc == 2) {
        int chapter_start = atoi(__argv[1]);
        if (chapter_start < 1 || chapter_start > 7)
            exit(1);

        chapter = chapter_start;
    }

    set_global_system_timer_frequency();
    srand(time(0));

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

    // TODO: Make this work with monitor refresh rate.
    // I think vsync does that automatically but then I
    // get really uneven frame times, so character movement
    // seems really jerky.
    SetTargetFPS(60);

    InitWindow(default_width, default_height, "Video Game");

    gladLoadGL();

    if (fullscreen)
        toggle_fullscreen();

    SetExitKey(0);

    atari_font    = LoadFont  (RES_DIR "art/romulus.png");
    atari_small_font = LoadFontEx(RES_DIR "art/cambriaz.ttf",  14, 0, 0);
    global_font   = LoadFontEx(RES_DIR "art/frabk.ttf",    32, 0, 0);
    comic_sans    = LoadFontEx(RES_DIR "art/comic.ttf",    16, 0, 0);
    italics_font  = LoadFontEx(RES_DIR "art/cambriaz.ttf", 16, 0, 0);
    bold_font     = LoadFontEx(RES_DIR "art/cambriab.ttf", 16, 0, 0);
    bold_font_big = LoadFontEx(RES_DIR "art/cambriab.ttf", 32, 0, 0);
    bold_2_font   = LoadFontEx(RES_DIR "art/BOOKOSB.TTF",  32, 0, 0);
    mono_font     = LoadFontEx(RES_DIR "art/cour.ttf",      8, 0, 0);
    titlescreen_font = LoadFontEx(RES_DIR "art/cambriaz.ttf", 48, 0, 0);
    titlescreen_minor_font = LoadFontEx(RES_DIR "art/cambriaz.ttf", 24, 0, 0);

    //DisableCursor();

    set_game_mode(GAME_MODE_TITLESCREEN);
    initialize_game_mode(game_mode);

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

        if ((IsKeyPressed(KEY_F4) && IsKeyDown(KEY_LEFT_ALT))) {
            exit(0);
        }

        Game_Mode previous_game_mode = game_mode;

        switch (game_mode) {
            case GAME_MODE_TITLESCREEN: {
                titlescreen_update_and_draw(&game_titlescreen);
            } break;
            case GAME_MODE_INTRO: {
                game_intro_run(&game_intro);
            } break;
            case GAME_MODE_ATARI: {
                game_atari_run(&game_atari);
            } break;
            case GAME_MODE_INVALID: assert(false);
        }

        if (previous_game_mode != game_mode)
            initialize_game_mode(game_mode);
    }

    CloseWindow();
    return 0;
}

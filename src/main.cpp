#define _CRT_SECURE_NO_WARNINGS

#ifdef RELEASE
  #define RES_DIR "data/"
#else
  #define RES_DIR "./"
#endif

// There was a problem with upgrading to raylib v5.5
// where they broke the model loader. So in case they
// fix that, I'm leaving the updated function names
// here (they changed some of 'em)

#define IsTextureValid       IsTextureReady
#define IsModelValid         IsModelReady
#define IsFontValid          IsFontReady
#define IsSoundValid         IsSoundReady
#define IsRenderTextureValid IsRenderTextureReady
#define IsShaderValid        IsShaderReady
#define IsMusicValid         IsMusicReady

#define RL_CULL_DISTANCE_NEAR 0.1
#define RL_CULL_DISTANCE_FAR  1000

#include <raylib.h>

#define GLSL_VERSION 330

#include <rlgl.h>
#include <raymath.h>

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

// For loading our movies
#include "pl_mpeg.h"

#ifdef RELEASE
  #define MainFunction int WinMain
  #define NDEBUG // remove asserts
#else
  #define MainFunction int main
  #define DEBUG
#endif

#define DIM_ATARI_WIDTH  192
#define DIM_ATARI_HEIGHT 160

#define DIM_3D_WIDTH     320
#define DIM_3D_HEIGHT    240

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

int chapter = 7;

Font global_font,
     atari_font,
     comic_sans,
     italics_font,
     bold_font,
     bold_2_font,
     bold_font_big,
     atari_small_font,
     titlescreen_font,
     titlescreen_minor_font,
     dos_font;

bool toggled_fullscreen_past_second = false;
float fullscreen_timer = 0;
bool fullscreen = false;

bool automatically_init_game_mode = true;

enum Keyboard_Focus {
    NO_KEYBOARD_FOCUS,      // we're free to move around
    KEYBOARD_FOCUS_TEXTBOX, // a textbox is currently opened.
};

void set_game_mode(Game_Mode mode);

void initialize_game_mode(Game_Mode mode);

#include "util.cpp"
#include "keys.cpp"
#include "audio.cpp"
#include "text.cpp"
#include "post_processing.cpp"

#include "chapter_1.h"
#include "chapter_2.h"
#include "chapter_3.h"
#include "chapter_4.h"

#include "movie.h"

// The chapter_n.cpp files are included in game.cpp.
#include "game.h"
#include "game.cpp"
#include "intro.cpp"

Game_Intro  game_intro;
Game        game_atari;

#include "movie.cpp"

#include "titlescreen.cpp"
Titlescreen game_titlescreen;

void toggle_fullscreen() {
    toggled_fullscreen_past_second = true;

    fullscreen_timer = 0.25;
    fullscreen = !fullscreen;

    ToggleBorderlessWindowed();
    //ToggleFullscreen();
}

void set_game_mode(Game_Mode mode) {
    game_mode = mode;
}

void initialize_game_mode(Game_Mode mode) {
    switch (mode) {
        case GAME_MODE_INTRO:       game_intro_init(&game_intro);        break;
        case GAME_MODE_ATARI:       game_init(&game_atari);              break;
        case GAME_MODE_TITLESCREEN: titlescreen_init(&game_titlescreen); break;
    }
}

MainFunction() {
    bool show_titlescreen = false;

    if (__argc == 2) {
        show_titlescreen = false;

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

    InitAudioDevice();
    //SetMasterVolume(0);

    InitWindow(default_width, default_height, "Veil");

    //gladLoadGL();

    if (fullscreen)
        toggle_fullscreen();

    SetExitKey(0);

    atari_font             = load_font("art/romulus.png");
    atari_small_font       = load_font("art/cambriaz.ttf", 14);
    global_font            = load_font("art/frabk.ttf",    32);
    comic_sans             = load_font("art/comic.ttf",    16);
    italics_font           = load_font("art/cambriaz.ttf", 16);
    bold_font              = load_font("art/cambriab.ttf", 16);
    bold_font_big          = load_font("art/cambriab.ttf", 32);
    bold_2_font            = load_font("art/BOOKOSB.TTF",  32);
    titlescreen_font       = load_font("art/cambriaz.ttf", 48);
    titlescreen_minor_font = load_font("art/cambriaz.ttf", 24);
    dos_font               = load_font("art/dos.ttf",      11);

    game_audio_init();

    DisableCursor();

    if (show_titlescreen)
        set_game_mode(GAME_MODE_TITLESCREEN);
    else
        set_game_mode(GAME_MODE_ATARI);

    initialize_game_mode(game_mode);

    while (!WindowShouldClose()) {
        fullscreen_timer -= GetFrameTime();

        if (fullscreen_timer <= 0)
            toggled_fullscreen_past_second = false;

        if (IsKeyPressed(KEY_F11))
            toggle_fullscreen();

        if (IsKeyPressed(KEY_F10)) {
            Image image   = {};

            image.width   = GetRenderWidth();
            image.height  = GetRenderHeight();
            image.data    = rlReadScreenPixels(image.width, image.height);
            image.format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
            image.mipmaps = 1;

            int i = 0;

            while (true) {
                const char *file = TextFormat("screenshots/screenshot%d.png", i++);

                if (!FileExists(file)) {
                    ExportImage(image, file);
                    break;
                }
            }
        }

        if (fullscreen && !IsWindowFocused()) {
            MinimizeWindow();
        }

        if ((IsKeyPressed(KEY_F4) && IsKeyDown(KEY_LEFT_ALT))) {
            exit(0);
        }

        game_audio_update(GetFrameTime());

        Game_Mode previous_game_mode = game_mode;

        switch (game_mode) {
            case GAME_MODE_TITLESCREEN: {
                titlescreen_update_and_draw(&game_titlescreen);
            } break;
            case GAME_MODE_INTRO: {
                game_intro_run(&game_intro);
            } break;
            case GAME_MODE_ATARI: {
                game_run(&game_atari);
            } break;
            case GAME_MODE_INVALID: assert(false);
        }

        if (automatically_init_game_mode && previous_game_mode != game_mode)
            initialize_game_mode(game_mode);

        automatically_init_game_mode = true;
    }

    CloseWindow();
    return 0;
}

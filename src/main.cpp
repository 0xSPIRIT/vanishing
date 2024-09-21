#define _CRT_SECURE_NO_WARNINGS

#define RES_DIR "./"

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

#include <iostream>

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

#define MAX_PARTICLES       1000

// Particle type
typedef struct Particle {
    float x;
    float y;
    float period;
} Particle;

GLuint vao, vbo;
Shader shader_;

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
bool epilogue_text_change_color = false; // please god forgive me

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
    set_global_system_timer_frequency();
    srand(time(0));

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

    InitWindow(default_width, default_height, "Video Game");

    gladLoadGL();

    shader_ = LoadShader(RES_DIR "shaders/point_particle.vs", RES_DIR "shaders/point_particle.fs");

    // Initialize the vertex buffer for the particles and assign each particle random values
    Particle particles[MAX_PARTICLES] = { 0 };

    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        particles[i].x = (float)GetRandomValue(20, render_width - 20);
        particles[i].y = (float)GetRandomValue(50, render_height - 20);
        
        // Give each particle a slightly different period. But don't spread it to much. 
        // This way the particles line up every so often and you get a glimps of what is going on.
        particles[i].period = (float)GetRandomValue(10, 30)/10.0f;
    }

    // Create a plain OpenGL vertex buffer with the data and an vertex array object 
    // that feeds the data from the buffer into the vertexPosition shader attribute.
    vao = 0;
    vbo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES*sizeof(Particle), particles, GL_STATIC_DRAW);
        // Note: LoadShader() automatically fetches the attribute index of "vertexPosition" and saves it in shader.locs[SHADER_LOC_VERTEX_POSITION]
        glVertexAttribPointer(shader_.locs[SHADER_LOC_VERTEX_POSITION], 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Allows the vertex shader to set the point size of each particle individually
    #ifndef GRAPHICS_API_OPENGL_ES2
    glEnable(GL_PROGRAM_POINT_SIZE);
    #endif
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

    set_game_mode(GAME_MODE_INTRO);

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

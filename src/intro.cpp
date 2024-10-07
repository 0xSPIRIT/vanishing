struct Game_Intro {
    RenderTexture2D render_target;
    const char *message;

    float fader; // 0.0 is 0 alpha, 1.0 is 255 alpha, 2.0 is 0 alpha.
};

void game_intro_init(Game_Intro *game) {
    render_width = DIM_3D_WIDTH;
    render_height = DIM_3D_HEIGHT;

    game->render_target = LoadRenderTexture(render_width, render_height);

    game->fader = 0;

    switch (chapter) {
        case 1: {
            game->message = "\"DYSANIA\"";
        } break;
        case 2: {
            game->message = "\"DINNER PARTY\"";
        } break;
        case 3: {
            game->message = "\"REPETITIONS\"";
        } break;
        case 4: {
            game->message = "\"THEOPHANY\"";
        } break;
        case 5: {
            game->message = "\"PERCEPTION\"";
        } break;
        case 6: {
            game->message = "\"PATIENCE &\n\nBLASPHEMY\"";
        } break;
        case 7: {
            game->message = "\"EPILOGUE\"";
        } break;
        case 8: {
            game->message = "Created by spritwolf\n\nThanks for playing.";
        } break;
    }
}

void game_intro_run(Game_Intro *game) {
    BeginDrawing();

    ClearBackground(BLACK);

    BeginTextureMode(game->render_target);

    if (chapter == 7) {
        ClearBackground({255,0,0,255});
    } else {
        ClearBackground(BLACK);
    }

    int spacing = 2;

    float dt = GetFrameTime();

    game->fader += dt * 0.2f;

    float x = game->fader;
    float fade = 0;

    if (x < 1) {
        fade = 3*x*x - 2*x*x*x;
    } else if (x <= 2) {
        fade = (2-x) * (2-x) * (2*x-1);
    }

    uint8_t alpha = 255 * fade;

    Font *font = &bold_font_big;

    Vector2 size = MeasureTextEx(*font, game->message, font->baseSize, spacing);

    Color color = WHITE;
    if (chapter == 7)
        color = BLACK;
    color.a = alpha;

    DrawTextEx(*font,
               game->message,
               { render_width/2 - size.x/2, render_height/2 - size.y/2 },
               font->baseSize,
               spacing,
               color);

    bool debug = false;

#ifdef DEBUG
    debug = true;
#endif

    if (x >= 2 || (debug && is_action_pressed())) {
        if (chapter == 8)
            exit(0);
        else
            set_game_mode(GAME_MODE_ATARI);
    }

    EndTextureMode();

    Rectangle destination = get_screen_rectangle();

    DrawTexturePro(game->render_target.texture,
                   {0, 0, (float)render_width, -(float)render_height},
                   destination,
                   {0, 0},
                   0,
                   WHITE);

    EndDrawing();
}

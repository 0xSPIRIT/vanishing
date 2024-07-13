struct Player {
    Vector2 pos;
    Texture2D texture;
};

struct Game_Atari {
    RenderTexture2D render_target;
    Texture2D background;

    Player player;

    Text_List text, *current;
};

Font atari_font;

Rectangle atari_get_screen_rectangle() {
    Rectangle result;

    float desired_aspect_ratio = 2.5f;

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

void atari_text_list_init(Text_List *list, char *speaker, char *line,
                          Text_List *next)
{
    list->font         = &atari_font;
    list->font_spacing = 1;
    list->scale        = 0.125;

    text_list_init(list, speaker, line, next);
}

void game_atari_init(Game_Atari *game) {
    render_width = 192;
    render_height = 160;
    
    game->background = LoadTexture("art/background.png");
    assert(IsTextureReady(game->background));

    game->player.texture = LoadTexture("art/player.png");
    assert(IsTextureReady(game->player.texture));

    atari_font = LoadFont("pixantiqua.png");

    game->render_target = LoadRenderTexture(render_width, render_height);

    game->player.pos.x = render_width/2  - game->player.texture.width/2;
    game->player.pos.y = render_height/2 - game->player.texture.height/2;

    atari_text_list_init(&game->text, "Ameer", "This is a test\rAnd another line.", nullptr);

    game->current = &game->text;
}

void game_atari_run(Game_Atari *game) {
    float dt = GetFrameTime();

    float player_speed = 30;

    // Tick
    if (IsKeyDown(KEY_UP)) {
        game->player.pos.y -= player_speed * dt;
    }
    if (IsKeyDown(KEY_LEFT)) {
        game->player.pos.x -= player_speed * dt;
    }
    if (IsKeyDown(KEY_DOWN)) {
        game->player.pos.y += player_speed * dt;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        game->player.pos.x += player_speed * dt;
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        set_game_mode(GAME_MODE_3D);
    }

    BeginDrawing();

    ClearBackground(BLACK);

    BeginTextureMode(game->render_target);
    {
        ClearBackground(BLACK);
        DrawTexture(game->background, 0, 0, WHITE);

        DrawTexture(game->player.texture, (int)game->player.pos.x, (int)game->player.pos.y, WHITE);

        if (game->current)
            game->current = text_list_update_and_draw(game->current);
    }
    EndTextureMode();

    Rectangle destination = atari_get_screen_rectangle();

    DrawTexturePro(game->render_target.texture,
                   {0, 0, (float)render_width, -(float)render_height},
                   destination,
                   {0, 0},
                   0,
                   WHITE);

    EndDrawing();
}

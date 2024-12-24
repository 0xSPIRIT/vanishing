struct Level_Chapter_Intro {
    // We prefer to use this instead of the Entity struct
    // since we know our use case doesn't need it. Simpler.
    Vector2 entities[16];
    int     entity_count;

    Model    scene;

    Camera3D camera;
};

void intro_text(Text_List *list,
                char *speaker,
                char *message,
                float speed,
                Text_List *next)
{
    list->font = &atari_font;
    list->font_spacing = 1;
    list->scale = 0.125;
    list->scroll_speed = speed;

    setup_text_scroll_sound(list, speaker);

    if (speaker) {
        if (list->scroll_sound == SOUND_TEXT_SCROLL_FEMALE) {
            list->color = PINK;
        }

        if (list->scroll_sound == SOUND_TEXT_SCROLL_MALE) {
            list->color = BLUE;
        }
    }

    list->bg_color = {0, 0, 0, 127};

    text_list_init(list, speaker, message, next);
}

void chapter_intro_init(Game *game) {
    Level_Chapter_Intro *level = (Level_Chapter_Intro *)game->level;

    Texture2D *textures = atari_assets.textures;

    level->scene = load_model("models/bedroom.glb");

    textures[0] = load_texture("art/guy.png");
    textures[1] = load_texture("art/girl.png");

    //level->entities[0] = { render_width / 4.f, render_height / 2.f };
    level->entity_count = 10;

    game->textbox_alpha = 220;

    intro_text(&game->text[0],
               0,
               "Have you seen Chase?\rThe long-haired boy?\rI heard he's been bad.",
               30,
               &game->text[1]);
    game->current = &game->text[0];

    intro_text(&game->text[1],
               "Eleanor",
               "Chase, my precious,\rstop running from us!\rWe're only trying to help!",
               30,
               &game->text[2]);

    intro_text(&game->text[2],
               "Trey",
               "C'mon Chase...\rDon't just stay in your\nroom...",
               30,
               &game->text[3]);

    intro_text(&game->text[3],
               "Judas",
               "We're watching you.",
               30,
               &game->text[4]);

    intro_text(&game->text[4],
               "Aria",
               "Open the door, Chase.",
               30,
               &game->text[5]);

    intro_text(&game->text[5],
               "Lucas",
               "We're watching you.",
               30,
               &game->text[6]);

    intro_text(&game->text[6],
               "Siphor",
               "You're going to have to\ncome outside eventually.",
               30,
               &game->text[7]);

    intro_text(&game->text[7],
               "Tyrell",
               "Look at him,\nhe hasn't been eating\nanything.",
               30,
               &game->text[8]);

    intro_text(&game->text[8],
               "Noah",
               "We're watching you.",
               30,
               &game->text[9]);

    intro_text(&game->text[9],
               "Olivia",
               "Look at him, he's such a\nloner.",
               30,
               &game->text[10]);

    intro_text(&game->text[10],
               "Jason",
               "Cut the games.\rOpen the door, Chase.",
               30,
               &game->text[11]);

    intro_text(&game->text[11],
               "Jessica",
               "CHASE.\rOPEN THE DOOR.",
               30,
               nullptr);

    auto callback = [](void *game_ptr) -> void {
        Game *game = (Game *)game_ptr;
        start_fade(game, FADE_OUT, 120, atari_queue_deinit_and_goto_intro);
    };

    game->text[11].callbacks[0] = callback;

    level->camera.fovy = FOV_DEFAULT;
    level->camera.up = { 0, 1, 0 };
    level->camera.projection = CAMERA_PERSPECTIVE;
    level->camera.position = {2.88f, 1.f, -1.5f};
    level->camera.target = {1.88f, 1, -1.5f};

    game->post_processing.type = POST_PROCESSING_VHS;
    Post_Processing_Vhs *vhs = &game->post_processing.vhs;

    post_process_vhs_set_intensity(vhs, VHS_INTENSITY_MEDIUM);
    vhs->vignette_mix = 0.5f;
    vhs->noise_intensity = 0.5f;
}

void chapter_intro_update(Game *game, float dt) {
    (void)dt;
    Level_Chapter_Intro *level = (Level_Chapter_Intro *)game->level;

    Texture2D *textures = atari_assets.textures;

    float freq = 2;
    level->camera.target.z = -1.5f + cos(freq * 3.f*GetTime()) * 0.125f;
    level->camera.target.y = 1.f + sin(freq * 1.2*GetTime()) * 0.125f;

    for (int i = 0; i < level->entity_count; i++) {
        if (i == 0) {
            level->entities[i].x = render_width/2.f - textures[0].width/2;
            level->entities[i].y = render_height/2.f - textures[0].height/2;
        } else {
            float amp = 55;
            level->entities[i].x = render_width/2.f  + amp * sin(GetTime() + 2 * PI * (float)i / level->entity_count) - textures[0].width/2;
            level->entities[i].y = render_height/2.f + amp * cos(GetTime() + 2 * PI * (float)i / level->entity_count) - textures[0].height/2;
        }
    }
}

void chapter_intro_draw(Game *game) {
    Level_Chapter_Intro *level = (Level_Chapter_Intro *)game->level;

    ClearBackground({10,10,10,255});

    if (sin(GetTime() * 2 * PI) > 10) {
        BeginMode3D(level->camera);
        DrawModel(level->scene, {}, 1, WHITE);
        EndMode3D();
    }

    Texture2D *textures = atari_assets.textures;

    for (int i = 0; i < level->entity_count; i++) {
        Texture t = (i % 2) ? textures[0] : textures[1];

        Vector2 pos = level->entities[i];
        pos.x = (int)pos.x;
        pos.y = (int)pos.y;

        DrawTextureV(t, pos, WHITE);

        int look = round(triangle_wave(GetTime()));
        if (i == 0) look = 0;
        DrawPixel(pos.x + 6 + look, pos.y + 9, BLACK);
    }
}

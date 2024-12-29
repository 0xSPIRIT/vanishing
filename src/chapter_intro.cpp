#define INTRO_ENTITY_COUNT_MAX 17

struct Intro_Guy {
    float visible_fader; // 0 == invisible, 1 == visible
};

struct Level_Chapter_Intro {
    // We prefer to use this instead of the Entity struct
    // since we know our use case doesn't need it. Simpler.

    Intro_Guy visible_indices[INTRO_ENTITY_COUNT_MAX];
    int       visible_counter;

    int       current_text_index;

    bool      view_2d;
    bool      view_3d;

    float     camera_height;

    Model     scene;

    Camera3D  camera;

    float     camera_t;
    Vector3   camera_door_pos;
    Vector3   camera_door_target;
    Vector3   camera_stored_pos, camera_stored_target;
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

        if (strcmp(speaker, "???") == 0) {
            Color pink = {255, 133, 220, 255};//232, 81, 193, 255};
            list->color = pink;
        }
    }

    list->bg_color = {0, 0, 0, 127};

    text_list_init(list, speaker, message, next);
}

void chapter_intro_init(Game *game) {
    Level_Chapter_Intro *level = (Level_Chapter_Intro *)game->level;

    Texture2D *textures = atari_assets.textures;

    play_music(MUSIC_DESERT_AMBIENCE);

    level->scene = load_model("models/hallway.glb");

    level->view_2d = true;
    level->view_3d = false;

    textures[0] = load_texture("art/guy.png");
    textures[1] = load_texture("art/girl.png");
    textures[2] = load_texture("art/girl_mystery.png");
    textures[3] = load_texture("art/veil.png");

    //level->entities[0] = { render_width / 4.f, render_height / 2.f };
    level->visible_indices[0].visible_fader = 1;

    game->textbox_alpha = 160;

    auto add_guy = [](void *game_ptr) -> void {
        Game *game = (Game *)game_ptr;

        auto add = [](Game *game) -> void {
            Level_Chapter_Intro *level = (Level_Chapter_Intro *)game->level;

            level->visible_counter += INTRO_ENTITY_COUNT_MAX/4;

            int max = 0;
            while (level->visible_indices[level->visible_counter % INTRO_ENTITY_COUNT_MAX].visible_fader != 0) {
                level->visible_counter++;
                max++;
                if (max >= INTRO_ENTITY_COUNT_MAX)
                    break;
            }

            level->visible_counter %= INTRO_ENTITY_COUNT_MAX;
            level->visible_indices[level->visible_counter].visible_fader += 0.01f;
        };

        Level_Chapter_Intro *level = (Level_Chapter_Intro *)game->level;

        add(game);
        if (level->current_text_index >= 8 && level->visible_counter < INTRO_ENTITY_COUNT_MAX)
            add_event(game, add, 1);
        if (level->current_text_index >= 8 && level->visible_counter < INTRO_ENTITY_COUNT_MAX)
            add_event(game, add, 1.5);
        if (level->current_text_index >= 8 && level->visible_counter < INTRO_ENTITY_COUNT_MAX)
            add_event(game, add, 2);

        auto next_text = [](Game *game) -> void {
            Level_Chapter_Intro *level = (Level_Chapter_Intro *)game->level;
            game->current = &game->text[++level->current_text_index];
        };

        add_event(game, next_text, 3);
    };
    
    // This whole thing is a depressive episode.
    // "You're going to have to come outside
    // eventually" -- DREAD.
    // "He's been bad" -- constant self-criticism.
    //
    // Here, the faceless girl represents meaning.
    // She remains faceless because he forgets
    // what it feels like.
    //
    // We metaphorize his relationship with meaning
    // as an ex-girlfriend type situation, where
    // they used to be together.
    //
    // Also, the faceless girl can represent Chase's
    // desire for partnership. She's faceless because
    // he can't predict the future-- who can it be?
    // This sets up Eleanor's phone call with Chase
    // near the end of Chapter 1.
    //
    // They want him to open the door and face his
    // problems, they're just being really negative,
    // and offensive because this is at the end of
    // the day a depressive episode, as if to spite
    // him.

    // ???: Have you seen Chase?  - She's trying to look for him!
    //      I heard he's been bad - He took her for granted, so they broke up.
    //      you bled me dry and never noticed - strain in the relationship

    // This intro shows how it seems that everyone
    // else has found meaning and fulfillment (symbolized
    // as a relationship, but Chase doesn't. 

    // Did he break up with her or did she break up with him?
    // It doesn't matter. What matters is they're separated.

    intro_text(&game->text[0],
               "???",
               "Have you seen Chase?\rThe long-haired boy?\rHe's been bad.",
               30,
               &game->text[50]);
    intro_text(&game->text[50],
               "???",
               "Chase, my precious,\rstop running from me!",
               30,
               &game->text[51]);
    intro_text(&game->text[51],
               "???",
               "You know you need me.\rBut you bled me dry and\nnever noticed.",
               30,
               nullptr);

    game->text[51].callbacks[0] = add_guy;

    auto start_text = [](Game *game) -> void {
        game->current = &game->text[0];
    };
    add_event(game, start_text, 2);

    intro_text(&game->text[1],
               "Eleanor",
               "Get that look off your\nface, Chase.\rWhy can't you be normal?",
               30,
               nullptr);

    intro_text(&game->text[2],
               "Ana",
               "Look at how pathetic he\nis.\rYou agree, don't you?",
               30,
               nullptr);

    intro_text(&game->text[3],
               "Joanne",
               "You want her back.\rI know.\rBut she remains faceless.",
               30,
               nullptr);

    intro_text(&game->text[4],
               "Aria",
               "Open the door, Chase.\rWe know you can hear\nus.",
               30,
               nullptr);

    intro_text(&game->text[5],
               "Tyrell",
               "You want to be one of\nus again.\rYour fault you lost her.",
               30,
               nullptr);

    intro_text(&game->text[6],
               "Siphor",
               "You're going to have to\ncome outside eventually.",
               30,
               nullptr);

    intro_text(&game->text[7],
               "Matt",
               "Look at him, he hasn't been\neating anything.",
               30,
               nullptr);

    intro_text(&game->text[8],
               "Noah",
               "We're watching you.\rSad, pathetic boy.\rYou'll never be like us.",
               30,
               nullptr);

    intro_text(&game->text[9],
               "Olivia",
               "Look at her, Chase.\rDon't you like her pretty\ndress?",
               30,
               nullptr);

    intro_text(&game->text[10],
               "Jessica",
               "CHASE.\rOPEN THE DOOR.",
               30,
               nullptr);

    for (int i = 1; i <= 9; i++) {
        game->text[i].callbacks[0] = add_guy;
    }

    auto callback = [](void *game_ptr) -> void {
        Game *game = (Game *)game_ptr;

        auto view_3d = [](Game *game) -> void {
            Level_Chapter_Intro *level = (Level_Chapter_Intro *)game->level;
            level->view_3d = true;
        };
        auto stop_2d = [](Game *game) -> void {
            Level_Chapter_Intro *level = (Level_Chapter_Intro *)game->level;
            level->view_2d = false;
        };

        add_event(game, view_3d, 2);
        add_event(game, stop_2d, 3.5);
        //start_fade(game, FADE_OUT, 60, atari_queue_deinit_and_goto_intro);
    };

    game->text[10].callbacks[0] = callback;

    {
        auto investigate_noise = [](void *game_ptr) -> void {
            Game *game = (Game *)game_ptr;
            Level_Chapter_Intro *level = (Level_Chapter_Intro *)game->level;

            level->camera_stored_pos = level->camera.position;
            level->camera_stored_target = level->camera.target;
            level->camera_t += 0.001f;
        };

        String choices[] = {const_string("Yes"), const_string("No")};
        Text_List *next[] = { 0, 0 };
        void (*function_pointers[])(void*) = { investigate_noise, nullptr };

        game->text[11].choice_color = WHITE;
        game->text[11].choice_backdrop_color = {};
        game->text[11].arrow_color = WHITE;

        atari_choice_text_list_init(&game->text[11],
                                    0,
                                    "Investigate the noise?",
                                    choices,
                                    next,
                                    function_pointers,
                                    2);
    }
    
    level->camera_height = 1.5f;

    level->camera_t = 0;
    level->camera_door_pos = {-1.192733f, level->camera_height, -0.927622f};
    level->camera_door_target = {-2.192733f, level->camera_height-0.15f, -0.927622f};

    level->camera.fovy = FOV_DEFAULT;
    level->camera.up = { 0, 1, 0 };
    level->camera.projection = CAMERA_PERSPECTIVE;

    level->camera.position = {-0.258068f, 1.500000f, 0.051428f};
    level->camera.target = {0.408597f, 1.253672f, -0.652049f};

    //level->camera.position = {2.88f, level->camera_height, -0.5f};
    //level->camera.target = {1.88f, level->camera_height, -0.5f};

    game->post_processing.type = POST_PROCESSING_VHS;
    Post_Processing_Vhs *vhs = &game->post_processing.vhs;

    post_process_vhs_set_intensity(vhs, VHS_INTENSITY_MEDIUM);
    vhs->vignette_mix = 0.5f;
    vhs->noise_intensity = 0.5f;
}

void chapter_intro_update(Game *game, float dt) {
    (void)dt;
    Level_Chapter_Intro *level = (Level_Chapter_Intro *)game->level;

    if (IsKeyPressed(KEY_P)) {
        level->visible_counter += INTRO_ENTITY_COUNT_MAX/4;
        level->visible_counter %= INTRO_ENTITY_COUNT_MAX;
        level->visible_indices[level->visible_counter].visible_fader += 0.01f;
    }

    for (int i = 0; i < INTRO_ENTITY_COUNT_MAX; i++) {
        Intro_Guy *guy = level->visible_indices + i;

        if (guy->visible_fader != 0) {
            guy->visible_fader += dt;
            if (guy->visible_fader > 1)
                guy->visible_fader = 1;
        }
    }

    if (level->view_3d && !level->view_2d) {
        if (level->camera_t > 0) {
            level->camera_t += 0.1f * dt;
            if (level->camera_t > 1)
                level->camera_t = 1;

            level->camera.position = smoothstep_vector3(level->camera_stored_pos, level->camera_door_pos, level->camera_t);
            level->camera.target = smoothstep_vector3(level->camera_stored_target, level->camera_door_target, level->camera_t);

            if (level->camera_t == 1) {
                level->camera.fovy -= 2 * dt;

                if (level->camera.fovy < 10) {
                    level->view_3d = false;
                    game->post_processing.type = POST_PROCESSING_PASSTHROUGH;
                    add_event(game, atari_queue_deinit_and_goto_intro, 2);
                    stop_music();
                }
            }
        } else if (!game->current) {
            Vector3 stored_camera_pos = level->camera.position;
            update_camera_3d(&level->camera, 1, true, dt);

            Vector3 velocity = Vector3Subtract(level->camera.position, stored_camera_pos);
            level->camera.position = stored_camera_pos;

            apply_3d_velocity(&level->camera, level->camera_height, level->scene, velocity, false);

            if (level->camera.position.x < -0.7f) {
                level->camera.position.x = -0.7f;

                if (!game->current)
                    game->current = &game->text[11];
            }

            update_camera_look(&level->camera, dt);
        }
    }
}

void chapter_intro_draw(Game *game) {
    Level_Chapter_Intro *level = (Level_Chapter_Intro *)game->level;

    ClearBackground({10,10,10,255});

    if (level->view_3d) {
        BeginMode3D(level->camera);
        DrawModel(level->scene, {}, 1, WHITE);
        EndMode3D();

        if (level->camera.fovy < 24) {
            Texture2D *textures = atari_assets.textures;

            DrawTexture(textures[3], 0, 0, WHITE);
        }
    }

    if (level->view_2d) {
        Texture2D *textures = atari_assets.textures;

        for (int i = INTRO_ENTITY_COUNT_MAX-1; i >= 0; i--) {
            float visible = level->visible_indices[i].visible_fader;
            if (visible == 0)
                continue;

            Texture t = (i % 2) ? textures[0] : textures[1];

            Vector2 pos;

            if (i == 0) {
                t = textures[2];
                pos.x = render_width/2.f - textures[0].width/2;
                pos.y = render_height/2.f - textures[0].height/2;
            } else {
                float amp = 55;
                float n = INTRO_ENTITY_COUNT_MAX - 1;
                pos.x = render_width/2.f  + amp * sin(GetTime() + 2 * PI * i / n) - textures[0].width/2;
                pos.y = render_height/2.f + amp * cos(GetTime() + 2 * PI * i / n) - textures[0].height/2;
            }

            pos.x = (int)pos.x;
            pos.y = (int)pos.y;

            Color color = WHITE;
            color.a = visible * 255;

            BeginBlendMode(BLEND_ADDITIVE);

            DrawTextureV(t, pos, color);

            EndBlendMode();

            //int look = round(triangle_wave(0.10f * GetTime() + 0.10f * i));

            Vector2 a = {pos.x+6.f,pos.y+9.f};
            Vector2 b = {render_width/2.f, render_height/2.f};
            Vector2 delta = Vector2Subtract(b, a);

            float angle = atan2f(delta.y, delta.x);

            int octant = get_octant(angle + PI/16.);

            int look_x = 0, look_y = 0;

            switch (octant) {
                case 7: { look_x = -1; look_y =  1; } break;
                case 6: { look_x =  0; look_y =  1; } break;
                case 5: { look_x =  1; look_y =  1; } break;
                case 4: { look_x =  1; look_y =  0; } break;
                case 3: { look_x =  1; look_y = -1; } break;
                case 2: { look_x =  0; look_y = -1; } break;
                case 1: { look_x = -1; look_y = -1; } break;
                case 0: { look_x = -1; look_y =  0; } break;
            }

            if (i != 0)
                DrawPixel(pos.x + 6 + look_x, pos.y + 9 + look_y, BLACK);
        }
    }
}

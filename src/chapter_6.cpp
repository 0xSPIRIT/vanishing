enum Chapter_6_State {
    CHAPTER_6_STATE_DESERT,
    CHAPTER_6_STATE_HOME,
    CHAPTER_6_STATE_TEXT,
};

struct Level_Chapter_6 {
    Chapter_6_State state;
    Entity *player;

    float fade_alpha;
    float white_fade_alpha;
    float red_fade_time; // 0.0 to 1.0
    bool  started_text;

    float god_scroll;

    int godtext;
};

float chapter_6_red_fade_alpha(float time) {
    return time * time * time;
}

void chapter_6_goto_home(Game *game) {
    Level_Chapter_6 *level = (Level_Chapter_6 *)game->level;
    level->state = CHAPTER_6_STATE_HOME;
}

void chapter_6_begin_text(Game *game) {
    game->current = &game->text[0];
}

void chapter_6_start_godtext(Game *game) {
    Level_Chapter_6 *level = (Level_Chapter_6 *)game->level;
    level->state = CHAPTER_6_STATE_TEXT;
}

void chapter_6_queue_godtext(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    add_event(game, chapter_6_start_godtext, 2);
}

void chapter_6_text(bool scroll, bool chase, Text_List *list, char *line,
                    Text_List *next)
{
    list->font         = &atari_font;
    list->font_spacing = 1;
    list->scale        = 0.125;
    list->scroll_speed = 15;
    //list->alpha_speed  = 0.5;

    if (chase)
        list->color        = WHITE;
    else
        list->color        = RED;

    list->center_text  = true;
    if (scroll)
        list->scroll_type = LetterByLetter;
    else
        list->scroll_type  = EntireLine;
    list->render_type  = Bare;
    list->location     = Top;
    list->take_keyboard_focus = true;

    text_list_init(list, 0, line, next);
}

Entity *chapter_6_make_entity(Entity_Type type, float x, float y) {
    Entity *result = allocate_entity();

    result->type = type;
    result->pos = {x, y};

    switch (type) {
        case ENTITY_PLAYER: {
            result->texture_id = 0;
        } break;
        case ENTITY_CACTUS: {
            result->texture_id = (rand() % 2 == 0) ? 1 : 2;
        } break;
        case ENTITY_ROCK: {
            result->texture_id = 3;
        } break;
        case ENTITY_FOOTSTEPS: {
            result->texture_id = 4 + rand() % 3;
        } break;
        case ENTITY_BLOOD: {
            result->texture_id = 7 + rand() % 3;
        } break;
        case ENTITY_NODE: {
            result->chap_1_node.active = true;
            result->texture_id = 12;
        } break;
    }

    float texture_width  = entity_texture_width(result);
    float texture_height = entity_texture_height(result);

    result->base_collider = {0, 3*texture_height/4, texture_width, texture_height/4};

    return result;
}

void chapter_6_init(Game *game) {
    Level_Chapter_6 *level = (Level_Chapter_6 *)game->level;
    memset(level, 0, sizeof(Level_Chapter_6));

    render_width = DIM_ATARI_WIDTH;
    render_height = DIM_ATARI_HEIGHT;

    // TODO
    game->textbox_target = LoadRenderTexture(render_width, render_height);
    game->textbox_alpha = 255;

    game->render_state = RENDER_STATE_ATARI;

    Texture *textures = atari_assets.textures;
    textures[0]  = load_texture(RES_DIR "art/player.png");
    textures[1]  = load_texture(RES_DIR "art/cactus_1.png");
    textures[2]  = load_texture(RES_DIR "art/cactus_2.png");
    textures[3]  = load_texture(RES_DIR "art/rock.png");
    textures[4]  = load_texture(RES_DIR "art/footsteps_1.png");
    textures[5]  = load_texture(RES_DIR "art/footsteps_2.png");
    textures[6]  = load_texture(RES_DIR "art/footsteps_3.png");
    textures[7]  = load_texture(RES_DIR "art/blood_1.png");
    textures[8]  = load_texture(RES_DIR "art/blood_2.png");
    textures[9]  = load_texture(RES_DIR "art/blood_3.png");
    textures[10] = load_texture(RES_DIR "art/player_crawl.png");
    textures[11] = load_texture(RES_DIR "art/node_inactive.png");
    textures[12] = load_texture(RES_DIR "art/node_active.png");
    textures[13] = load_texture(RES_DIR "art/apartment.png");
    textures[14] = load_texture(RES_DIR "art/player_white.png");
    textures[15] = load_texture(RES_DIR "art/godtext.png");
    textures[16] = load_texture(RES_DIR "art/godtext2.png");
    textures[17] = load_texture(RES_DIR "art/godtext3.png");

    chapter_6_text(true,
                   false,
                   &game->text[0],
                   "You didn't complete the\ntask that I asked.",
                   &game->text[1]);
    chapter_6_text(true,
                   true,
                   &game->text[1],
                   "I know.\rI changed my mind.",
                   &game->text[2]);
    chapter_6_text(true,
                   false,
                   &game->text[2],
                   "No, no, no.\rYou cannot understand.\rYou must complete this,\notherwise you won't get\nwhat you want-",
                   &game->text[3]);
    chapter_6_text(true,
                   false,
                   &game->text[3],
                   "What you NEED.\rMEANING. PURPOSE.\nCONTENTMENT.",
                   &game->text[4]);
    chapter_6_text(true,
                   false,
                   &game->text[4],
                   "I AM THE ALL-POWERFUL!\rI CAN GRANT YOU\nWHATEVER AND WHOMEVER\nYOU PLEASE.",
                   &game->text[5]);
    chapter_6_text(true,
                   true,
                   &game->text[5],
                   "I said I changed my mind.",
                   &game->text[6]);
    chapter_6_text(true,
                   false,
                   &game->text[6],
                   "...\rTHE INFINITE POSSESSES\nA POWER YOU CANNOT\nBEGIN TO GRASP.\rWHAT DO YOU HAVE?\rA JOB WHICH HOLDS AS\nMUCH MEANING AS\nCOUNTING GRAINS OF SAND?",
                   &game->text[7]);
    chapter_6_text(true,
                   false,
                   &game->text[7],
                   "A HOME AS LONELY AS\nTHE SILENCE BEFORE\nCREATION?",
                   &game->text[8]);
    chapter_6_text(true,
                   false,
                   &game->text[8],
                   "WHAT WILL YOU DO\nWITHOUT MY BENEVOLENCE?",
                   &game->text[9]);
    chapter_6_text(true,
                   true,
                   &game->text[9],
                   "I'll be... patient?",
                   &game->text[10]);
    chapter_6_text(true,
                   false,
                   &game->text[10],
                   "Fool.\rI am the ALMIGHTY!\nYOU SHOULD BE BOWING\nTO MY PRESENCE.",
                   &game->text[11]);
    chapter_6_text(true,
                   false,
                   &game->text[11],
                   "YOU ARE A PUNY\nVESSEL OF FLESH\n"
                   "WHICH HAS NOT THE\nSMALLEST INCLINATION OF\nTRUE POWER.",
                   &game->text[12]);
    chapter_6_text(true,
                   false,
                   &game->text[12],
                   "I HAVE EXISTED FOR\nETERNITY AND I WILL\nBE HERE FOR ETERNITY.",
                   &game->text[13]);
    chapter_6_text(true,
                   true,
                   &game->text[13],
                   "You will be here\nforever, yes.\rAnd you may return\n"
                   "stronger than ever.\rBut I will cherish\n"
                   "every second that I\nam able to overpower\nyou.",
                   nullptr);

    game->text[13].callbacks[0] = chapter_6_queue_godtext;

    game->entities = make_array<Entity*>(16);

    level->player = chapter_6_make_entity(ENTITY_PLAYER, render_width/2 - 4, 3*render_height/4);
    array_add(&game->entities, level->player);

    Entity *node = chapter_1_make_entity(ENTITY_NODE, render_width/2 - 16, render_height/4 - 16);
    array_add(&game->entities, node);
}

void chapter_6_entity_update(Entity *entity, Game *game, float dt) {
    Level_Chapter_6 *level = (Level_Chapter_6 *)game->level;
    level, dt;

    switch (entity->type) {
        case ENTITY_PLAYER: {
            if (game->current) break;

            float speed = (float)(entity->pos.y - 60) / (float)render_height;
            speed *= 60;

            if (IsKeyDown(KEY_P))
                speed *= 10;

            int move_x = input_movement_x_axis_int(dt);//key_right() - key_left();
            int move_y = input_movement_y_axis_int(dt);//key_down()  - key_up();

            Vector2 velocity = { speed * dt * move_x, speed * dt * move_y };

            apply_velocity(entity, velocity, &game->entities);

            if (Vector2Distance(entity->pos, entity->chap_1_player.stored_pos) >= 12 + rand_range(-7, 0)) {
                entity->chap_1_player.stored_pos = entity->pos;

                Entity *footstep = chapter_6_make_entity(ENTITY_BLOOD,
                                                         entity->pos.x,
                                                         entity->pos.y + entity_texture_height(entity) - 3);
                array_add(&game->entities, footstep);
            }

            if (entity->pos.x >= render_width)
                entity->pos.x -= render_width + entity_texture_width(entity);
            if (entity->pos.x < -entity_texture_width(entity))
                entity->pos.x += render_width + entity_texture_width(entity);
            if (entity->pos.y >= render_height - entity_texture_height(entity))
                entity->pos.y = render_height - entity_texture_height(entity);
            if (entity->pos.y < 0)
                entity->pos.y += render_height;
        } break;
        case ENTITY_NODE: {
            if (entity->texture_id != 0 && level->player->pos.y < 61) {
                entity->texture_id = 0;
                level->player->pos.y = 60;

                add_event(game, chapter_6_goto_home, 2);
            }
        } break;
    }
}

void chapter_6_entity_draw(Entity *entity, Game *game) {
    Level_Chapter_6 *level = (Level_Chapter_6 *)game->level;

    switch (entity->type) {
        case ENTITY_NODE: {
            Texture *node_texture = &atari_assets.textures[12];
            Texture *texture = entity_get_texture(entity);

            Rectangle source = {
                0, 0,
                texture->width, texture->height
            };

            Rectangle dest = {
                entity->pos.x, entity->pos.y,
                node_texture->width, node_texture->height
            };

            DrawTexturePro(*texture, source, dest, {}, 0, WHITE);
        } break;
        case ENTITY_PLAYER: {
            default_entity_draw(entity);
            if (level->white_fade_alpha > 0)
                DrawTexture(atari_assets.textures[14], entity->pos.x, entity->pos.y, {255, 255, 255, (uint8_t)(level->white_fade_alpha * 255)});
        } break;
        default: {
            default_entity_draw(entity);
        } break;
    }
}

void chapter_6_update(Game *game, float dt) {
    Level_Chapter_6 *level = (Level_Chapter_6 *)game->level;

    switch (level->state) {
        case CHAPTER_6_STATE_DESERT: {
            for (int i = 0; i < game->entities.length; i++) {
                chapter_6_entity_update(game->entities.data[i], game, dt);
            }
        } break;
        case CHAPTER_6_STATE_HOME: {
            if (level->fade_alpha < 1) {
                level->fade_alpha += 0.25f * dt;
                if (level->fade_alpha > 1)
                    level->fade_alpha = 1;
            }

            if (level->fade_alpha == 1) {
                for (int i = 0; i < game->entities.length; i++) {
                    Entity *entity = game->entities.data[i];

                    if (entity->type != ENTITY_PLAYER) {
                        free_entity(entity);
                        array_remove(&game->entities, i--);
                    }
                }

                if (!level->started_text) {
                    level->started_text = true;
                    add_event(game, chapter_6_begin_text, 2);
                } else {
                    level->white_fade_alpha += 0.025f * dt;
                    if (level->white_fade_alpha > 1)
                        level->white_fade_alpha = 1;
                }
            }
        } break;
        case CHAPTER_6_STATE_TEXT: {
            if (level->godtext != 2)
                level->god_scroll += 8 * dt;
            else
                level->god_scroll += 12 * dt;

            if (level->god_scroll > render_height) {
                level->god_scroll = render_height;
            }

            if (is_action_pressed()) {
                if (level->god_scroll == render_height) {
                    if (level->godtext < 2) {
                        level->godtext++;
                        if (level->godtext == 1)
                            level->god_scroll = 36;
                        else
                            level->god_scroll = 0;
                    }
                } else {
                    level->god_scroll = render_height;
                }
            }

            if (level->godtext == 2) {
                level->red_fade_time += 0.05f * dt;

                if (level->red_fade_time > 1) {
                    level->red_fade_time = 1;
                    atari_queue_deinit_and_goto_intro(game);
                }
            }
        } break;
    }
}

void chapter_6_draw(Game *game) {
    Level_Chapter_6 *level = (Level_Chapter_6 *)game->level;

    switch (level->state) {
        case CHAPTER_6_STATE_DESERT: {
            ClearBackground(DESERT_COLOR);

            sort_entities(&game->entities);

            for (int i = 0; i < game->entities.length; i++) {
                Entity *entity = game->entities.data[i];
                chapter_6_entity_draw(entity, game);
            }
        } break;
        case CHAPTER_6_STATE_HOME: {
            ClearBackground(DESERT_COLOR);

            for (int i = 0; i < game->entities.length; i++) {
                Entity *entity = game->entities.data[i];
                if (entity->type != ENTITY_PLAYER)
                    chapter_6_entity_draw(entity, game);
            }

            DrawTexture(atari_assets.textures[13],
                        0,
                        0,
                        {255, 255, 255, (uint8_t)(level->fade_alpha * 255)});

            DrawRectangle(0, 0, render_width, render_height, {0, 0, 0, (uint8_t)(level->white_fade_alpha * 255)});

            chapter_6_entity_draw(level->player, game);
        } break;
        case CHAPTER_6_STATE_TEXT: {
            ClearBackground(BLACK);

            DrawTexture(atari_assets.textures[15 + level->godtext], 0, 0, WHITE);

            DrawRectangle(0, level->god_scroll, render_width, render_height, BLACK);

            float red_fade_alpha = chapter_6_red_fade_alpha(level->red_fade_time);

            DrawRectangle(0, 0, render_width, render_height, {255, 0, 0, (uint8_t)(255 * red_fade_alpha)});
        } break;
    }
}

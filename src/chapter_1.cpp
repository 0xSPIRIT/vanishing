#define DESERT_COLOR { 214, 194, 105, 255 }
#define FOREST_COLOR { 50, 168, 82, 255 }
#define CACTUS_COUNT 4

enum Level_Chapter_1_State {
    LEVEL_CHAPTER_1_STATE_BEFORE_PHONE = 0,
    LEVEL_CHAPTER_1_STATE_PHONE_EXISTS,
    LEVEL_CHAPTER_1_STATE_PHONE_WAS_CALLED,
    LEVEL_CHAPTER_1_STATE_SUCCESSFUL_DIRECTIONS,
    LEVEL_CHAPTER_1_STATE_CRAWLING,
    LEVEL_CHAPTER_1_STATE_FOREST,
    LEVEL_CHAPTER_1_STATE_APARTMENT,
};

enum Direction {
    DIRECTION_INVALID,
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
};

int chapter_1_god_text_y[] = { 38, 5, 55, 16 };
int chapter_1_god_text_y_end[] = { 120, 153, 103, 146};

struct Level_Chapter_1 {
    Color background_color;

    Shader flashing_shader;

    float timer;
    int crawling_text_state;
    int screens_scrolled;
    int screens_scrolled_while_crawling;
    Level_Chapter_1_State state;

    bool about_to_exit;

    bool final_text;
    bool final_text_is_done; // the final part when you're crawling- when this is true you get to go to the forest.

    Direction last_movements[4];

    int god_index;
    bool intro;
    float god_scroll;
    int godtext;
};

Entity *chapter_1_make_entity(Entity_Type type, float x, float y) {
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
        case ENTITY_PHONE: {
            result->texture_id = 13;
        } break;
    }

    float texture_width  = entity_texture_width(result);
    float texture_height = entity_texture_height(result);

    result->base_collider = {0, 3*texture_height/4, texture_width, texture_height/4};

    return result;
}

void chapter_1_activate_node(void *ptr_game) {
    Game *game = (Game *)ptr_game;

    Entity *player = entities_get_player(&game->entities);

    for (size_t i = 0; i < game->entities.length; i++) {
        Entity *e = game->entities.data[i];
        if (e->type == ENTITY_NODE) {
            e->chap_1_node.active = false;
            e->texture_id = 11;

            player->chap_1_player.is_hurt = true;
            player->alarm[0] = 2; // seconds

            Entity *footstep = chapter_1_make_entity(ENTITY_BLOOD,
                                           player->pos.x,
                                           player->pos.y + entity_texture_height(player) - 3);
            array_add(&game->entities, footstep);

            int state = player->chap_1_player.walking_state;
            state++;
            player->chap_1_player.walking_state = (Player_Walking_State)state;

            if (player->chap_1_player.walking_state == PLAYER_WALKING_STATE_CRAWLING)
                player->pos.y += 8;

            break;
        }
    }
}

void set_player_alarm(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    Level_Chapter_1 *level = (Level_Chapter_1*)game->level;

    Entity *player = entities_get_player(&game->entities);

    player->alarm[2] = 3;
    level->about_to_exit = true;
}

void chapter_1_init(Game *game) {
    Level_Chapter_1 *level = (Level_Chapter_1 *)game->level;

    level->intro = true;
    level->god_scroll = chapter_1_god_text_y[0];

    Texture2D *textures = atari_assets.textures;

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
    textures[13] = load_texture(RES_DIR "art/phone.png");
    textures[14] = load_texture(RES_DIR "art/water.png");
    textures[15] = load_texture(RES_DIR "art/apartment.png");
    textures[16] = load_texture(RES_DIR "art/player_huff_puff.png");

    textures[17] = load_texture(RES_DIR "art/intro1.png");
    textures[18] = load_texture(RES_DIR "art/intro2.png");
    textures[19] = load_texture(RES_DIR "art/intro3.png");
    textures[20] = load_texture(RES_DIR "art/intro4.png");

    level->background_color = DESERT_COLOR;

    level->flashing_shader = LoadShader(0, RES_DIR "shaders/flashing.fs");

    game->entities = make_array<Entity*>(2);

    Entity *player = chapter_1_make_entity(ENTITY_PLAYER, render_width/2 - 4, render_height/2 - 8);

    array_add(&game->entities, player);


    float speed = 15;

    atari_text_list_init(&game->text[0],
                         0,
                         "And he awoke.",
                         speed,
                         &game->text[1]);
    atari_text_list_init(&game->text[1],
                         "Chase",
                         "So... thirsty...",
                         speed,
                         nullptr);

    atari_text_list_init(&game->text[2],
                         "Chase",
                         "God...\rI'm gonna... die here...",
                         speed,
                         nullptr);
    atari_text_list_init(&game->text[3],
                         "Chase",
                         "Someone... help...\rSo... thirsty...",
                         speed,
                         nullptr);

    speed = 30;


    atari_text_list_init(&game->text[7],
                         0,
                         "This one calls itself\n83D3960AA4B1 (READ ERROR).",
                         speed,
                         &game->text[10]);
    atari_text_list_init(&game->text[8],
                         "Chase",
                         "This one calls itself\nC0291BA16324 (READ ERROR).",
                         speed,
                         &game->text[10]);
    atari_text_list_init(&game->text[9],
                         0,
                         "This one calls itself\nDC6F8203351 (READ ERROR).",
                         speed,
                         &game->text[10]);

    String choices[] = {const_string("Yes"), const_string("No")};
    Text_List *next[] = { 0, 0 };
    void (*function_pointers[])(void*) = { chapter_1_activate_node, nullptr };

    atari_choice_text_list_init(&game->text[10],
                                0,
                                "Drink the liquid?",
                                choices,
                                next,
                                function_pointers,
                                2);

    atari_text_list_init(&game->text[11],
                         "Chase",
                         "*ring ring*\r*click*\rH... he- hello?",
                         speed,
                         &game->text[12]);
    atari_text_list_init(&game->text[12],
                         "      ",
                         "Yes?",
                         speed,
                         &game->text[13]);
    atari_text_list_init(&game->text[13],
                         "Chase",
                         "H-h... help...\rPlease...",
                         speed,
                         &game->text[14]);
    atari_text_list_init(&game->text[14],
                         "      ",
                         "Oh, you're stuck in the\ndesert?",
                         speed,
                         &game->text[15]);
    atari_text_list_init(&game->text[15],
                         "      ",
                         "Most people find their\nway out pretty easily.",
                         speed,
                         &game->text[16]);
    atari_text_list_init(&game->text[16],
                         "Chase",
                         "...\rI need... water.",
                         speed,
                         &game->text[17]);
    atari_text_list_init(&game->text[17],
                         "Chase",
                         "I-I keep walking... but it\ngoes on forever...",
                         speed,
                         &game->text[18]);
    atari_text_list_init(&game->text[18],
                         "      ",
                         "It's simple, silly!\rThere's water everywhere!",
                         speed,
                         &game->text[19]);
    atari_text_list_init(&game->text[19],
                         "      ",
                         "You just go up, then down,\nthen up, then left.\rYou'll find what you need.",
                         speed,
                         &game->text[20]);
    atari_text_list_init(&game->text[20],
                         "Chase",
                         "Th-that doesn't make sense.\rAnd I already drank the\nwater- it hurts!",
                         speed,
                         &game->text[21]);
    atari_text_list_init(&game->text[21],
                         "      ",
                         "Huh?\rWhen everyone else drinks\nit their thirst is quenched.",
                         speed,
                         &game->text[22]);
    atari_text_list_init(&game->text[22],
                         "      ",
                         "... Whatever-\rTrust me, you'll find\nit there.",
                         speed,
                         &game->text[23]);
    atari_text_list_init(&game->text[23],
                         "Chase",
                         "Okay.\r*click*",
                         speed,
                         nullptr);

    speed = 10;
    /*
       atari_text_list_init(&game->text[23],
       "Chase",
       "Please... God... help...",
       speed,
       nullptr);
     */

    atari_mid_text_list_init(&game->text[24],
                             "When will you reveal\nyourself to me?",
                             &game->text[25]);

    atari_mid_text_list_init(&game->text[25],
                             "I do not know what\nI see,\rbecause there is\nnothing visible.",
                             &game->text[26]);

    atari_mid_text_list_init(&game->text[26],
                             "I do not know how to\nname you,\rbecause I don't know\nwhat you are.",
                             &game->text[27]);

    atari_mid_text_list_init(&game->text[27],
                             "You have no form\rbecause your nature is\nboundless.",
                             &game->text[28]);

    atari_mid_text_list_init(&game->text[28],
                             "I saw you up until that\nnight, staring at the\nstars.",
                             &game->text[29]);

    atari_mid_text_list_init(&game->text[29],
                             "But, if you are here,",
                             &game->text[30]);

    atari_mid_text_list_init(&game->text[30],
                             "Please help me.",
                             nullptr);


    atari_mid_text_list_init(&game->text[31],
                             "He tried getting out\nof bed.",
                             &game->text[32]);
    atari_mid_text_list_init(&game->text[32],
                             "Maybe next time.",
                             nullptr);
    game->text[32].scroll_type = LetterByLetter;
    game->text[32].callbacks[0] = set_player_alarm;
}

void chapter_1_deinit(Game *game) {
    (void)game;
}

void add_cactuses_randomly(Array<Entity*> *entities, size_t cactus_count) {
    for (int i = 0; i < cactus_count; i++) {
        int x = rand()%(render_width - 8);
        int y = rand()%(render_height - 8);

        Entity_Type type = (rand()%2 == 0) ? ENTITY_CACTUS : ENTITY_ROCK;

        Entity *cactus_a = chapter_1_make_entity(type, x, y);
        array_add(entities, cactus_a);
        int cactus_a_index = (int)entities->length-1;

        // Check if it's colliding with any other entities. If so, remove it
        // and decrement i to have a go again in the next loop.

        for (int j = 0; j < entities->length; j++) {
            Entity *e = entities->data[j];
            
            if (e == cactus_a) continue;

            if (are_entities_visibly_colliding(cactus_a, e)) {
                free_entity(cactus_a);
                array_remove(entities, cactus_a_index);
                i--;
                break;
            }
        }
    }
}

void chapter_1_entity_update(Entity *e, Game *game, float dt) {
    Level_Chapter_1 *level = (Level_Chapter_1 *)game->level;
    Entity *player = entities_get_player(&game->entities);

    switch (e->type) {
        case ENTITY_PHONE: {
            bool within_region = is_player_close_to_entity(player, e, 8);

            if (keyboard_focus(game) == NO_KEYBOARD_FOCUS && within_region) {
                if (is_action_pressed()) {
                    if (e->chap_1_phone.called) {
                        game->current = &game->text[17];
                    } else {
                        game->current = &game->text[11];
                    }
                    e->chap_1_phone.called = true;
                    level->state = LEVEL_CHAPTER_1_STATE_PHONE_WAS_CALLED;
                }
            }
        } break;
        case ENTITY_NODE: {
            bool within_region = is_player_close_to_entity(player, e, 8);

            if (e->chap_1_node.active && keyboard_focus(game) == NO_KEYBOARD_FOCUS && within_region) {
                if (is_action_pressed()) {
                    switch (e->chap_1_node.type) {
                        case NODE_FIRST: {
                            game->current = &game->text[7];
                        } break;
                        case NODE_SECOND: {
                            game->current = &game->text[8];
                        } break;
                        case NODE_THIRD: {
                            game->current = &game->text[9];
                        } break;
                    }
                }
            }
        } break;
        case ENTITY_PLAYER: {
            if (keyboard_focus(game) != NO_KEYBOARD_FOCUS) break;
            if (!e->chap_1_player.huffing_and_puffing &&
                level->state == LEVEL_CHAPTER_1_STATE_APARTMENT &&
                e->alarm[1] == 0)
            {
                game->current = &game->text[31];
                e->chap_1_player.huffing_and_puffing = true;
            }

            if (e->chap_1_player.huffing_and_puffing) {
                if (level->about_to_exit && e->alarm[2] == 0) {
                    atari_queue_deinit_and_goto_intro(game);
                }
                break;
            }

            float player_speed = 30;

            int dir_x = key_right() - key_left();
            int dir_y = key_down()  - key_up();

            if (level->state == LEVEL_CHAPTER_1_STATE_FOREST)
                dir_x = 0;

            if (e->alarm[0] == 0) {
                e->chap_1_player.is_hurt = false;
            }

            switch (e->chap_1_player.walking_state) {
                case PLAYER_WALKING_STATE_NORMAL: {
                    e->texture_id = 0;
                    e->chap_1_player.blood_probability = 0;
                    player_speed = 30;
                } break;
                case PLAYER_WALKING_STATE_SLOWED_1: {
                    e->texture_id = 0; // TODO
                    e->chap_1_player.blood_probability = 0.2f;
                    player_speed = 25;
                } break;
                case PLAYER_WALKING_STATE_SLOWED_2: {
                    e->texture_id = 0; // TODO
                    e->chap_1_player.blood_probability = 0.5f;
                    player_speed = 20;
                } break;
                case PLAYER_WALKING_STATE_CRAWLING: {
                    e->chap_1_player.blood_probability = 1;
                    e->texture_id = 10;
                    player_speed = 10;
                } break;
            }

            if (level->state == LEVEL_CHAPTER_1_STATE_APARTMENT)
                e->texture_id = 10;

            float texture_width  = entity_texture_width(player);
            float texture_height = entity_texture_height(player);

            player->base_collider = {0, 3*texture_height/4, texture_width, texture_height/4};

            if (IsKeyDown(KEY_P))
                player_speed *= 10;

            Vector2 velocity = { dir_x * player_speed * dt, dir_y * player_speed * dt };

            apply_velocity(e, velocity, &game->entities);

            if (level->state == LEVEL_CHAPTER_1_STATE_FOREST) {
                if (e->pos.y < 2*render_height/3) {
                    level->state = LEVEL_CHAPTER_1_STATE_APARTMENT;
                    e->chap_1_player.walking_state = PLAYER_WALKING_STATE_SLOWED_1;

                    e->alarm[1] = 5;

                    for (int i = 0; i < game->entities.length; i++) {
                        Entity *entity = game->entities.data[i];
                        if (entity->type != ENTITY_PLAYER) {
                            free_entity(entity);
                            array_remove(&game->entities, i--);
                        }
                    }

                    add_wall(&game->entities, {53,73,19,78});
                    add_wall(&game->entities, {70,72,50,7});
                    add_wall(&game->entities, {116,76,7,77});
                    add_wall(&game->entities, {71,136,47,16});
                }
            } else if (level->state == LEVEL_CHAPTER_1_STATE_APARTMENT) {
                if (e->pos.y < 50)
                    e->pos.y = 50;
            }

            if (dir_x != 0 || dir_y != 0) {
                if (level->state != LEVEL_CHAPTER_1_STATE_APARTMENT && Vector2Length(Vector2Subtract(e->pos, e->chap_1_player.stored_pos)) >= 12) {
                    e->chap_1_player.stored_pos = e->pos;

                    Entity_Type type = ENTITY_FOOTSTEPS;
                    float random = (float)rand()/RAND_MAX;

                    if (random < e->chap_1_player.blood_probability)
                        type = ENTITY_BLOOD;

                    e->chap_1_player.footsteps_done++;

                    if (level->state != LEVEL_CHAPTER_1_STATE_CRAWLING &&
                        keyboard_focus(game) == NO_KEYBOARD_FOCUS)
                    {
                        if (e->chap_1_player.footsteps_done == 20) {
                            game->current = &game->text[2];
                        } else if (e->chap_1_player.footsteps_done == 40) {
                            game->current = &game->text[3];
                        }
                    }

                    Entity *footstep = chapter_1_make_entity(type,
                                                   e->pos.x,
                                                   e->pos.y + entity_texture_height(e) - 3);
                    array_add(&game->entities, footstep);
                }
            }

            float stored_x = e->pos.x;
            float stored_y = e->pos.y;

            int width = entity_texture_width(e);
            int height = entity_texture_height(e);

            // Don't scroll the screen to the next one if you
            // haven't deactivated the node!

            bool only_allow_up_scroll = level->final_text_is_done;
            bool scrolled_up = false;

            bool can_scroll = true;
            Entity *node_find = entities_find_from_type(&game->entities,
                                                        ENTITY_NODE);

            if (node_find && node_find->chap_1_node.active) {
                can_scroll = false;
            }

            if (can_scroll) {
                Direction dir = DIRECTION_INVALID;

                if (!only_allow_up_scroll) {
                    if (e->pos.x + width < 0) {
                        e->pos.x = render_width;
                        dir = DIRECTION_LEFT;
                    }
                    if (e->pos.x > render_width) {
                        e->pos.x = -width;
                        dir = DIRECTION_RIGHT;
                    }
                }

                if (e->pos.y + height < 0) {
                    e->pos.y = render_height;
                    dir = DIRECTION_UP;
                    scrolled_up = true;
                }

                if (!only_allow_up_scroll) {
                    if (e->pos.y > render_height) {
                        e->pos.y = -height;
                        dir = DIRECTION_DOWN;
                    }
                }

                if (only_allow_up_scroll) {
                    e->pos.x = Clamp(e->pos.x, 0, render_width - width);
                    if (e->pos.y >= render_height - height) e->pos.y = render_height - height;
                }

                if (!(only_allow_up_scroll && !scrolled_up) && (stored_x != e->pos.x || stored_y != e->pos.y)) {
                    if (level->state == LEVEL_CHAPTER_1_STATE_CRAWLING) {
                        level->screens_scrolled_while_crawling++;
                    }

                    level->screens_scrolled++;

                    if (player->chap_1_player.walking_state == PLAYER_WALKING_STATE_CRAWLING) {
                        level->state = LEVEL_CHAPTER_1_STATE_CRAWLING;
                    }

                    if (level->state == LEVEL_CHAPTER_1_STATE_PHONE_WAS_CALLED) {
                        Direction *directions = level->last_movements;

                        int index = 0;

                        for (int i = 0; i < 4; i++) {
                            if (directions[i] == DIRECTION_INVALID)
                            {
                                index = i;
                                directions[i] = dir;
                                break;
                            }
                        }


                        // up, down, up, left
                        Direction desired[] = { DIRECTION_UP, DIRECTION_DOWN, DIRECTION_UP, DIRECTION_LEFT };

                        if (directions[index] != desired[index]) {
                            memset(directions, 0, sizeof(Direction) * 4);
                            if (dir == desired[0]) directions[0] = dir;
                        } else if (index == 3) {
                            level->state = LEVEL_CHAPTER_1_STATE_SUCCESSFUL_DIRECTIONS;
                        }
                    }

                    // Remove all of the cacti and footsteps and nodes
                    for (int i = 0; i < game->entities.length; i++) {
                        Entity *entity = game->entities.data[i];
                        Entity_Type type = entity->type;

                        if (type == ENTITY_CACTUS || type == ENTITY_ROCK ||
                            type == ENTITY_FOOTSTEPS || type == ENTITY_BLOOD ||
                            type == ENTITY_NODE || type == ENTITY_PHONE)
                        {
                            free_entity(entity);
                            array_remove(&game->entities, i--);
                        }
                    }

                    Node_Type node_type = NODE_INVALID;

                    bool make_phone = false;

                    switch (level->screens_scrolled) {
                        case 4: {
                            node_type = NODE_FIRST;
                        } break;
                        case 8: {
                            node_type = NODE_SECOND;
                        } break;
                        case 10: {
                            make_phone = true;
                            level->state = LEVEL_CHAPTER_1_STATE_PHONE_EXISTS;
                        } break;
                    }

                    if (level->screens_scrolled_while_crawling >= 2 &&
                        level->final_text_is_done)
                    {
                        level->state = LEVEL_CHAPTER_1_STATE_FOREST;
                        e->pos.x = render_width/2 - entity_texture_width(e) / 2;
                    }

                    if (level->state == LEVEL_CHAPTER_1_STATE_SUCCESSFUL_DIRECTIONS) {
                        node_type = NODE_THIRD;
                    }

                    if (make_phone || level->state == LEVEL_CHAPTER_1_STATE_PHONE_WAS_CALLED || level->state == LEVEL_CHAPTER_1_STATE_PHONE_EXISTS) {
                        Entity *phone = chapter_1_make_entity(ENTITY_PHONE,
                                                    render_width/2 - 4,
                                                    render_height/4);
                        array_add(&game->entities, phone);
                    }

                    if (node_type != NODE_INVALID) {
                        Entity *node = chapter_1_make_entity(ENTITY_NODE, render_width/2 - 16, render_height/2 - 16);
                        node->chap_1_node.type = node_type;
                        array_add(&game->entities, node);
                    }

                    // And then re-add new ones.
                    if (level->state != LEVEL_CHAPTER_1_STATE_CRAWLING &&
                        level->state != LEVEL_CHAPTER_1_STATE_FOREST) {
                        add_cactuses_randomly(&game->entities, CACTUS_COUNT);
                    }
                }
            } else {
                e->pos.x = Clamp(e->pos.x, 0, render_width - width);
                e->pos.y = Clamp(e->pos.y, 0, render_height - height);
            }

        } break;
    }

    entity_update_alarms(e, dt);
}

void chapter_1_entity_draw(Entity *e, Game *game) {
    Level_Chapter_1 *level = (Level_Chapter_1 *)game->level;

    Texture2D *texture = entity_get_texture(e);
    int x = e->pos.x;
    int y = e->pos.y;

    switch (e->type) {
        case ENTITY_PLAYER: {
            if (e->chap_1_player.is_hurt) {
                int time_loc = GetShaderLocation(level->flashing_shader,
                                                 "time");
                float time = GetTime();
                SetShaderValue(level->flashing_shader, time_loc,
                               &time, SHADER_UNIFORM_FLOAT);

                int flash_color_loc = GetShaderLocation(level->flashing_shader,
                                                        "flash_color");
                Vector3 flash_color = { 1, 0, 0 }; // red
                SetShaderValue(level->flashing_shader, flash_color_loc,
                               &flash_color, SHADER_UNIFORM_VEC3);


                BeginShaderMode(level->flashing_shader);
            }

            if (texture) {
                DrawTexture(*texture, x, y, WHITE);
            }

            if (e->chap_1_player.is_hurt) {
                EndShaderMode();
            }
        } break;
        default: {
            if (texture) {
                DrawTexture(*texture, x, y, WHITE);
            }
        }
    }
}

void chapter_1_update(Game *game, float dt) {
    Level_Chapter_1 *level = (Level_Chapter_1 *)game->level;

    if (level->intro) {
        level->god_scroll += 8 * dt;

        if (level->god_scroll > chapter_1_god_text_y_end[level->god_index])
            level->god_scroll = render_height;

        if (is_action_pressed()) {
            if (level->god_scroll == render_height) {
                level->god_index++;
                if (level->god_index > 3) {
                    level->intro = false;
                    game->current = &game->text[0];
                }
                level->god_scroll = chapter_1_god_text_y[level->god_index];
            } else {
                level->god_scroll = render_height;
            }
        }
        return;
    }

    for (int i = 0; i < game->entities.length; i++) {
        Entity *e = game->entities.data[i];
        chapter_1_entity_update(e, game, dt);
    }

    if (level->state == LEVEL_CHAPTER_1_STATE_CRAWLING &&
        game->current == nullptr)
    {
        level->timer += dt;

        float time_end = -1;

        switch (level->crawling_text_state) {
            case 0: time_end = 3; break;
            case 1: time_end = 0; break;
        }

        if (time_end != -1 && level->timer >= time_end) {
            switch (level->crawling_text_state) {
                case 0: {
                    game->current = &game->text[24];
                    level->final_text = true;
                } break;
                case 1: {
                    level->final_text_is_done = true;
                } break;
            }

            level->timer = 0;
            level->crawling_text_state++;
        }
    }

    if (level->state == LEVEL_CHAPTER_1_STATE_FOREST) {
        level->background_color = FOREST_COLOR;
    }
}

void chapter_1_draw(Game *game) {
    Level_Chapter_1 *level = (Level_Chapter_1 *)game->level;

    ClearBackground(level->background_color);

    if (level->intro) {
        DrawTexture(atari_assets.textures[17 + level->god_index], 0, 0, WHITE);
        DrawRectangle(0, level->god_scroll, render_width, render_height, BLACK);
        return;
    }

    if (level->final_text)
        game->textbox_alpha = 255;
    else 
        game->textbox_alpha = 200;

    switch (level->state) {
        case LEVEL_CHAPTER_1_STATE_FOREST: {
            DrawTexture(atari_assets.textures[14], 0, 0, WHITE);
        } break;
        case LEVEL_CHAPTER_1_STATE_APARTMENT: {
            DrawTexture(atari_assets.textures[15], 0, 0, WHITE);
        } break;
    }

    sort_entities(&game->entities);

    size_t entity_count = game->entities.length;

    // Draw the entities
    for (int i = 0; i < entity_count; i++) {
        Entity *e = game->entities.data[i];
        chapter_1_entity_draw(e, game);
    }
}

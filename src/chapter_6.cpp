enum Chapter_6_State {
    CHAPTER_6_STATE_SUPPORT_GROUP,
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

    Text_List *final_support_text;

    float god_scroll;

    int godtext;

    bool end;
};

Entity *chapter_6_make_entity(Entity_Type type, float x, float y);

void chapter_6_goto_desert(Game *game) {
    Level_Chapter_6 *level = (Level_Chapter_6 *)game->level;

    level->state = CHAPTER_6_STATE_DESERT;

    level->player = chapter_6_make_entity(ENTITY_PLAYER, render_width/2 - 4, 3*render_height/4);
    array_add(&game->entities, level->player);

    Entity *node = chapter_1_make_entity(ENTITY_NODE, render_width/2 - 16, render_height/4 - 16);
    array_add(&game->entities, node);

    game->post_processing.type = POST_PROCESSING_VHS;
    post_process_vhs_set_intensity(&game->post_processing.vhs,
                                   VHS_INTENSITY_MEDIUM);
    game->post_processing.vhs.vignette_mix = 0.8f;

    play_music(MUSIC_VHS_BAD);
}

void chapter_6_delayed_goto_desert(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    add_event(game, chapter_6_goto_desert, 2);
}

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
    stop_music();
}

void chapter_6_support_text(Text_List *list, Font *font, char *speaker, char *line, Text_List *next) {
    list->font = font;
    list->font_spacing = 0;
    list->scale = 0.125;
    list->scroll_speed = 30;

    list->color = WHITE;
    list->bg_color = BLACK;

    list->render_type = DrawTextbox;
    list->location = Top;
    list->take_keyboard_focus = true;

    text_list_init(list, speaker, line, next);
}

void chapter_6_choice_text(Text_List *list,
                           Font *font,
                           char *speaker,
                           char *line,
                           String choices[],
                           Text_List *next[],
                           void (*hooks[])(void*),
                           int choice_count)
{
    list->font = font;
    list->font_spacing = 0;
    list->scale = 0.125;
    list->scroll_speed = 30;

    list->color = WHITE;
    list->bg_color = BLACK;

    list->choice = true;
    list->choice_count = choice_count;
    list->choice_index = -1;

    text_list_init(list, speaker, line, 0);

    memcpy(list->choices,
           choices,
           sizeof(choices[0]) * choice_count);
    memcpy(list->next,
           next,
           sizeof(next[0]) * choice_count);
    memcpy(list->callbacks,
           hooks,
           sizeof(hooks[0]) * choice_count);
}

void chapter_6_text(bool scroll, bool chase, Text_List *list, char *line,
                    Text_List *next)
{
    list->font         = &atari_font;
    list->font_spacing = 1;
    list->scale        = 0.125;
    list->scroll_speed = 15;

    if (chase) {
        list->color = WHITE;
        list->scroll_sound = SOUND_TEXT_SCROLL_LOW;
    } else {
        list->color = {255,0,0,255};
        list->backdrop_color = {100,0,0,255};
        list->scroll_sound = SOUND_TEXT_SCROLL_BAD;
    }

    list->center_text = true;
    if (scroll)
        list->scroll_type = LetterByLetter;
    else
        list->scroll_type = EntireLine;

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
            result->draw_layer = DRAW_LAYER_LOW;
        } break;
        case ENTITY_BLOOD: {
            result->texture_id = 7 + rand() % 3;
            result->draw_layer = DRAW_LAYER_LOW;
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

void chapter_6_start_text(Game *game) {
    game->current = &game->text[20];
}

void chapter_6_start_text_delayed(Game *game) {
    add_event(game, chapter_6_start_text, 10);
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

    game->entities = make_array<Entity*>(16);

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
    textures[13] = load_texture(RES_DIR "art/apartment2.png");
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
                   "What you NEED.\rMEANING. PURPOSE.\nCONTENTMENT.\rYOU NEED MY HELP MORE\nTHAN EVER BEFORE!",
                   &game->text[4]);
    chapter_6_text(true,
                   false,
                   &game->text[4],
                   "I AM THE ALL-POWERFUL!\rI CAN GRANT YOU\nWHATEVER AND WHOMEVER\nYOU PLEASE!",
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
                   "A HOME AS LONELY AS\nTHE SILENCE BEFORE\nCREATION?\n\rFRIENDS THAT BELITTLE\nYOU AT EVERY TURN?",
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


    // support group

    int i = 20;

    auto support_text = [&](char *speaker, char *line, bool end) -> void {
        Text_List *curr = game->text + i;
        i++;
        Text_List *next  = game->text + i;

        if (end)
            next = 0;

        chapter_6_support_text(curr,
                               &atari_small_font,
                               speaker,
                               line,
                               next);
        curr->textbox_height += 4;
    };

    auto support_choice = [&](char *cstr_choices[], int choice_count) {
        char *speaker = "Chase";
        char *text_string = "...........";

        Text_List *curr = game->text + i;
        i++;
        Text_List *next  = game->text + i;

        String choices[99] = {};
        for (int j = 0; j < choice_count; j++) {
            strcpy(choices[j].text, cstr_choices[j]);
            choices[j].length = strlen(cstr_choices[j]);
        }

        Text_List *nexts[99];
        for (int j = 0; j < choice_count; j++) {
            nexts[j] = next;
        }

        void (*hooks[99])(void*) = {};

        curr->font = &atari_small_font;
        curr->arrow_color = WHITE;
        curr->scale = 0.125;
        curr->scroll_speed = 30;
        curr->font_spacing = 0;

        text_list_init(curr, speaker, text_string, 0);
        curr->choice = true;
        curr->choice_count = choice_count;
        curr->choice_index = -1;

        memcpy(curr->choices,
               choices,
               sizeof(choices[0]) * choice_count);
        memcpy(curr->next,
               nexts,
               sizeof(nexts[0]) * choice_count);
        memcpy(curr->callbacks,
               hooks,
               sizeof(hooks[0]) * choice_count);
    };

    support_text(0, "It was a bright cold day in April, and\nthe clocks were striking thirteen.\rChase took a meditative breath.", false);
    support_text(0, "He found himself sitting despondently\nat a table with three of his friends.", false);
    support_text(0, "Although earlier that day he felt some\nsemblance of understanding of his\nsituation,", false);
    support_text(0, "all that he could feel now was malaise.", false);
    support_text(0, "Even after all that work,\nnothing had changed!", false);
    support_text(0, "Back to the way things were.\rBusiness as usual.", false);
    support_text(0, "He glared at his boulder at the base\nof the hill, his progress reset.", false);
    support_text(0, "His eyes wandered into the void\nbetween the floorboards.\rHe forgot to blink.", false);
    support_text(0, "There, the void whispered back to him,\nlike before.", false);
    support_text(0, "But, before he could understand what\nshe was saying...", false);

    support_text("Aria",   "*Casts the die*\rYou guys remember Sonja?\rI bumped into her yesterday.", 0);
    support_text(0,        "It landed on SIX.", 0);
    support_text(0,        "She threw a card zealously onto the\ntable.\rHe wasn't completely sure why.", 0);
    support_text("Tyrell", "Uhhh, Sonja from school or the other\none?", 0);
    support_text("Aria",   "Other one.\rLong hair, tall, white-indian, skinny,\nwears glasses...", 0);
    support_text("Tyrell", "HAH!\rLook, Noah's mouth is agape at the\nmention.", 0);
    support_text("Noah",   "*Casts the die*", 0);
    support_text(0,        "It landed on SIX.", 0);
    support_text("Noah",   "....! Me and Sonja?!\rNooooooooo.....!", 0);

    {
        char *choices[] = {"Who's Sonja", "What's with Noah and Sonja?", "What did y'all say? I missed something."};
        support_choice(choices, StaticArraySize(choices));
    }

    support_text("Chase",  "Um I thin--", 0);
    support_text("Tyrell", "Hahah, look how his eyes widened,\rhe SO has a type, he just doesn't want\nto admit it.", 0);
    support_text("Tyrell", "*Casts the die*", 0);
    support_text(0,        "It landed on SIX.", 0);
    support_text("Noah",   "..........\rOk maybe I do have a type.\rDoesn't she have a man though?", 0);
    support_text("Aria",   "She and Leon are *pbbt*\rKaput, no more, separated.", 0);
    support_text("Noah",   "How long since they broke up?", 0);
    support_text("Aria",   "Literally yesterday.", 0);
    support_text(0,        "Aria glances over at Chase with a\nlook of disapproval.\rShe realized he wasn't talking much.", 0);
    support_text("Aria",   "Damn you're such a nerd.", 0);
    support_text("Chase",  "...\rOh.", 0);
    support_text(0,        "Although she had anticipated a\nplayful remark, his response was\nanything but.", 0);
    support_text("Tyrell", "I guess it's Noah's turn for her, huh.\rIt'll make S jealous for sure.", 0);

    {
        char *choices[] = {"Who's S?", "I don't really know what to say.", "Can you guys hear me?"};
        support_choice(choices, StaticArraySize(choices));
    }

    support_text("Chase",  "Uh, whose turn is it to play?\rMine?\r...", 0);
    support_text("Tyrell", "... What?\rOh, uh whatever yeah your turn.", 0);
    support_text("Chase",  "*Casts the die*", 0);
    support_text(0,        "It landed on ONE.", 0);
    support_text(0,        "Chase's face remains blank.", 0);
    support_text("Tyrell", "...\rC'mon Chase, stop being like thaattttttt", 0);
    support_text("Aria",   "Chase.\rCome on man, what are you doing here.", 0);
    support_text("Chase",  "... Well I-", 0);
    support_text("Tyrell", "How about you actually contribute to\nthe conversation?", 0);
    support_text("Tyrell", "Why can't you just have fun?", 0);
    support_text("Chase",  "...\rWhat?\rWhy would you say that to m-", 1);

    //game->text[i-1].callbacks[0] = chapter_6_delayed_goto_desert;

    level->final_support_text = &game->text[i-1];

    game->text[13].callbacks[0] = chapter_6_queue_godtext;

    //chapter_6_goto_desert(game);

    movie_init(&game_movie, MOVIE_EMPTINESS);
    game_movie.end_movie_callback = chapter_6_start_text_delayed;
}

void chapter_6_entity_update(Entity *entity, Game *game, float dt) {
    Level_Chapter_6 *level = (Level_Chapter_6 *)game->level;
    level, dt;

    switch (entity->type) {
        case ENTITY_PLAYER: {
            if (game->current) break;

            float speed = (float)(entity->pos.y - 60) / (float)render_height;
            speed *= 60;

#ifdef DEBUG
            if (IsKeyDown(KEY_P))
                speed *= 10;
#endif

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

            if (entity->pos.x >= render_width/2+16)
                entity->pos.x = render_width/2+16;
            if (entity->pos.x <= render_width/2-16)
                entity->pos.x = render_width/2-16;
            if (entity->pos.y >= render_height - entity_texture_height(entity))
                entity->pos.y = render_height - entity_texture_height(entity);
            if (entity->pos.y < 0)
                entity->pos.y += render_height;
        } break;
        case ENTITY_NODE: {
            if (entity->texture_id != 0 && level->player->pos.y < 61) {
                entity->texture_id = 0;
                level->player->pos.y = 60;

                post_process_vhs_set_intensity(&game->post_processing.vhs, VHS_INTENSITY_MEDIUM_HIGH);

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

    if (game_movie.movie != 0) return;

    switch (level->state) {
        case CHAPTER_6_STATE_SUPPORT_GROUP: {
            if (game->current == level->final_support_text &&
                is_text_list_at_end(game->current))
            {
                game->current = 0;
                chapter_6_delayed_goto_desert(game);
            }
        } break;
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

                        post_process_vhs_set_intensity(&game->post_processing.vhs, VHS_INTENSITY_HIGH);

                        if (level->godtext == 1)
                            level->god_scroll = 36;
                        else
                            level->god_scroll = 0;
                    }
                } else {
#ifdef DEBUG
                    level->god_scroll = render_height;
#endif
                }
            }

            if (level->godtext == 2) {
                level->red_fade_time += 0.05f * dt;

                if (level->red_fade_time > 0.5f) {
                    game->post_processing.vhs.noise_intensity -= 0.25f * dt;
                    game->post_processing.vhs.vignette_mix    -= 0.04f * dt;
                }

                if (game->post_processing.vhs.noise_intensity < 0)
                    game->post_processing.vhs.noise_intensity = 0;

                if (game->post_processing.vhs.vignette_mix < 0)
                    game->post_processing.vhs.vignette_mix = 0;

                if (level->red_fade_time > 1) {
                    level->red_fade_time = 1;

                    if (!level->end) {
                        level->end = true;
                        add_event(game, atari_queue_deinit_and_goto_intro, 10);
                    }
                }
            }
        } break;
    }
}

void chapter_6_draw(Game *game) {
    Level_Chapter_6 *level = (Level_Chapter_6 *)game->level;

    if (game_movie.movie != 0) return;

    switch (level->state) {
        case CHAPTER_6_STATE_SUPPORT_GROUP: {
            ClearBackground(BLACK);
        } break;
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

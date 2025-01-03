#define DESERT_COLOR { 232, 204, 124, 255 }
//#define FOREST_COLOR { 50, 168, 82, 255 }
#define CACTUS_COUNT 6

enum Level_Chapter_1_State {
    LEVEL_CHAPTER_1_STATE_BEFORE_PHONE = 0,
    LEVEL_CHAPTER_1_STATE_PHONE_EXISTS,
    LEVEL_CHAPTER_1_STATE_PHONE_WAS_CALLED,
    LEVEL_CHAPTER_1_STATE_SUCCESSFUL_DIRECTIONS,
    LEVEL_CHAPTER_1_STATE_CRAWLING,
    LEVEL_CHAPTER_1_STATE_FOREST,
    LEVEL_CHAPTER_1_STATE_APARTMENT,
    LEVEL_CHAPTER_1_STATE_3D,
    LEVEL_CHAPTER_1_PHONE_CALL,
};

enum Direction {
    DIRECTION_INVALID,
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
};

int chapter_1_god_text_y[] = { 55, 16 };
int chapter_1_god_text_y_end[] = { 103, 146 };

struct Level_Chapter_1 {
    Color background_color;

    Shader flashing_shader;

    float prayer_fader, prayer_fader_to;

    Entity *player, *prayer_mat;

    float music_pitch;

    float timer;
    int crawling_text_state;
    int screens_scrolled;
    int screens_scrolled_while_crawling;
    Level_Chapter_1_State state;

    bool about_to_exit;

    bool final_text;
    bool final_text_is_done; // the final part when you're crawling- when this is true you get to go to the forest.

    Direction last_movements[4];

    bool before_first_text;
    int god_index;
    bool intro;
    float god_scroll;
    int godtext;

    // 3d part
    Model    scene_3d, scene_door;
    Camera3D camera;
    float    door_t;
    bool     move_3d;
    bool     hide_2d_elements;
    float    alpha_3d; // 0 is desert color bg, 1 is transparent showing 3d scene
};

void chapter_1_goto_3d(Game *game) {
    Level_Chapter_1 *level = (Level_Chapter_1 *)game->level;

    level->state = LEVEL_CHAPTER_1_STATE_3D;
    add_event(game,
              [](Game *game)->void{Level_Chapter_1 *l = (Level_Chapter_1*)game->level;l->move_3d=true;},
              5);
    add_event(game,
              [](Game *game)->void{Level_Chapter_1 *l = (Level_Chapter_1*)game->level;l->hide_2d_elements=true;},
              3);
}

void chapter_1_end(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    add_event(game, atari_queue_deinit_and_goto_intro, 2);
}

void chapter_1_first_text(Game *game) {
    Level_Chapter_1 *level = (Level_Chapter_1*)game->level;

    game->current = &game->text[0];
    level->before_first_text = false;
}

void chapter_1_end_intro(Game *game) {
    Level_Chapter_1 *level = (Level_Chapter_1*)game->level;

    level->intro = false;
    level->before_first_text = true;
    add_event(game, chapter_1_first_text, 2);
}

void chapter_1_start_phone_call(Game *game) {
    game->current = &game->text[35];
}

void chapter_1_prayer_text(Game *game) {
    game->current = &game->text[61];
}

void chapter_1_start_prayer(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    Level_Chapter_1 *level = (Level_Chapter_1*)game->level;

    level->prayer_fader_to = 1;

    add_event(game, chapter_1_prayer_text, 15);
}

void chapter_1_end_prayer(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    Level_Chapter_1 *level = (Level_Chapter_1*)game->level;

    level->prayer_fader_to = 0;
}

void chapter_1_draw_end_scene() {
    Texture2D *textures = atari_assets.textures;

    DrawTexture(textures[12], render_width/2-textures[11].width/2, render_height/4, WHITE);

    DrawTexture(textures[2], 51, 26, WHITE);
    DrawTexture(textures[1], 122, 19, WHITE);

    DrawTexture(textures[21], render_width/2 - textures[21].width/2, 10, WHITE);

    DrawTexture(textures[24], 49, 98, WHITE);
    DrawTexture(textures[25], 149, 96, WHITE);
}

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
            result->texture_id = rand_int(24, 31);
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
        case ENTITY_PHONE: {
            result->texture_id = 13;
        } break;
        case ENTITY_WINDOW: {
            result->texture_id = 21;
            result->has_dialogue = true;
        } break;
        case ENTITY_PRAYER_MAT: {
            result->texture_id = 22;
            result->draw_layer = DRAW_LAYER_LOW;
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
            play_sound(SOUND_ATARI_BASS_EFFECT);
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

            if (e->chap_1_node.type == NODE_SECOND)
                post_process_vhs_set_intensity(&game->post_processing.vhs, VHS_INTENSITY_MEDIUM);
            else if (e->chap_1_node.type == NODE_THIRD)
                post_process_vhs_set_intensity(&game->post_processing.vhs, VHS_INTENSITY_MEDIUM_HIGH);

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

void chapter_1_apartment_text(Text_List *list, char *line,
                              Text_List *next)
{
    list->font         = &atari_font;
    list->font_spacing = 2;
    list->scale        = 0.125;
    list->scroll_speed = 15;

    list->alpha_speed = 1;

    list->color        = WHITE;

    list->backdrop_color = {64,64,64,255};

    list->center_text  = true;
    list->scroll_type  = EntireLine;
    list->render_type  = Bare;
    list->location     = Middle;
    list->take_keyboard_focus = false;

    text_list_init(list, 0, line, next);
}

void chapter_1_call_text(Text_List *list, char *speaker,
                         char *line, float scroll_speed,
                         Text_List *next)
{
    list->font         = &atari_font;
    list->font_spacing = 1;
    list->scale        = 0.125;
    list->scroll_speed = scroll_speed;

    list->color = WHITE;
    list->bg_color = BLACK;
    list->center_text  = false;

    setup_text_scroll_sound(list, speaker);

    text_list_init(list, speaker, line, next);
}

void chapter_1_init(Game *game) {
    Level_Chapter_1 *level = (Level_Chapter_1 *)game->level;

    //-5.51753 m

    level->scene_3d = load_model("models/chap_1_end.glb");
    level->scene_door = load_model("models/door_chap_1_end.glb");

    level->camera.position = {-1, 1.5f, 0};
    level->camera.target = {-2, 1.5f, 0};
    level->camera.up = {0, 1, 0};
    level->camera.fovy = 70;
    level->camera.projection = CAMERA_PERSPECTIVE;

    level->music_pitch = 1;

    level->intro = true;
    level->god_scroll = chapter_1_god_text_y[0];

    game->textbox_alpha = 255;

    game->post_processing.type = POST_PROCESSING_VHS;
    post_process_vhs_set_intensity(&game->post_processing.vhs, VHS_INTENSITY_MEDIUM);
    game->post_processing.vhs.vignette_mix = 0.5f;
    game->post_processing.vhs.vignette_intensity = 1;

    play_music(MUSIC_DESERT_AMBIENCE);

    Texture2D *textures = atari_assets.textures;

    textures[0]  = load_texture("art/player.png");
    textures[1]  = load_texture("art/cactus_1.png");
    textures[2]  = load_texture("art/cactus_2.png");
    textures[3]  = load_texture("art/player_crawl_up.png");
    textures[4]  = load_texture("art/footsteps_1.png");
    textures[5]  = load_texture("art/footsteps_2.png");
    textures[6]  = load_texture("art/footsteps_3.png");
    textures[7]  = load_texture("art/blood_1.png");
    textures[8]  = load_texture("art/blood_2.png");
    textures[9]  = load_texture("art/blood_3.png");
    textures[10] = load_texture("art/player_crawl.png");
    textures[11] = load_texture("art/node_inactive.png");
    textures[12] = load_texture("art/node_active.png");
    textures[13] = load_texture("art/phone.png");
    textures[14] = load_texture("art/water.png");
    textures[15] = load_texture("art/apartment.png");
    textures[16] = load_texture("art/player_huff_puff.png");

    textures[17] = load_texture("art/intro1.png");
    textures[18] = load_texture("art/intro2.png");

    textures[21] = load_texture("art/window_1.png");
    textures[22] = load_texture("art/prayer_mat.png");
    textures[23] = load_texture("art/player_white.png");

    for (int i = 1; i <= 8; i++) {
        textures[24+i-1] = load_texture(TextFormat("art/rock%d.png", i));
    }

    textures[32] = load_texture("art/desert.png");
    textures[33] = load_texture("art/ready.png");

    level->background_color = DESERT_COLOR;

    level->flashing_shader = load_shader(0, "flashing.fs");

    game->entities = make_array<Entity*>(2);

    level->player = chapter_1_make_entity(ENTITY_PLAYER, render_width/2 - 4, render_height/2 - 8);

    array_add(&game->entities, level->player);

    float speed = 15;

    Dialogue_Segment segment[] = {
        text(0, "Chase", "I... need... to find... her...", speed, DLG_NEXT),
        text(1, "Chase", "So... thirsty...", speed, DLG_NULL),
    };

    dialogue_segment(game, segment, StaticArraySize(segment));

    /*
    atari_text_list_init(&game->text[0],
                         "Chase",
                         "I... need... to find... her...",
                         speed,
                         &game->text[1]);
    atari_text_list_init(&game->text[1],
                         "Chase",
                         "So... thirsty...",
                         speed,
                         nullptr);
                         */


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
                         "On the bottom is inscribed:\n83D3960AA4B1 (READ ERROR).",
                         speed,
                         &game->text[10]);
    atari_text_list_init(&game->text[8],
                         0,
                         "On the bottom is inscribed:\nC0291BA16324 (READ ERROR).",
                         speed,
                         &game->text[10]);
    atari_text_list_init(&game->text[9],
                         0,
                         "On the bottom is inscribed:\nDC6F8203351 (READ ERROR).",
                         speed,
                         &game->text[10]);

    {
        String choices[] = {const_string("Yes"), const_string("No")};
        Text_List *next[] = { 0, 0 };
        void (*function_pointers[])(void*) = { chapter_1_activate_node, nullptr };

        game->text[10].choice_color = BLACK;
        game->text[10].choice_backdrop_color = {};

        atari_choice_text_list_init(&game->text[10],
                                    0,
                                    "Drink the liquid?",
                                    choices,
                                    next,
                                    function_pointers,
                                    2);
    }

    atari_text_list_init(&game->text[11],
                         0,
                         "*ring ring*\r*ring ring*\r*click*",
                         speed,
                         &game->text[99]);
    atari_text_list_init(&game->text[99],
                         "Chase",
                         "H... he- hello?",
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
                         &game->text[70]);
    atari_text_list_init(&game->text[70],
                         "Chase",
                         "Okay.",
                         speed,
                         &game->text[71]);
    atari_text_list_init(&game->text[71],
                         "      ",
                         "Oh, and Chase?",
                         speed,
                         &game->text[23]);
    atari_text_list_init(&game->text[23],
                         "Chase",
                         "...\rI- I didn't give you my...",
                         speed,
                         &game->text[72]);
    atari_text_list_init(&game->text[72],
                         "      ",
                         "Open the door.",
                         speed,
                         &game->text[73]);
    atari_text_list_init(&game->text[73],
                         0,
                         "*click*",
                         speed,
                         nullptr);

    speed = 10;

    atari_mid_text_list_init(&game->text[24],
                             "the insurmountable veil\nstands firm, made of\ndiamond bars.",
                             &game->text[25]);
    atari_mid_text_list_init(&game->text[25],
                             "i try to peek through\nthe gaps to find her.\rto quench my thirst.",
                             &game->text[26]);
    atari_mid_text_list_init(&game->text[26],
                             "i don't know what i see,\rbecause there is\nnothing visible.",
                             &game->text[27]);
    atari_mid_text_list_init(&game->text[27],
                             "i don't know how to\nname you, because I\ndon't know what\nyou are.",
                             &game->text[28]);
    atari_mid_text_list_init(&game->text[28],
                             "should anyone tell me\nyou are named by\nthis name or that name,\ri know it must not\nbe.",
                             &game->text[29]);
    atari_mid_text_list_init(&game->text[29],
                             "because the veil beyond\nwhich you are suceeds\nthe boundary of names\nitself.",
                             &game->text[30]);
    atari_mid_text_list_init(&game->text[30],
                             "but if you do exist,\r\n... please help me.",
                             nullptr);

    chapter_1_apartment_text(&game->text[31],
                             "He tried getting out\nof bed.",
                             &game->text[32]);
    chapter_1_apartment_text(&game->text[32],
                             "Maybe next time.",
                             nullptr);
    game->text[32].scroll_type = LetterByLetter;
    game->text[32].callbacks[0] = set_player_alarm;

    speed = 30;

    chapter_1_call_text(&game->text[35],
                        0,
                        "*ring ring*\r*click*",
                        speed,
                        &game->text[36]);
    chapter_1_call_text(&game->text[36],
                        "Chase",
                        "Uh hello?",
                        speed,
                        &game->text[37]);
    chapter_1_call_text(&game->text[37],
                        "Eleanor",
                        "Heyyyy Chase!!\rI got someone for you.",
                        speed,
                        &game->text[38]);
    chapter_1_call_text(&game->text[38],
                        "Chase",
                        "Uhhh what?",
                        speed,
                        &game->text[39]);
    chapter_1_call_text(&game->text[39],
                        "Eleanor",
                        "Y'know, SOMEONE!!!",
                        speed,
                        &game->text[40]);
    chapter_1_call_text(&game->text[40],
                        "Chase",
                        "Does this really require\na phone call?",
                        speed,
                        &game->text[41]);
    chapter_1_call_text(&game->text[41],
                        "Chase",
                        "You could've sent me a\ntext that I would have\npretended not to read.",
                        speed,
                        &game->text[42]);
    chapter_1_call_text(&game->text[42],
                        "Eleanor",
                        "UGH. FINE.\rYou're an ass, you know\nthat?!",
                        speed,
                        &game->text[43]);
    chapter_1_call_text(&game->text[43],
                        "Chase",
                        "Waaait hold on,\rI've been trying to get\nup for the past 45 minutes.",
                        speed,
                        &game->text[44]);
    chapter_1_call_text(&game->text[44],
                        "Chase",
                        "I'm just cranky.\rWe're already here.\nWho do you got?",
                        speed,
                        &game->text[45]);
    chapter_1_call_text(&game->text[45],
                        "Eleanor",
                        "Penny.",
                        speed,
                        &game->text[46]);
    chapter_1_call_text(&game->text[46],
                        "Chase",
                        "Penny?\rUh, Jason's Penny?",
                        speed,
                        &game->text[47]);
    chapter_1_call_text(&game->text[47],
                        "Eleanor",
                        "Come on, she's perfect.\rShe even composes like\nyou!!!",
                        speed,
                        &game->text[48]);
    chapter_1_call_text(&game->text[48],
                        "Chase",
                        "Oh really?\rWhat type of music?",
                        speed,
                        &game->text[49]);
    chapter_1_call_text(&game->text[49],
                        "Eleanor",
                        "Talk to her yourself and\nfind out.",
                        speed,
                        &game->text[50]);
    chapter_1_call_text(&game->text[50],
                        "Chase",
                        "Can I state the obvious\nhere sis?\r",
                        speed,
                        &game->text[51]);
    chapter_1_call_text(&game->text[51],
                        "Chase",
                        "It's JASON's Penny.",
                        speed,
                        &game->text[52]);
    chapter_1_call_text(&game->text[52],
                        "Eleanor",
                        "Uh, the same Jason that\nswears at her and treats\nher like shit?",
                        speed,
                        &game->text[53]);
    chapter_1_call_text(&game->text[53],
                        "Eleanor",
                        "They broke up. They do\nthat like every 5 days.\rIt doesn't matter, trust me.",
                        speed,
                        &game->text[54]);
    chapter_1_call_text(&game->text[54],
                        "Chase",
                        "...",
                        speed,
                        &game->text[55]);
    chapter_1_call_text(&game->text[55],
                        "Eleanor",
                        "Soooo what do you think\nbro?",
                        speed,
                        &game->text[56]);
    chapter_1_call_text(&game->text[56],
                        "Chase",
                        "...\r...\rI'll think about it.",
                        speed,
                        &game->text[57]);
    chapter_1_call_text(&game->text[57],
                        "Eleanor",
                        "*sigh*\rWe all know what that\nmea--",
                        speed,
                        &game->text[58]);
    chapter_1_call_text(&game->text[58],
                        0,
                        "*click*",
                        speed,
                        nullptr);

    game->text[58].callbacks[0] = chapter_1_end;

    {
        String choices[] = {const_string("Yes"), const_string("No")};
        Text_List *next[] = { 0, 0 };
        void (*function_pointers[])(void*) = {
            chapter_1_start_prayer,
            nullptr
        };

        game->text[60].choice_color = BLACK;
        game->text[60].choice_backdrop_color = {};

        atari_choice_text_list_init(&game->text[60],
                                    0,
                                    "Pray to God?",
                                    choices,
                                    next,
                                    function_pointers,
                                    2);
    }

    atari_text_list_init(&game->text[61],
                         "Chase",
                         "...\r...\r...",
                         speed,
                         &game->text[62]);
    atari_text_list_init(&game->text[62],
                         "Chase",
                         "I don't hear anyone.",
                         speed,
                         nullptr);
    game->text[62].callbacks[0] = chapter_1_end_prayer;

    atari_text_list_init(&game->text[65],
                         0,
                         "It's a window.\rIt feels strange.\rChase feels watched.",
                         speed,
                         nullptr);

    Entity *e = chapter_1_make_entity(ENTITY_WINDOW, render_width/2 - 16, render_height/5);
    array_add(&game->entities, e);

    level->prayer_mat = 0;

    //level->state = LEVEL_CHAPTER_1_STATE_CRAWLING;
    //chapter_1_goto_3d(game);
}

void chapter_1_deinit(Game *game) {
    Level_Chapter_1 *level = (Level_Chapter_1 *)game->level;
    UnloadModel(level->scene_3d);
}

void add_cactuses_randomly(Array<Entity*> *entities, size_t cactus_count) {
    for (int i = 0; i < cactus_count; i++) {
        int x = rand()%(render_width - 8);
        int y = rand()%(render_height - 8);

        Entity_Type type = (rand_float() < 0.2) ? ENTITY_CACTUS : ENTITY_ROCK;

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
    Entity *player = level->player;//entities_get_player(&game->entities);

    bool within_region = is_player_close_to_entity(player, e, 8);

    switch (e->type) {
        case ENTITY_PHONE: {
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
        case ENTITY_PRAYER_MAT: {
            if (keyboard_focus(game) == NO_KEYBOARD_FOCUS && within_region) {
                if (is_action_pressed() && level->prayer_fader == 0) {
                    game->current = &game->text[60];
                }
            }
        } break;
        case ENTITY_WINDOW: {
            if (keyboard_focus(game) == NO_KEYBOARD_FOCUS && within_region) {
                if (is_action_pressed()) {
                    game->current = &game->text[65];
                }
            }
        } break;
        case ENTITY_NODE: {
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
            if (level->prayer_fader > 0) break;

            if (!e->chap_1_player.huffing_and_puffing &&
                level->state == LEVEL_CHAPTER_1_STATE_APARTMENT &&
                e->alarm[1] == 0)
            {
                game->current = &game->text[31];
                e->chap_1_player.huffing_and_puffing = true;
            }

            // this no longer occurs.
            /*
            if (e->chap_1_player.huffing_and_puffing) {
                if (level->about_to_exit && e->alarm[2] == 0) {
                    level->state = LEVEL_CHAPTER_1_PHONE_CALL;
                    level->background_color = BLACK;
                    add_event(game, chapter_1_start_phone_call, 3);
                }
                break;
            }
            */

            float player_speed = 30;

            int dir_x = input_movement_x_axis_int(dt);//key_right() - key_left();
            int dir_y = input_movement_y_axis_int(dt);//key_down()  - key_up();

            if (level->state == LEVEL_CHAPTER_1_STATE_FOREST)
                dir_x = 0;

            if (level->state == LEVEL_CHAPTER_1_STATE_APARTMENT)
                dir_x = dir_y = 0;

            e->chap_1_player.dir_x = dir_x;
            e->chap_1_player.dir_y = dir_y;

            if (e->alarm[0] == 0) {
                e->chap_1_player.is_hurt = false;
            }

            //e->chap_1_player.walking_state = PLAYER_WALKING_STATE_CRAWLING;

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

#ifdef DEBUG
            if (IsKeyDown(KEY_P))
                player_speed *= 10;
#endif

            Vector2 velocity = { dir_x * player_speed * dt, dir_y * player_speed * dt };
            /*
            velocity = Vector2Normalize(velocity);

            print_vec2(velocity);
            printf("\n");
            */

            apply_velocity(e, velocity, &game->entities);

            if (level->state == LEVEL_CHAPTER_1_STATE_FOREST) {
                if (e->pos.y < 95) {
                    chapter_1_goto_3d(game);

                    e->chap_1_player.walking_state = PLAYER_WALKING_STATE_SLOWED_1;
                    //game->post_processing.type = POST_PROCESSING_PASSTHROUGH;

                    post_process_vhs_set_intensity(&game->post_processing.vhs, VHS_INTENSITY_MEDIUM);
                    game->post_processing.vhs.vignette_mix = 0.5f;
                    game->post_processing.vhs.vignette_intensity = 1;

                    e->alarm[1] = 3;

                    for (int i = 0; i < game->entities.length; i++) {
                        Entity *entity = game->entities.data[i];
                        if (entity->type != ENTITY_PLAYER) {
                            free_entity(entity);
                            array_remove(&game->entities, i--);
                        }
                    }
                }
            } else if (level->state == LEVEL_CHAPTER_1_STATE_APARTMENT) {
                if (e->pos.y < 50)
                    e->pos.y = 50;
            }

            if (dir_x != 0 || dir_y != 0) {
                float length = Vector2Length(Vector2Subtract(e->pos, e->chap_1_player.stored_pos));
                float required_distance = 12;

                if (abs(dir_x) == abs(dir_y)) { // diagnoal
                    required_distance *= sqrtf(2);
                }

                if (level->state != LEVEL_CHAPTER_1_STATE_APARTMENT && length >= required_distance) {
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

                    play_sound((Sound_ID)(SOUND_SAND_FOOTSTEP_1 + rand()%4));
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

                if (dir != DIRECTION_INVALID) {
                    // reset the stored_pos which stores the last
                    // position of a footstep. if i dont do this
                    // then a footstep is made every time the screen
                    // has scrolled.
                    e->chap_1_player.stored_pos = e->pos;
                }

                if (!(only_allow_up_scroll && !scrolled_up) && (stored_x != e->pos.x || stored_y != e->pos.y)) {
                    if (level->state == LEVEL_CHAPTER_1_STATE_CRAWLING) {
                        level->screens_scrolled_while_crawling++;
                    }

                    level->screens_scrolled++;

                    if (player->chap_1_player.walking_state == PLAYER_WALKING_STATE_CRAWLING) {
                        level->state = LEVEL_CHAPTER_1_STATE_CRAWLING;
                        level->music_pitch = 0.5;
                    }

                    if (level->state == LEVEL_CHAPTER_1_STATE_PHONE_WAS_CALLED) {
                        Direction *directions = level->last_movements;

                        int index = 0;

                        for (int i = 0; i < 4; i++) {
                            if (directions[i] == DIRECTION_INVALID) {
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

                    for (int i = 0; i < game->entities.length; i++) {
                        Entity *entity = game->entities.data[i];
                        Entity_Type type = entity->type;

                        if (type == ENTITY_CACTUS || type == ENTITY_ROCK ||
                            type == ENTITY_FOOTSTEPS || type == ENTITY_BLOOD ||
                            type == ENTITY_NODE || type == ENTITY_PHONE ||
                            type == ENTITY_WINDOW || type == ENTITY_PRAYER_MAT)
                        {
                            free_entity(entity);
                            array_remove(&game->entities, i--);
                        }
                    }

                    Node_Type node_type = NODE_INVALID;

                    bool make_phone = false;
                    bool make_prayer_mat = false;

                    switch (level->screens_scrolled) {
                        case 4: {
                            node_type = NODE_FIRST;
                        } break;
                        case 9: {
                            make_prayer_mat = true;
                        } break;
                        case 8: {
                            node_type = NODE_SECOND;
                        } break;
                        case 10: {
                            make_phone = true;
                            level->state = LEVEL_CHAPTER_1_STATE_PHONE_EXISTS;
                        } break;
                    }

                    if (make_prayer_mat) {
                        level->prayer_mat = chapter_1_make_entity(ENTITY_PRAYER_MAT, render_width/2 - 6, render_height/2 - 11);
                        array_add(&game->entities, level->prayer_mat);
                    }

                    if (level->screens_scrolled_while_crawling >= 2 &&
                        level->final_text_is_done)
                    {
                        level->state = LEVEL_CHAPTER_1_STATE_FOREST;
                        e->pos.x = render_width/2 - entity_texture_width(e) / 2;

                        //level->music_pitch = 1;

                        /*
                        post_process_vhs_set_intensity(&game->post_processing.vhs, VHS_INTENSITY_HIGH);
                        game->post_processing.vhs.vignette_intensity = 1;
                        game->post_processing.vhs.vignette_mix = 1;
                        */

                        //game->post_processing.type = POST_PROCESSING_CRT;
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
                    if (level->state != LEVEL_CHAPTER_1_STATE_CRAWLING && level->state != LEVEL_CHAPTER_1_STATE_FOREST) {
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
                int time_loc = GetShaderLocation(level->flashing_shader, "time");
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

            int xoff = 0;
            int yoff = 0;

            if (level->prayer_fader > 0) {
                texture = &atari_assets.textures[23];
            }

            if (level->state == LEVEL_CHAPTER_1_STATE_FOREST || level->state == LEVEL_CHAPTER_1_STATE_APARTMENT) {
                e->texture_id = 3;
                texture = &atari_assets.textures[3];
                xoff += 4;
            }

            if (texture) {
                if (e->texture_id != 3 && e->chap_1_player.walking_state == PLAYER_WALKING_STATE_CRAWLING) {
                    static float x_sign = 1;
                    static float y_sign = 1;

                    if (e->chap_1_player.dir_x != 0)
                        x_sign = e->chap_1_player.dir_x;

                    if (e->chap_1_player.dir_x != 0 ||
                        e->chap_1_player.dir_y != 0)
                    {
                        y_sign = (int)(GetTime())%2==0 ? 1 : -1;
                    }

                    Rectangle src = {
                        0, 0,
                        x_sign * entity_texture_width(e),
                        y_sign * entity_texture_height(e),
                    };

                    Rectangle dst = {
                        floor(e->pos.x), floor(e->pos.y),
                        (float)entity_texture_width(e),
                        (float)entity_texture_height(e),
                    };

                    DrawTexturePro(*texture, src, dst, {}, 0, WHITE);
                } else {
                    DrawTexture(*texture, x+xoff, y+yoff, WHITE);
                }
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

    SetMusicPitch(game_audio.musics[MUSIC_DESERT_AMBIENCE], level->music_pitch);

    if (level->state == LEVEL_CHAPTER_1_PHONE_CALL || (level->intro && level->god_index == -1) || level->before_first_text) {
        return;
    }

    if (level->state == LEVEL_CHAPTER_1_STATE_3D) {
        if (level->move_3d) {
            float speed = 1;
#ifdef DEBUG
            speed = 1;
#endif

            level->camera.position.x -= speed * 0.25f * dt;
            level->camera.target.x   -= speed * 0.25f * dt;

            level->door_t += speed * 0.05f * dt;
            if (level->door_t > 1) level->door_t = 1;

            if (level->camera.position.x < -5.75f) {
                level->state = LEVEL_CHAPTER_1_PHONE_CALL;
                level->background_color = BLACK;
                stop_music();
                game->post_processing.type = POST_PROCESSING_PASSTHROUGH;
                add_event(game, chapter_1_start_phone_call, 3);
            }
        }

        level->alpha_3d += 5 * dt;
        if (level->alpha_3d > 1)
            level->alpha_3d = 1;

        return;
    }

    if (level->intro && level->god_index != -1) {
        level->god_scroll += 8 * dt;

        if (level->god_scroll > chapter_1_god_text_y_end[level->god_index])
            level->god_scroll = render_height;

        if (is_action_pressed()) {
            if (level->god_scroll == render_height) {
                level->god_index++;
                if (level->god_index > 1) {
                    level->god_index = -1;

                    float duration = 4;
#ifdef DEBUG
                    duration = 1;
#endif
                    add_event(game, chapter_1_end_intro, duration);

                    post_process_vhs_set_intensity(&game->post_processing.vhs, VHS_INTENSITY_LOW);
                }
                level->god_scroll = chapter_1_god_text_y[level->god_index];
            } else {
#ifdef DEBUG
                level->god_scroll = render_height;
#endif
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

    if (game->current == &game->text[57] &&
        is_text_list_at_end(game->current))
    {
        game->current = game->current->next[0];
    }

    float diff = level->prayer_fader_to - level->prayer_fader;

    if (diff != 0) {
        int dir = sign(diff);
        const float speed = 0.25f * dt;
        level->prayer_fader += dir * speed;

        if (dir == 1) {
            if (level->prayer_fader > level->prayer_fader_to)
                level->prayer_fader = level->prayer_fader_to;
        } else {
            if (level->prayer_fader < level->prayer_fader_to)
                level->prayer_fader = level->prayer_fader_to;
        }
    }
}

void chapter_1_draw(Game *game) {
    Level_Chapter_1 *level = (Level_Chapter_1 *)game->level;

    if (level->state == LEVEL_CHAPTER_1_STATE_3D) {
        ClearBackground(BLACK);

        BeginMode3D(level->camera);

        DrawModel(level->scene_3d, {}, 1, WHITE);
        DrawModelEx(level->scene_door, {-5.51753f, 0, 0.5}, {0,1,0}, 180 * level->door_t * level->door_t, {1,1,1}, WHITE);

        EndMode3D();

        Texture2D *textures = atari_assets.textures;

        if (!level->hide_2d_elements) {
            Entity *player = level->player;

            Color bg = WHITE;
            bg.a = (1 - level->alpha_3d) * 255;

            DrawTexture(textures[32], 0, 0, bg);

            DrawTexture(textures[3], player->pos.x+4, player->pos.y, WHITE);

            chapter_1_draw_end_scene();
        }

        if (level->camera.position.x < -3) {
            DrawTexture(textures[33], 0, 0, WHITE);
        }
        return;
    }

    ClearBackground(level->background_color);

    if (level->state == LEVEL_CHAPTER_1_PHONE_CALL) return;

    {
        Texture2D *textures = atari_assets.textures;
        DrawTexture(textures[32], 0, 0, WHITE);
    }


    if (level->intro) {
        if (level->god_index == -1) {
            ClearBackground(BLACK);
            return;
        }

        DrawTexture(atari_assets.textures[17 + level->god_index], 0, 0, WHITE);
        DrawRectangle(0, level->god_scroll, render_width, render_height, {0,0,0,255});

        if (level->god_scroll == render_height) {
            float size = 5;
            float pad = size*2;

            Vector2 v1 = { render_width - size - pad, render_height - size * 2 - pad/2};
            Vector2 v2 = { v1.x + size/2.f, v1.y + size};
            Vector2 v3 = { v1.x + size , v1.y };

            DrawTriangle(v1, v2, v3, WHITE);
        }
        return;
    }

    Texture2D *textures = atari_assets.textures;

    switch (level->state) {
        case LEVEL_CHAPTER_1_STATE_FOREST: {
            // 51, 26
            // 154, 64
            // 51, 118
            // 149, 116
            // 102, 19

            chapter_1_draw_end_scene();
        } break;
        case LEVEL_CHAPTER_1_STATE_APARTMENT: {
            DrawTexture(textures[15], 0, 0, WHITE);
        } break;
    }

    if (level->prayer_fader > 0) {
        Color c = {};
        c.a = level->prayer_fader * 255;

        DrawRectangle(0, 0, render_width, render_height, c);

        if (level->prayer_mat)
            chapter_1_entity_draw(level->prayer_mat, game);
        chapter_1_entity_draw(level->player, game);
    } else {
        sort_entities(&game->entities);

        size_t entity_count = game->entities.length;

        // Draw the entities
        for (int i = 0; i < entity_count; i++) {
            Entity *e = game->entities.data[i];
            chapter_1_entity_draw(e, game);
        }
    }
}

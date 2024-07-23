#define DINNER_SEAT_POSITION {898, 101}

enum Chapter_2_Area {
    CHAPTER_2_AREA_HYATT_1,
    CHAPTER_2_AREA_BATHROOM,
};

enum Chapter_2_Penny_State {
    CHAPTER_2_PENNY_STATE_INVALID,
    CHAPTER_2_PENNY_STATE_WALKING, // camera follows penny
    CHAPTER_2_PENNY_STATE_STOP_FIRST,
    CHAPTER_2_PENNY_STATE_FULLSCREEN_1,
    CHAPTER_2_PENNY_STATE_FULLSCREEN_2,
    CHAPTER_2_PENNY_STATE_STOP_SECOND,
    CHAPTER_2_PENNY_STATE_THE_REST, // camera follows penny
    CHAPTER_2_PENNY_STATE_STOP,    // stops at the group of girls
    CHAPTER_2_PENNY_STATE_DONE,
};

struct Level_Chapter_2 {
    Chapter_2_Area current_area;
    Entity *player; // quick access to the player
    Entity *penny;

    Chapter_2_Penny_State penny_state;

    float final_dialogue_alarm;

    bool show_window; // TODO: Show the window in fullscreen at the very end.
    bool window_popup;
    bool mirror_popup;
    int  bathroom_state;

    int width, height;

    Camera2D camera;
};

bool draw_walls = false;

Texture2D *chapter_2_get_texture_from_area(Chapter_2_Area current_area) {
    Texture2D *result = 0;

    int background_texture = -1;
    switch (current_area) {
        case CHAPTER_2_AREA_HYATT_1: {
            background_texture = 1;
        } break;
        case CHAPTER_2_AREA_BATHROOM: {
            background_texture = 10;
        } break;
    }

    if (background_texture != -1) {
        result = &atari_assets.textures[background_texture];
    }

    return result;
}

void chapter_2_window_text(bool scroll, Text_List *list, char *line,
                           Text_List *next)
{
    list->font         = &atari_font;
    list->font_spacing = 1;
    list->scale        = 0.125;
    list->scroll_speed = 10;
    //list->alpha_speed  = 0.5;
    list->color        = RED;
    list->center_text  = true;
    if (scroll)
        list->scroll_type = LetterByLetter;
    else
        list->scroll_type  = EntireLine;
    list->render_type  = Bare;
    list->location     = Middle;
    list->take_keyboard_focus = true;

    text_list_init(list, 0, line, next);
}

Entity *chapter_2_make_entity(Entity_Type type, float x, float y) {
    Entity *result = (Entity *)calloc(1, sizeof(Entity));

    result->type = type;
    result->pos = {x, y};

    switch (type) {
        case ENTITY_PLAYER: {
            result->texture_id = 0;
        } break;
        case ENTITY_CHAP_2_TABLE: {
            result->texture_id = 5;
        } break;
        case ENTITY_CHAP_2_RANDOM_GUY: {
            result->texture_id = 2;
        } break;
        case ENTITY_CHAP_2_PENNY: {
            result->texture_id = 7;
        } break;
        case ENTITY_CHAP_2_JAKE: case ENTITY_CHAP_2_MIKE:
        case ENTITY_CHAP_2_BOUNCER: {
            result->texture_id = 2;
            result->has_dialogue = true;
        } break;
        case ENTITY_CHAP_2_RANDOM_GIRL: case ENTITY_CHAP_2_JESSICA: {
            result->texture_id = 4;
        } break;
        case ENTITY_CHAP_2_ERICA: case ENTITY_CHAP_2_AMELIA:
        case ENTITY_CHAP_2_LUNA:
        case ENTITY_CHAP_2_ISABELLE: case ENTITY_CHAP_2_CAMILA: 
        case ENTITY_CHAP_2_OLIVIA: case ENTITY_CHAP_2_AVA: {
            result->texture_id = 4;
            result->has_dialogue = true;
        } break;
        case ENTITY_CHAP_2_ELEANOR: {
            result->texture_id = 6;
            result->has_dialogue = true;
        } break;
        case ENTITY_CHAP_2_BARTENDER: {
            result->texture_id = 3;
            result->has_dialogue = true;
        } break;
        default: {
            result->texture_id = -1;
        } break;
    }

    float texture_width  = entity_texture_width(result);
    float texture_height = entity_texture_height(result);

    if (type == ENTITY_CHAP_2_TABLE) {
        result->base_collider = {
            0,
            texture_height / 3,
            texture_width,
            2 * texture_height / 3
        };
    } else {
        result->base_collider = {
            0,
            3 * texture_height / 4,
            texture_width,
            texture_height / 4
        };
    }

    return result;
}

void add_wall(Array<Entity*> *entities, Rectangle r) {
    Entity *wall = chapter_2_make_entity(ENTITY_WALL, r.x, r.y);
    wall->base_collider.width  = r.width;
    wall->base_collider.height = r.height;

    array_add<Entity*>(entities, wall);
}

void add_door(Array<Entity*> *entities, Rectangle r) {
    Entity *door = chapter_2_make_entity(ENTITY_CHAP_2_DOOR, r.x, r.y);
    door->base_collider.width  = r.width;
    door->base_collider.height = r.height;
    door->chap_2_door.active = true;

    array_add<Entity*>(entities, door);
}

void chapter_2_end_bouncer_text(void *game_ptr) {
    Game_Atari *game = (Game_Atari *)game_ptr;

    Entity *bouncer = entities_find_from_type(&game->entities, ENTITY_CHAP_2_BOUNCER);
    if (bouncer) {
        bouncer->alarm[0] = 0.5;
    } else {
        assert(false);
    }

    Entity *door = entities_find_from_type(&game->entities,
                                           ENTITY_CHAP_2_DOOR);
    if (door) {
        door->chap_2_door.active = false;
    } else {
        assert(false);
    }
}

void chapter_2_sit_down(void *game_ptr) {
    Game_Atari *game = (Game_Atari *)game_ptr;
    Level_Chapter_2 *level = (Level_Chapter_2 *)game->level;

    Entity *player = level->player;
    Entity *eleanor = entities_find_from_type(&game->entities,
                                              ENTITY_CHAP_2_ELEANOR);

    eleanor->dialogue_state = 2;

    assert(player->chap_2_player.sitting_state == 0);

    player->chap_2_player.sitting_state = 1;
    player->pos = DINNER_SEAT_POSITION;

    player->alarm[0] = 1;
}

void chapter_2_end_eleanor_first_text(void *game_ptr) {
    Game_Atari *game = (Game_Atari *)game_ptr;
    Level_Chapter_2 *level = (Level_Chapter_2 *)game->level;

    Entity *player = level->player;

    player->chap_2_player.sitting_state = 2;
    player->alarm[0] = 2;
}

void chapter_2_end_eleanor_second_text(void *game_ptr) {
    Game_Atari *game = (Game_Atari *)game_ptr;
    Level_Chapter_2 *level = (Level_Chapter_2 *)game->level;

    level->penny_state = CHAPTER_2_PENNY_STATE_WALKING;
    level->penny = chapter_2_make_entity(ENTITY_CHAP_2_PENNY, 620, 70);
    level->penny->alarm[0] = 1;
    array_add<Entity*>(&game->entities, level->penny);
}

void chapter_2_end_eleanor_third_text(void *game_ptr) {
    Game_Atari *game = (Game_Atari *)game_ptr;
    Level_Chapter_2 *level = (Level_Chapter_2 *)game->level;

    Entity *player = level->player;
    player->chap_2_player.sitting_state = false;
    player->chap_2_player.speed_penny = 1;
}

void chapter_2_end_fade(void *game_ptr) {
    Game_Atari *game = (Game_Atari *)game_ptr;
    atari_queue_deinit_and_goto_intro(game);
}

void chapter_2_init(Game_Atari *game) {
    Level_Chapter_2 *level = (Level_Chapter_2 *)game->level;

    Texture2D *textures = atari_assets.textures;
    textures[0] = load_texture("art/player.png");
    textures[1] = load_texture("art/hyatt_1.png");
    textures[2] = load_texture("art/guy.png");
    textures[3] = load_texture("art/bartender.png");
    textures[4] = load_texture("art/girl.png");
    textures[5] = load_texture("art/dinner_table.png");
    textures[6] = load_texture("art/real_girl.png");
    textures[7] = load_texture("art/penny.png");
    textures[8] = load_texture("art/fullscreen_1.png");
    textures[9] = load_texture("art/fullscreen_2.png");
    textures[10] = load_texture("art/bathroom.png");
    textures[11] = load_texture("art/open_window.png");

    game->entities = make_array<Entity*>(20);

    Texture *t = chapter_2_get_texture_from_area(level->current_area);

    level->width  = t->width;
    level->height = t->height;

    level->camera.zoom = 1;
    level->camera.offset = {};
    level->camera.target = {};
    level->camera.rotation = 0;

    float speed = 30;

    atari_text_list_init(&game->text[0],
                         "Bouncer",
                         "What's your problem?",
                         speed,
                         &game->text[1]);
    atari_text_list_init(&game->text[1],
                         "Chase",
                         "I'm, uh, here for the\ndinner?\rAm I in the right place?",
                         speed,
                         &game->text[2]);
    atari_text_list_init(&game->text[2],
                         "Bouncer",
                         "... You sure you aren't\nlost, kid?\rIt's late.",
                         speed,
                         &game->text[3]);
    atari_text_list_init(&game->text[3],
                         "Chase",
                         "...\rUm...\rI think-",
                         speed,
                         &game->text[4]);
    atari_text_list_init(&game->text[4],
                         "Bouncer",
                         "Come on, you came to a\ndinner party wearing\nTHAT?",
                         speed,
                         &game->text[5]);
    atari_text_list_init(&game->text[5],
                         "Chase",
                         "Well... It's really just what\nI had laying around...",
                         speed,
                         &game->text[6]);
    atari_text_list_init(&game->text[6],
                         "Chase",
                         "Will it be a problem, sir?",
                         speed,
                         &game->text[7]);
    atari_text_list_init(&game->text[7],
                         "Bouncer",
                         "Whatever, get in.",
                         speed,
                         &game->text[8]);
    atari_text_list_init(&game->text[8],
                         "Chase",
                         "...\rOkay, thank you.",
                         speed,
                         nullptr);

    game->text[8].callbacks[0] = chapter_2_end_bouncer_text;

    // Bartender
    atari_text_list_init(&game->text[9],
                         "Bartender",
                         "Hey kid, what're you doin'\nhere?",
                         speed,
                         &game->text[10]);
    atari_text_list_init(&game->text[10],
                         "Bartender",
                         "Aren't you 'spossed to\nbe in school, or somethin'?",
                         speed,
                         &game->text[11]);
    atari_text_list_init(&game->text[11],
                         "Chase",
                         "I'm 23.",
                         speed,
                         &game->text[12]);
    atari_text_list_init(&game->text[12],
                         "Bartender",
                         "Haha, sure little man.\r"
                         "Get 'outta here before I\nneed to call your parents.",
                         speed,
                         nullptr);

    atari_text_list_init(&game->text[13],
                         "Bartender",
                         "Get 'outta here before I\nneed to call your parents.",
                         speed,
                         nullptr);

    // Jake and Erica
    atari_text_list_init(&game->text[14],
                         "Jake",
                         "Chase, we didn't think\nyou'd come!",
                         speed,
                         &game->text[15]);
    atari_text_list_init(&game->text[15],
                         "Chase",
                         "Yeah, well I guess I\nthought, \"Why not?\"",
                         speed,
                         &game->text[16]);
    atari_text_list_init(&game->text[16],
                         "Jake",
                         "Oh, I don't think you\nmet Erica before.",
                         speed,
                         &game->text[17]);
    atari_text_list_init(&game->text[17],
                         "Chase",
                         "No, I don't think so.\rNice to meet you.",
                         speed,
                         &game->text[18]);
    atari_text_list_init(&game->text[18],
                         "Erica",
                         "You too.",
                         speed,
                         &game->text[19]);
    atari_text_list_init(&game->text[19],
                         "Chase",
                         "...\r...",
                         speed,
                         &game->text[20]);
    atari_text_list_init(&game->text[20],
                         "Erica",
                         "...\r...",
                         speed,
                         &game->text[21]);
    atari_text_list_init(&game->text[21],
                         "Chase",
                         "*ahem* So, anyways, I'll\nfind a seat somewhere.",
                         speed,
                         &game->text[22]);
    atari_text_list_init(&game->text[22],
                         "Jake",
                         "They're over there\non the right.",
                         speed,
                         &game->text[23]);
    atari_text_list_init(&game->text[23],
                         "Chase",
                         "Alright, thanks.",
                         speed,
                         &game->text[24]);
    atari_text_list_init(&game->text[24],
                         "Jake",
                         "We'll make sure to keep\nan eye out for you.",
                         speed,
                         &game->text[25]);
    atari_text_list_init(&game->text[25],
                         "Chase",
                         "Um, alright.\rCatch you later.",
                         speed,
                         nullptr);

    // Erica
    atari_text_list_init(&game->text[26],
                         "Chase",
                         "Hi, I don't think we've\nmet before.",
                         speed,
                         &game->text[27]);
    atari_text_list_init(&game->text[27],
                         "Erica",
                         "...",
                         speed,
                         nullptr);

    // Mike and Jessica
    atari_text_list_init(&game->text[29],
                         "Mike",
                         "Chase, we didn't think\nyou'd come!",
                         speed,
                         &game->text[30]);
    atari_text_list_init(&game->text[30],
                         "Chase",
                         "I guess I just thought,\n\"why not?\"",
                         speed,
                         &game->text[31]);
    atari_text_list_init(&game->text[31],
                         "Mike",
                         "That's great.",
                         speed,
                         &game->text[32]);
    atari_text_list_init(&game->text[32],
                         "Mike",
                         "Oh, I don't think you met\nJessica before.",
                         speed,
                         &game->text[33]);
    atari_text_list_init(&game->text[33],
                         "Chase",
                         "No, I don't think so.\rNice to meet you.",
                         speed,
                         &game->text[34]);
    atari_text_list_init(&game->text[34],
                         "Jessica",
                         "You too.",
                         speed,
                         &game->text[35]);
    atari_text_list_init(&game->text[35],
                         "Chase",
                         "Mike always tells me nice\nthings about you.",
                         speed,
                         &game->text[36]);
    atari_text_list_init(&game->text[36],
                         "Chase",
                         "...\r...\r*ahem* Cool, well, I-",
                         speed,
                         &game->text[37]);
    atari_text_list_init(&game->text[37],
                         "Mike",
                         "Well, we're kind of in the\nmiddle of something here\nso if you'll excuse us.",
                         speed,
                         &game->text[38]);
    atari_text_list_init(&game->text[38],
                         "Mike",
                         "We'll catch up later,\nChase.",
                         speed,
                         &game->text[39]);
    atari_text_list_init(&game->text[39],
                         "Chase",
                         "Yeah.",
                         speed,
                         nullptr);

    // Amelia
    atari_text_list_init(&game->text[40],
                         "Chase",
                         "Hi, Amy.",
                         speed,
                         &game->text[41]);
    atari_text_list_init(&game->text[41],
                         "Amelia",
                         "Chase!\rI didn't think you'd come!",
                         speed,
                         &game->text[42]);
    atari_text_list_init(&game->text[42],
                         "Chase",
                         "Yeah, I thought why not?\rDon't have anything to\nlose.",
                         speed,
                         &game->text[43]);
    atari_text_list_init(&game->text[43],
                         "Amelia",
                         "It's good you're getting\noutside every now and\nagain!",
                         speed,
                         &game->text[44]);
    atari_text_list_init(&game->text[44],
                         "Chase",
                         "Thanks, it means a lot.\rAnyways, I'll get out of\nyour hair.",
                         speed,
                         nullptr);

    // Eleanor
    atari_text_list_init(&game->text[45],
                         "Eleanor",
                         "Hi, Chase!\rI knew you'd come.",
                         speed,
                         &game->text[46]);
    atari_text_list_init(&game->text[46],
                         "Chase",
                         "Hi Eleanor, yeah I just\nthought that I should.",
                         speed,
                         &game->text[47]);
    atari_text_list_init(&game->text[47],
                         "Chase",
                         "Oh, you don't have a date\ntoo, thank god.",
                         speed,
                         &game->text[48]);
    atari_text_list_init(&game->text[48],
                         "Chase",
                         "At least I won't look like\na loner.",
                         speed,
                         &game->text[49]);
    atari_text_list_init(&game->text[49],
                         "Eleanor",
                         "Oh don't worry,\ryou always look like that.",
                         speed,
                         &game->text[50]);
    atari_text_list_init(&game->text[50],
                         "Chase",
                         "?!?!?!!!?",
                         speed,
                         &game->text[51]);
    atari_text_list_init(&game->text[51],
                         "Eleanor",
                         "Ahahah, just kidding.",
                         speed,
                         &game->text[52]);
    {
        String choices[] = { const_string("Sure"), const_string("Look around more first") };
        Text_List *next[] = { nullptr, nullptr };
        void (*hooks[2])(void*) = {
            chapter_2_sit_down,
            nullptr
        };

        atari_choice_text_list_init(&game->text[52],
                                    "Eleanor",
                                    "You wanna have a seat\nhere?",
                                    choices,
                                    next,
                                    hooks,
                                    2);
    }

    /*
    // Paulie
    atari_text_list_init(&game->text[53],
                         "Chase",
                         "Hey, Paulie.\rWhat's up?",
                         speed,
                         &game->text[54]);
    atari_text_list_init(&game->text[54],
                         "Paulie",
                         "Oh, nothing much, y'know.\rOne thing that I was\nthinking-",
                         speed,
                         &game->text[55]);
    atari_text_list_init(&game->text[55],
                         "Paulie",
                         "8F332D706FC109EDF66B5AB\n"
                         "38ACCB5115B69EE1C01F9DE\n"
                         "E30C8139AC3549D19F89915?",
                         speed,
                         &game->text[56]);
    atari_text_list_init(&game->text[56],
                         "Paulie",
                         "6A5699D8DD0364583D77BB3\n"
                         "19AFF46309FBD517B9E4043\n"
                         "A6B57EFA95761A63C654AE!",
                         speed,
                         &game->text[57]);
    atari_text_list_init(&game->text[57],
                         "Paulie",
                         "Are you even listening\nto a word I'm saying?",
                         speed,
                         &game->text[58]);
    atari_text_list_init(&game->text[58],
                         "Chase",
                         "Huh?\rOh, sorry.",
                         speed,
                         nullptr);
                         */

    atari_text_list_init(&game->text[59],
                         "Chase",
                         "So, how's your artwork\ncoming along?",
                         speed,
                         &game->text[60]);
    atari_text_list_init(&game->text[60],
                         "Eleanor",
                         "Oh, it's really good,\nthanks for asking.",
                         speed,
                         &game->text[61]);
    atari_text_list_init(&game->text[61],
                         "Eleanor",
                         "I can tell I've been\ngetting better over time.",
                         speed,
                         &game->text[62]);
    atari_text_list_init(&game->text[62],
                         "Eleanor",
                         "Y'know, last month,\rI got commissioned by this\nbig restaurant chain.",
                         speed,
                         &game->text[63]);
    atari_text_list_init(&game->text[63],
                         "Eleanor",
                         "I basically had to make the\nlogos and flyers\nadvertising the thing.",
                         speed,
                         &game->text[64]);
    atari_text_list_init(&game->text[64],
                         "Chase",
                         "Woah cool, which\nrestaurant was it?",
                         speed,
                         &game->text[65]);
    atari_text_list_init(&game->text[65],
                         "Eleanor",
                         "The Picnic Barn.",
                         speed,
                         &game->text[66]);
    atari_text_list_init(&game->text[66],
                         "Eleanor",
                         "Damn, moving up in the\nworld...\rAnyways, continue.",
                         speed,
                         &game->text[67]);
    atari_text_list_init(&game->text[67],
                         "Eleanor",
                         "So I sent them a Google\ndrive folder with some\nvariants.",
                         speed,
                         &game->text[68]);
    atari_text_list_init(&game->text[68],
                         "Eleanor",
                         "They hated all of 'em.",
                         speed,
                         &game->text[69]);
    atari_text_list_init(&game->text[69],
                         "Chase",
                         "....!\rHow dare they!",
                         speed,
                         &game->text[70]);
    atari_text_list_init(&game->text[70],
                         "Eleanor",
                         "... Except for one...\rI put this one in as a joke,\r'cause I'm so quirky.",
                         speed,
                         &game->text[71]);
    atari_text_list_init(&game->text[71],
                         "Eleanor",
                         "But it turns out,\nthey actually loved it.",
                         speed,
                         &game->text[72]);
    atari_text_list_init(&game->text[72],
                         "Chase",
                         "What was it?",
                         speed,
                         &game->text[73]);
    atari_text_list_init(&game->text[73],
                         "Eleanor",
                         "... It was one that I used\nComic Sans for the main font.",
                         speed,
                         &game->text[74]);
    game->text[73].text[0].font = &comic_sans;
    atari_text_list_init(&game->text[74],
                         "Chase",
                         "No. Way.",
                         speed,
                         &game->text[75]);
    atari_text_list_init(&game->text[75],
                         "Eleanor",
                         "They liked it so much,\rthat they just used it.\rCheck their website.",
                         speed,
                         &game->text[76]);
    atari_text_list_init(&game->text[76],
                         "Chase",
                         "Welp.\rThat must've made you\ndie inside as an artist.",
                         speed,
                         &game->text[77]);
    atari_text_list_init(&game->text[77],
                         "Eleanor",
                         "Not really.\rI got paid beaucoup\nbucks, so I'm chilling.",
                         speed,
                         &game->text[78]);
    atari_text_list_init(&game->text[78],
                         "Eleanor",
                         "I'd work with them again.",
                         speed,
                         &game->text[79]);
    atari_text_list_init(&game->text[79],
                         "Chase",
                         "Please don't speak French\never again,\rI'll kill you.",
                         speed,
                         &game->text[120]);

    atari_text_list_init(&game->text[120],
                         "Eleanor",
                         "Hahahahah",
                         speed,
                         nullptr);

    atari_text_list_init(&game->text[80],
                         "Eleanor",
                         "Wait...",
                         speed,
                         &game->text[81]);
    atari_text_list_init(&game->text[81],
                         "Chase",
                         "Huh?",
                         speed,
                         &game->text[82]);
    atari_text_list_init(&game->text[82],
                         "Eleanor",
                         "Isn't that...?\r...",
                         speed,
                         &game->text[83]);
    atari_text_list_init(&game->text[83],
                         "Chase",
                         "Use your words, Eleanor.\rUtilise the old vocal cords.",
                         speed,
                         &game->text[84]);
    atari_text_list_init(&game->text[84],
                         "Eleanor",
                         "Isn't that Penny over\nthere?",
                         speed,
                         &game->text[85]);
    atari_text_list_init(&game->text[85],
                         "Chase",
                         "...",
                         speed,
                         nullptr);

    game->text[79].callbacks[0] = chapter_2_end_eleanor_first_text;
    game->text[85].callbacks[0] = chapter_2_end_eleanor_second_text;

    atari_text_list_init(&game->text[86],
                         "Eleanor",
                         "Maybe you should go talk\nto her.",
                         speed,
                         &game->text[87]);
    atari_text_list_init(&game->text[87],
                         "Chase",
                         "?!!!",
                         speed,
                         &game->text[88]);
    atari_text_list_init(&game->text[88],
                         "Chase",
                         "No way.",
                         speed,
                         &game->text[89]);
    atari_text_list_init(&game->text[89],
                         "Eleanor",
                         "You clearly like her.\rThis isn't primary school.",
                         speed,
                         nullptr);

    game->text[89].callbacks[0] = chapter_2_end_eleanor_third_text;

    atari_text_list_init(&game->text[90],
                         0,
                         "\"Hi Penny!\", Chase said.",
                         speed,
                         &game->text[91]);
    atari_text_list_init(&game->text[91],
                         0,
                         "Or maybe he said,\n\"Hey, what's up?\"",
                         speed,
                         &game->text[92]);
    atari_text_list_init(&game->text[92],
                         0,
                         "And then she might have\nsaid \"hi\" back.",
                         speed,
                         &game->text[93]);
    atari_text_list_init(&game->text[93],
                         0,
                         "Or she said, \"Um, who are\nyou again? I forgot.\"",
                         speed,
                         &game->text[94]);
    atari_text_list_init(&game->text[94],
                         0,
                         "But maybe she said,\n\"Would you like to\nhave a seat with me?\"",
                         speed,
                         &game->text[95]);
    atari_text_list_init(&game->text[95],
                         0,
                         "He couldn't really\nremember exactly what he\nhad said.",
                         speed,
                         &game->text[96]);
    atari_text_list_init(&game->text[96],
                         0,
                         "He said something, though.\rHe knew he definitely said\nsomething.",
                         speed,
                         &game->text[97]);
    atari_text_list_init(&game->text[97],
                         0,
                         "And she said something\nback.",
                         speed,
                         &game->text[98]);
    atari_text_list_init(&game->text[98],
                         0,
                         "He hoped he didn't look\ncreepy.",
                         speed,
                         nullptr);

    chapter_2_window_text(false,
                          &game->text[100],
                          "He prayed for courage,\rbut found only echoes.",
                          &game->text[101]);
    chapter_2_window_text(false,
                          &game->text[101],
                          "He prayed for purpose,\rbut found only echoes.",
                          &game->text[102]);
    chapter_2_window_text(false,
                          &game->text[102],
                          "He thought,\r\"I must be cured from this!\",\rHe asked,\r\"When will you reveal\nyourself to me?\"",
                          &game->text[103]);
    chapter_2_window_text(true,
                          &game->text[103],
                          "But the Lord whispered\nto him:\r"
                          "78c4f1a6506db3ff\n"
                          "ab7d9d2ccbd8866d\n"
                          "34f2616ed69da985\n"
                          "982b1ca85d0e6eb8\n"
                          "43e122f4ba89f1e1\n"
                          "ec9626b38e01dcac\n"
                          "6041947ab7793747",
                          &game->text[104]);
    chapter_2_window_text(false,
                          &game->text[104],
                          "But he couldn't understand.\rBecause it was not meant\nto be understood.",
                          nullptr);
    game->text[104].callbacks[0] = chapter_2_end_fade;

    Entity *jake    = chapter_2_make_entity(ENTITY_CHAP_2_JAKE,    638,  30);
    Entity *erica   = chapter_2_make_entity(ENTITY_CHAP_2_ERICA,   618,  14);
    Entity *mike    = chapter_2_make_entity(ENTITY_CHAP_2_MIKE,    705,  90);
    Entity *jessica = chapter_2_make_entity(ENTITY_CHAP_2_JESSICA, 705,  115);
    Entity *amelia  = chapter_2_make_entity(ENTITY_CHAP_2_AMELIA,  814,  22);
    Entity *guy     = chapter_2_make_entity(ENTITY_CHAP_2_RANDOM_GUY, 834,  42);
    Entity *amelia_1= chapter_2_make_entity(ENTITY_CHAP_2_LUNA,    192+828+50,  24);
    Entity *amelia_2= chapter_2_make_entity(ENTITY_CHAP_2_ISABELLE,192+800+50,  28);
    Entity *amelia_3= chapter_2_make_entity(ENTITY_CHAP_2_CAMILA,  192+814+50,  42);
    Entity *amelia_4= chapter_2_make_entity(ENTITY_CHAP_2_OLIVIA,  192+830+50,  42);
    Entity *amelia_5= chapter_2_make_entity(ENTITY_CHAP_2_AVA,     192+845+50,  30);
    Entity *eleanor = chapter_2_make_entity(ENTITY_CHAP_2_ELEANOR, 886,  100);
    //Entity *paulie  = chapter_2_make_entity(ENTITY_CHAP_2_PAULIE,  192+880,  95);

    Entity *bartender  = chapter_2_make_entity(ENTITY_CHAP_2_BARTENDER, 21, 190);

    Entity *bouncer = chapter_2_make_entity(ENTITY_CHAP_2_BOUNCER, 553, 80);

    level->player   = chapter_2_make_entity(ENTITY_PLAYER, 118, 287);
    //level->player   = chapter_2_make_entity(ENTITY_PLAYER, 600, 80); // skip to table section

    array_add<Entity*>(&game->entities, level->player);
    array_add<Entity*>(&game->entities, jake);
    array_add<Entity*>(&game->entities, erica);
    array_add<Entity*>(&game->entities, bartender);
    array_add<Entity*>(&game->entities, bouncer);
    array_add<Entity*>(&game->entities, mike);
    array_add<Entity*>(&game->entities, jessica);
    array_add<Entity*>(&game->entities, amelia);
    array_add<Entity*>(&game->entities, guy);
    array_add<Entity*>(&game->entities, amelia_1);
    array_add<Entity*>(&game->entities, amelia_2);
    array_add<Entity*>(&game->entities, amelia_3);
    array_add<Entity*>(&game->entities, amelia_4);
    array_add<Entity*>(&game->entities, amelia_5);
    array_add<Entity*>(&game->entities, eleanor);
    //array_add<Entity*>(&game->entities, paulie);

    array_add<Entity*>(&game->entities, chapter_2_make_entity(ENTITY_CHAP_2_TABLE, 3.75*192-24-10, 3*160/4-9));
    array_add<Entity*>(&game->entities, chapter_2_make_entity(ENTITY_CHAP_2_TABLE, 3.25*192-24+10, 160/4-9));

    array_add<Entity*>(&game->entities, chapter_2_make_entity(ENTITY_CHAP_2_TABLE, 4.75*192-24-15, 3*160/4-4));
    array_add<Entity*>(&game->entities, chapter_2_make_entity(ENTITY_CHAP_2_TABLE, 4.25*192-24+13, 160/4+2));

    array_add<Entity*>(&game->entities, chapter_2_make_entity(ENTITY_CHAP_2_TABLE, 5.75*192-24-15, 3*160/4-4));
    array_add<Entity*>(&game->entities, chapter_2_make_entity(ENTITY_CHAP_2_TABLE, 5.25*192-24+13+50, 160/4+2));

    add_wall(&game->entities, { 0, 188, 76, 27 }); // bar
    add_wall(&game->entities, { 64, 133, 19, 123 });
    add_wall(&game->entities, { 173, 133, 19, 123 });
    add_wall(&game->entities, { 189, 160, 3, 160 });
    add_wall(&game->entities, { 0, 133, 78, 10 });
    add_wall(&game->entities, { 192, 0, 192*2 + 7, 47 });
    add_wall(&game->entities, { 0, 0, 105, 22 });
    add_wall(&game->entities, { 192, 133, 192*2 + 7, 27 });
    add_wall(&game->entities, { 0, 22, 6, 132 });
    add_wall(&game->entities, { 569, 47, 14, 31});
    add_wall(&game->entities, { 569, 103, 14, 30});
    add_wall(&game->entities, { 576, 160-3, 192*3, 8});

    add_door(&game->entities, { 569, 78, 7, 25 });
}

void chapter_2_deinit(Game_Atari *game) {
    (void)game;
}

void chapter_2_entity_update(Entity *e, Game_Atari *game, float dt) {
    Level_Chapter_2 *level = (Level_Chapter_2 *)game->level;

    Entity *player = level->player;

    bool open_dialogue = is_action_pressed() &&
                         can_open_dialogue(game, e, player) &&
                         level->penny_state != CHAPTER_2_PENNY_STATE_DONE;

    switch (e->type) {
        case ENTITY_PLAYER: {
            int dir_x = key_right() - key_left();
            int dir_y = key_down()  - key_up();

            if (level->current_area == CHAPTER_2_AREA_BATHROOM) {
                e->chap_2_player.speed_penny = 0;
            }

            if (player->chap_2_player.sitting_state) {
                dir_x = dir_y = 0;

                if (!game->current && player->alarm[0] == 0) {
                    switch (player->chap_2_player.sitting_state) {
                        case 1: {
                            game->current = &game->text[59];
                        } break;
                        case 2: {
                            // Begin the part with Penny
                            game->current = &game->text[80];
                            player->chap_2_player.sitting_state = 3;
                        } break;
                    }
                }
            }

            float player_speed = 60;

            if (e->chap_2_player.speed_penny != 0) {
                Entity *penny = level->penny;

                float distance = Vector2Length(Vector2Subtract(player->pos,
                                                               penny->pos));

                player_speed = Clamp(distance * 0.25, 5, 60);
            }

            if (IsKeyDown(KEY_P)) player_speed *= 3;

            if (keyboard_focus(game) != NO_KEYBOARD_FOCUS)
                dir_x = dir_y = 0;

            Vector2 stored_position = e->pos;

            Vector2 velocity = { dir_x * player_speed * dt, dir_y * player_speed * dt };

            apply_velocity(e, velocity, &game->entities);

            int texture_width = entity_texture_width(e);
            int texture_height = entity_texture_height(e);

            e->pos.x = Clamp(e->pos.x, 0, level->width - texture_width);
            e->pos.y = Clamp(e->pos.y, 0, level->height - texture_height);
        } break;
        case ENTITY_CHAP_2_JAKE: {
            if (open_dialogue) {
                switch (e->dialogue_state) {
                    case 0: { game->current = &game->text[14]; } break;
                }
                e->dialogue_state++;
            }
        } break;
        case ENTITY_CHAP_2_ERICA: {
            if (open_dialogue) {
                switch (e->dialogue_state) {
                    case 0: { game->current = &game->text[26]; } break;
                }
                e->dialogue_state++;
            }
        } break;
        case ENTITY_CHAP_2_MIKE: {
            if (open_dialogue) {
                switch (e->dialogue_state) {
                    case 0: { game->current = &game->text[29]; } break;
                }
                e->dialogue_state++;
            }
        } break;
        case ENTITY_CHAP_2_ELEANOR: {
            if (open_dialogue) {
                switch (e->dialogue_state) {
                    case 0: {
                        game->current = &game->text[45];
                        e->dialogue_state = 1;
                    } break;
                    // TODO: This case still triggers
                    //       even after you select an option
                    //       because dialogue_state is still 1,
                    //       when it should be 2.
                    case 1: {
                        game->current = &game->text[52];
                    } break;
                }
            }
        } break;
        case ENTITY_CHAP_2_AMELIA: {
            if (open_dialogue) {
                if (e->dialogue_state == 0)
                    game->current = &game->text[40];
                e->dialogue_state++;
            }
        } break;
        /*
        case ENTITY_CHAP_2_PAULIE: {
            if (open_dialogue) {
                switch (e->dialogue_state) {
                    case 0: { game->current = &game->text[53]; } break;
                }
                e->dialogue_state++;
            }
        } break;
        */
        case ENTITY_CHAP_2_BOUNCER: {
            if (e->dialogue_state == 0 && open_dialogue) {
                game->current = &game->text[0];
                e->dialogue_state++;
            }

            if (e->alarm[0] != 0) {
                e->pos.y -= dt * 60;
            }
        } break;
        case ENTITY_CHAP_2_BARTENDER: {
            if (open_dialogue) {
                switch (e->dialogue_state) {
                    case 0: { game->current = &game->text[9];  } break;
                    case 1: { game->current = &game->text[13]; } break;
                }
                e->dialogue_state++;
            }
        } break;
        case ENTITY_CHAP_2_PENNY: {
            bool dialogue = is_player_close_to_entity(e, player, 4);

            if (dialogue && player->chap_2_player.speed_penny == 1) {
                level->current_area = CHAPTER_2_AREA_BATHROOM;
                break;
            }

            switch (level->penny_state) {
                case CHAPTER_2_PENNY_STATE_WALKING: 
                case CHAPTER_2_PENNY_STATE_THE_REST: {
                    if (e->alarm[0] == 0) {
                        Vector2 velocity = { 30 * dt, 0 };

                        if (level->penny_state == CHAPTER_2_PENNY_STATE_WALKING &&
                            fabs(e->pos.x - player->pos.x) < 1)
                        {
                            level->penny_state = CHAPTER_2_PENNY_STATE_STOP_FIRST;
                            e->alarm[0] = 2;
                        }

                        if (e->pos.x >= player->pos.x + 50) {
                            velocity.y = -10 * dt;
                        }

                        Vector2 stored_pos = e->pos;
                        apply_velocity(e, velocity, &game->entities);

                        if (e->chap_2_penny.state == 0 &&
                            e->pos.x == stored_pos.x)
                        {
                            level->penny_state = CHAPTER_2_PENNY_STATE_STOP;
                            game->current = 0;
                            e->chap_2_penny.state = 1;
                            e->alarm[0] = 2;
                        }
                    }
                } break;
                case CHAPTER_2_PENNY_STATE_STOP_FIRST: {
                    if (e->alarm[0] == 0) {
                        level->penny_state = CHAPTER_2_PENNY_STATE_FULLSCREEN_1;
                        e->alarm[0] = 2;
                    }
                } break;
                case CHAPTER_2_PENNY_STATE_STOP_SECOND: {
                    if (e->alarm[0] == 0) {
                        level->penny_state = CHAPTER_2_PENNY_STATE_THE_REST;
                        e->alarm[0] = 0;
                    }
                } break;
                case CHAPTER_2_PENNY_STATE_STOP: {
                    if (e->alarm[0] == 0) {
                        level->penny_state = CHAPTER_2_PENNY_STATE_DONE;
                        e->alarm[0] = 2;
                    }
                } break;
                case CHAPTER_2_PENNY_STATE_FULLSCREEN_1: {
                    if (e->alarm[0] == 0) {
                        level->penny_state = CHAPTER_2_PENNY_STATE_FULLSCREEN_2;
                        e->alarm[0] = 2;
                    }
                } break;
                case CHAPTER_2_PENNY_STATE_FULLSCREEN_2: {
                    if (e->alarm[0] == 0) {
                        level->penny_state = CHAPTER_2_PENNY_STATE_STOP_SECOND;
                        e->alarm[0] = 2;
                    }
                } break;
                case CHAPTER_2_PENNY_STATE_DONE: {
                    if (!e->chap_2_penny.final_text && e->alarm[0] == 0) {
                        game->current = &game->text[86];
                        e->chap_2_penny.final_text = true;
                    }
                } break;
            }
        } break;
    }

    entity_update_alarms(e, dt);
}

void chapter_2_entity_draw(Entity *e, Game_Atari *game) {
    (void)game;

    Texture *texture = entity_get_texture(e);

    if (texture) {
        DrawTexture(*texture, e->pos.x, e->pos.y, WHITE);
    } else if (draw_walls) {
        Rectangle rect = {
            e->pos.x + e->base_collider.x,
            e->pos.y + e->base_collider.y,
            e->base_collider.width,
            e->base_collider.height
        };

        DrawRectangleRec(rect, {255, 0, 0, 32});
    }
}

void chapter_2_update(Game_Atari *game, float dt) {
    size_t entity_count = game->entities.length;

    Level_Chapter_2 *level = (Level_Chapter_2 *)game->level;

    for (size_t i = 0; i < entity_count; i++) {
        Entity *e = game->entities.data[i];
        chapter_2_entity_update(e, game, dt);
    }

    if (level->current_area == CHAPTER_2_AREA_BATHROOM) {
        if (level->show_window) {
            float *alarm = &level->final_dialogue_alarm;

            if (*alarm != -1) {
                *alarm -= dt;
                if (*alarm <= 0) {
                    *alarm = -1;
                    game->current = &game->text[100];
                }
            }
        } else {
            Rectangle window_area = {
                27,
                79,
                31,
                53
            };

            Rectangle mirror_area = {
                53,
                32,
                105,
                40
            };

            Entity *p = level->player;

            Rectangle player = {
                p->pos.x + p->base_collider.x,
                p->pos.y + p->base_collider.y,
                p->base_collider.width,
                p->base_collider.height
            };

            if (level->bathroom_state == 1) {
                level->window_popup = CheckCollisionRecs(player, window_area);
            } else {
                level->window_popup = false;
            }

            if (level->window_popup && is_action_pressed()) {
                level->show_window = true;
                level->final_dialogue_alarm = 2;
            }

            level->mirror_popup = CheckCollisionRecs(player, mirror_area);
            if (level->bathroom_state != 0) level->mirror_popup = false;

            if (level->mirror_popup && game->current == 0 && is_action_pressed()) {
                game->current = &game->text[90];
                level->bathroom_state = 1;
            }

            Entity *penny = entities_find_from_type(&game->entities, ENTITY_CHAP_2_PENNY);

            level->player->pos.x = fmod(level->player->pos.x, render_width);
            level->player->pos.y = fmod(level->player->pos.y, render_height);

            if (penny) {
                for (int i = 0; i < game->entities.length; i++) {
                    Entity *e = game->entities.data[i];

                    if (e->type != ENTITY_PLAYER) {
                        free(e);
                        array_remove<Entity*>(&game->entities, i--);
                    }
                }
                level->penny = 0;

                add_wall(&game->entities, {0, 0, 39, 160});
                add_wall(&game->entities, {0, 0, 192, 37});
            }
        }
    }

    if (level->penny_state >= CHAPTER_2_PENNY_STATE_WALKING &&
        level->penny_state <= CHAPTER_2_PENNY_STATE_STOP)
    {
        level->camera.offset.x = -((int)level->penny->pos.x - render_width/2);
    } else {
        int player_x = level->player->pos.x + entity_texture_width(level->player) / 2;
        int player_y = level->player->pos.y + entity_texture_height(level->player) / 2;

        int screen_x = player_x / render_width;
        int screen_y = player_y / render_height;

        level->camera.offset.x = -screen_x * render_width;
        level->camera.offset.y = -screen_y * render_height;
    }
}

void chapter_2_draw_popup(const char *text) {
    int pad = 6;

    Vector2 size = MeasureTextEx(atari_font, text, atari_font.baseSize, 1);
    Vector2 pos = {
        render_width/2 - size.x/2,
        render_height - size.y - pad
    };
    DrawTextEx(atari_font, text, pos, atari_font.baseSize, 1, GOLD);
}

void chapter_2_draw(Game_Atari *game) {
    Level_Chapter_2 *level = (Level_Chapter_2 *)game->level;

    ClearBackground(BLACK);

    game->textbox_alpha = 200;

    if (level->penny_state == CHAPTER_2_PENNY_STATE_FULLSCREEN_1 ||
        level->penny_state == CHAPTER_2_PENNY_STATE_FULLSCREEN_2 ||
        level->show_window)
    {
        Texture2D *texture = 0;
        float scale = 2;

        if (level->show_window)
            texture = &atari_assets.textures[11], scale = 1, game->textbox_alpha = 255;
        else if (level->penny_state == CHAPTER_2_PENNY_STATE_FULLSCREEN_1)
            texture = &atari_assets.textures[8];
        else
            texture = &atari_assets.textures[9];

        assert(texture);

        DrawTextureEx(*texture, {}, 0, scale, WHITE);
    } else {
        BeginMode2D(level->camera);

        Texture *background_texture = chapter_2_get_texture_from_area(level->current_area);

        if (background_texture) {
            DrawTexture(*background_texture, 0, 0, WHITE);
        }

        sort_entities(&game->entities);

        size_t entity_count = game->entities.length;

        for (int i = 0; i < entity_count; i++) {
            Entity *e = game->entities.data[i];
            chapter_2_entity_draw(e, game);
        }

        EndMode2D();

        if (game->current == 0) {
            if (level->window_popup) {
                chapter_2_draw_popup("Look out the window");
            } else if (level->mirror_popup) {
                chapter_2_draw_popup("Stare at the mirror");
            }
        }
    }
}

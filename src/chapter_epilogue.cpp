#define EPILOGUE_NODE_MAX 64
#define EPILOGUE_DOOR_MAX 64

enum Epilogue_State {
    EPILOGUE_STATE_FIRST,
    EPILOGUE_STATE_SECOND,
    EPILOGUE_STATE_THIRD,
    EPILOGUE_STATE_FOURTH,
    EPILOGUE_STATE_ENDING
};

struct Epilogue_Node {
    Vector3 position;
    Text_List *text;
    float time_up; // the time that it's been up
    bool moving_up, moving_down;
};

struct Epilogue_Door {
    Vector3 pos;
};

struct Level_Chapter_Epilogue {
    Epilogue_State state;
    bool           transitioned;
    //bool           black_overlay; // for the fourth scene

    float          camera_height;
    Camera3D       camera;
    Shader         shader;
    Model          scene, bars, node, door, timer_model;

    float          fog_factor;
    float          fog_factor_to;

    Epilogue_Node  nodes[EPILOGUE_NODE_MAX];
    int            num_nodes;
    int            next_node_to_appear; // index
    bool           node_popup;
    Epilogue_Node *current_node;

    Epilogue_Door  doors[EPILOGUE_DOOR_MAX];
    int            num_doors;
    float          door_timer;
    bool           pause_door_timer;
    float          door_y;

    float          node_timer, node_timer_max;

    RenderTexture  timer_texture, flipped_timer_texture;
    Texture        pink_dot, orange_dot, black_texture;

    bool           door_popup;
    bool           door_strike_popup;

    bool           is_transitioning;
    float          transition_timer; // 0.0 to 1.0
};

void epilogue_start_transition(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    Level_Chapter_Epilogue *level = (Level_Chapter_Epilogue*)game->level;
    level->is_transitioning = true;

    //level->black_overlay = true;
}

void epilogue_start_final_conversation(Game *game) {
    game->current = &game->text[70];
}

void epilogue_goto_credits(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    add_event(game, atari_queue_deinit_and_goto_intro, 5);
}

void epilogue_start_congratulations_text(Game *game) {
    game->current = &game->text[64];
}

void epilogue_raise_node_last_scene(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    Level_Chapter_Epilogue *level = (Level_Chapter_Epilogue*)game->level;

    level->nodes[0].moving_up = true;
}

void chapter_epilogue_init(Game *game) {
    Level_Chapter_Epilogue *level = (Level_Chapter_Epilogue*)game->level;

    DisableCursor();

    render_width  = DIM_3D_WIDTH;
    render_height = DIM_3D_HEIGHT;

    game->render_state = RENDER_STATE_3D;

    game->textbox_target = LoadRenderTexture(render_width, render_height);
    game->textbox_alpha = 220;

    level->fog_factor = level->fog_factor_to = 1.0f/28.0f;

    level->camera_height = 1.67f;

    level->camera.position = {-2.86f, level->camera_height, -26.26f};
    level->camera.target = {2.63f, 7.40f, -30.50f};

    level->camera.up         = { 0, 1, 0 };
    level->camera.fovy       = FOV_DEFAULT;
    level->camera.projection = CAMERA_PERSPECTIVE;

    level->shader      = LoadShader(RES_DIR "shaders/basic.vs", RES_DIR "shaders/fog.fs");
    level->scene       = LoadModel(RES_DIR "models/epilogue.glb");
    level->bars        = LoadModel(RES_DIR "models/epilogue_bars.glb");
    level->node        = LoadModel(RES_DIR "models/node.glb");
    level->door        = LoadModel(RES_DIR "models/epilogue_door.glb");
    level->timer_model = LoadModel(RES_DIR "models/clock.glb");

    level->pink_dot      = LoadTexture(RES_DIR "models/pink_dot.png");
    level->orange_dot    = LoadTexture(RES_DIR "models/orange_dot.png");
    level->black_texture = LoadTexture(RES_DIR "models/black.png");

    SetTextureFilter(level->pink_dot, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(level->orange_dot, TEXTURE_FILTER_BILINEAR);

    level->node_timer_max = 8;
    level->node_timer = 8;

    level->door_timer = 120;

    level->next_node_to_appear = 0;

    model_set_shader(&level->bars,  level->shader);
    model_set_shader(&level->scene, level->shader);
    model_set_shader(&level->node,  level->shader);
    model_set_shader(&level->door,  level->shader);
    model_set_shader(&level->timer_model, level->shader);

    model_set_bilinear(&level->scene);
    model_set_bilinear(&level->bars);
    model_set_bilinear(&level->node);

    float door_distance = 28.7f;
    level->num_doors = 4;

    level->doors[0].pos = { -door_distance, 0, 0 };
    level->doors[1].pos = { +door_distance, 0, 0 };
    level->doors[2].pos = { 0, 0, -door_distance };
    level->doors[3].pos = { 0, 0, +door_distance };

    level->timer_texture = LoadRenderTexture(64, 24);
    level->flipped_timer_texture = LoadRenderTexture(64, 24);

    //SetTextureFilter(level->timer_texture.texture, TEXTURE_FILTER_BILINEAR);

    float speed = 30;

    atari_text_list_init(&game->text[0],
                         0,
                         "On the bottom is inscribed:\r[Hanging out with friends] (READ SUCCESS)",
                         speed,
                         &game->text[1]);
    atari_text_list_init(&game->text[4],
                         0,
                         "On the bottom is inscribed:\r[Learning an instrument] (READ SUCCESS)",
                         speed,
                         &game->text[5]);
    atari_text_list_init(&game->text[7],
                         0,
                         "On the bottom is inscribed:\r[Taking an evening walk] (READ SUCCESS)",
                         speed,
                         &game->text[8]);
    atari_text_list_init(&game->text[10],
                         0,
                         "On the bottom is inscribed:\r[Watching a movie with your friends] (READ\nSUCCESS)",
                         speed,
                         &game->text[11]);
    atari_text_list_init(&game->text[13],
                         0,
                         "On the bottom is inscribed:\r[Composing more music] (READ SUCCESS)",
                         speed,
                         &game->text[14]);
    atari_text_list_init(&game->text[40],
                         0,
                         "On the bottom is inscribed:\r[Learning how to knit] (READ SUCCESS)",
                         speed,
                         &game->text[41]);
    atari_text_list_init(&game->text[43],
                         0,
                         "On the bottom is inscribed:\r[Meeting new people] (READ SUCCESS)",
                         speed,
                         &game->text[44]);
    atari_text_list_init(&game->text[46],
                         0,
                         "On the bottom is inscribed:\r[Going to Eleanor's party] (READ SUCCESS)",
                         speed,
                         &game->text[47]);

    auto node_text = [&](int index, int yes_index) {
        String choices[] = {const_string("Yes"), const_string("No")};
        Text_List *next[] = { &game->text[yes_index], 0 };
        void (*function_pointers[])(void*) = { nullptr, nullptr };

        atari_choice_text_list_init(&game->text[index],
                                    0,
                                    "Drink the liquid?",
                                    choices,
                                    next,
                                    function_pointers,
                                    2);
    };

    // first part
    node_text(1, 2);
    node_text(5, 6);
    node_text(8, 9);
    node_text(11, 12);
    node_text(14, 15);

    // second part
    node_text(41, 42);
    node_text(44, 45);
    node_text(47, 48);

    {
        String choices[] = {
            const_string("Um no"),
            const_string("No thanks"),
            const_string("No, I have to find a way out of here first"),
            const_string("Definitely not lol"),
            const_string("No I just can't"),
            const_string("I'm busy I have things to do"),
        };
        Text_List *next[6] = {};
        void (*function_pointers[6])(void*) = {};

        atari_choice_text_list_init(&game->text[3],
                                    0,
                                    "Will you drink the damn water?",
                                    choices,
                                    next,
                                    function_pointers,
                                    6);
    }

    {
        String choices[] = {
            const_string("Probably not a good idea, no"),
            const_string("Not now, I'll do it when I get out"),
            const_string("Ugh no"),
            const_string("Nooo it's sooooo much effort"),
            const_string("It's meaningless anyways"),
            const_string("I'm just trying to get by. No.")
        };
        Text_List *next[6] = {};
        void (*function_pointers[6])(void*) = {};

        atari_choice_text_list_init(&game->text[19],
                                    0,
                                    "Drink the liquid?",
                                    choices,
                                    next,
                                    function_pointers,
                                    6);
    }


    atari_text_list_init(&game->text[2],
                         0,
                         "Why would you drink this?\rDon't you remember how it hurt you before?\rHow you bled and screamed and bruised?",
                         speed,
                         &game->text[3]);
    atari_text_list_init(&game->text[6],
                         0,
                         "Why would you drink this?\rYou have other things to do.\rImportant things.\rYou need to figure out how to get out of\nthis prison, Chase- you can't get distracted!",
                         speed,
                         &game->text[3]);
    atari_text_list_init(&game->text[9],
                         0,
                         "Come on, Chase.\rYou know you're not feeling up to it.\rJust stay home.\rDon't drink it.",
                         speed,
                         &game->text[3]);
    atari_text_list_init(&game->text[12],
                         0,
                         "Seriously, Chase.\rYou remember what happened last time.\rIt's the smart thing not to go.\rI bet it'll be awkward too.",
                         speed,
                         &game->text[3]);
    atari_text_list_init(&game->text[15],
                         0,
                         "It's already been too long-\ryour focus right now should be getting out of\nthis place.\rI'm sure even if you did, it'll be mediocre.",
                         speed,
                         &game->text[3]);
    atari_text_list_init(&game->text[42],
                         0,
                         "What would even be the point?\rYour thirst won't be quenched anyways,\nremember?\rJust wait for the gate to open again.",
                         speed,
                         &game->text[19]);
    atari_text_list_init(&game->text[45],
                         0,
                         "Chase, Chase, Chase.\r*sigh*\rHow many times are we gonna have to do \nthis?\rYou know the drill.",
                         speed,
                         &game->text[19]);
    atari_text_list_init(&game->text[48],
                         0,
                         "You aren't a party-going person, Chase.\rYou just don't have that personality-\rI'm sure you'd end up alone in the corner\nanyways.",
                         speed,
                         &game->text[19]);


    int red_text[] = { 2, 6, 9, 12, 15, 42, 45, 48 };

    for (int i = 0; i < StaticArraySize(red_text); i++) {
        int text_index = red_text[i];
        for (int j = 0; j < game->text[text_index].text_count; j++) {
            game->text[text_index].text[j].color = {128,0,0,255};
        }
    }

    atari_text_list_init(&game->text[20],
                         "You",
                         "Woah, I got out!\rI need to make a phone call.",
                         speed,
                         &game->text[21]);
    atari_text_list_init(&game->text[21],
                         0,
                         "*ring ring*\r*ring ring*\r*ring ring*",
                         speed,
                         &game->text[22]);
    atari_text_list_init(&game->text[22],
                         "Chase",
                         "... Hello?",
                         speed,
                         &game->text[23]);
    atari_text_list_init(&game->text[23],
                         "You",
                         "Oh, hello.",
                         speed,
                         &game->text[24]);
    atari_text_list_init(&game->text[24],
                         "Chase",
                         "Who is this?",
                         speed,
                         &game->text[25]);
    atari_text_list_init(&game->text[25],
                         "You",
                         "I'm you from the future!",
                         speed,
                         &game->text[26]);
    atari_text_list_init(&game->text[26],
                         "Chase",
                         "Did you escape the prison?",
                         speed,
                         &game->text[27]);
    atari_text_list_init(&game->text[27],
                         "You",
                         "Yes, I did!\rWould you like to know how I did it?",
                         speed,
                         &game->text[28]);
    atari_text_list_init(&game->text[28],
                         "Chase",
                         "Yes.",
                         speed,
                         &game->text[29]);
    atari_text_list_init(&game->text[29],
                         "You",
                         "Ok, so all you have to do is have patience,\nand-\r...\r...",
                         speed,
                         &game->text[30]);
    atari_text_list_init(&game->text[30],
                         "Chase",
                         "Are you still there...?",
                         speed,
                         &game->text[31]);
    atari_text_list_init(&game->text[31],
                         "You",
                         "I'm sorry.\rI just realized I didn't escape after all.",
                         speed,
                         &game->text[32]);
    atari_text_list_init(&game->text[32],
                         "You",
                         "It turns out I'm still in the prison,\rthere was just a bigger one around\nthe previous one.",
                         speed,
                         &game->text[33]);
    atari_text_list_init(&game->text[33],
                         "You",
                         "I'll call you back if I do escape.\rI promise.",
                         speed,
                         &game->text[34]);
    atari_text_list_init(&game->text[34],
                         "Chase",
                         "Wait wh-",
                         speed,
                         &game->text[35]);
    atari_text_list_init(&game->text[35],
                         0,
                         "*click*",
                         speed,
                         nullptr);

    atari_text_list_init(&game->text[60],
                         0,
                         "We need to focus on getting out of here.\rI'm not going to be distracted.",
                         speed,
                         nullptr);

    atari_text_list_init(&game->text[61],
                         0,
                         "This is the final door to the other side.",
                         speed,
                         &game->text[62]);

    {
        String choices[] = {
            const_string("Yes"),
            const_string("No")
        };

        Text_List *next[2] = { 0, &game->text[63] };
        void (*function_pointers[2])(void*) = {epilogue_start_transition, 0};

        atari_choice_text_list_init(&game->text[62],
                                    0,
                                    "Are you ready?",
                                    choices,
                                    next,
                                    function_pointers,
                                    2);
    }

    atari_text_list_init(&game->text[63],
                         0,
                         "Too bad.",
                         speed,
                         nullptr);
    game->text[63].callbacks[0] = epilogue_start_transition;

    atari_text_list_init(&game->text[64],
                         0,
                         "Congratulations!\r...\r...\rYou have wasted your time.\rGoodbye, I guess!",
                         speed,
                         nullptr);
    game->text[64].callbacks[0] = epilogue_raise_node_last_scene;

    {
        String choices[] = {
            const_string("Yes, there's no reason not to."),
            const_string("No, there's no reason to.")
        };

        Text_List *next[2] = { 0, &game->text[66] };
        void (*function_pointers[2])(void*) = {0, 0};

        atari_choice_text_list_init(&game->text[65],
                                    0,
                                    "Take a sip?",
                                    choices,
                                    next,
                                    function_pointers,
                                    2);
    }

    {
        String choices[] = {
            const_string("There's no reason not to!"),
            const_string("There's no reason not to!"),
            const_string("NO! There's no reason to!"),
        };

        Text_List *next[3] = { 0, 0, &game->text[67] };
        void (*function_pointers[3])(void*) = {};

        atari_choice_text_list_init(&game->text[67],
                                    0,
                                    "Take the damn sip.",
                                    choices,
                                    next,
                                    function_pointers,
                                    3);
    }

    atari_text_list_init(&game->text[66],
                         0,
                         "Why not?",
                         speed,
                         &game->text[67]);

    speed = 30;

    atari_text_list_init(&game->text[70],
                         "Penny",
                         "Oh, electronic music?\rThat's lame.",
                         speed,
                         &game->text[71]);
    atari_text_list_init(&game->text[71],
                         "Chase",
                         "WHAT?!\rYou don't like deadmau5, Daft Punk...?\rAphex Twin?!",
                         speed,
                         &game->text[72]);
    atari_text_list_init(&game->text[72],
                         "Penny",
                         "...\r...\rWho them?",
                         speed,
                         &game->text[73]);
    atari_text_list_init(&game->text[73],
                         "Chase",
                         "Woah, I can't believe that!\rOk wait, but I do make other stuff too.",
                         speed,
                         &game->text[74]);
    atari_text_list_init(&game->text[74],
                         "Penny",
                         "Like what?",
                         speed,
                         &game->text[75]);
    atari_text_list_init(&game->text[75],
                         "Chase",
                         "Solo piano pieces, mainly.\rKind of impressionistic.",
                         speed,
                         &game->text[76]);
    atari_text_list_init(&game->text[76],
                         "Penny",
                         "Oh, nice.\rI like Debussy.\rSatie too.",
                         speed,
                         &game->text[77]);
    atari_text_list_init(&game->text[77],
                         "Chase",
                         "Yeah, those were big inspirations.",
                         speed,
                         &game->text[78]);
    atari_text_list_init(&game->text[78],
                         "Penny",
                         "So, how does it work?\rDo you record yourself playing, or do you\njust program in the notes?",
                         speed,
                         &game->text[79]);
    atari_text_list_init(&game->text[79],
                         "Chase",
                         "Yeah I record myself playing.\rWhat about you?",
                         speed,
                         &game->text[80]);
    atari_text_list_init(&game->text[80],
                         "Penny",
                         "I do as well- I have this shitty mic that\nI use for my acoustic guitar.\rI layer everything up, then mix and master\neverything myself.",
                         speed,
                         &game->text[81]);
    atari_text_list_init(&game->text[81],
                         "Chase",
                         "Well, the shitter the mic the better in\nmy experience.\rGives it the ~vibe~",
                         speed,
                         &game->text[82]);
    atari_text_list_init(&game->text[82],
                         "Penny",
                         "Hahah, so true.\r...\r.....\r......",
                         speed,
                         &game->text[83]);
    atari_text_list_init(&game->text[83],
                         "Chase",
                         "...\r...\r...",
                         speed,
                         &game->text[84]);
    atari_text_list_init(&game->text[84],
                         "Penny",
                         "...\r...\r...",
                         speed,
                         &game->text[85]);
    atari_text_list_init(&game->text[85],
                         "Chase",
                         "...\r...\r...",
                         speed,
                         &game->text[86]);
    atari_text_list_init(&game->text[86],
                         "Penny",
                         "Hey.",
                         speed,
                         &game->text[87]);
    atari_text_list_init(&game->text[87],
                         "Chase",
                         "...?",
                         speed,
                         &game->text[88]);
    atari_text_list_init(&game->text[88],
                         "Penny",
                         "Maybe we could work on a song together.",
                         speed,
                         &game->text[89]);
    atari_text_list_init(&game->text[89],
                         "Chase",
                         "Oh.\r...",
                         speed,
                         &game->text[90]);
    atari_text_list_init(&game->text[90],
                         "Chase",
                         "I'd like that a lot.",
                         speed,
                         nullptr);
    game->text[90].callbacks[0] = epilogue_goto_credits;

    level->num_nodes = 5;
    Epilogue_Node *nodes = level->nodes;
    memset(nodes, 0, sizeof(Epilogue_Node) * level->num_nodes);

    float y = -5;

    nodes[0].position = { -13, y, -18 };
    nodes[1].position = {   5, y, -10 };
    nodes[2].position = {  10, y,   5 };
    nodes[3].position = {  16, y,  16 };
    nodes[4].position = { -10, y,  15 };

    nodes[0].text = &game->text[0];
    nodes[1].text = &game->text[4];
    nodes[2].text = &game->text[7];
    nodes[3].text = &game->text[10];
    nodes[4].text = &game->text[13];
}

void epilogue_handle_transition(Game *game, float dt) {
    Level_Chapter_Epilogue *level = (Level_Chapter_Epilogue*)game->level;

    if (level->is_transitioning) {
        level->transition_timer += 0.25f * dt;

        if (!level->transitioned && level->transition_timer >= 0.5f) {
            // transition to next part
            level->transitioned = true;

            level->camera.position = {0, level->camera_height, 2};
            level->camera.target   = {0, level->camera_height, 4};

            memset(level->nodes, 0, sizeof(level->nodes));
            level->num_nodes = 0;
            level->next_node_to_appear = 0;
            level->node_popup = false;
            level->current_node = 0;
            level->door_y     = 0;
            level->node_timer = 8;

            level->door_popup = false;
            level->door_strike_popup = false;

            Epilogue_Node *nodes = level->nodes;
            memset(nodes, 0, sizeof(Epilogue_Node) * level->num_nodes);

            level->door_timer = 180;

            if (level->state == EPILOGUE_STATE_FIRST) {
                level->state = EPILOGUE_STATE_SECOND;

                level->fog_factor = level->fog_factor_to = 1.0f/20.0f;

                level->pause_door_timer = true;

                level->scene.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = level->pink_dot;

                float y = -5;

                level->num_nodes = 5;

                nodes[0].position = { -9, y, -10 };
                nodes[1].position = {  9, y,  10 };
                nodes[2].position = { -19, y, -5 };
                nodes[3].position = { 6, y,  19 };
                nodes[4].position = { 0, y, -7 };

                nodes[0].text = &game->text[40];
                nodes[1].text = &game->text[43];
                nodes[3].text = &game->text[46];
            } else if (level->state == EPILOGUE_STATE_SECOND) {
                level->state = EPILOGUE_STATE_THIRD;

                level->scene.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = level->orange_dot;
                level->fog_factor = level->fog_factor_to = 0;

                level->num_nodes = 5;

                float y = -5;

                nodes[0].position = { -9, y, -10 };
                nodes[1].position = {  9, y,  10 };
                nodes[2].position = { -19, y, -5 };
                nodes[3].position = { 6, y,  19 };
                nodes[4].position = { 0, y, -7 };

                for (int i = 0; i < level->num_nodes; i++)
                    nodes[i].text = &game->text[60];
            } else if (level->state == EPILOGUE_STATE_THIRD) {
                level->state = EPILOGUE_STATE_FOURTH;

                epilogue_text_change_color = true;

                game->textbox_alpha = 255;

                add_event(game, epilogue_start_congratulations_text, 5);

                level->num_nodes = 1;

                float y = -15;

                nodes[0].position = { -1, y, 0 };
                nodes[0].text = &game->text[65];

                level->scene.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = level->black_texture;

                level->camera.position = {1.74f, 1.67f, 2.10f};
                level->camera.target   = {0.23f, 1.19f, 0.88f};
            } else {
                level->state = EPILOGUE_STATE_ENDING;
            }
        }

        if (level->transition_timer > 1) {
            level->transition_timer = 0;
            level->is_transitioning = false;
            level->transitioned = false;

            if (level->state == EPILOGUE_STATE_SECOND)
                game->current = &game->text[20];
        }
    }
}

void chapter_epilogue_update_ending(Game *game, float dt) {
    (void)game,dt;
}

void chapter_epilogue_update(Game *game, float dt) {
    Level_Chapter_Epilogue *level = (Level_Chapter_Epilogue*)game->level;

    if (level->state == EPILOGUE_STATE_ENDING) {
        chapter_epilogue_update_ending(game, dt);
        return;
    }

#ifdef DEBUG
    if (IsKeyPressed(KEY_TAB)) {
        level->door_timer = 0;
    }
    if (IsKeyPressed(KEY_Q)) {
        //level->door_timer = 0;
        if (level->num_nodes > 0) {
            Epilogue_Node *node = level->nodes + level->next_node_to_appear;

            if (!node->moving_up && !node->moving_down) {
                node->moving_up = true;
                level->node_timer = level->node_timer_max;

                level->next_node_to_appear++;
                if (level->next_node_to_appear >= level->num_nodes) {
                    level->next_node_to_appear = 0;
                }
            }
        }
    }
#endif

    if (level->num_nodes > 0 && level->state != EPILOGUE_STATE_FOURTH) {
        level->node_timer -= dt;

        if (level->node_timer < 0) {
            Epilogue_Node *node = level->nodes + level->next_node_to_appear;

            if (!node->moving_up && !node->moving_down) {
                node->moving_up = true;
                level->node_timer = level->node_timer_max;

                level->next_node_to_appear++;
                if (level->next_node_to_appear >= level->num_nodes) {
                    level->next_node_to_appear = 0;
                }
            }
        }
    }

    for (int i = 0; i < level->num_nodes; i++) {
        Epilogue_Node *node = level->nodes + i;

        if (level->state == EPILOGUE_STATE_THIRD) {
            if (node->position.y == 0) {
                node->time_up += dt;
            } else {
                node->time_up = 0;
            }

            if (level->current_node != node && node->time_up > 10) {
                node->moving_down = true;
            }
        }

        if (node->moving_up) {
            node->position.y += 2 * dt;

            if (node->position.y >= 0) {
                node->position.y = 0;
                node->moving_up = false;
            }
        } else if (node->moving_down) {
            node->position.y -= 2 * dt;

            if (node->position.y <= -5) {
                node->position.y = -5;
                node->moving_down = false;
            }
        }
    }

    bool update_camera = true;

    update_camera &= (game->current == 0);
    update_camera &= (level->transition_timer == 0);
    
    if (level->state == EPILOGUE_STATE_FOURTH && level->nodes[0].position.y < 0) {
        update_camera = false;
    }

    if (update_camera) {
        float speed = PLAYER_SPEED_3D;

        float dir_x = input_movement_x_axis(dt);//key_right() - key_left();
        float dir_y = input_movement_y_axis(dt);//key_down()  - key_up();

        if (IsKeyDown(KEY_LEFT_SHIFT) ||
            (IsGamepadAvailable(0) && IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_2))) speed=15;//speed = 7;
        if (IsKeyDown(KEY_LEFT_ALT)) speed = 0.5f;

        Vector3 *cam = &level->camera.position;

        Vector3 saved_target = level->camera.target;
        Vector3 saved_cam = *cam;

        CameraMoveForward(&level->camera, -dir_y * speed * dt, true);
        CameraMoveRight(&level->camera, dir_x * speed * dt, true);

        level->camera.target = saved_target;

        cam->x = Clamp(cam->x, -28, 28);
        cam->z = Clamp(cam->z, -28, 28);

        if (level->state == EPILOGUE_STATE_SECOND) {
            float dist = 28 * 0.05f;

            if (cam->x < dist && cam->x > -dist && cam->z < dist && cam->z > -dist) {
                *cam = saved_cam;
            }
        }

        level->camera.target = Vector3Add(level->camera.target, Vector3Subtract(*cam, saved_cam));

        update_camera_look(&level->camera, dt);
    }

    level->node_popup = false;
    if (game->current == nullptr && level->transition_timer == 0) {
        if (level->current_node) {
            if (level->state == EPILOGUE_STATE_FOURTH) {
                add_event(game, epilogue_start_final_conversation, 5);
                level->is_transitioning = true;
            } else {
                level->current_node->text = 0;
                level->current_node->moving_down = true;
                level->current_node->position.y -= 0.01f;
            }

            level->current_node = nullptr;
        }

        Vector2 cam_pos = {level->camera.position.x, level->camera.position.z};

        for (int i = 0; i < level->num_nodes; i++) {
            Epilogue_Node *node = level->nodes + i;
            Vector2 node_pos = {node->position.x, node->position.z};

            if (node->position.y == 0 && Vector2Distance(cam_pos, node_pos) < 3) {
                level->node_popup = true;

                if (is_action_pressed()) {
                    game->current = node->text;
                    level->current_node = node;
                }
            }
        }
    }

    level->door_popup = false;
    level->door_strike_popup = false;

    if (level->transition_timer == 0 && game->current == nullptr) {
        Vector2 cam_pos = {level->camera.position.x, level->camera.position.z};

        for (int i = 0; i < level->num_doors; i++) {
            Vector2 door_pos = { level->doors[i].pos.x, level->doors[i].pos.z };

            if (Vector2Distance(cam_pos, door_pos) < 3) {
                if (level->door_timer > 0) 
                    level->door_strike_popup = true;
                else
                    level->door_popup = true;


                if (is_action_pressed()) {
                    if (level->door_popup && level->state == EPILOGUE_STATE_THIRD) {
                        game->current = &game->text[61];
                    } else {
                        if (level->door_popup) {
                            level->is_transitioning = true;
                        } else {
                            // TODO: Door strike sound
                            level->door_strike_popup = false;
                        }
                    }
                }
                break;
            }
        }
    }

    if (!level->pause_door_timer) {
        level->door_timer -= dt;
    }

    if (level->door_timer <= 0) {
        level->door_y -= dt;
        level->door_timer = 0;
    }

    epilogue_handle_transition(game, dt);

    if (game->current == &game->text[29]) {
        level->fog_factor_to = 0;
        level->pause_door_timer = false;
    }

    if (level->fog_factor != level->fog_factor_to) {
        int dir = sign(level->fog_factor_to - level->fog_factor);

        float d = dt * 0.0075f * dir;
        level->fog_factor += d;

        if (dir == 1) {
            if (level->fog_factor >= level->fog_factor_to) {
                level->fog_factor  = level->fog_factor_to;
            }
        } else if (dir == -1) {
            if (level->fog_factor <= level->fog_factor_to) {
                level->fog_factor  = level->fog_factor_to;
            }
        }
    }
}

void chapter_epilogue_draw_ending(Game *game) {
    (void)game;
    ClearBackground(BLACK);
}

void chapter_epilogue_draw(Game *game) {
    Level_Chapter_Epilogue *level = (Level_Chapter_Epilogue*)game->level;

    if (level->state == EPILOGUE_STATE_ENDING) {
        chapter_epilogue_draw_ending(game);
        return;
    }

    SetShaderValue(level->shader, GetShaderLocation(level->shader, "viewPos"), &level->camera.position, SHADER_UNIFORM_VEC3);
    SetShaderValue(level->shader, GetShaderLocation(level->shader, "fog_factor"), &level->fog_factor, SHADER_UNIFORM_FLOAT);

    if (level->state == EPILOGUE_STATE_FOURTH) {
        ClearBackground(BLACK);
    } else {
        ClearBackground(WHITE);
    }

    BeginMode3D(level->camera);

    DrawModel(level->scene, {}, 1, WHITE);

    if (level->state != EPILOGUE_STATE_FOURTH) {
        auto draw_bars = [&](float scale) {
            Vector3 scale_vec = { scale, scale, scale };

            DrawModelEx(level->bars, {}, {0,1,0}, 0,   scale_vec, WHITE);
            DrawModelEx(level->bars, {}, {0,1,0}, 90,  scale_vec, WHITE);
            DrawModelEx(level->bars, {}, {0,1,0}, 180, scale_vec, WHITE);
            DrawModelEx(level->bars, {}, {0,1,0}, 270, scale_vec, WHITE);
        };

        draw_bars(1);

        if (level->state == EPILOGUE_STATE_SECOND || level->state == EPILOGUE_STATE_THIRD) {
            draw_bars(0.05f);
        }

        if (level->state == EPILOGUE_STATE_THIRD) {
            draw_bars(0.05f * 0.05f);
        }

        // Draw door timers

        EndMode3D();

        BeginTextureMode(level->flipped_timer_texture);

        ClearBackground({});
        Font *font = &bold_2_font;

        char time_string[64] = {};
        seconds_to_minutes_and_seconds(level->door_timer, time_string);
        DrawTextEx(*font, time_string, {0,-3}, font->baseSize, 0, WHITE);

        EndTextureMode();

        BeginTextureMode(level->timer_texture);

        float w = level->flipped_timer_texture.texture.width;
        float h = level->flipped_timer_texture.texture.height;

        ClearBackground(BLACK);

        DrawTexturePro(level->flipped_timer_texture.texture,
                       {0, 0, -w, h},
                       {0, 0, w, h},
                       {0, 0},
                       0,
                       WHITE);
        EndTextureMode();

        BeginTextureMode(game->render_target_3d);
        BeginMode3D(level->camera);

        for (int i = 0; i < level->num_doors; i++) {
            Epilogue_Door *door = level->doors + i;

            int rotation = 0;

            switch (i) {
                case 0: rotation = -90; break;
                case 1: rotation = 90;  break;
                case 2: rotation = 180; break;
                case 3: rotation = 0;   break;
            }

            Vector3 pos = door->pos;
            pos.y += 5.25f;

            float s = 1;

            level->timer_model.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = level->timer_texture.texture;

            DrawModelEx(level->timer_model, pos, {0,1,0}, rotation, {s,s,s}, WHITE);
            DrawModelEx(level->door, {0,level->door_y,0}, {0,1,0}, rotation, {1,1,1}, WHITE);
            // door->texture now has the 1:59 on it
        }
    }

    for (int i = 0; i < level->num_nodes; i++) {
        Epilogue_Node *node = level->nodes + i;
        Color color = WHITE;

        if (level->state == EPILOGUE_STATE_THIRD)
            color = BLACK;

        DrawModel(level->node, node->position, 2, color);
    }

    EndMode3D();

    if (level->node_popup) {
        Color c = BLACK;

        if (level->state == EPILOGUE_STATE_FOURTH)
            c = WHITE;

        draw_popup("Inspect Capsule", c, Top);
    }

    if (level->door_strike_popup) {
        draw_popup("Try to strike the door", RED, Bottom);
    }
    if (level->door_popup) {
        draw_popup("Exit the prison", BLACK, Bottom);
    }

    float transition_fade = sinf(level->transition_timer * PI);

    DrawRectangle(0, 0, render_width, render_height, {0,0,0,(uint8_t)(transition_fade * 255)});
}

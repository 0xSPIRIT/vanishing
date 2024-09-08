#define EPILOGUE_NODE_MAX 64
#define EPILOGUE_DOOR_MAX 64

enum Epilogue_State {
    EPILOGUE_STATE_FIRST,
    EPILOGUE_STATE_SECOND
};

struct Epilogue_Node {
    Vector3 position;
    Text_List *text;
    bool moving_up, moving_down;
};

struct Epilogue_Door {
    Vector3 pos;
};

struct Level_Chapter_Epilogue {
    Epilogue_State state;

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
    float          door_y;

    float          node_timer, node_timer_max;

    RenderTexture  timer_texture, flipped_timer_texture;
    Texture        pink_dot;

    bool           door_popup;
    bool           door_strike_popup;

    bool           is_transitioning;
    float          transition_timer; // 0.0 to 1.0
};

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

    level->pink_dot    = LoadTexture(RES_DIR "models/pink_dot.png");
    SetTextureFilter(level->pink_dot, TEXTURE_FILTER_BILINEAR);

    level->node_timer_max = 10;
    level->node_timer = 5;

    level->door_timer = 1;

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
                         "This one calls itself hanging out with friends.\n(READ SUCCESS)",
                         speed,
                         &game->text[1]);
    atari_text_list_init(&game->text[4],
                         0,
                         "This one calls itself learning an instrument.\n(READ SUCCESS)",
                         speed,
                         &game->text[5]);
    atari_text_list_init(&game->text[7],
                         0,
                         "This one is named taking an evening walk.\n(READ SUCCESS)",
                         speed,
                         &game->text[8]);
    atari_text_list_init(&game->text[10],
                         0,
                         "This one is named watching a movie with\nyour friends. (READ SUCCESS)",
                         speed,
                         &game->text[11]);
    atari_text_list_init(&game->text[13],
                         0,
                         "This one is named composing music again.\n(READ SUCCESS)",
                         speed,
                         &game->text[14]);
    atari_text_list_init(&game->text[40],
                         0,
                         "This one calls itself learning how to knit.\n(READ SUCCESS)",
                         speed,
                         &game->text[41]);
    atari_text_list_init(&game->text[43],
                         0,
                         "This one calls itself meeting new people.\n(READ SUCCESS)",
                         speed,
                         &game->text[44]);

    auto node_text = [&](int index, int yes_index) {
        String choices[] = {const_string("Yes"), const_string("No")};
        Text_List *next[] = { &game->text[yes_index], 0 };
        void (*function_pointers[])(void*) = { nullptr, nullptr };

        atari_choice_text_list_init(&game->text[index],
                                    0,
                                    "Will you drink the water from the capsule?",
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
            const_string("Noooooo it's sooooo much effort"),
            const_string("It's meaningless anyways"),
            const_string("I'm just trying to get by. No.")
        };
        Text_List *next[6] = {};
        void (*function_pointers[6])(void*) = {};

        atari_choice_text_list_init(&game->text[19],
                                    0,
                                    "Drink the water?",
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


    int red_text[] = { 2, 6, 9, 12, 15, 42, 45 };

    for (int i = 0; i < StaticArraySize(red_text); i++) {
        int text_index = red_text[i];
        for (int j = 0; j < game->text[text_index].text_count; j++) {
            game->text[text_index].text[j].color = GOLD;
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
                         "What wh-",
                         speed,
                         &game->text[35]);
    atari_text_list_init(&game->text[35],
                         0,
                         "*click*",
                         speed,
                         nullptr);

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

void chapter_epilogue_update(Game *game, float dt) {
    Level_Chapter_Epilogue *level = (Level_Chapter_Epilogue*)game->level;

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

    for (int i = 0; i < level->num_nodes; i++) {
        Epilogue_Node *node = level->nodes + i;

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

    if (game->current == 0 && level->transition_timer == 0) {
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

        chapter_5_update_camera_look(&level->camera, dt);
    }

    level->node_popup = false;
    if (game->current == nullptr) {
        if (level->current_node) {
            level->current_node->moving_down = true;
            level->current_node->position.y -= 0.01f;
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

    {
        Vector2 cam_pos = {level->camera.position.x, level->camera.position.z};

        for (int i = 0; i < level->num_doors; i++) {
            Vector2 door_pos = { level->doors[i].pos.x, level->doors[i].pos.z };

            if (Vector2Distance(cam_pos, door_pos) < 3) {
                if (level->door_timer > 0) 
                    level->door_strike_popup = true;
                else
                    level->door_popup = true;


                if (is_action_pressed()) {
                    if (level->door_popup) {
                        level->is_transitioning = true;
                    } else {
                        // TODO: Door strike sound
                    }
                }
                break;
            }
        }
    }

    level->door_timer -= dt;

    if (level->door_timer <= 0) {
        level->door_y -= dt;
        level->door_timer = 0;
    }

    if (level->is_transitioning) {
        level->transition_timer += 0.5f * dt;

        if (level->transition_timer >= 0.5f && level->state == EPILOGUE_STATE_FIRST) {
            // transition to next part

            level->state = EPILOGUE_STATE_SECOND;
            level->camera.position = {0, level->camera_height, 2};
            level->camera.target   = {0, level->camera_height, 4};

            memset(level->nodes, 0, sizeof(level->nodes));
            level->num_nodes = 0;
            level->next_node_to_appear = 0;
            level->node_popup = false;
            level->current_node = 0;
            level->door_timer = 180;
            level->door_y     = 0;
            level->node_timer = 10;

            level->fog_factor = level->fog_factor_to = 1.0f/20.0f;

            level->scene.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = level->pink_dot;

            level->door_popup = false;
            level->door_strike_popup = false;

            float y = -5;

            Epilogue_Node *nodes = level->nodes;
            memset(nodes, 0, sizeof(Epilogue_Node) * level->num_nodes);

            level->num_nodes = 5;

            nodes[0].position = { -9, y, -10 };
            nodes[1].position = {  9, y,  10 };
            nodes[2].position = { -19, y, -5 };
            nodes[3].position = { 6, y,  19 };
            nodes[4].position = { 0, y, -7 };

            nodes[0].text = &game->text[40];
            nodes[1].text = &game->text[43];

        }

        if (level->transition_timer > 1) {
            level->transition_timer = 0;
            level->is_transitioning = false;

            game->current = &game->text[20];
        }
    }

    if (game->current == &game->text[29]) {
        level->fog_factor_to = 0;
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

void chapter_epilogue_draw(Game *game) {
    Level_Chapter_Epilogue *level = (Level_Chapter_Epilogue*)game->level;

    SetShaderValue(level->shader, GetShaderLocation(level->shader, "viewPos"), &level->camera.position, SHADER_UNIFORM_VEC3);
    SetShaderValue(level->shader, GetShaderLocation(level->shader, "fog_factor"), &level->fog_factor, SHADER_UNIFORM_FLOAT);

    ClearBackground(WHITE);

    BeginMode3D(level->camera);

    DrawModel(level->scene, {}, 1, WHITE);

    auto draw_bars = [&](float scale) {
        Vector3 scale_vec = { scale, scale, scale };

        DrawModelEx(level->bars, {}, {0,1,0}, 0,   scale_vec, WHITE);
        DrawModelEx(level->bars, {}, {0,1,0}, 90,  scale_vec, WHITE);
        DrawModelEx(level->bars, {}, {0,1,0}, 180, scale_vec, WHITE);
        DrawModelEx(level->bars, {}, {0,1,0}, 270, scale_vec, WHITE);
    };

    draw_bars(1);

    if (level->state == EPILOGUE_STATE_SECOND) {
        draw_bars(0.05f);
    }

    for (int i = 0; i < level->num_nodes; i++) {
        Epilogue_Node *node = level->nodes + i;
        Color color = WHITE;

        DrawModel(level->node, node->position, 2, color);
    }

    EndShaderMode();
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

    EndShaderMode();
    EndMode3D();

    if (level->node_popup) {
        draw_popup("Inspect Node", BLACK, Top);
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

#define EPILOGUE_NODE_MAX 64
#define EPILOGUE_DOOR_MAX 64

struct Epilogue_Node {
    Vector3 position;
    Text_List *text;
};

struct Epilogue_Door {
    Vector3       pos;
};

struct Level_Chapter_Epilogue {
    float    camera_height;
    Camera3D camera;
    Shader   shader;
    Model    scene, bars, node, timer_model;

    Epilogue_Node nodes[EPILOGUE_NODE_MAX];
    int           num_nodes;
    bool          node_popup;

    Epilogue_Door doors[EPILOGUE_DOOR_MAX];
    int           num_doors;
    float         door_timer;

    RenderTexture timer_texture, flipped_timer_texture;
};

void chapter_epilogue_init(Game *game) {
    Level_Chapter_Epilogue *level = (Level_Chapter_Epilogue*)game->level;

    DisableCursor();

    render_width  = DIM_3D_WIDTH;
    render_height = DIM_3D_HEIGHT;

    game->render_state = RENDER_STATE_3D;

    game->textbox_target = LoadRenderTexture(render_width, render_height);
    game->textbox_alpha = 255;

    level->camera_height = 1.67f;

    level->camera.position   = { 0, level->camera_height, -7 };
    level->camera.target     = { 0.00f, level->camera_height, 2.00f };
    level->camera.up         = { 0, 1, 0 };
    level->camera.fovy       = FOV_DEFAULT;
    level->camera.projection = CAMERA_PERSPECTIVE;

    level->shader      = LoadShader(RES_DIR "shaders/basic.vs", RES_DIR "shaders/fog.fs");
    level->scene       = LoadModel(RES_DIR "models/epilogue.glb");
    level->bars        = LoadModel(RES_DIR "models/epilogue_bars.glb");
    level->node        = LoadModel(RES_DIR "models/node.glb");
    level->timer_model = LoadModel(RES_DIR "models/clock.glb");

    level->door_timer = 70;

    model_set_shader(&level->bars,  level->shader);
    model_set_shader(&level->scene, level->shader);
    model_set_shader(&level->node,  level->shader);
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

    auto node_text = [&](int index, int yes_index) {
        String choices[] = {const_string("Yes"), const_string("No")};
        Text_List *next[] = { &game->text[yes_index], 0 };
        void (*function_pointers[])(void*) = { nullptr, nullptr };

        atari_choice_text_list_init(&game->text[index],
                                    0,
                                    "Will you drink the liquid?",
                                    choices,
                                    next,
                                    function_pointers,
                                    2);
    };

    node_text(1, 2);

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
                                    "Will you drink the damn liquid?",
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

    game->text[2].text[0].color = {128,0,0,255};
    game->text[2].text[1].color = {128,0,0,255};
    game->text[2].text[2].color = {128,0,0,255};

    level->num_nodes = 2;
    for (int i = 0; i < level->num_nodes; i++) {
        Epilogue_Node *node = level->nodes + i;
        node->position = { 2.5f + i * -5.f, 0, 0 };
        node->text     = &game->text[0];
    }
}

void chapter_epilogue_update(Game *game, float dt) {
    Level_Chapter_Epilogue *level = (Level_Chapter_Epilogue*)game->level;

    if (game->current == 0) {
        float speed = PLAYER_SPEED_3D;

        float dir_x = input_movement_x_axis(dt);//key_right() - key_left();
        float dir_y = input_movement_y_axis(dt);//key_down()  - key_up();

        if (IsKeyDown(KEY_LEFT_SHIFT) ||
            (IsGamepadAvailable(0) && IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_2))) speed=15;//speed = 7;
        if (IsKeyDown(KEY_LEFT_ALT)) speed = 0.5f;

        CameraMoveForward(&level->camera, -dir_y * speed * dt, true);
        CameraMoveRight(&level->camera, dir_x * speed * dt, true);

        chapter_5_update_camera_look(&level->camera, dt);
    }

    level->node_popup = false;
    if (game->current == nullptr) {
        Vector2 cam_pos = {level->camera.position.x, level->camera.position.z};
        for (int i = 0; i < level->num_nodes; i++) {
            Epilogue_Node *node = level->nodes + i;
            Vector2 node_pos = {node->position.x, node->position.z};

            if (Vector2Distance(cam_pos, node_pos) < 3) {
                level->node_popup = true;

                if (is_action_pressed()) {
                    game->current = node->text;
                }
            }
        }
    }

    level->door_timer -= dt;
}

void chapter_epilogue_draw(Game *game) {
    Level_Chapter_Epilogue *level = (Level_Chapter_Epilogue*)game->level;

    SetShaderValue(level->shader, GetShaderLocation(level->shader, "viewPos"), &level->camera.position, SHADER_UNIFORM_VEC3);

    ClearBackground(WHITE);

    BeginMode3D(level->camera);
    BeginShaderMode(level->shader);

    DrawModel(level->scene, {}, 1, WHITE);

    DrawModelEx(level->bars, {}, {0,1,0}, 0,   {1,1,1}, WHITE);
    DrawModelEx(level->bars, {}, {0,1,0}, 90,  {1,1,1}, WHITE);
    DrawModelEx(level->bars, {}, {0,1,0}, 180, {1,1,1}, WHITE);
    DrawModelEx(level->bars, {}, {0,1,0}, 270, {1,1,1}, WHITE);

    for (int i = 0; i < level->num_nodes; i++) {
        Epilogue_Node *node = level->nodes + i;
        DrawModel(level->node, node->position, 2, WHITE);
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
                   BLACK);
    EndTextureMode();

    BeginTextureMode(game->render_target_3d);
    BeginMode3D(level->camera);

    for (int i = 0; i < level->num_doors; i++) {
        Epilogue_Door *door = level->doors + i;

        int rotation = 0;

        switch (i) {
            case 0: rotation = -90; break;
            case 1: rotation = 90; break;
            case 2: rotation = 180; break;
            case 3: rotation = 0; break;
        }

        Vector3 pos = door->pos;
        pos.y += 5.25f;

        float s = 1;

        level->timer_model.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = level->timer_texture.texture;

        DrawModelEx(level->timer_model, pos, {0,1,0}, rotation, {s,s,s}, WHITE);
        // door->texture now has the 1:59 on it
    }

    EndShaderMode();
    EndMode3D();

    if (level->node_popup) {
        draw_popup("Inspect Node", BLACK, Top);
    }
}

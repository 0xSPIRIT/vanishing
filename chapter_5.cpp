enum {
    CHAPTER_5_STATE_INTRO,
    CHAPTER_5_STATE_TRAIN_STATION_1,
    CHAPTER_5_STATE_TRAIN_STATION_2,
};

enum Chapter_5_Scene {
    CHAPTER_5_SCENE_TRAIN_STATION,
    CHAPTER_5_SCENE_STAIRCASE,
    CHAPTER_5_SCENE_DINNER_PARTY,
    CHAPTER_5_SCENE_COTTAGE,
    CHAPTER_5_SCENE_COUNT
};

struct Chapter_5_Clerk {
    Vector3 position;

    Model   body;

    float saved_head_rotation;

    bool  has_real_head;

    float head_rotation;
    float body_rotation;

    bool  do_180_head;

    bool  talk_popup;
    bool  talked;
};

struct Chapter_5_Train {
    Vector3     position;
    float       delta_x;
    bool        closed;
    bool        able_to_close;
    bool        moving;
    bool        player_in;
    float       player_in_timer;
    float       door_openness; // 0.0 to 1.0
    BoundingBox bounding_box;
    int         instances;
    float       setoff_timer;
    float       door_open_alarm;
    Vector3     instance_positions;
};

// Represents both the guy and the chair.
struct Chapter_5_Guy {
    bool  is_male;
    float table_angle;
    struct Chapter_5_Table *table;
};

struct Chapter_5_Table {
    Vector3       position;
    int           num_guys;
    Chapter_5_Guy guys[16];
};

struct Level_Chapter_5 {
    Chapter_5_Clerk clerk;
    Chapter_5_Train train;

    // Dinner Party scene
    Chapter_5_Table *tables;
    int              num_tables;

    struct Models {
        Model guy_sitting,
              chair,
              table,
              pyramid_head,
              real_head,
              train,
              train_door;
    } models;

    bool     door_popup;

    int      state;

    Camera3D camera;
    Shader   shader;

    float    black_screen_timer;
    float    transition_fade;

    Model    scenes[CHAPTER_5_SCENE_COUNT];
    int      current_scene;

    bool     ticket;
};

void chapter_5_begin_head_flip(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    level->clerk.do_180_head = true;
}

void chapter_5_give_ticket(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;
    
    level->ticket = true;
}

void chapter_5_begin_black_transition(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;
    
    level->black_screen_timer = 2;
}

void chapter_5_train_move(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    level->train.moving = true;
}

int chapter_5_train_length(Chapter_5_Train *train) {
    return train->bounding_box.max.x - train->bounding_box.min.x + 1;
}

void chapter_5_table(Chapter_5_Table *table, Vector2 pos, int num_guys) {
    assert(num_guys < StaticArraySize(table->guys));

    table->position = { pos.x, 0, pos.y };
    table->num_guys = num_guys;

    for (int i = 0; i < num_guys; i++) {
        Chapter_5_Guy *guy = &table->guys[i];

        guy->is_male = rand()%2==0;
        guy->table_angle = i * 2 * PI / (float)num_guys;
        guy->table = table;
    }
}

void chapter_5_train_station_init_positions(Game *game, bool refresh) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    Chapter_5_Clerk *clerk = &level->clerk;

    clerk->do_180_head = false;
    clerk->talk_popup = false;
    clerk->position = { 16.3f, 2.f, -14.1f };
    clerk->body_rotation = -90;
    clerk->head_rotation = -90;
    clerk->saved_head_rotation = clerk->head_rotation;
    clerk->has_real_head = false;
    clerk->talked = false;

    if (refresh) clerk->talked = true;

    level->camera.position   = { -7.19f, 2.00f, 6.68f };
    level->camera.target     = { 11.60f, 18.12f, -46.27f };
    level->camera.up         = { 0, 1, 0 };
    level->camera.fovy       = FOV_DEFAULT;
    level->camera.projection = CAMERA_PERSPECTIVE;
    
    if (refresh) {
        level->camera.position = {13.59f, 4.00f, -14.02f};
        level->camera.target = {71.95f, 7.18f, -15.17f};
    }
}

void chapter_5_goto_scene(Game *game, Chapter_5_Scene scene) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    level->current_scene = scene;

    if (IsShaderReady(level->shader))
        UnloadShader(level->shader);

    switch (scene) {
        case CHAPTER_5_SCENE_TRAIN_STATION: {
            level->state = CHAPTER_5_STATE_INTRO;//CHAPTER_5_STATE_TRAIN_STATION_1;
            level->black_screen_timer = -1;

            chapter_5_train_station_init_positions(game, false);

            Chapter_5_Train *train = &level->train;

            train->position     = {-500, 1.8f, -22.0f};
            train->bounding_box = GetMeshBoundingBox(level->models.train.meshes[0]);
            train->instances    = 5;
            train->setoff_timer = 0;
            train->closed       = true;
            train->moving       = false;
            train->able_to_close = true;

            train->door_open_alarm = 2;
            train->position.x = 0;

            level->shader = LoadShader("shaders/basic.vs", "shaders/basic.fs");

            level->shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(level->shader, "matModel");
            level->shader.locs[SHADER_LOC_VECTOR_VIEW]  = GetShaderLocation(level->shader, "viewPos");

            // TODO: Apply to all scenes
            model_set_shader(&level->scenes[0],        level->shader);
            model_set_shader(&level->models.train,      level->shader);
            model_set_shader(&level->models.train_door, level->shader);

            CreateLight(LIGHT_POINT, { 0, 1, 0 }, Vector3Zero(), ORANGE, level->shader);
            CreateLight(LIGHT_POINT, { 0, 1, 9 }, Vector3Zero(), ORANGE, level->shader);
            CreateLight(LIGHT_POINT, { 4, 4, 4 }, Vector3Zero(), ORANGE, level->shader);
            CreateLight(LIGHT_POINT, { 0, 1, -28 }, Vector3Zero(), ORANGE, level->shader);

            game->current = &game->text[0];
        } break;
        case CHAPTER_5_SCENE_STAIRCASE: {
            level->shader = LoadShader("shaders/basic.vs", "shaders/dinner.fs");

            model_set_shader(&level->scenes[1], level->shader);

            float train_distance = -600;

            level->camera.position   = { train_distance, 2, 0 };
            level->camera.target     = { 0.00f, 2.00f, 2.00f };
            level->camera.up         = { 0, 1, 0 };
            level->camera.fovy       = FOV_DEFAULT;
            level->camera.projection = CAMERA_PERSPECTIVE;

            Chapter_5_Train *train = &level->train;

            train->position     = { train_distance, 0, 0 };
            train->bounding_box = GetMeshBoundingBox(level->models.train.meshes[0]);
            train->instances    = 5;
            train->setoff_timer = 0;
            train->closed       = true;
            train->moving       = true;
            train->player_in    = true;
            train->able_to_close = false;
            //level->train.player_in    = false;
        } break;
        case CHAPTER_5_SCENE_DINNER_PARTY: {
            level->shader = LoadShader("shaders/basic.vs", "shaders/dinner.fs");
            model_set_shader(&level->scenes[2], level->shader);

            level->camera.position   = { 0, 2, 0 };
            level->camera.target     = { 0.00f, 2.00f, 2.00f };
            level->camera.up         = { 0, 1, 0 };
            level->camera.fovy       = FOV_DEFAULT;
            level->camera.projection = CAMERA_PERSPECTIVE;

            Vector2 table_positions[] = {
                {-39.0f, -3.7f}, 
                {-25.6f, -4.4f}, 
                {-13.0f, -1.0f}, 
                {-32.5f, +3.2f}, 
                {-22.1f, +2.3f}, 
                {-30.0f, +11.7f},
                {-21.0f, +12.5f},
                {-12.7f, +9.6f}, 
                {-33.2f, +20.2f},
                {-22.4f, +22.3f},
                {-13.9f, +19.6f},
            };

            level->num_tables = StaticArraySize(table_positions);
            level->tables = (Chapter_5_Table *)arena_push(&game->level_arena, sizeof(Chapter_5_Table) * level->num_tables);

            Chapter_5_Table *tables = level->tables;
            for (int i = 0; i < level->num_tables; i++) {
                chapter_5_table(tables+i, table_positions[i], 2 + (i+2) % 5);
            }
        } break;
        case CHAPTER_5_SCENE_COTTAGE: {
        } break;
    }
}

void chapter_5_window_text(bool scroll, Text_List *list, char *line, Color color, Text_List *next) {
    list->font         = &atari_font;
    list->font_spacing = 1;
    list->scale        = 0.125;
    list->scroll_speed = 30;
    list->color        = color;
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

void chapter_5_text(Text_List *list, char *speaker, char *line, float scroll_speed, Text_List *next) {
    list->font = &atari_font;
    list->font_spacing = 1;
    list->scale = 0.125;
    list->scroll_speed = scroll_speed;

    list->color = GOLD;
    list->bg_color = BLACK;

    list->render_type = DrawTextbox;
    list->location = Bottom;
    list->take_keyboard_focus = true;

    text_list_init(list, speaker, line, next);

    list->textbox_height = render_height / 3.5f;
}

void chapter_5_init(Game *game) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    DisableCursor();

    render_width  = DIM_3D_WIDTH;
    render_height = DIM_3D_HEIGHT;

    game->render_state = RENDER_STATE_3D;

    atari_assets.textures[0] = load_texture("art/ticket_pov.png");

    game->textbox_target = LoadRenderTexture(render_width, render_height);
    game->textbox_alpha = 255;

    level->scenes[0]           = LoadModel("models/train_station.glb");
    level->scenes[1]           = LoadModel("models/chap_5_dinner.glb");
    level->scenes[2]           = LoadModel("models/dinner_party.glb");

    level->models.train         = LoadModel("models/train.glb");
    level->models.train_door    = LoadModel("models/train_door.glb");

    level->clerk.body          = LoadModel("models/guy.glb");

    level->models.guy_sitting   = LoadModel("models/guy_sitting.glb");
    level->models.chair         = LoadModel("models/chair.glb");
    level->models.table         = LoadModel("models/dinner_table.glb");
    level->models.pyramid_head  = LoadModel("models/pyramid_head.glb");
    level->models.real_head     = LoadModel("models/real_head.glb");

    chapter_5_window_text(true,
                          &game->text[0],
                          "I don't have the courage to do\nwhat the Lord has asked.",
                          WHITE,
                          &game->text[1]);
    chapter_5_window_text(true,
                          &game->text[1],
                          "It's been weeks.",
                          WHITE,
                          &game->text[2]);
    chapter_5_window_text(true,
                          &game->text[2],
                          "I need to clear my head.",
                          WHITE,
                          nullptr);

    // clerk dialogue

    chapter_5_text(&game->text[3],
                   "Chase",
                   "Um...\rGood night.",
                   30,
                   &game->text[4]);
    chapter_5_text(&game->text[4],
                   "Clerk",
                   "What the hell do you want?\rWhy are you out here so late?",
                   30,
                   &game->text[5]);
    chapter_5_text(&game->text[5],
                   "Chase",
                   "Um...",
                   30,
                   &game->text[6]);
    chapter_5_text(&game->text[6],
                   "Clerk",
                   "Do your parents know that you're here?\rCome on, spit it out, boy!",
                   30,
                   &game->text[7]);
    chapter_5_text(&game->text[7],
                   "Chase",
                   "C-can you stop looking at me like that?\rIt's a bit creepy.",
                   30,
                   &game->text[8]);
    chapter_5_text(&game->text[8],
                   "Clerk",
                   "Looking at you like what, sir?",
                   30,
                   nullptr);

    game->text[8].callbacks[0] = chapter_5_begin_head_flip;

    chapter_5_text(&game->text[10],
                   "Clerk",
                   "Looking at you like what, sir?",
                   30,
                   &game->text[11]);
    chapter_5_text(&game->text[11],
                   "Clerk",
                   "Looking at you like what, sir?",
                   30,
                   &game->text[12]);
    chapter_5_text(&game->text[12],
                   "Clerk",
                   "Looking at you like what, sir?",
                   30,
                   nullptr);

    game->text[12].callbacks[0] = chapter_5_begin_black_transition;

    chapter_5_text(&game->text[15],
                   "Clerk",
                   "Um... sir?\r... Good night.",
                   30,
                   &game->text[16]);
    chapter_5_text(&game->text[16],
                   "Clerk",
                   "I was just saying,\ryou should be careful out there, it's very late.\r"
                   "... Erm, how old are you?",
                   30,
                   &game->text[17]);
    chapter_5_text(&game->text[17],
                   "Chase",
                   "Don't worry, I get that a lot.\rI look young.",
                   30,
                   &game->text[18]);
    chapter_5_text(&game->text[18],
                   "Chase",
                   "Uh, one ticket to Nirvana, please.",
                   30,
                   &game->text[19]);
    chapter_5_text(&game->text[19],
                   "Clerk",
                   "Going for quite an adventure today, are we?",
                   30,
                   &game->text[20]);
    chapter_5_text(&game->text[20],
                   "Chase",
                   "Like you won't believe.",
                   30,
                   &game->text[21]);

    game->text[20].callbacks[0] = chapter_5_give_ticket;

    chapter_5_text(&game->text[21],
                   "Clerk",
                   "Here's the ticket.\rBe safe.\rThe train is coming any minute.",
                   30,
                   &game->text[22]);
    chapter_5_text(&game->text[22],
                   "Chase",
                   "Thanks.",
                   30,
                   nullptr);
    game->text[22].callbacks[0] = chapter_5_train_move;

    chapter_5_goto_scene(game, CHAPTER_5_SCENE_DINNER_PARTY);
}

void chapter_5_update_clerk(Game *game, float dt) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    Chapter_5_Clerk *clerk = &level->clerk;

    Vector2 player_p = { level->camera.position.x, level->camera.position.z };
    Vector2 clerk_p  = { clerk->position.x, clerk->position.z  };

    if (!clerk->has_real_head) {
        float time = GetTime();
        float sine = 1.5f * (sinf(time) + sinf(2 * time) + sinf(2.5f * time)); 
        float t    = 0.5f;

        if (level->state == CHAPTER_5_STATE_TRAIN_STATION_1 && clerk->do_180_head) {
            float to_angle = atan2f(clerk_p.y - player_p.y, clerk_p.x - player_p.x);
            float to = RAD2DEG * -to_angle + 90;

            float diff = to - clerk->head_rotation;

            if (fabs(diff) > 2) {
                int dir = sign(diff);

                float speed = 30;
                //speed = 120;

                clerk->head_rotation += dir * speed * dt;
            } else if (game->current < &game->text[10] && level->black_screen_timer == -1) {
                game->current = &game->text[10];
                clerk->head_rotation = to;
            }

            clerk->saved_head_rotation = clerk->head_rotation;
        } else if (Vector2Distance(player_p, clerk_p) <= 10) {
            float to_angle = RAD2DEG * -atan2f(clerk_p.y - player_p.y, clerk_p.x - player_p.x);

            to_angle += sine;

            clerk->head_rotation = lerp_dt(clerk->head_rotation, to_angle - 90, t, dt);
            clerk->saved_head_rotation = clerk->head_rotation;
        } else {
            float to_angle = clerk->saved_head_rotation + 90;

            to_angle += sine;
            clerk->head_rotation = lerp_dt(clerk->head_rotation, to_angle - 90, t, dt);
        }
    }

    clerk->talk_popup = (game->current == nullptr &&
                         Vector2Distance(player_p, clerk_p) <= 6 &&
                         clerk->do_180_head == false &&
                         !clerk->talked);

    if (is_action_pressed() && clerk->talk_popup) {
        if (level->state == CHAPTER_5_STATE_TRAIN_STATION_1) {
            clerk->talked = true;
            game->current = &game->text[3];
        }
    }
}

void chapter_5_update_train(Game *game, float dt) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;
    Chapter_5_Train *train = &level->train;

#if 1
    if (IsKeyPressed(KEY_K)) {
        train->position.x = 0;
        train->closed = true;
        train->moving = false;
    }

    if (IsKeyPressed(KEY_C)) {
        train->closed = !train->closed;
    }

    if (IsKeyPressed(KEY_G)) {
        train->moving = true;
    }
#endif

    float prev_x = train->position.x;

    float train_speed = 60;

    if (train->moving) {
        if (train->position.x < 0) {
            train->position.x += train_speed * dt;
            if (train->position.x >= 0) {
                train->position.x = 0;
                train->moving = false;
                train->door_open_alarm = 2;
            }
        } else {
            train->position.x += train_speed * dt;
        }
    }

    train->delta_x = train->position.x - prev_x;

    if (train->door_open_alarm > 0) {
        train->door_open_alarm -= dt;

        if (train->door_open_alarm <= 0) {
            train->door_open_alarm = 0;
            train->closed = false;
        }
    }

    if (!train->closed && train->able_to_close) {
        if (train->player_in) {
            train->player_in_timer += dt;

            if (train->player_in_timer >= 2) {
                train->closed = true;
                train->setoff_timer = 3;
            }
        } else {
            train->player_in_timer = 0;
        }
    }

    if (train->setoff_timer > 0) {
        train->setoff_timer -= dt;
        if (train->setoff_timer <= 0) {
            train->setoff_timer = 0;
            train->moving = true;
            level->ticket = false;
        }
    }

    if (level->current_scene == CHAPTER_5_SCENE_TRAIN_STATION &&
        train->moving && train->player_in)
    {
        level->transition_fade += 0.5 * dt;

        if (level->transition_fade >= 1) {
            level->transition_fade = 0;
            chapter_5_goto_scene(game, CHAPTER_5_SCENE_STAIRCASE);
        }
    }

    float openness_to;
    if (train->closed)
        openness_to = 0;
    else
        openness_to = 1;

    // TODO: dt
    train->door_openness = go_to(train->door_openness,
                                 openness_to,
                                 1 * dt);
}

void chapter_5_draw_table(Level_Chapter_5 *level, Chapter_5_Table *table) {
    DrawModel(level->models.table, table->position, 1, WHITE);

    // Draw the chairs
    Model *chair = &level->models.chair;
    Model *person = &level->models.guy_sitting;
    Model *head = &level->models.pyramid_head;

    for (int i = 0; i < table->num_guys; i++) {
        Chapter_5_Guy *guy = &table->guys[i];

        Color color = PINK;
        if (guy->is_male) {
            color = BLUE;
        }

        float radius = 1.5;

        float x = table->position.x + radius * cosf(guy->table_angle);
        float z = table->position.z + radius * sinf(guy->table_angle);

        float angle = atan2f(table->position.z - z, table->position.x - x);

        float head_angle = atan2f(level->camera.position.z - z, level->camera.position.x - x);

        float time = GetTime();
        float sine = 1.5f * (sinf(time) + sinf(2 * time) + sinf(2.5f * time)); 
        
        head_angle += 0.02f * sine;

        Vector3 chair_pos = { x, 0, z };
        DrawModelEx(*chair, chair_pos, {0,1,0}, RAD2DEG * -angle, {1,1,1}, WHITE);
        DrawModelEx(*person, chair_pos, {0,1,0}, RAD2DEG * -angle, {1,1,1}, color);
        DrawModelEx(*head, Vector3Add(chair_pos, {0,1.6f,0}), {0,1,0}, 90 + RAD2DEG * -head_angle, {0.75f,0.75f,0.75f}, WHITE);
    }
}

void chapter_5_draw_train(Level_Chapter_5 *level, Chapter_5_Train *train) {
    int length = chapter_5_train_length(train);

    for (int i = 0; i < train->instances; i++) {
        int k = i - train->instances/2;

        DrawMesh(level->models.train.meshes[0],
                 level->models.train.materials[1],
                 MatrixTranslate(train->position.x + k * length,
                                 train->position.y,
                                 train->position.z));
    }

    float openness = train->door_openness * 2;

    for (int i = 0; i < train->instances; i++) {
        int k = i - train->instances/2;

        {
            Vector3 door_position = train->position;
            door_position.x += k * length;
            door_position = Vector3Add(door_position, {-1.2f, 0.3f, 1.9f});

            if (k == 0)
                door_position.x += openness;

            DrawMesh(level->models.train_door.meshes[0],
                     level->models.train_door.materials[1],
                     MatrixTranslate(door_position.x,
                                     door_position.y,
                                     door_position.z));
        }

        {
            Vector3 door_position = train->position;
            door_position.x += k * length;
            door_position = Vector3Add(door_position, {-2.9f, 0.3f, 1.9f});

            if (k == 0)
                door_position.x -= openness;

            float x_inverse_floats[] = {
                -1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 0, 0, 0, 1,
            };
            Matrix x_inverse;
            memcpy(&x_inverse, x_inverse_floats, sizeof(x_inverse_floats));

            Matrix translate = MatrixTranslate(door_position.x,
                                               door_position.y,
                                               door_position.z);

            DrawMesh(level->models.train_door.meshes[0],
                     level->models.train_door.materials[1],
                     MatrixMultiply(x_inverse, translate));
        }
    }
}

void chapter_5_update_camera(Camera3D *camera, float dt) {
    int dir_x = key_right() - key_left();
    int dir_y = key_down() - key_up();

    float speed = 5;
    if (IsKeyDown(KEY_LEFT_SHIFT)) speed = 20;

    Vector3 saved = camera->target;
    CameraMoveForward(camera, -dir_y * speed * dt, true);
    CameraMoveRight(camera, dir_x * speed * dt, true);
    camera->target = saved;
}

void chapter_5_update_camera_look(Camera3D *camera) {
    Vector2 mouse = GetMouseDelta();

    const float sensitivity = 0.005f;

    mouse.x *= sensitivity;
    mouse.y *= sensitivity;

    CameraYaw(camera, -mouse.x, false);
    CameraPitch(camera, -mouse.y, true, false, false);
}

bool is_mesh_collider(Model *model, int mesh_index) {
    int material_index = model->meshMaterial[mesh_index];
    Material *material = model->materials + material_index;

    Color c = material->maps[0].color;

    if (c.r == 255 && c.g == 0 && c.b == 0) {
        return true;
    }

    return false;
}

void apply_3d_velocity(Camera3D *camera, Model world, Vector3 pos_vel) {
    struct Is_Bad_Position_Result {
        bool ok;
        RayCollision collision_result;
    };

    auto is_bad_position = [&](Vector3 initial_pos, Vector3 pos, Model model) -> Is_Bad_Position_Result {
        Is_Bad_Position_Result result = {};

        Ray ray = {};
        ray.direction = {0, -1, 0};
        ray.position = pos;

        // Check bottom collision
        RayCollision highest = {};
        highest.point.y = -9999;

        for (int j = 0; j < model.meshCount; j++) {
            if (!is_mesh_collider(&model, j)) {
                continue;
            }

            Mesh mesh = model.meshes[j];

            Matrix transform = MatrixIdentity();
            result.collision_result = GetRayCollisionMesh(ray, mesh, transform);

            if (result.collision_result.hit) {
                if (result.collision_result.point.y > highest.point.y) {
                    highest = result.collision_result;
                    result.ok = true;
                }
            }
        }

        result.collision_result = highest;

        // Check forward direction.
        ray = {};
        ray.direction = Vector3Normalize(Vector3Subtract(pos, initial_pos));
        ray.position = initial_pos;
        ray.position.y -= 1;

        float distance = Vector3Distance(pos, initial_pos);

        for (int j = 0; j < model.meshCount; j++) {
            if (!is_mesh_collider(&model, j)) {
                continue;
            }

            RayCollision col = GetRayCollisionMesh(ray, model.meshes[j], MatrixIdentity());

            if (col.hit) {
                if (col.distance < distance) {
                    result.ok = false;
                }
            }
        }

        return result;
    };

    auto resolve_axis = [&](float *axis, float axis_vel, Camera *cam, Model model) -> void {
        Vector3 initial = cam->position;

        *axis += axis_vel;

        Is_Bad_Position_Result result = is_bad_position(initial, cam->position, model);
        if (!result.ok) {
            *axis -= axis_vel;
        } else {
            cam->position.y = result.collision_result.point.y + 2;
        }
    };

    Vector3 saved_pos = camera->position;

    resolve_axis(&camera->position.x, pos_vel.x, camera, world);
    resolve_axis(&camera->position.z, pos_vel.z, camera, world);

    Vector3 diff = Vector3Subtract(camera->position, saved_pos);
    camera->target = Vector3Add(camera->target, diff);
}

void chapter_5_update_player_on_train(Game *game) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    Chapter_5_Train *train = &level->train;
    Vector3 *camera = &level->camera.position;

    float curb = 0, train_start_z = 0, train_end_z = 0, door_start = 0, door_end = 0, player_y = 0;

    switch (level->current_scene) {
        case CHAPTER_5_SCENE_TRAIN_STATION: {
            curb          = -20.0;
            train_start_z = -20.5;
            train_end_z   = -23.0;
            door_start    =  -3.5;
            door_end      =  +0.5;

            player_y = 4;
        } break;
        case CHAPTER_5_SCENE_STAIRCASE: {
            curb = 2.5;

            train_start_z = 1.5;
            train_end_z   = -1.5;

            door_start = -3.5;
            door_end = 0;

            player_y = 2;
        } break;
    }

    door_start += train->position.x;
    door_end   += train->position.x;

    if (train->player_in) {
        camera->y = player_y;

        level->camera.position.x += train->delta_x;
        level->camera.target.x   += train->delta_x;

        if (camera->z > train_start_z) { // trying back out
            if (train->closed) {
                camera->z = train_start_z;
            } else {
                if (camera->x >= door_start && camera->x <= door_end) {
                    train->player_in = false;
                } else {
                    camera->z = train_start_z;
                }
            }
        }

        int length = chapter_5_train_length(train);
        int x_start = -length * train->instances/2;
        int x_end   = +length * train->instances/2;

        x_start += train->position.x;
        x_end   += train->position.x;

        if (camera->x < x_start)
            camera->x = x_start;
        else if (camera->x > x_end)
            camera->x = x_end;

        if (camera->z < train_end_z)
            camera->z = train_end_z;
    } else if (camera->z < curb) { // trying to go in
        if (train->closed) {
            camera->z = curb;
        } else { 
            if (camera->x >= door_start && camera->x <= door_end) {
                if (camera->z < train_start_z)
                    train->player_in = true;
            } else {
                camera->z = curb;
            }
        }
    }
}

void chapter_5_update_player_train_station(Game *game, float dt) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    Vector3 stored_camera_pos = level->camera.position;

    bool can_move = keyboard_focus(game) == 0 && !level->clerk.do_180_head;

    if (can_move) {
        chapter_5_update_camera(&level->camera, dt);
    }

    chapter_5_update_player_on_train(game);

    Vector3 *camera = &level->camera.position;
    Chapter_5_Train *train = &level->train;

    float curb          = -20.0;
    if (camera->z > curb && camera->z < -8 && camera->x > 15) {
        *camera = stored_camera_pos;
        //level->camera.target = stored_camera_target;
    }

    if (!train->player_in && camera->z > curb) {
        Vector3 velocity = Vector3Subtract(level->camera.position, stored_camera_pos);
        level->camera.position = stored_camera_pos;

        apply_3d_velocity(&level->camera, level->scenes[0], velocity);
    }

    if (can_move) {
        chapter_5_update_camera_look(&level->camera);
    }

    /*
    printf("{%.2ff, %.2ff, %.2ff} {%.2ff, %.2ff, %.2ff}\n",
           level->camera.position.x,
           level->camera.position.y,
           level->camera.position.z,
           level->camera.target.x,
           level->camera.target.y,
           level->camera.target.z);
           */
}

void chapter_5_update_player_staircase(Game *game, float dt) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    Vector3 stored_camera_pos = level->camera.position;

    bool can_move = keyboard_focus(game) == 0;

    if (can_move)
        chapter_5_update_camera(&level->camera, dt);

    level->door_popup = (Vector3Distance(level->camera.position, {13.25f, 25.75f, 48.25f}) < 5);
    if (level->door_popup && is_action_pressed()) {
        chapter_5_goto_scene(game, CHAPTER_5_SCENE_DINNER_PARTY);
    }

    chapter_5_update_player_on_train(game);

    if (!level->train.player_in && level->camera.position.z > 0) {
        Vector3 velocity = Vector3Subtract(level->camera.position, stored_camera_pos);
        level->camera.position = stored_camera_pos;

        apply_3d_velocity(&level->camera, level->scenes[1], velocity);
    }

    if (can_move)
        chapter_5_update_camera_look(&level->camera);

    /*
    printf("{%.2ff, %.2ff, %.2ff} {%.2ff, %.2ff, %.2ff}\n",
           level->camera.position.x,
           level->camera.position.y,
           level->camera.position.z,
           level->camera.target.x,
           level->camera.target.y,
           level->camera.target.z);
           */
}

void chapter_5_update_player_dinner_party(Game *game, float dt) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    Vector3 stored_camera_pos = level->camera.position;

    chapter_5_update_camera(&level->camera, dt);
    Vector3 velocity = Vector3Subtract(level->camera.position, stored_camera_pos);
    level->camera.position = stored_camera_pos;

    apply_3d_velocity(&level->camera, level->scenes[2], velocity);

    chapter_5_update_camera_look(&level->camera);
}

void chapter_5_update(Game *game, float dt) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    switch (level->current_scene) {
        case CHAPTER_5_SCENE_TRAIN_STATION: {
            switch (level->state) {
                case CHAPTER_5_STATE_INTRO: {
                    if (game->current == nullptr) {
                        level->state = CHAPTER_5_STATE_TRAIN_STATION_1;
                    }
                } break;
                case CHAPTER_5_STATE_TRAIN_STATION_1: {
                    if (level->black_screen_timer > 0) {
                        level->black_screen_timer -= dt;
                        if (level->black_screen_timer <= 0) {
                            level->black_screen_timer = 0.5;
                            level->state = CHAPTER_5_STATE_TRAIN_STATION_2;
                        }
                    }

                    chapter_5_update_train(game, dt);
                    chapter_5_update_clerk(game, dt);
                    chapter_5_update_player_train_station(game, dt);
                } break;
                case CHAPTER_5_STATE_TRAIN_STATION_2: {
                    if (level->black_screen_timer > 0) {
                        level->black_screen_timer -= dt;
                        if (level->black_screen_timer <= 0) {
                            level->black_screen_timer = 0;
                            chapter_5_train_station_init_positions(game, true);
                            level->clerk.has_real_head = true;
                            game->current = &game->text[15];
                        }
                    }

                    chapter_5_update_train(game, dt);
                    chapter_5_update_clerk(game, dt);
                    chapter_5_update_player_train_station(game, dt);
                } break;
            }
        } break;
        case CHAPTER_5_SCENE_STAIRCASE: {
            chapter_5_update_train(game, dt);
            chapter_5_update_player_staircase(game, dt);
        } break;
        case CHAPTER_5_SCENE_DINNER_PARTY: {
            chapter_5_update_player_dinner_party(game, dt);
        } break;
        case CHAPTER_5_SCENE_COTTAGE: {
        } break;
    }
}

void chapter_5_draw(Game *game) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    ClearBackground(BLACK);

    switch (level->current_scene) {
        case CHAPTER_5_SCENE_TRAIN_STATION: {
            switch (level->state) {
                case CHAPTER_5_STATE_TRAIN_STATION_1: case CHAPTER_5_STATE_TRAIN_STATION_2: {
                    SetShaderValue(level->shader, level->shader.locs[SHADER_LOC_VECTOR_VIEW], &level->camera.position.x, SHADER_UNIFORM_VEC3);

                    if (level->state == CHAPTER_5_STATE_TRAIN_STATION_2 && level->black_screen_timer != 0) {
                        break;
                    }

                    game->textbox_alpha = 180;

                    BeginMode3D(level->camera);
                    BeginShaderMode(level->shader);

                    DrawModel(level->scenes[0], {}, 1, WHITE);

                    DrawModelEx(level->clerk.body, level->clerk.position, {0,1,0}, level->clerk.body_rotation, {1,1,1}, WHITE);

                    Model *model = &level->models.pyramid_head;
                    Vector3 scale = { 1, 1, 1 };

                    if (level->clerk.has_real_head) {
                        model = &level->models.real_head;
                        scale = { 0.75, 0.75, 0.75 };
                    }

                    DrawModelEx(model[0], Vector3Add(level->clerk.position, {0,1.95f,0}), {0,1,0}, level->clerk.head_rotation, scale, WHITE);

                    chapter_5_draw_train(level, &level->train);

                    EndShaderMode();

#if 0
                    if (level->train.moving && level->train.player_in) {
                        for (int i = 0; i < StaticArraySize(level->stars); i++) {
                            DrawSphere(level->stars[i], 1, WHITE);
                        }
                    }
#endif

                    EndMode3D();

                    if (level->ticket) {
                        DrawTexture(atari_assets.textures[0],
                                    render_width - atari_assets.textures[0].width,
                                    render_height - atari_assets.textures[0].height,
                                    WHITE);
                    }

                    DrawRectangle(0, 0, render_width, render_height, {0,0,0,(uint8_t)(level->transition_fade * 255)});

                    if (level->clerk.talk_popup) {
                        draw_popup("Talk to the Clerk");
                    }
                } break;
            }
        } break;
        case CHAPTER_5_SCENE_STAIRCASE: {
            BeginMode3D(level->camera);

            BeginShaderMode(level->shader);
            DrawModel(level->scenes[1], {}, 1, WHITE);
            chapter_5_draw_train(level, &level->train);

            EndShaderMode();

            EndMode3D();

            if (level->door_popup)
                draw_popup("Open door", BLACK);
        } break;
        case CHAPTER_5_SCENE_DINNER_PARTY: {
            game->textbox_alpha = 180;

            BeginMode3D(level->camera);

            BeginShaderMode(level->shader);
            DrawModel(level->scenes[2], {}, 1, WHITE);

            for (int i = 0; i < level->num_tables; i++) {
                chapter_5_draw_table(level, level->tables + i);
            }
            EndShaderMode();

            EndMode3D();
        } break;
        case CHAPTER_5_SCENE_COTTAGE: {
        } break;
    }
}

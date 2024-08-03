enum Chapter_5_State {
    CHAPTER_5_STATE_INTRO,
    CHAPTER_5_STATE_TRAIN_STATION_1,
    CHAPTER_5_STATE_TRAIN_STATION_2,
};

enum Chapter_5_Scene {
    CHAPTER_5_SCENE_TRAIN_STATION,
    CHAPTER_5_SCENE_DINNER_PARTY,
    CHAPTER_5_SCENE_COTTAGE,
    CHAPTER_5_SCENE_COUNT
};

struct Chapter_5_Clerk {
    Vector3 position;
    Model   body;
    Model   pyramid_head;
    Model   real_head;

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
    BoundingBox bounding_box;
    Model       model;
    int         instances;
    float       time;
};

struct Level_Chapter_5 {
    Chapter_5_State state;
    Chapter_5_Clerk clerk;
    Chapter_5_Train train;

    Camera3D camera;
    Shader   shader;

    float    black_screen_timer;

    Model    scenes[CHAPTER_5_SCENE_COUNT];
    int      current_scene;

    bool     ticket;
};

void chapter_5_window_text(bool scroll, Text_List *list, char *line, Color color, Text_List *next);
void chapter_5_text(Text_List *list, char *speaker, char *line, float scroll_speed, Text_List *next);
void chapter_5_update_camera(Camera3D *camera, float dt);
void chapter_5_begin_head_flip(void *game_ptr);
void chapter_5_begin_black_transition(void *game_ptr);
void chapter_5_give_ticket(void *game_ptr);

void chapter_5_init_positions(Game *game) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    Chapter_5_Clerk *clerk = &level->clerk;

    clerk->do_180_head = false;
    clerk->talk_popup = false;
    clerk->position = { 16.3f, 2.f, -14.1f };
    clerk->body_rotation = -90;
    clerk->head_rotation = -90;
    level->clerk.saved_head_rotation = clerk->head_rotation;
    clerk->has_real_head = false;
    clerk->talked = false;

    level->camera.position = {-7.19f, 2.00f, 6.68f};
    level->camera.target = {11.60f, 18.12f, -46.27f};
    level->camera.up       = {0, 1, 0};
    level->camera.fovy     = FOV_DEFAULT;
    level->camera.projection = CAMERA_PERSPECTIVE;
}

void chapter_5_init(Game *game) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    game->render_state = RENDER_STATE_3D;

    level->state = CHAPTER_5_STATE_TRAIN_STATION_1;
    level->black_screen_timer = -1;

    DisableCursor();

    render_width  = DIM_3D_WIDTH;
    render_height = DIM_3D_HEIGHT;

    atari_assets.textures[0] = load_texture("art/ticket_pov.png");

    game->textbox_target = LoadRenderTexture(render_width, render_height);
    game->textbox_alpha = 255;

    chapter_5_init_positions(game);

    level->scenes[0] = LoadModel("models/train_station.glb");

    level->train.position     = {-500, 1.8f, -21.5f};
    level->train.model        = LoadModel("models/train.glb");
    level->train.bounding_box = GetMeshBoundingBox(level->train.model.meshes[0]);
    level->train.instances    = 10;
    level->train.time = 0;

    level->clerk.body = LoadModel("models/guy.glb");

    level->clerk.real_head    = LoadModel("models/real_head.glb");
    level->clerk.pyramid_head = LoadModel("models/pyramid_head.glb");

    level->shader = LoadShader("shaders/basic.vs", "shaders/basic.fs");

    level->shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(level->shader, "matModel");
    level->shader.locs[SHADER_LOC_VECTOR_VIEW]  = GetShaderLocation(level->shader, "viewPos");

    // TODO: Apply to all scenes
    for (int i = 0; i < level->scenes[0].materialCount; i++) {
        level->scenes[0].materials[i].shader = level->shader;
    }

    CreateLight(LIGHT_POINT, { 0, 1, 0 }, Vector3Zero(), ORANGE, level->shader);
    CreateLight(LIGHT_POINT, { 0, 1, 9 }, Vector3Zero(), ORANGE, level->shader);
    CreateLight(LIGHT_POINT, { 4, 4, 4 }, Vector3Zero(), ORANGE, level->shader);
    CreateLight(LIGHT_POINT, { 0, 1, -28 }, Vector3Zero(), ORANGE, level->shader);

    int ambientLoc = GetShaderLocation(level->shader, "ambient");
    float data[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    SetShaderValue(level->shader, ambientLoc, data, SHADER_UNIFORM_VEC4);

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

    chapter_5_text(&game->text[3],
                   "Chase",
                   "Um...\rGood night.",
                   30,
                   &game->text[4]);
    chapter_5_text(&game->text[4],
                   "Clerk",
                   "What the hell do you want?\rWhy are you out here so late?\rDo your parents know you're out?",
                   30,
                   &game->text[5]);
    chapter_5_text(&game->text[5],
                   "Chase",
                   "W-well, I'd like um-\rO-one ti-",
                   30,
                   &game->text[6]);
    chapter_5_text(&game->text[6],
                   "Clerk",
                   "Come on, spit it out, boy!",
                   30,
                   &game->text[7]);
    chapter_5_text(&game->text[7],
                   "Chase",
                   "C-can you stop looking at me like that?\rIt's a bit creepy.\rAnd what are you saying? Your tone is really\ninappropriate for a ticket clerk.",
                   30,
                   nullptr);
    game->text[7].callbacks[0] = chapter_5_begin_head_flip;
    chapter_5_text(&game->text[8],
                   "Clerk",
                   "...\r... Sorry, what do you mean?\rI wasn't acting odd.\rAre you feeling alright, sir?",
                   30,
                   nullptr);
    game->text[8].callbacks[0] = chapter_5_begin_black_transition;

    chapter_5_text(&game->text[10],
                   "Chase",
                   "Um...\rGood night.",
                   30,
                   &game->text[11]);
    chapter_5_text(&game->text[11],
                   "Clerk",
                   "Oh, hello!\rYou should be careful out there, it's very late.\r"
                   "My apologies, but you look sort of young,\ndo your--",
                   30,
                   &game->text[12]);
    chapter_5_text(&game->text[12],
                   "Chase",
                   "Don't worry, I get that a lot.\rI'm fine.",
                   30,
                   &game->text[13]);
    chapter_5_text(&game->text[13],
                   "Chase",
                   "Uh, one ticket to El Dorado, please.",
                   30,
                   &game->text[14]);
    chapter_5_text(&game->text[14],
                   "Clerk",
                   "Going for quite an adventure today, are we?",
                   30,
                   &game->text[15]);
    chapter_5_text(&game->text[15],
                   "Chase",
                   "Like you won't believe.",
                   30,
                   &game->text[16]);

    game->text[15].callbacks[0] = chapter_5_give_ticket;

    chapter_5_text(&game->text[16],
                   "Clerk",
                   "Here's the ticket.\rBe safe.\rThe train is coming any minute.",
                   30,
                   &game->text[17]);
    chapter_5_text(&game->text[17],
                   "Chase",
                   "Thanks.",
                   30,
                   nullptr);
}

void chapter_5_update_clerk(Game *game, float dt) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;
    (void)dt;

    Chapter_5_Clerk *clerk = &level->clerk;

    Vector2 player_p = { level->camera.position.x, level->camera.position.z };
    Vector2 clerk_p  = { clerk->position.x,  clerk->position.z  };

    if (!clerk->has_real_head) {
        float time = GetTime();
        float sine = 1.5f * (sinf(time) + sinf(2*time) + sinf(2.5f * time)); 
        float t = 0.5f;

        if (level->state == CHAPTER_5_STATE_TRAIN_STATION_1 && clerk->do_180_head) {
            float to_angle = RAD2DEG * -atan2f(clerk_p.y - player_p.y, clerk_p.x - player_p.x);
            float to = to_angle + 90;

            float diff = to - clerk->head_rotation;

            if (fabs(diff) > 2) {
                int dir = sign(diff);

                clerk->head_rotation += dir * 0.5;
            } else if (level->black_screen_timer == -1) {
                game->current = &game->text[8];
                clerk->head_rotation = to;
            }

            clerk->saved_head_rotation = clerk->head_rotation;
        } else if (Vector2Distance(player_p, clerk_p) <= 10) {
            float to_angle = RAD2DEG * -atan2f(clerk_p.y - player_p.y, clerk_p.x - player_p.x);

            to_angle += sine;

            clerk->head_rotation = Lerp(clerk->head_rotation, to_angle - 90, t);
            clerk->saved_head_rotation = clerk->head_rotation;
        } else {
            float to_angle = clerk->saved_head_rotation + 90;

            to_angle += sine;
            clerk->head_rotation = Lerp(clerk->head_rotation, to_angle - 90, t);
        }
    }

    clerk->talk_popup = (game->current == nullptr &&
                         Vector2Distance(player_p, clerk_p) <= 6 &&
                         clerk->do_180_head == false &&
                         !clerk->talked);

    if (is_action_pressed() && clerk->talk_popup) {
        clerk->talked = true;

        if (level->state == CHAPTER_5_STATE_TRAIN_STATION_1) {
            game->current = &game->text[3];
        } else {
            game->current = &game->text[10];
        }
    }
}

void chapter_5_update_player(Game *game, float dt) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    Vector3 stored_camera_pos = level->camera.position;
    Vector3 stored_camera_target = level->camera.target;

    if (keyboard_focus(game) == 0)
        if (!level->clerk.do_180_head)
            chapter_5_update_camera(&level->camera, dt);

    BoundingBox train_box = level->train.bounding_box;
    Vector3 translation = level->train.position;
    train_box.min = Vector3Add(train_box.min, translation);
    train_box.max = Vector3Add(train_box.max, translation);

    if (CheckCollisionBoxSphere(train_box, level->camera.position, 1)) {
        level->camera.position.y = 4;
    } else {
        Ray ray = {};

        ray.position = level->camera.position;
        ray.direction = {0, -1, 0};

        bool hit = true;

        RayCollision highest = {};
        highest.point.y = -999;

        for (int i = 0; i < level->scenes[0].meshCount; i++) {
            Mesh mesh = level->scenes[0].meshes[i];
            Matrix transform = MatrixIdentity();
            RayCollision result = GetRayCollisionMesh(ray, mesh, transform);

            if (result.hit) {
                if (result.point.y > highest.point.y) {
                    highest = result;
                    hit = true;
                }
            }
        }

        if (hit) {
            RayCollision result = highest;

            Vector3 desired_camera_position = Vector3Add(result.point, {0, 2, 0});

            Vector3 delta = Vector3Subtract(desired_camera_position, level->camera.position);

            level->camera.position = desired_camera_position;
            level->camera.target = Vector3Add(level->camera.target, delta);
        } else {
            // Revert position
            level->camera.position = stored_camera_pos;
            level->camera.target = stored_camera_target;
        }
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

void chapter_5_update(Game *game, float dt) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    switch (level->state) {
        case CHAPTER_5_STATE_INTRO: {
            if (game->current == nullptr) {
                level->state = CHAPTER_5_STATE_TRAIN_STATION_1;
            }
        } break;
        case CHAPTER_5_STATE_TRAIN_STATION_2: {
            if (level->black_screen_timer > 0) {
                level->black_screen_timer -= dt;
                if (level->black_screen_timer <= 0) {
                    level->black_screen_timer = 0;
                    chapter_5_init_positions(game);
                    level->clerk.has_real_head = true;
                }
            }

            if (IsKeyDown(KEY_P)) {
                if (level->train.position.x < 0) {
                    level->train.position.x += 2.5;
                    if (level->train.position.x > 0)
                        level->train.position.x = 0;
                }
            }

            chapter_5_update_clerk(game, dt);
            chapter_5_update_player(game, dt);
        } break;
        case CHAPTER_5_STATE_TRAIN_STATION_1: {
            if (level->black_screen_timer > 0) {
                level->black_screen_timer -= dt;
                if (level->black_screen_timer <= 0) {
                    level->black_screen_timer = 0.5;
                    level->state = CHAPTER_5_STATE_TRAIN_STATION_2;
                }
            }

            chapter_5_update_clerk(game, dt);
            chapter_5_update_player(game, dt);
        } break;
    }
}

void chapter_5_draw(Game *game) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    ClearBackground(BLACK);

    switch (level->state) {
        case CHAPTER_5_STATE_TRAIN_STATION_1: case CHAPTER_5_STATE_TRAIN_STATION_2: {
            if (level->state == CHAPTER_5_STATE_TRAIN_STATION_2 && level->black_screen_timer != 0) {
                break;
            }

            game->textbox_alpha = 180;

            SetShaderValue(level->shader, level->shader.locs[SHADER_LOC_VECTOR_VIEW], &level->camera.position.x, SHADER_UNIFORM_VEC3);

            BeginMode3D(level->camera);
            BeginShaderMode(level->shader);

            DrawModel(level->scenes[0], {}, 1, WHITE);

            DrawModelEx(level->clerk.body, level->clerk.position, {0,1,0}, level->clerk.body_rotation, {1,1,1}, WHITE);

            Model *model = &level->clerk.pyramid_head;
            if (level->clerk.has_real_head)
                model = &level->clerk.real_head;

            DrawModelEx(model[0], Vector3Add(level->clerk.position, {0,1.95f,0}), {0,1,0}, level->clerk.head_rotation, {1,1,1}, WHITE);

            // Draw train

            //DrawModel(level->train.model, level->train.position, 1, WHITE);
            /*
            DrawMesh(level->train.model.meshes[0],
                     level->train.model.materials[1],
                     MatrixTranslate(level->train.position.x,
                                     level->train.position.y,
                                     level->train.position.z));
                                     */

            /*
            Matrix *transforms = (Matrix *)arena_push(&game->frame_arena, level->train.instances * sizeof(Matrix));

            Vector3 p = level->train.position;
            for (int i = 0; i < level->train.instances; i++) {
                transforms[i] = MatrixTranslate(p.x, p.y, p.z);
                //p.x += level->train.bounding_box.max.x - level->train.bounding_box.min.x;
            }

            DrawMeshInstanced(level->train.model.meshes[0],
                              level->train.model.materials[1],
                              transforms,
            1);

            Matrix transforms[1] = {MatrixIdentity()};
            DrawMeshInstanced(level->train.model.meshes[0],
                              level->train.model.materials[1],
                              transforms,
                              1);
            */


            int length = level->train.bounding_box.max.x - level->train.bounding_box.min.x + 1;

            for (int i = 0; i < level->train.instances; i++) {
                int k = i - level->train.instances/2;

                DrawMesh(level->train.model.meshes[0],
                         level->train.model.materials[1],
                         MatrixTranslate(level->train.position.x + k * length,
                                         level->train.position.y,
                                         level->train.position.z));
            }

            EndShaderMode();

            EndMode3D();

            if (level->ticket) {
                DrawTexture(atari_assets.textures[0],
                            render_width - atari_assets.textures[0].width,
                            render_height - atari_assets.textures[0].height,
                            WHITE);
            }

            if (level->clerk.talk_popup) {
                draw_popup("Talk to the Clerk");
            }
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

void chapter_5_update_camera(Camera3D *camera, float dt) {
    int dir_x = key_right() - key_left();
    int dir_y = key_down() - key_up();

    float speed = 5;
    if (IsKeyDown(KEY_LEFT_SHIFT)) speed = 20;
    
    CameraMoveForward(camera, -dir_y * speed * dt, true);
    CameraMoveRight(camera, dir_x * speed * dt, true);

    Vector2 mouse = GetMouseDelta();

    const float sensitivity = 0.005f;

    mouse.x *= sensitivity;
    mouse.y *= sensitivity;

    CameraYaw(camera, -mouse.x, false);
    CameraPitch(camera, -mouse.y, true, false, false);
}

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

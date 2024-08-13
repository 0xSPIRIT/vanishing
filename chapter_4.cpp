enum Chapter_4_State {
    CHAPTER_4_STATE_ATARI,
    CHAPTER_4_STATE_BED_1,
    CHAPTER_4_STATE_3D
};

struct Cutscene_Frame {
    int   texture_id;
    float frame_time;
};

struct Cutscene {
    bool active;

    float scale;

    Cutscene_Frame *frames;

    int frame_count;
    int current_frame;

    float timer;
};

struct Chapter_4_Text {
    // Each alarm corresponds to a rest in the text, denoted by when
    // the current Text_List's next pointer is null. After alarm[current_index],
    // we set game->current to the actual next Text_List.
    float      alarms[20];
    Text_List *next_list[20];

    int        current_index;
};

struct Level_Chapter_4 {
    Chapter_4_State state;
    float text_start_timer, end_timer;

    Cutscene bed_cutscene;

    Chapter_4_Text text_handler;

    Entity *player;
    Entity *window;

    bool window_popup;
    bool bed_popup;

    Camera3D camera;
    Model scene;
    Shader shader;
};

Entity *chapter_4_make_entity  (Entity_Type type, float x, float y);
void    chapter_4_entity_update(Entity *entity, Game *game, float dt);
void    chapter_4_entity_draw  (Entity *entity, Game *game);

void    draw_cutscene(Cutscene *cutscene, float dt);

void chapter_4_window_text(bool scroll, Text_List *list, char *line,
                           Color color, Text_List *next)
{
    list->font         = &atari_font;
    list->font_spacing = 1;
    list->scale        = 0.125;
    list->scroll_speed = 10;
    //list->alpha_speed  = 0.5;
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

void chapter_4_start_end_timer(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    Level_Chapter_4 *level = (Level_Chapter_4 *)game->level;

    level->end_timer = 4;
}

void chapter_4_3d_init(Game *game) {
    Level_Chapter_4 *level = (Level_Chapter_4 *)game->level;

    level->state = CHAPTER_4_STATE_3D;

    level->text_start_timer = 3;
    game->render_state = RENDER_STATE_3D;

    DisableCursor();

    render_width  = DIM_3D_WIDTH;
    render_height = DIM_3D_HEIGHT;

    // TODO: memory leak
    game->textbox_target = LoadRenderTexture(render_width, render_height);
    game->textbox_alpha = 255;

    level->scene = LoadModel("models/bedroom.glb");

    float height = 1.67f;

    height = 0.9f;

    level->camera.position = { 2.81f, height, -0.126f };
    level->camera.target   = { 2.46f, 1.094f, 0.793f };
    level->camera.up       = { 0, 1, 0 };
    level->camera.fovy     = FOV_DEFAULT;
    level->camera.projection = CAMERA_PERSPECTIVE;

    level->shader = LoadShader("shaders/basic.vs", "shaders/chapter_4.fs");

    level->shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(level->shader, "matModel");
    level->shader.locs[SHADER_LOC_VECTOR_VIEW]  = GetShaderLocation(level->shader, "viewPos");

    for (int i = 0; i < level->scene.materialCount; i++) {
        level->scene.materials[i].shader = level->shader;
    }

    int ambientLoc = GetShaderLocation(level->shader, "ambient");
    float data[4] = { 0.2f, 0.2f, 0.2f, 1 };
    SetShaderValue(level->shader, ambientLoc, data, SHADER_UNIFORM_VEC4);

    /*
    float fogDensity = 0.22f;
    int fogDensityLoc = GetShaderLocation(level->shader, "fogDensity");
    SetShaderValue(level->shader, fogDensityLoc, &fogDensity, SHADER_UNIFORM_FLOAT);
    */

    CreateLight(LIGHT_POINT, { 0, 2, 0 }, Vector3Zero(), ORANGE, level->shader);
}

void chapter_4_init(Game *game) {
    Level_Chapter_4 *level = (Level_Chapter_4 *)game->level;

    level->state = CHAPTER_4_STATE_ATARI;

    Texture2D *textures = atari_assets.textures;
    textures[0] = load_texture("art/apartment2.png");
    textures[1] = load_texture("art/player.png");
    textures[2] = load_texture("art/window_opened.png");
    textures[3] = load_texture("art/window_closed.png");

    textures[4] = load_texture("art/chap4_scene_1.png");
    textures[5] = load_texture("art/chap4_scene_2.png");
    textures[6] = load_texture("art/chap4_scene_3.png");
    textures[7] = load_texture("art/chap4_scene_4.png");

    level->state = CHAPTER_4_STATE_ATARI;

    // Setup cutscene
    Cutscene *cutscene = &level->bed_cutscene;

    Cutscene_Frame bed_cutscene[] = {
        { 4, 4.0 },
        { 5, 4.5 },
        { 6, 1.0 },
        { 7, 5.0 },
    };

    cutscene->frame_count = StaticArraySize(bed_cutscene);
    cutscene->frames = (Cutscene_Frame *)arena_push(&game->level_arena,
                                                    cutscene->frame_count * sizeof(Cutscene_Frame));
    memcpy(cutscene->frames, bed_cutscene, sizeof(bed_cutscene));
    cutscene->scale = 2;
    cutscene->active = true;

    game->entities = make_array<Entity*>(20);

    level->player = chapter_4_make_entity(ENTITY_PLAYER, 84, 84);
    array_add(&game->entities, level->player);

    level->window = chapter_4_make_entity(ENTITY_CHAP_4_WINDOW, 151, 72);
    array_add(&game->entities, level->window);

    add_wall(&game->entities, {80, 8, 29, 44});

    chapter_4_window_text(true,
                          &game->text[0],
                          "Is that you?",
                          WHITE,
                          &game->text[1]);
    chapter_4_window_text(true,
                          &game->text[1],
                          "Have you come for me?",
                          WHITE,
                          nullptr);

    level->text_handler.alarms[0]    = 2;
    level->text_handler.next_list[0] = &game->text[2];

    chapter_4_window_text(true,
                          &game->text[2],
                          "I HEAR THEE, MY SON.",
                          RED,
                          &game->text[3]);
    chapter_4_window_text(true,
                          &game->text[3],
                          "WHAT DOST THOU DESIRE?",
                          RED,
                          nullptr);

    level->text_handler.alarms[1]    = 2;
    level->text_handler.next_list[1] = &game->text[4];

    chapter_4_window_text(true,
                          &game->text[4],
                          "Meaning and fulfillment.",
                          WHITE,
                          nullptr);

    level->text_handler.alarms[2]    = 2;
    level->text_handler.next_list[2] = &game->text[5];

    chapter_4_window_text(true,
                          &game->text[5],
                          "GOOD.",
                          RED,
                          &game->text[6]);
    chapter_4_window_text(true,
                          &game->text[6],
                          "TO GRANT YOUR PRAYER,\nYOU MUST COMPLETE A TASK FOR ME.",
                          RED,
                          &game->text[7]);
    chapter_4_window_text(true,
                          &game->text[7],
                          "SHOW ME THAT YOUR FAITH IS PURE.",
                          RED,
                          nullptr);

    level->text_handler.alarms[3]    = 2;
    level->text_handler.next_list[3] = &game->text[8];

    chapter_4_window_text(true,
                          &game->text[8],
                          "I will do anything you ask, Lord.",
                          WHITE,
                          nullptr);

    level->text_handler.alarms[4]    = 3;
    level->text_handler.next_list[4] = &game->text[9];

    game->text[9].font         = &atari_font;
    game->text[9].font_spacing = 4;
    game->text[9].scale        = 0.125;
    game->text[9].alpha_speed  = 0.125;
    game->text[9].scroll_speed = 120;
    game->text[9].color        = RED;
    game->text[9].center_text  = true;
    game->text[9].scroll_type  = LetterByLetter;
    game->text[9].render_type  = Bare;
    game->text[9].location     = Top;
    game->text[9].take_keyboard_focus = true;

    text_list_init(&game->text[9],
                   nullptr,
                   "ACEDD2EAAC3582AD42435742073BF948\n"\
                   "286F4F1392AC1B722EA896C3D0E3E3AB\n"\
                   "9E5EEA55961C85624D3C62706CF85097\n"\
                   "9E686B9E0A47B7E103C10928FA8A8143\n"\
                   "D63A8310EB8BCBDADFC6D36A1155C2FF\n"\
                   "ACEDD2EAAC3582AD42435742073BF948\n"\
                   "286F4F1392AC1B722EA896C3D0E3E3AB\n"\
                   "9E5EEA55961C85624D3C62706CF85097\n"\
                   "9E686B9E0A47B7E103C10928FA8A8143\n"\
                   "ACEDD2EAAC3582AD42435742073BF948\n"\
                   "286F4F1392AC1B722EA896C3D0E3E3AB\n"\
                   "9E5EEA55961C85624D3C62706CF85097\n"\
                   "9E686B9E0A47B7E103C10928FA8A8143\n"\
                   "D63A8310EB8BCBDADFC6D36A1155C2FF\n"\
                   "D63A8310EB8BCBDADFC6D36A1155C2FF\n"\
                   "ACEDD2EAAC3582AD42435742073BF948\n"\
                   "286F4F1392AC1B722EA896C3D0E3E3AB\n"\
                   "9E5EEA55961C85624D3C62706CF85097\n"\
                   "9E686B9E0A47B7E103C10928FA8A8143\n"\
                   "D63A8310EB8BCBDADFC6D36A1155C2FF",
                   nullptr);

    level->text_handler.alarms[5]    = 2;
    level->text_handler.next_list[5] = &game->text[10];

    chapter_4_window_text(true,
                          &game->text[10],
                          "DO YOU UNDERSTAND?",
                          RED,
                          &game->text[11]);

    chapter_4_window_text(true,
                          &game->text[11],
                          "I...",
                          WHITE,
                          &game->text[12]);

    chapter_4_window_text(true,
                          &game->text[12],
                          "DO YOU UNDERSTAND?",
                          RED,
                          nullptr);

    level->text_handler.alarms[6]    = 4;
    level->text_handler.next_list[6] = &game->text[13];

    chapter_4_window_text(true,
                          &game->text[13],
                          "Yes.",
                          WHITE,
                          nullptr);

    game->text[13].callbacks[0] = chapter_4_start_end_timer;
}

void chapter_4_3d_update_camera(Level_Chapter_4 *level, Camera3D *camera) {
    (void)level;

    Vector2 mouse = GetMouseDelta();

    const float sensitivity = 0.0025f;

    mouse.x *= sensitivity;
    mouse.y *= sensitivity;

    //CameraYaw(camera, -mouse.x, false);
    //CameraPitch(camera, -mouse.y, true, false, false);

    // Pitch Camera
    {
        float pitch_angle = -mouse.y;
        Vector3 up = GetCameraUp(camera);
        Vector3 target_position = Vector3Subtract(camera->target, camera->position);

        // clamp up
        float max_angle_up = Vector3Angle(up, target_position) - PI/2.5;
        max_angle_up -= 0.001f; // avoid numerical errors
        if (pitch_angle > max_angle_up)
            pitch_angle = max_angle_up;

        // clamp down
        float max_angle_down = Vector3Angle(Vector3Negate(up), target_position) - PI/2.25;
        max_angle_down *= -1;
        max_angle_down += 0.001f; // avoid numerical errors
        if (pitch_angle < max_angle_down)
            pitch_angle = max_angle_down;

        // Rotation axis
        Vector3 right = GetCameraRight(camera);

        target_position = Vector3RotateByAxisAngle(target_position, right, pitch_angle);
        camera->target = Vector3Add(camera->position, target_position);
    }

    // Yaw Camera
    {
        float yaw_angle = -mouse.x;

        Vector3 up = GetCameraUp(camera);
        Vector3 right = GetCameraRight(camera);

        Vector3 target_position = Vector3Subtract(camera->target, camera->position);

        float max_angle_left = Vector3Angle({1,0,0}, target_position) - 1.7;
        if (yaw_angle > max_angle_left)
            yaw_angle = max_angle_left;

        float max_angle_right = Vector3Angle({-1,0,0}, target_position) - 0.9;
        max_angle_right *= -1;
        if (yaw_angle < max_angle_right)
            yaw_angle = max_angle_right;

        target_position = Vector3RotateByAxisAngle(target_position, up, yaw_angle);

        camera->target = Vector3Add(camera->position, target_position);
    }
}

void chapter_4_update_text(Game *game, float dt) {
    Level_Chapter_4 *level = (Level_Chapter_4 *)game->level;
    Chapter_4_Text  *text_handler = &level->text_handler;

    if (level->text_start_timer > 0 || game->current || text_handler->alarms[text_handler->current_index] < 0) return;

    text_handler->alarms[text_handler->current_index] -= dt;

    if (text_handler->alarms[text_handler->current_index] <= 0) {
        game->current = text_handler->next_list[text_handler->current_index];

        if (game->current && text_handler->current_index < StaticArraySize(text_handler->alarms) - 1) {
            text_handler->current_index++;
        }
    }
}

void chapter_4_update(Game *game, float dt) {
    Level_Chapter_4 *level = (Level_Chapter_4 *)game->level;

    if (level->end_timer > 0) {
        level->end_timer -= dt;
        if (level->end_timer <= 0) {
            chapter++;
            atari_queue_deinit_and_goto_intro(game);
            return;
        }
    }
    if (level->text_start_timer > 0) {
        level->text_start_timer -= dt;
        if (level->text_start_timer <= 0) {
            game->current = &game->text[0];
        }
    }

    switch (level->state) {
        case CHAPTER_4_STATE_ATARI: {
            for (int i = 0; i < game->entities.length; i++) {
                Entity *e = game->entities.data[i];
                chapter_4_entity_update(e, game, dt);
            }

            Rectangle player = level->player->base_collider;
            player.x += level->player->pos.x;
            player.y += level->player->pos.y;

            Rectangle bed_interact = { 70, 0, 48, 60 };

            level->bed_popup = CheckCollisionRecs(player, bed_interact);
        } break;
        case CHAPTER_4_STATE_BED_1: {
        } break;
        case CHAPTER_4_STATE_3D: {
            chapter_4_3d_update_camera(level, &level->camera);

            chapter_4_update_text(game, dt);

            /*
            if (IsKeyDown(KEY_MINUS)) {
                level->camera.fovy += 5 * dt;
            }
            if (IsKeyDown(KEY_EQUAL)) {
                level->camera.fovy -= 5 * dt;
            }
            */
 
            if (game->current && !is_text_list_at_end(game->current) && colors_equal(game->current->color, RED)) {
                level->camera.fovy -= 1.25 * dt;
            }

            /*
            printf("%f, %f, %f || %f, %f, %f\n",
                   level->camera.position.x,
                   level->camera.position.y,
                   level->camera.position.z,
                   level->camera.target.x,
                   level->camera.target.y,
                   level->camera.target.z);
                   */
        } break;
    }
}

void chapter_4_draw(Game *game, float dt) {
    Level_Chapter_4 *level = (Level_Chapter_4 *)game->level;

    switch (level->state) {
        case CHAPTER_4_STATE_ATARI: {
            ClearBackground(BLACK);

            DrawTexture(atari_assets.textures[0], 0, 0, WHITE);

            for (int i = 0; i < game->entities.length; i++) {
                Entity *e = game->entities.data[i];
                chapter_4_entity_draw(e, game);
            }

            if (level->window_popup) {
                if (level->window->chap_4_window.closed) {
                    draw_popup("Open Window");
                } else {
                    draw_popup("Close Window");
                }
            }

            if (level->bed_popup) {
                if (level->window->chap_4_window.closed) {
                    draw_popup("Go to bed.");
                } else {
                    draw_popup("Ensure the windows\nare closed.");
                }

                if (is_action_pressed()) {
                    level->state = CHAPTER_4_STATE_BED_1;
                }
            }
        } break;
        case CHAPTER_4_STATE_BED_1: {
            ClearBackground(BLACK);

                chapter_4_3d_init(game);
            draw_cutscene(&level->bed_cutscene, dt);
            if (!level->bed_cutscene.active) {
            }
        } break;
        case CHAPTER_4_STATE_3D: {
            ClearBackground(BLUE);

            SetShaderValue(level->shader, level->shader.locs[SHADER_LOC_VECTOR_VIEW], &level->camera.position.x, SHADER_UNIFORM_VEC3);

            BeginTextureMode(game->render_target_3d);
            {
                ClearBackground(BLACK);

                BeginMode3D(level->camera);

                BeginShaderMode(level->shader);
                DrawModel(level->scene, {}, 1, WHITE);
                EndShaderMode();

                EndMode3D();
            }
            EndTextureMode();
        } break;
    }
}

Entity *chapter_4_make_entity(Entity_Type type, float x, float y) {
    Entity *result = allocate_entity();

    result->type = type;
    result->pos = {x, y};

    switch (type) {
        case ENTITY_PLAYER: {
            result->texture_id = 1;
        } break;
        case ENTITY_CHAP_4_WINDOW: {
            result->texture_id = 2;
        } break;
    }

    float texture_width  = entity_texture_width(result);
    float texture_height = entity_texture_height(result);

    result->base_collider = {
        0,
        3 * texture_height / 4,
        texture_width,
        texture_height / 4
    };

    return result;
}

void chapter_4_entity_update(Entity *entity, Game *game, float dt) {
    Level_Chapter_4 *level = (Level_Chapter_4 *)game->level;

    switch (entity->type) {
        case ENTITY_PLAYER: {
            int dir_x = key_right() - key_left();
            int dir_y = key_down()  - key_up();

            const float speed = 60;

            Vector2 vel = { speed * dir_x * dt, speed * dir_y * dt };
            apply_velocity(entity, vel, &game->entities);

            entity->pos.x = Clamp(entity->pos.x, 0, render_width - entity_texture_width(entity));
            entity->pos.y = Clamp(entity->pos.y, 0, render_height - entity_texture_height(entity));
        } break;
        case ENTITY_CHAP_4_WINDOW: {
            Rectangle player = level->player->base_collider;
            player.x += level->player->pos.x;
            player.y += level->player->pos.y;

            float texture_width  = entity_texture_width(entity);
            float texture_height = entity_texture_height(entity);

            Rectangle window = { entity->pos.x, entity->pos.y, texture_width, texture_height };

            int pad = 5;
            window.x -= pad;
            window.y -= pad;
            window.width  += pad * 2;
            window.height += pad * 2;

            level->window_popup = CheckCollisionRecs(player, window);

            if (level->window_popup && is_action_pressed()) {
                entity->chap_4_window.closed = !entity->chap_4_window.closed;
                if (entity->chap_4_window.closed) {
                    entity->texture_id = 3;
                } else {
                    entity->texture_id = 2;
                }
            }
        } break;
    }
}

void chapter_4_entity_draw(Entity *entity, Game *game) {
    (void)game;
    default_entity_draw(entity);
}

void draw_cutscene(Cutscene *cutscene, float dt) {
    if (!cutscene->active)
        return;

    cutscene->timer += dt;

    float target = cutscene->frames[cutscene->current_frame].frame_time;

    if (cutscene->timer >= target) {
        cutscene->timer = 0;
        cutscene->current_frame++;

        if (cutscene->current_frame >= cutscene->frame_count) {
            cutscene->active = false;
            return;
        }
    }

    DrawTextureEx(atari_assets.textures[cutscene->frames[cutscene->current_frame].texture_id],
                  {0, 0}, 0, cutscene->scale, WHITE);
}

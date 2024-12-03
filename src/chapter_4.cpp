#define GOD_COLOR {225,0,0,255}
#define GOD_COLOR_BACKDROP {100,0,0,255}

enum Chapter_4_State {
    CHAPTER_4_STATE_INITIAL_BLACK,
    CHAPTER_4_STATE_ATARI,
    CHAPTER_4_STATE_BED_1,
    CHAPTER_4_STATE_3D,
    CHAPTER_4_STATE_WINDOW
};

struct Chapter_4_Text {
    // Each alarm corresponds to a rest in the text, denoted by when
    // the current Text_List's next pointer is null. After alarm[current_index],
    // we set game->current to the actual next Text_List.
    float      alarms[20];
    Text_List *next_list[20];

    int        current_index;

    float      start_timer;
};

struct Level_Chapter_4 {
    Chapter_4_State state;
    float end_timer;

    bool black;

    Chapter_4_Text text_handler;

    bool wait_devil;

    float music_pitch;
    float music_volume;
    float music_volume_desired;

    Entity *player;
    Entity *window;
    Entity *djinn;

    bool yield_control;

    bool checked_window;

    bool window_popup; // bedroom
    bool check_window_popup; // kitchen
    bool bed_popup;
    bool microwave_popup;

    Camera2D camera_2d;

    Camera3D camera;
    Model scene;
    Shader shader;
};

Entity *chapter_4_make_entity  (Entity_Type type, float x, float y);
void    chapter_4_entity_update(Entity *entity, Game *game, float dt);
void    chapter_4_entity_draw  (Entity *entity, Game *game);

void chapter_4_set_state_atari(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    Level_Chapter_4 *level = (Level_Chapter_4 *)game->level;

    game->post_processing.crt.vignette_mix = 1;
    game->post_processing.crt.do_scanline_effect = true;

    //set_music_volume(MUSIC_GLITCH, 1);

    level->state = CHAPTER_4_STATE_ATARI;
}

void chapter_4_start_text(Game *game) {
    game->current = &game->text[20];
    Level_Chapter_4 *level = (Level_Chapter_4 *)game->level;

    level->yield_control = false;
}

void chapter_4_window_text(bool scroll, Text_List *list, char *line,
                           Color color, Text_List *next, bool sound = true)
{
    list->font         = &atari_font;
    list->font_spacing = 1;
    list->scale        = 0.125;
    list->scroll_speed = 10;
    //list->alpha_speed  = 0.5;
    list->color        = color;
    list->center_text  = true;
    list->disallow_skipping = true;
    
    if (colors_equal(color, GOD_COLOR)) {
        list->scroll_sound = SOUND_EMPTY;
        list->backdrop_color = GOD_COLOR_BACKDROP;
        list->background = true;
        list->scroll_sound = SOUND_TEXT_SCROLL_BAD;
    } else {
        list->scroll_sound = SOUND_TEXT_SCROLL_LOW;
        list->background = true;
        list->backdrop_color.a = 255;
    }

    if (scroll)
        list->scroll_type = LetterByLetter;
    else
        list->scroll_type  = EntireLine;

    list->render_type  = Bare;
    list->location     = Middle;
    list->take_keyboard_focus = true;

    if (!sound) {
        list->scroll_sound = SOUND_EMPTY;
    }

    text_list_init(list, 0, line, next);
}

void chapter_4_start_end_timer(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    Level_Chapter_4 *level = (Level_Chapter_4 *)game->level;

    level->end_timer = 4;
}

void chapter_4_goto_movie(Game *game) {
    (void)game;

    movie_init(&game_movie, MOVIE_DRACULA);
}

void chapter_4_3d_init(Game *game) {
    Level_Chapter_4 *level = (Level_Chapter_4 *)game->level;

    level->black = false;

    level->state = CHAPTER_4_STATE_3D;

    level->text_handler.start_timer = 3;
    game->render_state = RENDER_STATE_3D;

    DisableCursor();

    play_music(MUSIC_VHS_BAD);

    render_width  = DIM_3D_WIDTH;
    render_height = DIM_3D_HEIGHT;

    // TODO: memory leak
    game->textbox_target = LoadRenderTexture(render_width, render_height);
    game->textbox_alpha = 255;

    level->scene = LoadModel(RES_DIR "models/bedroom.glb");

    //float height = 0.9f;

    level->camera.position = {2.880000f, 0.900000f, -1.500000f};
    level->camera.target = {2.470594f, 1.091905f, -2.412352f};
    level->camera.up       = { 0, 1, 0 };
    level->camera.fovy     = FOV_DEFAULT;
    level->camera.projection = CAMERA_PERSPECTIVE;

    level->shader = LoadShader(RES_DIR "shaders/basic.vs", RES_DIR "shaders/chapter_4.fs");

    level->shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(level->shader, "matModel");
    level->shader.locs[SHADER_LOC_VECTOR_VIEW]  = GetShaderLocation(level->shader, "viewPos");

    for (int i = 0; i < level->scene.materialCount; i++) {
        level->scene.materials[i].shader = level->shader;
    }

    int ambientLoc = GetShaderLocation(level->shader, "ambient");
    float data[4] = { 0.2f, 0.2f, 0.2f, 1 };
    SetShaderValue(level->shader, ambientLoc, data, SHADER_UNIFORM_VEC4);

    CreateLight(LIGHT_POINT, { 0, 2, 0 }, Vector3Zero(), ORANGE, level->shader);

    game->post_processing.type = POST_PROCESSING_VHS;
    post_process_vhs_set_intensity(&game->post_processing.vhs, VHS_INTENSITY_MEDIUM);
    game->post_processing.vhs.scan_intensity = 0;

    game->post_processing.vhs.vignette_mix = 1;
}

void chapter_4_3d_init_after_delay(Game *game) {
    Level_Chapter_4 *level = (Level_Chapter_4 *)game->level;

    level->black = true;

    auto play_open_window = [](Game *game) -> void {
        (void)game;
        play_sound(SOUND_CREAKING);
    };

    add_event(game, play_open_window, 4);
    add_event(game, chapter_4_3d_init, 10);
}

void chapter_4_init(Game *game) {
    Level_Chapter_4 *level = (Level_Chapter_4 *)game->level;

    level->yield_control = true;

    level->state = CHAPTER_4_STATE_ATARI;
    //level->state = CHAPTER_4_STATE_INITIAL_BLACK;

    game->textbox_alpha = 255;

    level->music_volume = level->music_volume_desired = 1;
    level->music_pitch = 1;

    game->post_processing.type = POST_PROCESSING_CRT;

    Post_Processing_Crt *crt = &game->post_processing.crt;

    crt->do_scanline_effect = true;
    crt->do_warp_effect = false;
    crt->abberation_intensity = 1;
    crt->vignette_intensity = 2;

    crt->red_offset   = 0.006f;
    crt->green_offset = 0.009f;
    crt->blue_offset  = 0.01f;

    crt->should_spin_randomly = true;

    Texture2D *textures = atari_assets.textures;
    textures[0] = load_texture(RES_DIR "art/apartment_test.png");
    textures[1] = load_texture(RES_DIR "art/player.png");
    textures[2] = load_texture(RES_DIR "art/window_opened.png");
    textures[3] = load_texture(RES_DIR "art/window_closed.png");

    textures[8] = load_texture(RES_DIR "art/tv_back.png");

    textures[9] = load_texture(RES_DIR "art/djinn.png");
    textures[10] = load_texture(RES_DIR "art/microwave_off.png");
    textures[11] = load_texture(RES_DIR "art/microwave_on.png");

    textures[12] = load_texture(RES_DIR "art/windows_closed.png");
    textures[13] = load_texture(RES_DIR "art/windows_open.png");

    textures[14] = load_texture(RES_DIR "art/open_window.png");

    level->camera_2d.offset   = {};
    level->camera_2d.target   = {};
    level->camera_2d.rotation = 0;
    level->camera_2d.zoom     = 1;

    game->entities = make_array<Entity*>(20);

    level->player = chapter_4_make_entity(ENTITY_PLAYER, 69, 277);
    //level->player = chapter_4_make_entity(ENTITY_PLAYER, 291, 142);

    array_add(&game->entities, level->player);

    Entity *microwave = chapter_4_make_entity(ENTITY_CHAP_4_MICROWAVE, 128, 189);
    array_add(&game->entities, microwave);

    level->window = chapter_4_make_entity(ENTITY_CHAP_4_WINDOW, 216, 15);
    array_add(&game->entities, level->window);

    //add_wall(&game->entities, {80, 8, 29, 44});
    {
        Array<Entity*> *e = &game->entities;
        
        add_wall(e, {91, 223, 34, 21});
        add_wall(e, {37, 198, 108, 8});
        add_wall(e, {36, 312, 117, 2});
        add_wall(e, {139, 266, 13, 40});
        add_wall(e, {144, 191, 134, 29});
        add_wall(e, {29, 188, 8, 127});
        add_wall(e, {46, 268, 55, 21});
        add_wall(e, {152, 260, 174, 58});
        add_wall(e, {267, 160, 11, 60});
        add_wall(e, {318, 160, 26, 103});
        add_wall(e, {233, 27, 6, 133});
        add_wall(e, {238, 160, 40, 10});
        add_wall(e, {318, 160, 19, 6});
        add_wall(e, {337, 27, 13, 134});
        add_wall(e, {238, 21, 100, 6});
        add_wall(e, {246, 27, 31, 47});
        add_wall(e, {279, 29, 10, 11});
        add_wall(e, {322, 40, 15, 58});
        add_wall(e, {310, 66, 11, 12});
    }

    chapter_4_window_text(true,
                          &game->text[0],
                          "is that you?",
                          WHITE,
                          &game->text[1]);
    chapter_4_window_text(true,
                          &game->text[1],
                          "have you come for me?",
                          WHITE,
                          nullptr);

    level->text_handler.alarms[0]    = 2;
    level->text_handler.next_list[0] = &game->text[50];

    chapter_4_window_text(true,
                          &game->text[50],
                          "BE NOT AFRAID.",
                          GOD_COLOR,
                          &game->text[2]);
    chapter_4_window_text(true,
                          &game->text[2],
                          "I HEAR THEE, MY SON.",
                          GOD_COLOR,
                          &game->text[3]);
    chapter_4_window_text(true,
                          &game->text[3],
                          "WHAT DOST THOU DESIRE?",
                          GOD_COLOR,
                          nullptr);

    level->text_handler.alarms[1]    = 2;
    level->text_handler.next_list[1] = &game->text[4];

    chapter_4_window_text(true,
                          &game->text[4],
                          "meaning and fulfillment.",
                          WHITE,
                          nullptr);

    level->text_handler.alarms[2]    = 2;
    level->text_handler.next_list[2] = &game->text[5];

    chapter_4_window_text(true,
                          &game->text[5],
                          "GOOD.",
                          GOD_COLOR,
                          &game->text[6]);
    chapter_4_window_text(true,
                          &game->text[6],
                          "TO GRANT YOUR PRAYER,\nYOU MUST COMPLETE A TASK FOR ME.",
                          GOD_COLOR,
                          &game->text[7]);
    chapter_4_window_text(true,
                          &game->text[7],
                          "SHOW ME THAT YOUR FAITH IS PURE.",
                          GOD_COLOR,
                          nullptr);

    level->text_handler.alarms[3]    = 2;
    level->text_handler.next_list[3] = &game->text[8];

    chapter_4_window_text(true,
                          &game->text[8],
                          "i will do anything you ask, Lord.",
                          WHITE,
                          nullptr);

    level->text_handler.alarms[4]    = 3;
    level->text_handler.next_list[4] = &game->text[9];

    for (int i = 9; i == 9 || i == 100; i += 100-9) {
        game->text[i].font         = &atari_font;
        game->text[i].font_spacing = 4;
        game->text[i].scale        = 0.125;
        game->text[i].alpha_speed  = 0.125;
        game->text[i].color        = GOD_COLOR;
        game->text[i].center_text  = true;
        game->text[i].scroll_type  = EntireLine;
        game->text[i].backdrop_color = GOD_COLOR_BACKDROP;
        game->text[i].render_type  = Bare;
        game->text[i].location     = Top;
        game->text[i].take_keyboard_focus = true;
    }

    /*
       9 When they reached the place God had told him about,
       Abraham built an altar there and arranged the wood on it.
       He bound his son Isaac and laid him on the altar, on top of the wood.
       10 Then he reached out his hand and took the knife to slay his son.
       11 But the angel of the Lord called out to him from heaven, "Abraham!
       Abraham!" "Here I am," he replied. 12 "Do not lay a hand on the boy,"
       he said. "Do not do anything to him. Now I know that you fear God,
       because you have not withheld from me your son, your only son.
     */

    game->text[9].scroll_sound = SOUND_EMPTY;
    text_list_init(&game->text[9],
                   nullptr,
                   "39205768656E20746865792072656163\n"
                   "6865642074686520706C61636520476F\n"
                   "642068616420746F6C642068696D2061\n"
                   "626F75742C204162726168616D206275\n"
                   "696C7420616E20616C74617220746865\n"
                   "726520616E6420617272616E67656420\n"
                   "74686520776F6F64206F6E2069742E20\n"
                   "486520626F756E642068697320736F6E\n"
                   "20497361616320616E64206C61696420\n"
                   "68696D206F6E2074686520616C746172\n"
                   "2C206F6E20746F70206F662074686520\n"
                   "776F6F642E203130205468656E206865\n"
                   "2072656163686564206F757420686973\n"
                   "2068616E6420616E6420746F6F6B2074\n"
                   "6865206B6E69666520746F20736C6179\n"
                   "2068697320736F6E2E20313120427574\n"
                   "2074686520616E67656C206F66207468\n"
                   "65204C6F72642063616C6C6564206F75\n"
                   "7420746F2068696D2066726F6D206865\n"
                   "6176656E2C20224162726168616D2120",
                   &game->text[100]);

    game->text[100].scroll_sound = SOUND_EMPTY;
    text_list_init(&game->text[100],
                   nullptr,
                   "4162726168616D212220224865726520\n"
                   "4920616D2C22206865207265706C6965\n"
                   "642E2031322022446F206E6F74206C61\n"
                   "7920612068616E64206F6E2074686520\n"
                   "626F792C2220686520736169642E2022\n"
                   "446F206E6F7420646F20616E79746869\n"
                   "6E6720746F2068696D2E204E6F772049\n"
                   "206B6E6F77207468617420796F752066\n"
                   "65617220476F642C2062656361757365\n"
                   "20796F752068617665206E6F74207769\n"
                   "746868656C642066726F6D206D652079\n"
                   "6F757220736F6E2C20796F7572206F6E\n"
                   "6C7920736F6E2E2\n\n47656E657369732032323A392D3132\n204E4956",
                   nullptr);

    level->text_handler.alarms[5]    = 2;
    level->text_handler.next_list[5] = &game->text[10];

    chapter_4_window_text(true,
                          &game->text[10],
                          "DO YOU UNDERSTAND?",
                          GOD_COLOR,
                          &game->text[11], false);

    chapter_4_window_text(true,
                          &game->text[11],
                          "i...",
                          WHITE,
                          &game->text[12]);

    chapter_4_window_text(true,
                          &game->text[12],
                          "DO YOU UNDERSTAND?",
                          GOD_COLOR,
                          nullptr, false);

    level->text_handler.alarms[6]    = 4;
    level->text_handler.next_list[6] = &game->text[13];

    chapter_4_window_text(true,
                          &game->text[13],
                          "yes.",
                          WHITE,
                          nullptr);

    game->text[13].callbacks[0] = chapter_4_start_end_timer;

    atari_text_list_init(&game->text[20],
                         "Chase",
                         "... Did that noise come from\noutside?",
                         20,
                         nullptr);

    atari_text_list_init(&game->text[21],
                         0,
                         "The night is still.\r...\r...",
                         10,
                         0);

    auto goto_text_24 = [](void *game_ptr) -> void {
        Game *game = (Game *)game_ptr;

        auto text_event = [](Game *game) -> void {
            game->current = &game->text[24];
        };

        add_event(game, text_event, 3);
    };

    game->text[21].callbacks[0] = goto_text_24;
    atari_text_list_init(&game->text[24],
                         "Chase",
                         "Something feels strange.\rI should go to bed.",
                         10,
                         nullptr);

    auto chapter_4_close_window = [](void *game_ptr) -> void {
        (void)game_ptr;
        play_sound(SOUND_CLOSE_WINDOW);
    };

    game->text[24].callbacks[0] = chapter_4_close_window;

    int speed = 20;

    atari_text_list_init(&game->text[22],
                         "Chase",
                         "Family.",
                         speed,
                         nullptr);

    atari_text_list_init(&game->text[23],
                         "Chase",
                         "There's nothing to watch.",
                         speed,
                         nullptr);

    //chapter_4_3d_init(game);

    atari_text_list_init(&game->text[30],
                         0,
                         "On nights like these, he\n"
                         "yearned for the\nintangible.",
                         speed,
                         &game->text[31]);
    atari_text_list_init(&game->text[31],
                         0,
                         "A thing only seen in\n"
                         "mirages at the horizon.",
                         speed,
                         &game->text[32]);
    atari_text_list_init(&game->text[32],
                         0,
                         "Visions of the warmth of\n"
                         "her hand pressed on\n"
                         "his cheek,",
                         speed,
                         &game->text[33]);
    atari_text_list_init(&game->text[33],
                         0,
                         "shattering this great\nveil of his.\r"
                         "Yet completely abstract.",
                         speed,
                         &game->text[34]);
    atari_text_list_init(&game->text[34],
                         0,
                         "A welling of emotion\nstirred in his throat\nand his hands.",
                         speed,
                         &game->text[35]);
    atari_text_list_init(&game->text[35],
                         0,
                         "His hollow gaze clung\nto the ceiling.",
                         speed,
                         &game->text[36]);

    atari_text_list_init(&game->text[36],
                         0,
                         "And yet, th--",
                         20,
                         nullptr);

    auto goto_37_delay = [](void *game_ptr) -> void {
        play_sound(SOUND_KNOCKING_WEIRD);

        Game *game = (Game *)game_ptr;
        auto goto_37 = [](Game *game) -> void {
            game->current = &game->text[37];
        };

        add_event(game, goto_37, 4);
    };

    game->text[36].callbacks[0] = goto_37_delay;

    atari_text_list_init(&game->text[37],
                         0,
                         "...\rDid you hear that?",
                         speed,
                         &game->text[38]);
    atari_text_list_init(&game->text[38],
                         0,
                         "...",
                         speed,
                         nullptr);

    auto goto_movie_delay = [](void *game_ptr) -> void {
        Game *game = (Game *)game_ptr;
        add_event(game, chapter_4_goto_movie, 5);
    };
    game->text[38].callbacks[0] = goto_movie_delay;
}

void chapter_4_3d_update_camera(Level_Chapter_4 *level, Camera3D *camera) {
    (void)level;

    Vector2 mouse = get_mouse_delta();

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
        target_position = Vector3RotateByAxisAngle(target_position, up, yaw_angle);

        float angle = atan2f(target_position.z, target_position.x);

        float start = -2.34f;
        float end   = -1.50f;

        angle = Clamp(angle, start, end);

        // clamped direction
        float target_x = cos(angle);
        float target_z = sin(angle);

        target_position.x = target_x;
        target_position.z = target_z;

        camera->target = Vector3Add(camera->position, target_position);
    }
}

void chapter_4_update_text(Text_List **game_current, Chapter_4_Text *text_handler, float dt) {
    if (text_handler->start_timer > 0 || *game_current || text_handler->alarms[text_handler->current_index] < 0) return;

    text_handler->alarms[text_handler->current_index] -= dt;

    if (text_handler->alarms[text_handler->current_index] <= 0) {
        *game_current = text_handler->next_list[text_handler->current_index];

        if (*game_current && text_handler->current_index < StaticArraySize(text_handler->alarms) - 1) {
            text_handler->current_index++;
        }
    }
}

void chapter_4_update(Game *game, float dt) {
    Level_Chapter_4 *level = (Level_Chapter_4 *)game->level;

    if (IsKeyPressed(KEY_L)) play_sound(SOUND_TEXT_SCROLL_CHASE);

    if (level->black)
        return;

    if (level->end_timer > 0) {
        level->end_timer -= dt;
        if (level->end_timer <= 0) {
            atari_queue_deinit_and_goto_intro(game);
            return;
        }
    }
    if (level->text_handler.start_timer > 0) {
        level->text_handler.start_timer -= dt;
        if (level->text_handler.start_timer <= 0) {
            game->current = &game->text[0];
        }
    }

    switch (level->state) {
        case CHAPTER_4_STATE_INITIAL_BLACK: {
            static bool first = true;

            if (first) {
                auto play_knocking_1 = [](Game *game) -> void {
                    (void)game;
                    play_sound(SOUND_KNOCKING_2);
                };
                auto play_knocking_2 = [](Game *game) -> void {
                    (void)game;
                    play_sound(SOUND_KNOCKING_1);
                };

                auto goto_atari = [](Game *game) -> void {
                    Level_Chapter_4 *level = (Level_Chapter_4 *)game->level;
                    level->state = CHAPTER_4_STATE_ATARI;
                };

                add_event(game, play_knocking_1, 5);
                add_event(game, play_knocking_2, 9);
                add_event(game, goto_atari, 12);
                first = false;
            }
        } break;
        case CHAPTER_4_STATE_ATARI: {
            static bool first = true;

            if (first) {
                add_event(game, chapter_4_start_text, 1);
                play_music(MUSIC_GLITCH, false);
                first = false;
            }

            for (int i = 0; i < game->entities.length; i++) {
                Entity *e = game->entities.data[i];
                chapter_4_entity_update(e, game, dt);
            }

            Rectangle player = level->player->base_collider;
            player.x += level->player->pos.x;
            player.y += level->player->pos.y;

            Rectangle bed_interact = { 275, 31, 12, 43 };

            level->bed_popup = CheckCollisionRecs(player, bed_interact);
        } break;
        case CHAPTER_4_STATE_BED_1: {
        } break;
        case CHAPTER_4_STATE_3D: {
            chapter_4_3d_update_camera(level, &level->camera);

            chapter_4_update_text(&game->current, &level->text_handler, dt);

            if (is_music_playing(MUSIC_VHS_BAD) && level->text_handler.current_index == 6) {
                stop_music();
            }

            if (game->current && colors_equal(game->current->color, GOD_COLOR)) {
                if (!is_text_list_at_end(game->current)) {
                    level->camera.fovy -= 1.25 * dt;
                }

                //level->music_volume = level->music_volume_desired = 0;
                level->music_pitch = 0.5;
            } else {
                //level->music_volume = level->music_volume_desired = 1;
                level->music_pitch = 1;
            }
        } break;
        case CHAPTER_4_STATE_WINDOW: {
            static bool first = true;
            
            if (first) {
                play_sound(SOUND_OPEN_WINDOW);
                first = false;
            }

            static bool can_end = false;

            if (is_sound_playing(SOUND_CLOSE_WINDOW)) {
                can_end = true;
            }

            if (can_end && !is_sound_playing(SOUND_CLOSE_WINDOW)) {
                chapter_4_set_state_atari(game);
            }
        } break;
    }

    level->music_volume = go_to(level->music_volume,
                                level->music_volume_desired,
                                0.25 * dt);
    set_music_volume((Music_ID)game_audio.current_music, level->music_volume);
    set_music_pitch((Music_ID)game_audio.current_music,  level->music_pitch);
}

void chapter_4_draw(Game *game, float dt) {
    (void)dt;

    Level_Chapter_4 *level = (Level_Chapter_4 *)game->level;

    if (level->black) {
        ClearBackground(BLACK);
        return;
    }

    switch (level->state) {
        case CHAPTER_4_STATE_ATARI: {
            BeginMode2D(level->camera_2d);

            ClearBackground(BLACK);

            if (level->djinn)
                chapter_4_entity_draw(level->djinn, game);

            DrawTexture(atari_assets.textures[0], 0, 0, WHITE);

            sort_entities(&game->entities);

            for (int i = 0; i < game->entities.length; i++) {
                Entity *e = game->entities.data[i];

                if (e->type != ENTITY_CHAP_4_DEVIL)
                    chapter_4_entity_draw(e, game);
            }

            DrawTexture(atari_assets.textures[8], 59, 160+134, WHITE);

            EndMode2D();

            if (level->window_popup) {
                if (level->window->chap_4_window.closed) {
                    draw_popup("Open windows");
                } else {
                    draw_popup("Close windows");
                }
            }

            if (level->check_window_popup) {
                draw_popup("Check windows");
            }

            if (level->microwave_popup) {
                draw_popup("I lost my apetite");
            }

            if (level->bed_popup) {
                if (level->window->chap_4_window.closed) {
                    draw_popup("Go to bed.");

                    if (is_action_pressed()) {
                        level->state = CHAPTER_4_STATE_BED_1;
                        stop_music();
                    }
                } else {
                    draw_popup("Ensure the windows\n     are closed.");
                }
            }
        } break;
        case CHAPTER_4_STATE_BED_1: {
            ClearBackground(BLACK);

            static bool first = true;

            if (first) {
                //add_event(game, chapter_4_3d_init, 5);
                //add_event(game, chapter_4_goto_movie, 5);
                game->post_processing.type = POST_PROCESSING_PASSTHROUGH;

                auto yearning = [](Game *game) -> void {
                    game->current = &game->text[30];
                };

                add_event(game, yearning, 5);
                game_movie.end_movie_callback = chapter_4_3d_init_after_delay;
                first = false;
            }

            /*
            draw_cutscene(&level->bed_cutscene, dt);

            if (!level->bed_cutscene.active) {
                chapter_4_3d_init(game);
            }
            */
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
        case CHAPTER_4_STATE_WINDOW: {
            ClearBackground(BLACK);

            // hack lmao
            static bool first_frame = true;

            if (!first_frame && !is_sound_playing(SOUND_OPEN_WINDOW) && !is_sound_playing(SOUND_CLOSE_WINDOW)) {
                DrawTexture(atari_assets.textures[14],
                            0,
                            0,
                            WHITE);
            }

            first_frame = false;
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
            result->texture_id = 13;
            result->chap_4_window.rect = { 239, 37, 15, 111 };
        } break;
        case ENTITY_CHAP_4_DEVIL: {
            result->texture_id = 9;
        } break;
        case ENTITY_CHAP_4_MICROWAVE: {
            result->texture_id = 10;
            result->has_dialogue = true;
            result->draw_layer = DRAW_LAYER_LOW;
            result->chap_4_microwave.time = 10;
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
            int dir_x = input_movement_x_axis_int(dt);//key_right() - key_left();
            int dir_y = input_movement_y_axis_int(dt);//key_down()  - key_up();

            if (game->current || level->yield_control)
                dir_x = dir_y = 0;

            if (entity->pos.x > render_width && level->wait_devil)
                dir_x = dir_y = 0;

            float speed = 30;

            if (IsKeyDown(KEY_P))
                speed = 120;

            Vector2 vel = { speed * dir_x * dt, speed * dir_y * dt };
            apply_velocity(entity, vel, &game->entities);

            {
                static bool first = true;

                int x = (int)entity->pos.x;// + entity_texture_width(entity)/2;
                int y = (int)entity->pos.y + entity_texture_height(entity)/2;

                if (!level->checked_window) {
                    if (entity->pos.x > render_width-entity_texture_width(entity))
                        entity->pos.x = render_width-entity_texture_width(entity);
                }

                int screen_x = (x / render_width);
                int screen_y = (y / render_height);

                int desired_x = -render_width  * screen_x;
                int desired_y = -render_height * screen_y;

                if (level->camera_2d.offset.x != desired_x &&
                    level->camera_2d.offset.x == 0)
                {
                    if (first) {
                        level->yield_control = true;
                        level->wait_devil = true;
                        first = false;

                        //set_music_volume(MUSIC_GLITCH, 0);
                        level->music_volume_desired = 0;

                        level->djinn = chapter_4_make_entity(ENTITY_CHAP_4_DEVIL, -55, 145);
                        array_add(&game->entities, level->djinn);
                    }
                }

                if (!level->wait_devil) {
                    level->camera_2d.offset.x = desired_x;
                    level->camera_2d.offset.y = desired_y;
                }
            }

            if (!game->current) {
                Rectangle player = level->player->base_collider;
                player.x += level->player->pos.x;
                player.y += level->player->pos.y;

                level->check_window_popup = !level->checked_window && CheckCollisionRecs(player, {50, 199, 60, 10});
                
                bool pictures = CheckCollisionRecs(player, {128, 266, 11, 39});
                bool tv = CheckCollisionRecs(player, {44, 287, 58, 25});

                if (is_action_pressed()) {
                    if (level->check_window_popup) {
                        level->state = CHAPTER_4_STATE_WINDOW;

                        //set_music_volume(MUSIC_GLITCH, 0);

                        game->post_processing.crt.vignette_mix = 0.4f;
                        //game->post_processing.crt.do_scanline_effect = false;

                        auto look_outside_text = [](Game *game) -> void{
                            game->current = &game->text[21];
                        };

                        add_event(game, look_outside_text, 5);
                        level->checked_window = true;
                    }

                    if (pictures) {
                        game->current = &game->text[22];
                    }

                    if (tv) {
                        game->current = &game->text[23];
                    }
                }

            }

            //entity->pos.x = Clamp(entity->pos.x, 0, render_width - entity_texture_width(entity));
            //entity->pos.y = Clamp(entity->pos.y, 0, render_height - entity_texture_height(entity));
        } break;
        case ENTITY_CHAP_4_WINDOW: {
            Rectangle player = level->player->base_collider;
            player.x += level->player->pos.x;
            player.y += level->player->pos.y;

            level->window_popup = CheckCollisionRecs(player, entity->chap_4_window.rect);

            if (level->window_popup && is_action_pressed()) {
                entity->chap_4_window.closed = !entity->chap_4_window.closed;

                if (entity->chap_4_window.closed) {
                    entity->texture_id = 12;
                    play_sound(SOUND_CLOSE_WINDOW);
                } else {
                    entity->texture_id = 13;
                    play_sound(SOUND_OPEN_WINDOW);
                }
            }
        } break;
        case ENTITY_CHAP_4_DEVIL: {
            if (!level->wait_devil) break;

            entity->pos.x += 0.075f * (180 + sin(25 * entity->chap_4_devil.time) * 120) * dt;
            entity->pos.y += sin(0.25f*entity->pos.x) * 5 * dt;

            if (entity->pos.x > render_width / 3) {
                entity->pos.x = 0;
                entity->pos.y = 0;

                level->wait_devil = false;
                level->yield_control = false;

                //set_music_volume(MUSIC_GLITCH, 1);
                level->music_volume = level->music_volume_desired = 1;
            }

            entity->chap_4_devil.time += dt;
        } break;
        case ENTITY_CHAP_4_MICROWAVE: {
            entity->chap_4_microwave.time -= dt;

            if (entity->chap_4_microwave.time <= 0) {
                entity->chap_4_microwave.time = 0;
                entity->texture_id = 10;
            }

            bool dlg = can_open_dialogue(game, entity, level->player);
            level->microwave_popup = (dlg && entity->texture_id == 10);
        } break;
    }
}

void chapter_4_entity_draw(Entity *entity, Game *game) {
    (void)game;
    /*
    if (entity->type == ENTITY_CHAP_4_DEVIL) {
        Texture2D *texture = entity_get_texture(entity);
        if (texture) {
            DrawTexture(*texture, entity->pos.x, entity->pos.y, {200,200,200,255});
        }
    } else {
    */
        default_entity_draw(entity);
    //}
}

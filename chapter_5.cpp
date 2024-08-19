#define GUY_HEIGHT 1.8f

enum {
    CHAPTER_5_STATE_INTRO,
    CHAPTER_5_STATE_TRAIN_STATION_1,
    CHAPTER_5_STATE_TRAIN_STATION_2,
};

enum Chapter_5_Scene {
    CHAPTER_5_SCENE_TRAIN_STATION,
    CHAPTER_5_SCENE_STAIRCASE,
    CHAPTER_5_SCENE_DINNER_PARTY,
    CHAPTER_5_SCENE_GALLERY,
};

struct Chapter_5_Clerk {
    Vector3 position;

    Model body;

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

enum Chair_State {
    CHAIR_HAS_MALE,
    CHAIR_HAS_FEMALE,
    CHAIR_HAS_PENNY,
    CHAIR_EMPTY
};

// Represents both the guy and the chair.
struct Chapter_5_Chair {
    Chair_State state;
    float table_angle;

    Chapter_5_Chair *looking_at; // null for looking at the camera
    float look_timer;
    float look_angle;

    struct Chapter_5_Table *table;
};

struct Chapter_5_Table {
    Vector3         position;
    int             num_chairs;
    Chapter_5_Chair chairs[16];

    int  text_index; // index into game->text
    bool talked;
};

struct Chapter_5_Podium {
    Text_List *text;
    Vector3    position;
    float      rotation;
};

struct Level_Chapter_5 {
    // Train station
    struct {
        Chapter_5_Clerk clerk;
        Chapter_5_Train train;

        bool     ticket;

        float    black_screen_timer;
        float    transition_fade;
    };

    // Staircase
    struct {
        bool door_popup;
    };

    // Dinner Party
    struct {
        bool             good;
        Chapter_5_Table  tables[64];
        int              num_tables;

        Chapter_5_Chair *penny;
        bool             sitting_at_table;
        bool             rotating_heads;
        float            sitting_timer;
        float            whiteness_overlay;
        bool             black_state;

        bool             talk_popup;
    };

    // Museum
    struct {
        Chapter_5_Podium podiums[64];
        int              podium_count;

        bool             read_popup;
    };

    struct Models {
        Model guy_sitting,
              chair,
              table,
              pyramid_head,
              real_head,
              train,
              train_door,
              podium;
    } models;

    int      state;

    Camera3D camera;
    Shader   shader;

    Model    scenes[16];
    int      current_scene;
};

void chapter_5_goto_scene(Game *game, Chapter_5_Scene scene);

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

//

void chapter_5_start_sitting_text(Game *game) {
    game->current = &game->text[54];
}

void chapter_5_start_sitting_text_good(Game *game) {
    game->current = &game->text[95];
}

void chapter_5_sit_at_table(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    level->sitting_at_table = true;

    Vector3 *camera = &level->camera.position;

    camera->x = -33.5f;
    camera->y = 1.0f;
    camera->z = 2.19f;

    Vector3 *target = &level->camera.target;

    target->x = -32.98f;
    target->y = 0.81f; // for some reason it adds 1.0f, probably because of camera->y. so our desired value is actually 1.81f.
    target->z = 3.54f;

    if (level->good) {
        add_event(game, chapter_5_start_sitting_text_good, 3);
    } else {
        add_event(game, chapter_5_start_sitting_text, 3);
    }
}

void chapter_5_dinner_goto_good_part(Game *game) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    level->good = true;
    level->sitting_at_table = false;
    level->penny = nullptr;
    level->rotating_heads = false;
    level->sitting_timer = 0;
    level->whiteness_overlay = 0;
    level->black_state = false;

    // Reset if we talked to them before.
    for (int i = 0; i < level->num_tables; i++) {
        level->tables[i].talked = false;
    }

    chapter_5_goto_scene(game, CHAPTER_5_SCENE_DINNER_PARTY);
}

void chapter_5_dinner_goto_black(Game *game) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    level->black_state = true;

    add_event(game, chapter_5_dinner_goto_good_part, 0.5f);
}

void chapter_5_finish_dinner_party(Game *game) {
    chapter_5_goto_scene(game, CHAPTER_5_SCENE_GALLERY);
}

void chapter_5_table(Chapter_5_Table *table, Vector2 pos, int num_chairs, float angle_offset) {
    assert(num_chairs < StaticArraySize(table->chairs));

    table->position = { pos.x, 0, pos.y };
    table->num_chairs = num_chairs;
    table->text_index = -1;

    for (int i = 0; i < num_chairs; i++) {
        Chapter_5_Chair *chair = &table->chairs[i];

        chair->state = rand()%2==0 ? CHAIR_HAS_MALE : CHAIR_HAS_FEMALE;
        chair->table_angle = i * 2 * PI / (float)num_chairs;
        chair->table_angle += angle_offset;

        chair->look_timer = rand_float();

        chair->table = table;
    }
}

void chapter_5_train_station_init_positions(Game *game, bool refresh) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    Chapter_5_Clerk *clerk = &level->clerk;

    clerk->do_180_head = false;
    clerk->talk_popup = false;
    clerk->position = { 0.7225f * 16.3f, 0.7225f * 2.f, 0.7225f * -14.3f };
    clerk->body_rotation = -90;
    clerk->head_rotation = -90;
    clerk->saved_head_rotation = clerk->head_rotation;
    clerk->has_real_head = false;
    clerk->talked = false;

    if (refresh) clerk->talked = true;

    if (!refresh) {
        level->camera.position   = { -7.19f, GUY_HEIGHT, 6.68f };
        level->camera.target     = { 11.60f, 18.12f, -46.27f };
        level->camera.up         = { 0, 1, 0 };
        level->camera.fovy       = FOV_DEFAULT;
        level->camera.projection = CAMERA_PERSPECTIVE;
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

    list->color = SKYBLUE;
    list->bg_color = BLACK;

    list->render_type = DrawTextbox;
    list->location = Bottom;
    list->take_keyboard_focus = true;

    text_list_init(list, speaker, line, next);

    list->textbox_height = render_height / 3.5f;
}

void chapter_5_podium_text(Text_List *list, bool italics, char *line, Text_List *next) {
    list->font = &bold_font;

    if (italics)
        list->font = &italics_font;

    list->font_spacing = 1;
    list->scale = 0.125;
    list->render_type = DrawTextbox;
    list->location = Top;
    list->take_keyboard_focus = true;
    list->scroll_type = EntireLine;
    list->alpha_speed = 7;
    list->center_text = true;

    list->color = WHITE;
    list->bg_color = BLACK;

    text_list_init(list, 0, line, next);
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

            train->position     = {-200, 0.7225f * 1.8f, 0.7225f * -22.5f};
            train->bounding_box = GetMeshBoundingBox(level->models.train.meshes[0]);
            train->instances    = 5;
            train->setoff_timer = 0;
            train->closed       = true;
            train->moving       = false;
            train->able_to_close = true;

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

            level->camera.position   = { train_distance, GUY_HEIGHT, 0 };
            level->camera.target     = { 0.00f, GUY_HEIGHT, 2.00f };
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
        } break;
        case CHAPTER_5_SCENE_DINNER_PARTY: {
            game->textbox_alpha = 200;

            level->shader = LoadShader("shaders/basic.vs", "shaders/dinner.fs");
            model_set_shader(&level->scenes[2], level->shader);
            model_set_shader(&level->scenes[3], level->shader);

            level->camera.position   = { 0, GUY_HEIGHT, 0 };
            level->camera.target     = { -2.00f, GUY_HEIGHT, 0.00f };
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
            memset(level->tables, 0, sizeof(level->tables));

            Chapter_5_Table *tables = level->tables;
            for (int i = 0; i < level->num_tables; i++) {
                int num_chairs = 2 + (i+2) % 5;

                float angle_offset = 0;

                if (i == 3) {
                    num_chairs = 4;
                    angle_offset = PI / 4;
                }

                chapter_5_table(tables+i, table_positions[i], num_chairs, angle_offset);
            }

            level->tables[3].chairs[2].state = CHAIR_EMPTY;
            level->tables[3].chairs[0].state = CHAIR_HAS_FEMALE;
            level->tables[3].chairs[1].state = CHAIR_HAS_MALE;
            level->tables[3].chairs[3].state = CHAIR_HAS_MALE;

            // penny
            level->tables[5].chairs[1].state = CHAIR_HAS_PENNY;
            level->penny = &level->tables[5].chairs[1];

            if (level->good) {
                level->tables[0].text_index = 75;
                level->tables[1].text_index = 73;
                level->tables[2].text_index = 71;
                level->tables[3].text_index = 93;
                level->tables[4].text_index = 78;
                level->tables[5].text_index = 84;
                level->tables[6].text_index = 80;
                level->tables[7].text_index = 89;
                level->tables[8].text_index = 91;
                level->tables[9].text_index = 92;
            } else {
                level->tables[0].text_index = 34;
                level->tables[1].text_index = 32;
                level->tables[2].text_index = 30;
                level->tables[3].text_index = 53;
                level->tables[4].text_index = 37;
                level->tables[5].text_index = 46;
                level->tables[6].text_index = 45;
                level->tables[7].text_index = 40;
                level->tables[8].text_index = 49;
                level->tables[9].text_index = 51;
                level->tables[10].text_index = 52;
            }
        } break;
        case CHAPTER_5_SCENE_GALLERY: {
            level->shader = LoadShader("shaders/basic.vs", "shaders/cottage.fs");

            model_set_shader(&level->scenes[4], level->shader);

            CreateLight(LIGHT_POINT, { 0, 2, -30 }, Vector3Zero(), {255, 214, 179, 255}, level->shader);

            level->camera.position   = BlenderPosition3D(-239.1f, -131.f, 53.62f + GUY_HEIGHT);
            level->camera.target     = { 0, 0, 0 };
            level->camera.up         = { 0, 1, 0 };
            level->camera.fovy       = FOV_DEFAULT;
            level->camera.projection = CAMERA_PERSPECTIVE;

            game->textbox_alpha = 220;

            memset(game->text, 0, sizeof(game->text));

            chapter_5_podium_text(&game->text[0],
                                  true,
                                  "Again, I awoke.",
                                  nullptr);

            chapter_5_podium_text(&game->text[1],
                                  true,
                                  "Again, I found myself in the boiling void.\rI was THIRSTY.",
                                  nullptr);

            chapter_5_podium_text(&game->text[2],
                                  true,
                                  "After an eternity of unbearable pain,\nI finally found a house.\rI opened the door.",
                                  nullptr);

            chapter_5_podium_text(&game->text[3],
                                  false,
                                  "A wave of chill soothed his aching body,\nhalf-dead from the outside.\rThere was a stock of crisp, ice-cold water.\rSweet, sweet relief.",
                                  nullptr);
            game->text[3].text[2].font = &italics_font;

            chapter_5_podium_text(&game->text[4],
                                  false,
                                  "Collapsing on the ground,\nat last he had a moment to think!",
                                  nullptr);

            chapter_5_podium_text(&game->text[5],
                                  false,
                                  "Although the questions of how and why\nhe came here persisted in his mind,\none remained at the forefront:",
                                  &game->text[6]);
            chapter_5_podium_text(&game->text[6],
                                  true,
                                  "484F572043414E2057452046494E44204D4\n5414E494E4720494E2054484520434F4E53\n54414E542C2053484152502053554646455\n2494E47204F46204558495354454E43453F",
                                  nullptr);
            // HOW CAN WE FIND MEANING IN THE CONSTANT, SHARP SUFFERING OF EXISTENCE?

            chapter_5_podium_text(&game->text[7],
                                  false,
                                  "This question chiseled lines into his face\nthe more he pondered it.\rDread filled him!",
                                  &game->text[8]);
            game->text[7].text[1].font = &italics_font;

            chapter_5_podium_text(&game->text[8],
                                  false,
                                  "No matter what the answer was,\nit would mean cosmic horror.\rIt would mean that God had surely\nforsaken him.",
                                  nullptr);

            chapter_5_podium_text(&game->text[9],
                                  true,
                                  "After a night's stay,\nI exited the house, renewed.\rI kept walking North.\rBut why, exactly, I didn't understand.",
                                  nullptr);

            chapter_5_podium_text(&game->text[10],
                                  true,
                                  "Something compells me to brave this\nhellscape.\rAnd it fascinates me.",
                                  nullptr);

            auto add_podium = [&](Text_List *text, Vector2 position_2d, float rotation) -> void {
                Vector3 position = {
                    position_2d.x,
                    999,
                    -position_2d.y
                };

                Ray ray = {};
                ray.direction = {0, -1, 0};
                ray.position = position;

                // Check bottom collision
                RayCollision highest = {};
                highest.point.y = -9999;

                RayCollision result = GetRayCollisionMesh(ray, level->scenes[4].meshes[0], MatrixIdentity());

                position.y = result.point.y;

                Chapter_5_Podium podium = { text, position, rotation };
                level->podiums[level->podium_count++] = podium;
            };

            add_podium(&game->text[0], {-226.8f, -123.9f}, -62);
            add_podium(&game->text[1], {-216.5f, -110.2f}, -68);
            add_podium(&game->text[2], {-202.8f, -103.2f}, -52);
            add_podium(&game->text[3], {-187.0f, -87.51f}, -72);
            add_podium(&game->text[4], {-169.4f, -78.83f}, -70);
            add_podium(&game->text[5], {-136.f, -63}, -82);
            add_podium(&game->text[7], {-109.f, -67}, -52);
            add_podium(&game->text[9], {-74, -47}, -72);
            add_podium(&game->text[10], {-39, -27}, -68);
        } break;
    }
}

// init chapter 5

void chapter_5_init(Game *game) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    DisableCursor();

    render_width  = DIM_3D_WIDTH;
    render_height = DIM_3D_HEIGHT;

    game->render_state = RENDER_STATE_3D;

    game->textbox_target = LoadRenderTexture(render_width, render_height);
    game->textbox_alpha = 255;

    atari_assets.textures[0]   = load_texture("art/ticket_pov.png");

    level->scenes[0]           = LoadModel("models/train_station.glb");
    level->scenes[1]           = LoadModel("models/chap_5_dinner.glb");
    level->scenes[2]           = LoadModel("models/dinner_party.glb");
    level->scenes[3]           = LoadModel("models/dinner_party_good.glb");
    level->scenes[4]           = LoadModel("models/chap_5_cottage.glb");

    assert(IsModelReady(level->scenes[4]));

    level->models.train        = LoadModel("models/train.glb");
    level->models.train_door   = LoadModel("models/train_door.glb");

    level->clerk.body          = LoadModel("models/guy.glb");

    level->models.guy_sitting  = LoadModel("models/guy_sitting.glb");
    level->models.chair        = LoadModel("models/chair.glb");
    level->models.table        = LoadModel("models/dinner_table.glb");
    level->models.pyramid_head = LoadModel("models/pyramid_head.glb");
    level->models.real_head    = LoadModel("models/real_head.glb");
    level->models.podium       = LoadModel("models/podium.glb");

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

    // Dinner table dialogue

    chapter_5_text(&game->text[30],
                   "Jenny",
                   "...\rUm... hi?",
                   30,
                   &game->text[31]);
    chapter_5_text(&game->text[31],
                   "Mark",
                   "What's your problem?",
                   30,
                   nullptr);

    chapter_5_text(&game->text[32],
                   "Kate",
                   "We didn't think you'd come.",
                   30,
                   &game->text[33]);
    chapter_5_text(&game->text[33],
                   "Chase",
                   "What's that supposed to mean?",
                   30,
                   nullptr);

    chapter_5_text(&game->text[34],
                   "Kevin",
                   "Oh it's, uh, you!\rWhat's your name again?",
                   30,
                   &game->text[35]);
    chapter_5_text(&game->text[35],
                   "Chase",
                   "We went to the same class for years,\nKevin.",
                   30,
                   &game->text[36]);
    chapter_5_text(&game->text[36],
                   "Kevin",
                   "...",
                   30,
                   nullptr);

    chapter_5_text(&game->text[37],
                   "Chase",
                   "Is there any space for me?",
                   30,
                   &game->text[38]);
    chapter_5_text(&game->text[38],
                   "Amy",
                   "Um... who are you?",
                   30,
                   &game->text[39]);
    chapter_5_text(&game->text[39],
                   "Chase",
                   "...",
                   30,
                   nullptr);

    chapter_5_text(&game->text[40],
                   "Kenny",
                   "Oh, hey, you.",
                   30,
                   &game->text[41]);
    chapter_5_text(&game->text[41],
                   "Chase",
                   "Hi Kenny, what's up?",
                   30,
                   &game->text[42]);
    chapter_5_text(&game->text[42],
                   "Kenny",
                   "*sigh*\rIf you're looking for Eleanor and\nthe rest of them, they're somewhere back\nthere.",
                   30,
                   &game->text[43]);
    chapter_5_text(&game->text[43],
                   "Chase",
                   "... Uh, thanks.",
                   30,
                   &game->text[44]);
    chapter_5_text(&game->text[44],
                   "Kenny",
                   "So anyways, as I was saying, Darlene,",
                   30,
                   nullptr);

    chapter_5_text(&game->text[45],
                   "Joanne",
                   "What's your problem?",
                   30,
                   nullptr);

    chapter_5_text(&game->text[46],
                   "Penny",
                   "Hi, Chase.",
                   30,
                   &game->text[47]);
    chapter_5_text(&game->text[47],
                   "Chase",
                   "...\rOh gosh, hi Penny.\rI didn't realize you were gonna be here.",
                   30,
                   &game->text[48]);
    chapter_5_text(&game->text[48],
                   "Penny",
                   "We're not that close yet.\rCall me Penelope.\rCreep.",
                   30,
                   nullptr);


    chapter_5_text(&game->text[49],
                   "Luke",
                   "Why are you just walking around,\ntalking to random people?\rFind somewhere to sit down, man.\rIt's embarassing.",
                   30,
                   &game->text[50]);
    chapter_5_text(&game->text[50],
                   "Chase",
                   "...\rOh.",
                   30,
                   nullptr);

    chapter_5_text(&game->text[51],
                   "Adam",
                   "... Hi?",
                   30,
                   nullptr);

    chapter_5_text(&game->text[52],
                   "Lorane",
                   "... What?",
                   30,
                   nullptr);

    {
        String choices[] = { const_string("Sit now"), const_string("No, look around more first") };
        Text_List *next[] = { nullptr, nullptr };
        void (*hooks[2])(void*) = { chapter_5_sit_at_table, nullptr };

        game->text[53].color = SKYBLUE;
        game->text[53].bg_color = BLACK;

        atari_choice_text_list_init(&game->text[53],
                                    "Eleanor",
                                    "Oh hey Chase!\rWe have a seat here,\ndo you wanna sit now or?",
                                    choices,
                                    next,
                                    hooks,
                                    2);

        game->text[53].textbox_height = render_height / 3.5f;
    }

    chapter_5_text(&game->text[54],
                   "Eleanor",
                   "5AEEDF2BA00377AE1153792FB59A9A736C19CF43\n"
                   "55DC24B22F201506FB00C5D486E770493CFC55CA\n"
                   "ED970AFD7A0EE851E2A106F1C8E2BDB335004C6A\n"
                   "6037E9CA12775B867B2BC33F86C3156905213E64\n",
                   30,
                   &game->text[55]);
    chapter_5_text(&game->text[55],
                   "Trey",
                   "89A624687E892054EB11623F0470E9220C125F89\n"
                   "F8AA8354E501877A54885BA749746C83CDE1A680\n"
                   "F80994415B281B3B42CE83AFF8E10EA4D28644F3\n"
                   "40ED8294FCAED9A48200119BEFFBF8185541AA7B\n",
                   30,
                   &game->text[56]);
    chapter_5_text(&game->text[56],
                   "Siphor",
                   "B0DEA0E3122B637511DDA3F1BB2B02E241FD5A85\n"
                   "78293A9E95E271929FC4DFF528A50F9984F8A562\n"
                   "668E59A4D7DAA840214676573A664943AD0CCFAF\n"
                   "A5EDA0F968549118BD7DCF32A73440B982A771B7\n",
                   30,
                   &game->text[54]);

    chapter_5_text(&game->text[70],
                   "Eleanor",
                   "CHASE!\r... Are you good bro?",
                   30,
                   nullptr);

    chapter_5_text(&game->text[71],
                   "Jenny",
                   "Oh, hi Chase!",
                   30,
                   &game->text[72]);
    chapter_5_text(&game->text[72],
                   "Mark",
                   "Hey man!\rIt's good to see you!",
                   30,
                   nullptr);

    chapter_5_text(&game->text[73],
                   "Kevin",
                   "What's up man?\rWe didn't think you were coming,\rbut I'm glad you did.",
                   30,
                   &game->text[74]);
    chapter_5_text(&game->text[74],
                   "Chase",
                   "Thanks!",
                   30,
                   nullptr);

    chapter_5_text(&game->text[75],
                   "Kevin",
                   "Oh it's you!\rHi Chase.",
                   30,
                   &game->text[76]);
    chapter_5_text(&game->text[76],
                   "Chase",
                   "You remembered my name!",
                   30,
                   &game->text[77]);
    chapter_5_text(&game->text[77],
                   "Kevin",
                   "... Why wouldn't I?",
                   30,
                   nullptr);

    chapter_5_text(&game->text[78],
                   "Chase",
                   "Hi Amy!\nWhat's up?",
                   30,
                   &game->text[79]);
    chapter_5_text(&game->text[79],
                   "Amy",
                   "Oh, hi Chase.\rWe're all booked here sadly...\rI think Eleanor's over there, they saved\na seat for you.",
                   30,
                   &game->text[127]);
    chapter_5_text(&game->text[127],
                   "Chase",
                   "Thanks for letting me know!",
                   30,
                   nullptr);

    chapter_5_text(&game->text[80],
                   "Kenny",
                   "Oh, hey, Chase.",
                   30,
                   &game->text[81]);
    chapter_5_text(&game->text[81],
                   "Chase",
                   "Hey Kenny, what's up?",
                   30,
                   &game->text[82]);
    chapter_5_text(&game->text[82],
                   "Kenny",
                   "Nothin' much, we just came a few minutes ago.\rGood to see you, man.",
                   30,
                   &game->text[83]);
    chapter_5_text(&game->text[83],
                   "Darlene",
                   "Hi Chase!",
                   30,
                   nullptr);

    chapter_5_text(&game->text[84],
                   "Penny",
                   "Hi, Chase.",
                   30,
                   &game->text[85]);
    chapter_5_text(&game->text[85],
                   "Chase",
                   "Oh, hi Penny!",
                   30,
                   &game->text[86]);
    chapter_5_text(&game->text[86],
                   "Penny",
                   "It's good to see you!",
                   30,
                   &game->text[87]);
    chapter_5_text(&game->text[87],
                   "Chase",
                   "You too!!\rI guess we finally met in-person, then.\rThat's good, haha.",
                   30,
                   &game->text[88]);
    chapter_5_text(&game->text[88],
                   "Penny",
                   "Yep.\rWell, I'll talk to you later.",
                   30,
                   nullptr);

    chapter_5_text(&game->text[89],
                   "Luke",
                   "Hey Chase, do you need a seat?",
                   30,
                   &game->text[90]);
    chapter_5_text(&game->text[90],
                   "Chase",
                   "Ah, hi Luke.\rNah I think Eleanor has a seat for me.\rThanks, though.",
                   30,
                   nullptr);

    chapter_5_text(&game->text[91],
                   "Adam",
                   "Hi Chase!!!\rI'm glad you're here!",
                   30,
                   nullptr);

    chapter_5_text(&game->text[92],
                   "Lorane",
                   "Hey man, what's up?",
                   30,
                   nullptr);
    {
        String choices[] = { const_string("Sit now"), const_string("No, look around more first") };
        Text_List *next[] = { nullptr, nullptr };
        void (*hooks[2])(void*) = { chapter_5_sit_at_table, nullptr };


        game->text[93].color = SKYBLUE;
        game->text[93].bg_color = BLACK;

        atari_choice_text_list_init(&game->text[93],
                                    "Eleanor",
                                    "Oh hey Chase!\rWe have a seat here,\ndo you wanna sit now or?",
                                    choices,
                                    next,
                                    hooks,
                                    2);

        game->text[93].textbox_height = render_height / 3.5f;
    }

    chapter_5_text(&game->text[95],
                   "Eleanor",
                   "Yo, the VIP cinema in Richlands has specials\non Tuesdays.",
                   30,
                   &game->text[96]);
    chapter_5_text(&game->text[96],
                   "Trey",
                   "Oh yeahhh, I heard that.\rHow much?",
                   30,
                   &game->text[97]);
    chapter_5_text(&game->text[97],
                   "Eleanor",
                   "Like $6 or something.\rThere's all like, recliner chairs and shit.\rThe fancy works.",
                   30,
                   &game->text[98]);
    chapter_5_text(&game->text[98],
                   "Chase",
                   "Whoa, $6!?!\rThat's real good...\rWhen we going?\rNever went in that cinema before.",
                   30,
                   &game->text[99]);
    chapter_5_text(&game->text[99],
                   "Siphor",
                   "Damn.\rIt is good but uh,\rTuesday is results day.",
                   30,
                   &game->text[100]);
    chapter_5_text(&game->text[100],
                   "Trey",
                   "Ohhhhhhhhhhh don't remind meeee....",
                   30,
                   &game->text[101]);
    chapter_5_text(&game->text[101],
                   "Chase",
                   "Ahaha Trey's getting flashbacks to him\nwriting that fuckin' pure maths exam.",
                   30,
                   &game->text[102]);
    chapter_5_text(&game->text[102],
                   "Siphor",
                   "THAT REMINDS ME!\rDid you guys know, before the exam, right,\rTrey came up to me and asked what an\nintegral was?",
                   30,
                   &game->text[103]);
    chapter_5_text(&game->text[103],
                   "Eleanor",
                   "Broooo are you serious?",
                   30,
                   &game->text[104]);
    chapter_5_text(&game->text[104],
                   "Trey",
                   "Inte--what...?\rY'all must be making shit up at this point.\rWhat even is that.",
                   30,
                   &game->text[105]);
    chapter_5_text(&game->text[105],
                   "Chase",
                   "Hey Trey what's the derivative of x squared?",
                   30,
                   &game->text[106]);
    chapter_5_text(&game->text[106],
                   "Trey",
                   "Uhhhhhhhhhhhhhhhhhhhhhhhhhhhhh\nhhhhhhhhhhhhhhhhhhhhhhhhhhhh\nhhhhhhhhhhhhhhhhhhhhhhhhh...\rDoes anyone have a calculator?",
                   30,
                   &game->text[107]);
    chapter_5_text(&game->text[107],
                   "Siphor",
                   "You are COOKED my guy.",
                   30,
                   nullptr);

    //level->good = true;
    chapter_5_goto_scene(game, CHAPTER_5_SCENE_GALLERY);
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
        level->transition_fade += 0.25f * dt;

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

void chapter_5_update_table_talking(Chapter_5_Table *table, float dt) {
    for (int i = 0; i < table->num_chairs; i++) {
        Chapter_5_Chair *chair = &table->chairs[i];

        chair->look_timer -= dt;

        if (chair->look_timer < 0) {
            int index = i;
            while (index == i)
                index = rand_int(-1, table->num_chairs-1);

            if (index == -1)
                table->chairs[i].looking_at = nullptr;
            else
                table->chairs[i].looking_at = &table->chairs[index];

            chair->look_timer = rand_range(2, 3);
        }
    }
}

Vector3 get_chair_position(Chapter_5_Chair *chair) {
    Vector3 result;

    Chapter_5_Table *table = chair->table;

    float radius = 1.5;

    result.x = table->position.x + radius * cosf(chair->table_angle);
    result.y = 0;
    result.z = table->position.z + radius * sinf(chair->table_angle);

    return result;
}

void chapter_5_draw_table(Game *game, Chapter_5_Table *table) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    DrawModel(level->models.table, table->position, 1, WHITE);

    // Draw the chairs
    Model *chair_model = &level->models.chair;
    Model *person_model = &level->models.guy_sitting;
    Model *pyramid_head_model = &level->models.pyramid_head;
    Model *real_head_model = &level->models.real_head;

    for (int i = 0; i < table->num_chairs; i++) {
        Chapter_5_Chair *chair = &table->chairs[i];

        Model *head_model = pyramid_head_model;

        Color color = PINK;
        if (chair->state == CHAIR_HAS_MALE) {
            color = BLUE;
        }

        Color head_color = WHITE;
        float head_size = 0.85f;

        if (chair->state == CHAIR_HAS_PENNY || level->good) {
            head_model = real_head_model;
            head_color = color;
            head_size = 1;
        }

        Vector3 chair_position = get_chair_position(chair);

        float x = chair_position.x;
        float z = chair_position.z;

        float angle = atan2f(table->position.z - z, table->position.x - x);

        float desired_head_angle;

        if (!level->good) {
            if (chair->looking_at == nullptr) {
                desired_head_angle = atan2f(level->camera.position.z - z, level->camera.position.x - x);
            } else {
                Chapter_5_Chair *look = chair->looking_at;

                desired_head_angle = atan2f(get_chair_position(look).z - z, get_chair_position(look).x - x);
            }
        } else {
            desired_head_angle = angle;
        }

        static bool done = false;

        if (game->current == 0 && level->rotating_heads) {
            desired_head_angle += PI;

            float delta = GetFrameTime() * 0.3f * sign(desired_head_angle - chair->look_angle);
            if (desired_head_angle - chair->look_angle > delta) {
                chair->look_angle += delta;
            } else if (!done) {
                done = true;
                chair->look_angle = desired_head_angle;
                add_event(game, chapter_5_dinner_goto_black, 4);
            }
        } else {
            float time = GetTime();
            float sine = 1.5f * (sinf(time) + sinf(2 * time) + sinf(2.5f * time)); 
        
            desired_head_angle += 0.02f * sine;

            chair->look_angle = Lerp(chair->look_angle, desired_head_angle, 0.20f);
        }

        Vector3 chair_pos = { x, 0, z };
        DrawModelEx(*chair_model,  chair_pos, {0,1,0}, RAD2DEG * -angle, {1,1,1}, WHITE);
        if (chair->state != CHAIR_EMPTY) {
            DrawModelEx(*person_model, chair_pos, {0,1,0}, RAD2DEG * -angle, {1,1,1}, color);
            DrawModelEx(*head_model, Vector3Add(chair_pos, {0,1.4f,0}), {0,1,0}, 90 + RAD2DEG * -chair->look_angle, {head_size,head_size,head_size}, head_color);
        }
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

    float openness = train->door_openness * 1.5f;

    for (int i = 0; i < train->instances; i++) {
        int k = i - train->instances/2;

        {
            Vector3 door_position = train->position;
            door_position.x += k * length;
            door_position = Vector3Add(door_position, {-1.2f * 0.7225f, 0.3f * 0.7225f, 1.95f * 0.7225f});

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
            door_position = Vector3Add(door_position, {-2.9f * 0.7225f, 0.3f * 0.7225f, 1.95f * 0.7225f});

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
            cam->position.y = result.collision_result.point.y + GUY_HEIGHT;
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
            curb          = 0.7225f * -20.0f;
            train_start_z = 0.7225f * -20.5f;
            train_end_z   = 0.7225f * -23.0f;
            door_start    = 0.7225f *  -3.5f;
            door_end      = 0.7225f *  +0.5f;

            player_y = 3.19f;
        } break;
        case CHAPTER_5_SCENE_STAIRCASE: {
            curb = 2.5;

            train_start_z = 1.5;
            train_end_z   = -1.5;

            door_start = -3.5;
            door_end = 0;

            player_y = GUY_HEIGHT;
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

    chapter_5_update_player_on_train(game);

    Vector3 stored_camera_pos = level->camera.position;

    bool can_move = keyboard_focus(game) == 0 && !level->clerk.do_180_head;

    if (can_move) {
        chapter_5_update_camera(&level->camera, dt);
    }

    Vector3 *camera = &level->camera.position;
    Chapter_5_Train *train = &level->train;

    float curb = 0.7225f * -20.0f;
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
}

void chapter_5_update_player_gallery(Game *game, float dt) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    if (keyboard_focus(game)) return;

    Vector3 stored_camera_pos = level->camera.position;
    chapter_5_update_camera(&level->camera, dt);

    Vector3 velocity = Vector3Subtract(level->camera.position, stored_camera_pos);
    level->camera.position = stored_camera_pos;

    apply_3d_velocity(&level->camera, level->scenes[4], velocity);

    chapter_5_update_camera_look(&level->camera);
}

void chapter_5_update_player_dinner_party(Game *game, float dt) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    Vector3 stored_camera_pos = level->camera.position;

    bool look = true;

    if (level->sitting_timer != -1 && level->sitting_at_table)
        level->sitting_timer += dt;

    if (!level->good) {
        if (level->sitting_timer >= 12) {
            float debug_speed_slider = 1;

            level->camera.target = lerp_vector3(level->camera.target,
                                                Vector3Add(get_chair_position(level->penny), {0,1.65f,0}),
                                                0.005f);

            level->camera.fovy  -= debug_speed_slider * 1.5f * dt;
            if (game->textbox_alpha < 0)
                game->textbox_alpha = 0;
            else
                game->textbox_alpha -= debug_speed_slider * 4.0f * dt;

            if (level->camera.fovy <= -25) {
                // Jumpscare!
                level->camera.fovy = FOV_DEFAULT;
                level->sitting_timer = -1;
                game->textbox_alpha = 255;
                game->current = &game->text[70];

                level->rotating_heads = true;

                for (int i = 0; i < level->num_tables; i++) {
                    Chapter_5_Table *table = &level->tables[i];

                    for (int j = 0; j < table->num_chairs; j++) {
                        Chapter_5_Chair *chair = &table->chairs[j];
                        chair->looking_at = nullptr;
                    }
                }
            }

            if (level->camera.fovy < 30)
                look = false;
        }
    } else {
        if (level->sitting_timer >= 3) {
            Text_List *current = game->current;

            if (current) {
                level->whiteness_overlay = (float) (game->current - &game->text[95]) / 19.f;
            } else {
                level->whiteness_overlay = 1;
            }

            game->textbox_alpha = 200 * (1 - level->whiteness_overlay);
            if (level->whiteness_overlay >= 1) {
                level->whiteness_overlay = 1;
                add_event(game, chapter_5_finish_dinner_party, 5);
            }
        }
    }

    bool can_move = (game->current == 0);

    if (can_move && !level->sitting_at_table) {
        chapter_5_update_camera(&level->camera, dt);
    }

    Vector3 velocity = Vector3Subtract(level->camera.position, stored_camera_pos);
    level->camera.position = stored_camera_pos;

    Model *scene = nullptr;

    if (level->good) {
        scene = &level->scenes[3];
    } else {
        scene = &level->scenes[2];
    }
    apply_3d_velocity(&level->camera, *scene, velocity);

    if (look)
        chapter_5_update_camera_look(&level->camera);

    level->talk_popup = false;
    
    if (can_move && !level->sitting_at_table) {
        for (int i = 0; i < level->num_tables; i++) {
            Vector3 table = level->tables[i].position;

            float distance = Vector3Distance(level->camera.position, table);

            if (level->tables[i].text_index != -1 && distance < 4) {
                if (!level->tables[i].talked || i == 3) {
                    level->talk_popup = true;

                    if (game->current == 0 && is_action_pressed()) {
                        game->current = &game->text[level->tables[i].text_index];
                        level->tables[i].talked = true;
                    }
                }
            }
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

            if (level->sitting_at_table && !level->rotating_heads) {
                for (int i = 0; i < level->num_tables; i++) {
                    chapter_5_update_table_talking(&level->tables[i], dt);
                }
            }
        } break;
        case CHAPTER_5_SCENE_GALLERY: {
            chapter_5_update_player_gallery(game, dt);

            level->read_popup = false;

            if (game->current == 0) {
                for (int i = 0; i < level->podium_count; i++) {
                    Vector3 pos = level->podiums[i].position;
                    Vector2 player = {level->camera.position.x, level->camera.position.z};

                    if (Vector2Distance({pos.x, pos.z}, player) < 3) {
                        if (IsKeyDown(KEY_EQUAL)) {
                            level->podiums[i].rotation += 10 * PI * dt;
                        }
                        if (IsKeyDown(KEY_MINUS)) {
                            level->podiums[i].rotation -= 10 * PI * dt;
                        }

                        //printf("%f\n", level->podiums[i].rotation);

                        level->read_popup = true;

                        if (is_action_pressed()) {
                            game->current = level->podiums[i].text;
                        }
                    }
                }
            }
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
                    Vector3 scale = { 0.85f, 0.85f, 0.85f };

                    if (level->clerk.has_real_head) {
                        model = &level->models.real_head;
                        scale = { 0.75, 0.75, 0.75 };
                    }

                    DrawModelEx(*model, Vector3Add(level->clerk.position, {0,1.55f,0}), {0,1,0}, level->clerk.head_rotation, scale, WHITE);

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
                draw_popup("OPEN DOOR", BLACK);
        } break;
        case CHAPTER_5_SCENE_DINNER_PARTY: {
            if (level->black_state) break;

            if (level->good) {
                ClearBackground(WHITE);
            }

            BeginMode3D(level->camera);

            BeginShaderMode(level->shader);

            if (level->good) {
                DrawModel(level->scenes[3], {}, 1, WHITE);
            } else {
                DrawModel(level->scenes[2], {}, 1, WHITE);
            }

            for (int i = 0; i < level->num_tables; i++) {
                chapter_5_draw_table(game, level->tables + i);
            }
            EndShaderMode();

            EndMode3D();

            if (level->talk_popup) {
                if (level->good) {
                    draw_popup("Talk to them", GOLD, Top);
                } else {
                    draw_popup("TALK TO THEM\nTALK TO THEM\nTALK TO THEM\nTALK TO THEM\nTALK TO THEM", GOLD, Top);
                }
            }

            if (level->whiteness_overlay > 0) {
                Color color = WHITE;

                color.a = (uint8_t) (255 * level->whiteness_overlay);

                DrawRectangleRec({0,0,(float)render_width,(float)render_height}, color);
            }
        } break;
        case CHAPTER_5_SCENE_GALLERY: {
            ClearBackground(SKYBLUE);

            BeginMode3D(level->camera);

            DrawModel(level->scenes[4], {}, 1, WHITE);
            //DrawModel(level->models.podium, {-9.308f, -3.294f, -10.69f}, 2.5, WHITE);
            for (int i = 0; i < level->podium_count; i++) {
                DrawModelEx(level->models.podium, level->podiums[i].position, {0,1,0}, level->podiums[i].rotation, {1.3f, 1.3f, 1.3f}, WHITE);
            }

            EndMode3D();

            if (level->read_popup) {
                draw_popup("Read the Text", BLACK, Top);
            }
        } break;
    }
}

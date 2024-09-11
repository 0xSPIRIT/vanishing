#define GUY_HEIGHT 1.8f
#define QUOTE_COUNT 7

enum {
    CHAPTER_5_STATE_INTRO,
    CHAPTER_5_STATE_TRAIN_STATION_1,
    CHAPTER_5_STATE_TRAIN_STATION_2,
};

enum Chapter_5_Scene {
    CHAPTER_5_SCENE_INVALID,
    CHAPTER_5_SCENE_TRAIN_STATION,
    CHAPTER_5_SCENE_STAIRCASE,
    CHAPTER_5_SCENE_DINNER_PARTY,
    CHAPTER_5_SCENE_DESERT,
    CHAPTER_5_SCENE_GALLERY,
    CHAPTER_5_SCENE_SEASIDE,
};

struct Chapter_5_Clerk {
    Vector3 position;

    float saved_head_rotation;

    bool  has_real_head;

    float head_rotation;
    float body_rotation;

    bool  do_180_head;

    bool  talk_popup;
    bool  talked;
};

struct Chapter_5_Bartender {
    Vector3 position;
    bool    talked_front, talked_behind;
    bool    talk_popup;
};

struct Chapter_5_Train {
    bool        flipped;
    Vector3     position;
    float       delta_x;
    float       speed;
    bool        closed;
    bool        able_to_close;
    bool        moving;
    bool        player_in;
    float       player_in_timer;
    float       door_openness; // 0.0 to 1.0
    BoundingBox bounding_box;
    int         instances;
    float       setoff_timer;
    float       move_timer;
    float       door_open_alarm;
    Vector3     instance_positions;
};

struct Chapter_5_Quote {
    Text_List *text;
    Vector3 position;
    Vector3 target;
    float   fov;
};

enum Chair_State {
    CHAIR_HAS_MALE,
    CHAIR_HAS_FEMALE,
    CHAIR_HAS_HOPE,
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
    bool       read;
};

struct Level_Chapter_5 {
    float camera_height;

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
        int                 staircase_podium_current;
        Chapter_5_Podium    staircase_podiums[7];
        Chapter_5_Bartender bartender;

        float               staircase_fade;
        bool                fade_to_dinner;
    };

    // Dinner Party
    struct {
        bool             good;
        Chapter_5_Table  tables[64];
        int              num_tables;

        Chapter_5_Chair *hope;
        bool             sitting_at_table;
        bool             rotating_heads;
        float            sitting_timer;
        float            whiteness_overlay;
        bool             black_state;

        bool             talk_popup;
        bool             finished_dinner_party;
    };

    // Desert
    struct {
        Chapter_5_Podium podiums[32];
        int              podium_count;

        bool             read_popup;
        bool             desert_door_popup;
    };

    // Gallery
    struct {
        Chapter_5_Quote  quotes[QUOTE_COUNT];
        Chapter_5_Quote *current_quote;
        float            current_quote_time;

        int              quote_count;

        Vector3          camera_target_saved;
        Vector3          camera_position_saved;

        Vector2          door_position;
        bool             gallery_door_popup;
    };

    // Seaside
    struct {
        bool talk_to_hope_popup;
        bool talked_to_hope;
        bool seaside_ending; // a little hang time before cutting to the next chapter.
    };

    struct Models {
        Model guy_sitting,
              chair,
              table,
              body,
              pyramid_head,
              real_head,
              train,
              train_flipped,
              train_door,
              podium;
    } models;

    int      state;

    Camera3D camera;
    Shader   shader;

    Model    scenes[16];
    int      current_scene;
    int      queued_scene;
};

void chapter_5_goto_scene(Game *game, Chapter_5_Scene scene);

void chapter_5_end_text_begin(Game *game) {
    game->current = &game->text[50];
}

void chapter_5_end_text_begin_delayed(void *game_ptr) {
    Game *game = (Game *)game_ptr;

    add_event(game, chapter_5_end_text_begin, 2);
}

void chapter_5_end(void *game_ptr) {
    atari_queue_deinit_and_goto_intro((Game *)game_ptr);
}

void chapter_5_2_second_hang_goto_text(Game *game) {
    game->current = &game->text[53];
}

void chapter_5_2_second_hang_end_text(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    add_event(game, chapter_5_2_second_hang_goto_text, 2);
}

void chapter_5_begin_phone_call(Game *game) {
    game->current = &game->text[0];
}

void end_chapter_5_after_stop(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    level->seaside_ending = true;

    add_event(game, chapter_5_begin_phone_call, 2);
}

void chapter_5_exit_gallery(void *game_ptr) {
    Game *game = (Game *)game_ptr;

    chapter_5_goto_scene(game, CHAPTER_5_SCENE_SEASIDE);
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

    /*
    level->camera.position = { 0, level->camera_height, 0 };
    level->camera.target     = { -2.00f, level->camera_height, 0.00f };
    level->camera.fovy       = FOV_DEFAULT;
    */

    level->good = true;
    level->sitting_at_table = false;
    level->hope = nullptr;
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
    chapter_5_goto_scene(game, CHAPTER_5_SCENE_DESERT);
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
    clerk->position = { -9.4f, 0, 0 };
    clerk->body_rotation = 90;
    clerk->head_rotation = 90;
    clerk->saved_head_rotation = clerk->head_rotation;
    clerk->has_real_head = false;
    clerk->talked = false;

    if (refresh) clerk->talked = true;

    level->camera.fovy       = 50;

    if (!refresh) {
        level->camera.position   = { 50, level->camera_height, 0 };
        level->camera.target     = { 0, level->camera_height, 0 };
        level->camera.up         = { 0, 1, 0 };
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

    list->color = {255, 199, 199, 255};
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

    list->font_spacing = 0;
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

void chapter_5_podium_text_red(Text_List *list, bool italics, char *line, Text_List *next) {
    list->font = &bold_font;

    if (italics)
        list->font = &italics_font;

    list->font_spacing = 0;
    list->scale = 0.125;
    list->render_type = DrawTextbox;
    list->location = Top;
    list->take_keyboard_focus = true;
    list->scroll_type = EntireLine;
    list->alpha_speed = 7;
    list->center_text = true;

    list->color = RED;
    list->bg_color = BLACK;

    text_list_init(list, 0, line, next);
}

void chapter_5_end_text(Text_List *list, bool italics, char *line, Text_List *next) {
    list->font = &bold_font;

    if (italics)
        list->font = &italics_font;

    list->font_spacing = 0;
    list->scale = 0.125;
    list->render_type = DrawTextbox;
    list->location = Top;
    list->take_keyboard_focus = true;
    list->scroll_type = EntireLine;
    list->alpha_speed = 2;
    list->center_text = true;

    list->color = WHITE;
    list->bg_color = BLACK;

    text_list_init(list, 0, line, next);

    list->textbox_height = render_height / 2;
}

void chapter_5_sacred_text(Text_List *list, char *line, Text_List *next) {
    list->font = &italics_font;

    list->font_spacing = 0;
    list->scale = 0.125;
    list->render_type = DrawTextbox;
    list->location = Middle;
    list->take_keyboard_focus = true;
    list->scroll_type = EntireLine;
    list->alpha_speed = 1;
    list->center_text = true;

    list->color = WHITE;
    list->bg_color = {0, 0, 0, 160};

    text_list_init(list, 0, line, next);

    list->textbox_height = 7.f * render_height / 8.f;
}

void chapter_5_scene_init(Game *game) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    int scene = level->queued_scene;

    level->current_scene = scene;

    if (IsShaderReady(level->shader))
        UnloadShader(level->shader);

    switch (scene) {
        case CHAPTER_5_SCENE_TRAIN_STATION: {
            level->camera_height = 1.67f;

            level->state = CHAPTER_5_STATE_INTRO;//CHAPTER_5_STATE_TRAIN_STATION_1;
            level->black_screen_timer = -1;

            chapter_5_train_station_init_positions(game, false);

            Chapter_5_Train *train = &level->train;

            train->position     = {500, 0, 10};
            train->bounding_box = GetMeshBoundingBox(level->models.train.meshes[0]);
            train->instances    = 4;
            train->setoff_timer = 0;
            train->closed       = true;
            train->moving       = false;
            train->able_to_close = true;
            train->speed         = 0;

            level->shader = LoadShader(RES_DIR "shaders/basic.vs", RES_DIR "shaders/cottage.fs");
//
            model_set_bilinear(&level->scenes[0]);

            model_set_shader(&level->scenes[0],         level->shader);
            model_set_shader(&level->models.train,      level->shader);
            model_set_shader(&level->models.train_door, level->shader);

            memset(game->text, 0, sizeof(game->text));

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

            game->current = &game->text[0];
        } break;
        case CHAPTER_5_SCENE_STAIRCASE: {
            level->camera_height = 1.67f;

            level->shader = LoadShader(RES_DIR "shaders/basic.vs", RES_DIR "shaders/dinner.fs");

            float train_distance = -510;

            game->textbox_alpha = 220;

            level->camera.position   = { train_distance, level->camera_height, 0 };
            level->camera.target     = { 0.00f, level->camera_height, 2.00f };
            level->camera.up         = { 0, 1, 0 };
            level->camera.fovy       = FOV_DEFAULT;
            level->camera.projection = CAMERA_PERSPECTIVE;

            Chapter_5_Train *train = &level->train;

            memset(train, 0, sizeof(*train));

            train->flipped      = true;
            train->position     = { train_distance, 0, 0 };
            train->bounding_box = GetMeshBoundingBox(level->models.train.meshes[0]);
            train->instances    = 4;
            train->setoff_timer = 0;
            train->closed       = true;
            train->moving       = true;
            train->player_in    = true;
            train->able_to_close = false;
            train->speed = 0;

            model_set_bilinear(&level->scenes[1]);

            model_set_shader(&level->scenes[1], level->shader);

            model_set_shader(&level->models.train,      level->shader);
            model_set_shader(&level->models.train_door, level->shader);

            memset(game->text, 0, sizeof(game->text));

            chapter_5_podium_text_red(&game->text[0],
                                      true,
                                      "Are you ready for the dinner tomorrow night?",
                                      nullptr);
            chapter_5_podium_text_red(&game->text[1],
                                      true,
                                      "Tomorrow you're going to have to meet them and\ntalk to them.",
                                      nullptr);
            chapter_5_podium_text_red(&game->text[2],
                                      true,
                                      "How are you going to enter the room?\rWill you look presentable?\rMake sure to smile.",
                                      nullptr);
            chapter_5_podium_text_red(&game->text[3],
                                      true,
                                      "What are you going to say?\rDon't look stupid.",
                                      nullptr);
            chapter_5_podium_text_red(&game->text[4],
                                      true,
                                      "What will they say in return?",
                                      nullptr);
            chapter_5_podium_text_red(&game->text[5],
                                      true,
                                      "It might be awkward.\rSoooooooooooooooo awkward.",
                                      nullptr);
            chapter_5_podium_text_red(&game->text[6],
                                      true,
                                      "They might hate you.\rCome, let me show you what could happen.",
                                      nullptr);
            chapter_5_podium_text_red(&game->text[7],
                                      true,
                                      "If found, please return to owner:\r(xxx-xxxx-xxx)\rLast seen: 2 years ago.",
                                      nullptr);

            void chapter_5_text(Text_List *list, char *speaker, char *line, float scroll_speed, Text_List *next);

            float speed = 30;
            chapter_5_text(&game->text[10],
                           "Bartender",
                           "HEY KID!\rWhat're you doin' here?!",
                           speed,
                           &game->text[11]);
            chapter_5_text(&game->text[11],
                           "Bartender",
                           "Aren't you 'spossed to be in school, or\nsomethin'?",
                           speed,
                           &game->text[12]);
            chapter_5_text(&game->text[12],
                           "Chase",
                           "I'm 20.",
                           speed,
                           &game->text[13]);
            chapter_5_text(&game->text[13],
                           "Bartender",
                           "Haha, sure little man.\rGet 'outta here before I--",
                           speed,
                           &game->text[14]);
            chapter_5_text(&game->text[14],
                           "Chase",
                           "Haven't I heard this before?",
                           speed,
                           &game->text[15]);
            chapter_5_text(&game->text[15],
                           "Bartender",
                           "...",
                           speed,
                           nullptr);

            chapter_5_text(&game->text[16],
                           "Bartender",
                           "HEY!\rYou're not allowed back here, idiot.",
                           speed,
                           nullptr);
            chapter_5_text(&game->text[17],
                           "Bartender",
                           "WHY ARE YOU STILL HERE???\rSTOP TALKING TO ME.",
                           speed,
                           nullptr);
            chapter_5_text(&game->text[18],
                           "Bartender",
                           "...",
                           speed,
                           nullptr);

            level->staircase_podiums[0] = {
                &game->text[1],
                { -15.9f, 52.43f },
            };

            level->staircase_podiums[1] = {
                &game->text[2],
                { -15.2f, 67.8f },
            };

            level->staircase_podiums[2] = {
                &game->text[3],
                { -2.1f, 61.1f },
            };

            level->staircase_podiums[3] = {
                &game->text[4],
                { -11.5f, 80.8f },
            };

            level->staircase_podiums[4] = {
                &game->text[5],
                { 2.4f, 80.8f },
            };

            level->staircase_podiums[5] = {
                &game->text[6],
                { 9.4f, 64.9f },
            };

            level->staircase_podiums[6] = {
                &game->text[7],
                { 21, 51 },
            };

            level->bartender.position = { 0, 0, 51 };
            level->bartender.talked_front = false;
            level->bartender.talked_behind = false;

            //game->current = &game->text[0];
        } break;
        case CHAPTER_5_SCENE_DINNER_PARTY: {
            level->camera_height = 1.9f;

            game->textbox_alpha = 220;

            level->shader = LoadShader(RES_DIR "shaders/basic.vs", RES_DIR "shaders/dinner.fs");
            model_set_shader(&level->scenes[2], level->shader);
            model_set_shader(&level->scenes[3], level->shader);

            level->camera.position   = { 0, level->camera_height, 0 };
            level->camera.target     = { -2.00f, level->camera_height, 0.00f };
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

            // Dinner table dialogue

            memset(game->text, 0, sizeof(game->text));

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
                           "Hope",
                           "Hi, Chase.",
                           30,
                           &game->text[47]);
            chapter_5_text(&game->text[47],
                           "Chase",
                           "...\rOh gosh, hi Hope.\rI didn't realize you were gonna be here.",
                           30,
                           &game->text[48]);
            chapter_5_text(&game->text[48],
                           "Hope",
                           "We're not that close yet.\rCreep.",
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
                           "Hope",
                           "Hi, Chase.",
                           30,
                           &game->text[85]);
            chapter_5_text(&game->text[85],
                           "Chase",
                           "Oh, hi Hope!",
                           30,
                           &game->text[86]);
            chapter_5_text(&game->text[86],
                           "Hope",
                           "It's good to see you!",
                           30,
                           &game->text[87]);
            chapter_5_text(&game->text[87],
                           "Chase",
                           "You too!!\rI guess we finally met in-person, then.\rThat's good, haha.",
                           30,
                           &game->text[88]);
            chapter_5_text(&game->text[88],
                           "Hope",
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
                           "Ahaha Trey's getting flashbacks to him\nwriting that fuckin' maths exam.",
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

            // hope
            level->tables[5].chairs[1].state = CHAIR_HAS_HOPE;
            level->hope = &level->tables[5].chairs[1];

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
        case CHAPTER_5_SCENE_DESERT: {
            level->camera_height = GUY_HEIGHT;

            level->shader = LoadShader(RES_DIR "shaders/basic.vs", RES_DIR "shaders/cottage.fs");

            Model *scene_model = &level->scenes[4];
            model_set_bilinear(scene_model);

            model_set_shader(&level->scenes[4], level->shader);

            level->camera.position   = BlenderPosition3D(-239.1f, -131.f, 53.62f + level->camera_height);
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
                                  "After an eternity of unbearable pain,\nI came upon a strange house.\rI opened the door.",
                                  nullptr);

            chapter_5_podium_text(&game->text[3],
                                  false,
                                  "A wave of chill soothed his aching body,\nhalf-dead from the outside.\rThere was a stock of crisp, ice-cold water.\rSweet, sweet relief.",
                                  nullptr);
            game->text[3].text[2].font = &italics_font;

            chapter_5_podium_text(&game->text[4],
                                  false,
                                  "Collapsing on the ground,\nat last he had a moment to think!",
                                  &game->text[100]);
            chapter_5_podium_text(&game->text[100],
                                  false,
                                  "But he knew, at some point,\nhe'd have to brave the outside\nAgain.\rThis made him sad.",
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
                                  false,
                                  "After nineteen years had passed,\nhe stumbled upon a strange place.\rAn art gallery, alone in the desert.",
                                  &game->text[11]);
            chapter_5_podium_text(&game->text[11],
                                  false,
                                  "He groveled to the building,\rexpecting to learn nothing new.",
                                  &game->text[12]);
            chapter_5_podium_text(&game->text[12],
                                  true,
                                  "Blisters became the bottoms of my feet.\r"
                                  "And yet, something compels me to brave this\nhellscape.\rAnd it fascinates me.",
                                  nullptr);

            for (int i = 45; i <= 49; i++) {
                game->text[i].location = Middle;
            }

            level->podium_count = 0;

            auto add_podium = [&](Text_List *text, Vector2 position_2d, float rotation) -> void {
                // position_2d you get from blender's (x, y)

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

                RayCollision result = GetRayCollisionMesh(ray, level->scenes[4].meshes[6], MatrixIdentity());

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
        case CHAPTER_5_SCENE_GALLERY: {
            level->camera_height = GUY_HEIGHT;

            level->shader = LoadShader(RES_DIR "shaders/basic.vs", RES_DIR "shaders/cottage.fs");
            model_set_shader(&level->scenes[5], level->shader);

            memset(game->text, 0, sizeof(game->text));

            chapter_5_sacred_text(&game->text[20],
                                  "\"It is during that return, that pause, that Sisyphus\n"
                                  "interests me. A face that toils so close to stones is\n"
                                  "already stone itself! I see that man going back down\n"
                                  "with a heavy yet measured step toward the torment of\n"
                                  "which he will never know the end. That hour like a\n"
                                  "breathing-space which returns as surely as his\n"
                                  "suffering, that is the hour of consciousness.\n\n\r"
                                  "At each of those moments when he leaves the\n"
                                  "heights and gradually sinks toward the lairs\n"
                                  "of the gods, he is superior to his fate.\"",
                                  &game->text[21]);
            chapter_5_sacred_text(&game->text[21],
                                  "\n\"... He is stronger than his rock.\"\rAlbert Camus, The Myth of Sisyphus.",
                                  nullptr);
            game->text[21].text[1].font = &bold_font;

            chapter_5_sacred_text(&game->text[22],
                                  "\n\"Not only so, but we also glory in our sufferings,\n"
                                  "because we know that suffering produces\n"
                                  "perseverance; perseverance, character; and character,\n"
                                  "Hope. And Hope does not put us to shame, because God's\n"
                                  "love has been poured out into our hearts through the\n"
                                  "Holy Spirit, who has been given to us.\"\n\n\r"
                                  "Romans 5:3-5 (NIV)",
                                  nullptr);
            game->text[22].text[1].font = &bold_font;

            chapter_5_sacred_text(&game->text[23],
                                  "\"Do you think you will be admitted into Paradise\n"
                                  "without being tested like those before you? They were\n"
                                  "afflicted with suffering and adversity and were so\n"
                                  "violently shaken that even the Messenger and the\n"
                                  "believers with him cried out, 'When will Allah's\nhelp come?'\n\n"
                                  "Indeed, Allah's help is always near.\"\n\n\r"
                                  "Qur'an, Surah Al-Baqarah (Verse 214)",
                                  nullptr);
            game->text[23].text[1].font = &bold_font;

            chapter_5_sacred_text(&game->text[24],
                                  "\n\n\"Having realized Me as the enjoyer of all sacrifices and\n"
                                  "austerities, the Supreme Lord of all the worlds and the\n"
                                  "selfless friend of all living beings, My devotee attains\npeace.\"\n\n\r"
                                  "Bhagavad Gita 5:29",
                                  nullptr);
            game->text[24].text[1].font = &bold_font;

            chapter_5_sacred_text(&game->text[25],
                                  "\"We are what we think.\n"
                                  "All that we are arises with our\n"
                                  "thoughts.\n\n"
                                  "With our thoughts we make the world.\n"
                                  "Speak or act with an impure mind\n"
                                  "And trouble will follow you\n"
                                  "As the wheel follows the ox that draws\n"
                                  "the cart.\"",
                                  &game->text[50]);
            chapter_5_sacred_text(&game->text[50],
                                  "\"We are what we think.\n"
                                  "All that we are arises with our\n"
                                  "thoughts.\n\n"
                                  "With our thoughts we make the world.\n"
                                  "Speak or act with a pure mind\n"
                                  "And happiness will follow you\n"
                                  "As your shadow, unshakable.\"",
                                  &game->text[51]);
            chapter_5_sacred_text(&game->text[51],
                                  "\"'Look how he abused me and hurt me,\n"
                                  "How he threw me down and robbed me.'\n\n"
                                  "Live with such thoughts and you live in\n"
                                  "hate.\"\n\r"
                                  "The Dhammapada (1)",
                                  nullptr);
            game->text[51].text[1].font = &bold_font;


            chapter_5_sacred_text(&game->text[26],
                                  "\n\n\"Atheistic existentialism, of which I am a\n"
                                  "representative, declares with greater consistency that\n"
                                  "if God does not exist there is at least one being whose\n"
                                  "existence comes before its essence, a being which\n"
                                  "exists before it can be defined by any conception of it.\n\n"
                                  "That being is man or, as Heidegger has it, the\n"
                                  "human reality.\"",
                                  &game->text[27]);

            chapter_5_sacred_text(&game->text[27],
                                  "\n\"What do we mean by saying that existence precedes\n"
                                  "essence? We mean that man first of all exists,\n"
                                  "encounters himself, surges up in the world - and\n"
                                  "defines himself afterwards. If man as the\n"
                                  "existentialist sees him is not definable, it is because\n"
                                  "to begin with he is nothing.\"\n\n\r"
                                  "Jean-Paul Sartre, Existentialism is a Humanism",
                                  nullptr);
            game->text[27].text[1].font = &bold_font;

            chapter_5_sacred_text(&game->text[28],
                                  "\n\n\"Man, the bravest animal and the one most inured to\n"
                                  "suffering, does not repudiate suffering in itself: he\n"
                                  "wills it, he even seeks it out, provided that he is\n"
                                  "shown a meaning for it, a purpose of suffering. Not\n"
                                  "suffering, but the senselessness of suffering was the\n"
                                  "curse which till then lay spread over humanity...\"\n\r"
                                  "Nietzsche, The Geneaology of Morals",
                                  nullptr);
            game->text[28].text[1].font = &bold_font;

            {
                String choices[] = { const_string("I've seen enough"), const_string("No, I'd like to look around more") };
                Text_List *next[] = { nullptr, nullptr };
                void (*hooks[2])(void*) = { chapter_5_exit_gallery, nullptr };

                game->text[100].color = WHITE;
                game->text[100].bg_color = BLACK;
                game->text[100].location = Bottom;
                game->text[100].render_type = DrawTextbox;

                atari_choice_text_list_init(&game->text[100],
                                            nullptr,
                                            "Exit the gallery?\rYou won't be able to come back.\rFound: X/7",
                                            choices,
                                            next,
                                            hooks,
                                            2);

                game->text[100].textbox_height = render_height / 3.5f;
            }

            // Set all the textures to bilinear.
            Model *scene_model = &level->scenes[5];
            model_set_bilinear(scene_model);

            level->camera.position   = { 0, level->camera_height, 0 };
            level->camera.target     = { 0, level->camera_height, -2 };
            level->camera.up         = { 0, 1, 0 };
            level->camera.fovy       = FOV_DEFAULT;
            level->camera.projection = CAMERA_PERSPECTIVE;

            level->door_position = { -8.392f, -202.2f };

            Chapter_5_Quote *quote = 0;

            quote = &level->quotes[0];
            quote->text     = &game->text[20];
            quote->position = { 1.8562f, 1.8f, -31.239f };
            quote->target   = { -4.36f, 0.71f, -0.64f };
            quote->fov      = FOV_DEFAULT/2;

            quote = &level->quotes[1];
            quote->text = &game->text[22];
            quote->position = { 8.2117f, 1.8f, -69.794f };
            quote->target = {7.27f, 3.35f, -70.18f};//-30.14f, 56.12f, -85.32f};
            quote->fov      = FOV_DEFAULT/2;

            quote = &level->quotes[2];
            quote->text = &game->text[23];
            quote->position = { -11.432f, 1.8f, -147.07f };
            quote->target = {-40.34f, 8.11f, -2.54f};
            quote->fov      = FOV_DEFAULT/2;

            quote = &level->quotes[3];
            quote->text = &game->text[24];
            quote->position = { -8.4255f, 1.8f, -140.33f };
            quote->target = { 46.18f, 3.86f, -270.94f };
            quote->fov      = FOV_DEFAULT/2;

            quote = &level->quotes[4];
            quote->text = &game->text[25];
            quote->position = { 23.59f, 1.8f, -144.08f };
            quote->target = { -103.69f, 3.22f, -164.23f };
            quote->fov      = FOV_DEFAULT/6;

            quote = &level->quotes[5];
            quote->text = &game->text[28];
            quote->position = { 13.524f, 1.8f, -190.22f };
            quote->target = { -94.74f, 113.83f, -269.42f };
            quote->fov      = FOV_DEFAULT/2;

            quote = &level->quotes[6];
            quote->text     = &game->text[26];
            quote->position = {18.06f, 1.80f, -125.51f};
            quote->target   = {16.45f, 0.98f, -126.37f};
            quote->fov      = FOV_DEFAULT/4;
        } break;
        case CHAPTER_5_SCENE_SEASIDE: {
            level->camera_height = GUY_HEIGHT;

            level->camera.position   = { 0, level->camera_height, 0 };
            level->camera.target     = { -2, level->camera_height, 0 };
            level->camera.up         = { 0, 1, 0 };
            level->camera.fovy       = FOV_DEFAULT;
            level->camera.projection = CAMERA_PERSPECTIVE;

            float speed = 30;

            memset(game->text, 0, sizeof(game->text));

            game->textbox_alpha = 255;

            chapter_5_text(&game->text[0],
                           0,
                           "*ring ring*\r*click*",
                           speed,
                           &game->text[1]);
            chapter_5_text(&game->text[1],
                           "Chase",
                           "... Hello?",
                           speed,
                           &game->text[2]);
            chapter_5_text(&game->text[2],
                           "      ",
                           "Oh, hello.",
                           speed,
                           &game->text[3]);
            chapter_5_text(&game->text[3],
                           "Chase",
                           "Who is this?",
                           speed,
                           &game->text[4]);
            chapter_5_text(&game->text[4],
                           "      ",
                           "I'm you from the future!",
                           speed,
                           &game->text[5]);
            chapter_5_text(&game->text[5],
                           "Chase",
                           "Did you escape the prison?",
                           speed,
                           &game->text[6]);
            chapter_5_text(&game->text[6],
                           "      ",
                           "Yes! I did,\rwould you like to know\nhow I did it?",
                           speed,
                           &game->text[7]);
            chapter_5_text(&game->text[7],
                           "Chase",
                           "Yes.",
                           speed,
                           &game->text[8]);
            chapter_5_text(&game->text[8],
                           "      ",
                           "Ok, so all you have to do is have patience,\nand-\r...\r...",
                           speed,
                           &game->text[9]);
            chapter_5_text(&game->text[9],
                           "Chase",
                           "Are you still there...?",
                           speed,
                           &game->text[10]);
            chapter_5_text(&game->text[10],
                           "      ",
                           "I'm sorry.\rI just realized I didn't escape after all.",
                           speed,
                           &game->text[11]);
            chapter_5_text(&game->text[11],
                           "      ",
                           "It turns out I'm still in the prison,\rthere was just a bigger one around\nthe previous one.",
                           speed,
                           &game->text[12]);
            chapter_5_text(&game->text[12],
                           "      ",
                           "I'll call you back if I do escape.\rI Promise.",
                           speed,
                           &game->text[13]);
            chapter_5_text(&game->text[13],
                           "Chase",
                           "Wait wh-",
                           speed,
                           &game->text[14]);
            chapter_5_text(&game->text[14],
                           0,
                           "*click*",
                           speed,
                           nullptr);

            game->text[14].callbacks[0] = chapter_5_end_text_begin_delayed;

            chapter_5_text(&game->text[30],
                           "Hope",
                           "So, what do you think?",
                           speed,
                           &game->text[31]);
            chapter_5_text(&game->text[31],
                           "Chase",
                           "What do you mean?",
                           speed,
                           &game->text[32]);
            chapter_5_text(&game->text[32],
                           "Hope",
                           "Have you found what you're looking for?\rMeaning in your suffering?",
                           speed,
                           &game->text[33]);
            chapter_5_text(&game->text[33],
                           "Chase",
                           "I'm not sure, I don't think I've found it yet.",
                           speed,
                           &game->text[34]);
            chapter_5_text(&game->text[34],
                           "Hope",
                           "Me too.\rWhat, do you think you're the only one who\nhasn't?",
                           speed,
                           &game->text[35]);
            chapter_5_text(&game->text[35],
                           "Chase",
                           "It's not just that.\rNothing I accomplish is fulfilling for me.\rAre you telling me that happens to everyone?",
                           speed,
                           &game->text[36]);
            chapter_5_text(&game->text[36],
                           "Hope",
                           "No, of course not.\rSome people have it easier than others,\rbut that is how it is.",
                           speed,
                           &game->text[37]);
            chapter_5_text(&game->text[37],
                           "Chase",
                           "Seriously? \"That is how it is?\"\rYou've got to be kidding.",
                           speed,
                           &game->text[38]);
            chapter_5_text(&game->text[38],
                           "Hope",
                           "You're trying too hard to find a solution,\nChase.\rHave you always been like this?",
                           speed,
                           &game->text[39]);
            chapter_5_text(&game->text[39],
                           "Chase",
                           "I don't know.",
                           speed,
                           &game->text[40]);
            chapter_5_text(&game->text[40],
                           "Hope",
                           "Not always?",
                           speed,
                           &game->text[41]);
            chapter_5_text(&game->text[41],
                           "Chase",
                           "Ah, great. So we're \"focusing on the positives.\"\r"
                           "I understand now that you don't understand.\r"
                           "Goodbye.",
                           speed,
                           nullptr);

            for (int i = 30; i <= 41; i++) {
                game->text[i].color = WHITE;
                game->text[i].location = Bottom;
                for (int j = 0; j < game->text[i].text_count; j++) {
                    game->text[i].text[j].color = WHITE;
                }
            }

            chapter_5_end_text(&game->text[50],
                               false,
                               "Perhaps he didn't understand what\nhe had found.",
                               &game->text[51]);
            chapter_5_end_text(&game->text[51],
                               false,
                               "Maybe the right moment had not\nreached him yet.\rOr, maybe the right person had not\nreached him yet.\rOr, maybe the right relationship had not\nreached him yet.",
                               &game->text[52]);
            chapter_5_end_text(&game->text[52],
                               false,
                               "How do you create your own\nmeaning?",
                               nullptr);
            game->text[52].callbacks[0] = chapter_5_2_second_hang_end_text;

            chapter_5_end_text(&game->text[53],
                               false,
                               "But something inside him still urged\nhim to go on.\rFor what reason?\rHe didn't know that either.\rHe doesn't know anything.",
                               &game->text[54]);
            chapter_5_end_text(&game->text[54],
                               false,
                               "He remained unenlightened.",
                               nullptr);

            game->text[54].callbacks[0] = chapter_5_end;
        } break;
    }
}

void chapter_5_goto_scene(Game *game, Chapter_5_Scene scene) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;
    level->queued_scene = scene;
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

    atari_assets.textures[0]   = load_texture(RES_DIR "art/ticket_pov.png");

    level->scenes[0]           = LoadModel(RES_DIR "models/train_station2.glb");
    level->scenes[1]           = LoadModel(RES_DIR "models/staircase.glb");
    level->scenes[2]           = LoadModel(RES_DIR "models/dinner_party.glb");
    level->scenes[3]           = LoadModel(RES_DIR "models/dinner_party_good.glb");
    level->scenes[4]           = LoadModel(RES_DIR "models/chap_5_cottage.glb");
    level->scenes[5]           = LoadModel(RES_DIR "models/artgallery.glb");
    level->scenes[6]           = LoadModel(RES_DIR "models/balcony.glb");

    level->camera_height = GUY_HEIGHT;

    assert(IsModelReady(level->scenes[5]));

    level->models.train         = LoadModel(RES_DIR "models/train.glb");
    level->models.train_flipped = LoadModel(RES_DIR "models/train_flipped.glb");
    level->models.train_door    = LoadModel(RES_DIR "models/train_door.glb");

    level->models.body          = LoadModel(RES_DIR "models/guy.glb");

    level->models.guy_sitting   = LoadModel(RES_DIR "models/guy_sitting.glb");
    level->models.chair         = LoadModel(RES_DIR "models/chair.glb");
    level->models.table         = LoadModel(RES_DIR "models/dinner_table.glb");
    level->models.pyramid_head  = LoadModel(RES_DIR "models/pyramid_head.glb");
    level->models.real_head     = LoadModel(RES_DIR "models/real_head.glb");
    level->models.podium        = LoadModel(RES_DIR "models/podium.glb");

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

            if (sign(clerk->head_rotation) != sign(to_angle - 90))
            {
                clerk->head_rotation = to_angle - 90;
            }

            clerk->head_rotation = lerp_dt(clerk->head_rotation, to_angle - 90, t, dt);
            clerk->saved_head_rotation = clerk->head_rotation;
        } else {
            float to_angle = clerk->saved_head_rotation + 90;

            to_angle += sine;
            clerk->head_rotation = lerp_dt(clerk->head_rotation, to_angle - 90, t, dt);
        }
    }

    clerk->talk_popup = (game->current == nullptr &&
                         Vector2Distance(player_p, clerk_p) <= 3 &&
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

    const float top_speed = 60;
    const float train_time = 8.5;
    const float acceleration = 30;

    int direction = 1;

    if (level->current_scene == CHAPTER_5_SCENE_TRAIN_STATION) {
        direction = -1;
    }

    if (train->moving) {
        train->position.x += direction * train->speed * dt;
        train->move_timer += dt;

        if (train->move_timer >= train_time) {
            train->speed -= acceleration * dt;

            if (train->speed < 0) {
                train->speed = 0;

                train->moving = false;
                train->door_open_alarm = 2;
                train->move_timer = 0;
            }
        } else {
            if (train->speed < top_speed) {
                train->speed += acceleration * dt;
            }
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

        if (chair->state == CHAIR_HAS_HOPE || level->good) {
            head_model = real_head_model;
            head_color = color;
            head_size = 1;
        }

        Vector3 chair_position = get_chair_position(chair);

        float x = chair_position.x;
        float z = chair_position.z;

        float angle = atan2f(table->position.z - z, table->position.x - x);

        float desired_head_angle;

        if (!level->good && chair->state != CHAIR_HAS_HOPE) {
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

        Matrix transform = MatrixTranslate(train->position.x + k * length,
                                           train->position.y,
                                           train->position.z);

        Model *model = &level->models.train;
        if (train->flipped) {
            model = &level->models.train_flipped;
        }

        DrawMesh(model->meshes[0], model->materials[1], transform);
    }

    float openness = train->door_openness * 1.5f;

    for (int i = 0; i < train->instances; i++) {
        int k = i - train->instances/2;

        {
            Vector3 door_position = train->position;
            door_position.x += k * length;
            door_position = Vector3Add(door_position, {2.9f * 0.7225f, 0.3f * 0.7225f, -2.00f * 0.7225f});

            if (train->flipped) {
                door_position.z += 2.9f;
                door_position.x -= 3;
            }

            door_position.x += openness;

            Matrix translate = MatrixTranslate(door_position.x,
                                               door_position.y,
                                               door_position.z);

            DrawMesh(level->models.train_door.meshes[0],
                     level->models.train_door.materials[1],
                     translate);
        }

        {
            Vector3 door_position = train->position;
            door_position.x += k * length;
            door_position = Vector3Add(door_position, {1.2f * 0.7225f, 0.3f * 0.7225f, -2.00f * 0.7225f});

            door_position.x -= openness;

            if (train->flipped) {
                door_position.z += 2.9f;
                door_position.x -= 3;
            }


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

void chapter_5_update_camera_look(Camera3D *camera, float dt) {
    Vector2 look = input_movement_look(dt);
    CameraYaw(camera,   -look.x, false);
    CameraPitch(camera, -look.y, true, false, false);
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

bool apply_3d_velocity(Camera3D *camera, float camera_height, Model world,
                       Vector3 pos_vel, bool use_red_material_for_collision)
{
    bool had_hit = false;

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
            if (use_red_material_for_collision && !is_mesh_collider(&model, j)) {
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
            if (use_red_material_for_collision && !is_mesh_collider(&model, j)) {
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
            had_hit = true;
            *axis -= axis_vel;
        } else {
            cam->position.y = result.collision_result.point.y + camera_height;
        }
    };

    Vector3 saved_pos = camera->position;

    resolve_axis(&camera->position.x, pos_vel.x, camera, world);
    resolve_axis(&camera->position.z, pos_vel.z, camera, world);

    Vector3 diff = Vector3Subtract(camera->position, saved_pos);
    camera->target = Vector3Add(camera->target, diff);

    return had_hit;
}

bool is_x_in_door_area(Chapter_5_Train *train, float x) {
    int length = chapter_5_train_length(train);

    for (int i = 0; i < train->instances; i++) {
        int k = i - train->instances/2;

        float off_start = 0;
        float off_end   = 0;

        if (train->flipped) {
            off_start = -3.5;
            off_end   = -1.0;
        } else {
            off_start = +0.2f;
            off_end   = +2.5f;
        }

        float door_start = off_start + train->position.x + k * length;
        float door_end   = off_end   + train->position.x + k * length;

        if (x > door_start && x < door_end) {
            return true;
        }
    }

    return false;
}

void chapter_5_update_player_train_station(Game *game, float dt) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    //chapter_5_update_player_on_train(game);

    Vector3 stored_camera_pos = level->camera.position;

    bool can_move = keyboard_focus(game) == 0 && !level->clerk.do_180_head;

    if (can_move) {
        update_camera_3d(&level->camera, PLAYER_SPEED_3D, dt);
    }

    Vector3 *camera = &level->camera.position;
    Chapter_5_Train *train = &level->train;

    bool clamp_to_curb = true;

    float curb = 8.3f;
    float train_end = 11;

    if (is_x_in_door_area(train, camera->x) && !train->closed) {
        clamp_to_curb = false;
        if (camera->z > curb)
            train->player_in = true;
    }

    if (clamp_to_curb) {
        if (camera->z > curb) {
            *camera = stored_camera_pos;
            //level->camera.target = stored_camera_target;
        }
    }

    if (camera->z > train_end) {
        *camera = stored_camera_pos;
    }

    if (camera->z < curb)
        train->player_in = false;

    if (!train->player_in && camera->z < curb) {
        Vector3 velocity = Vector3Subtract(level->camera.position, stored_camera_pos);
        level->camera.position = stored_camera_pos;

        apply_3d_velocity(&level->camera, level->camera_height, level->scenes[0], velocity, false);
    }

    if (train->player_in) {
        camera->y = level->camera_height;

        level->camera.position.x += train->delta_x;
        level->camera.target.x   += train->delta_x;
    }

    if (can_move) {
        chapter_5_update_camera_look(&level->camera, dt);
    }

}

void chapter_5_update_player_staircase(Game *game, float dt) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    Vector3 stored_camera_pos = level->camera.position;

    bool can_move = keyboard_focus(game) == 0;

    if (can_move)
        update_camera_3d(&level->camera, PLAYER_SPEED_3D, dt);

    Chapter_5_Train *train = &level->train;
    Vector3 *camera = &level->camera.position;

    bool is_in_door = is_x_in_door_area(train, camera->x);

    float curb = 1.f; // + makes you go out of the train more

    if (!train->closed) {
        if (is_in_door) {
            if (camera->z > curb) { // out of train
                train->player_in = false;
            } else {
                train->player_in = true;
            }
        } else if (train->player_in) {
            if (train->player_in) {
                camera->z = Clamp(camera->z, -curb, curb);
            } else {
                if (camera->z < curb) {
                    camera->z = curb;
                }
            }
        } else {
            if (camera->z < curb)
                camera->z = curb;
        }
    } else {
        camera->z = Clamp(camera->z, -curb, curb);
    }

    if (!level->train.player_in) {
        Vector3 velocity = Vector3Subtract(level->camera.position, stored_camera_pos);
        level->camera.position = stored_camera_pos;

        apply_3d_velocity(&level->camera, level->camera_height, level->scenes[1], velocity, false);
    } else {
        level->camera.position.y = level->camera_height;

        level->camera.position.x += level->train.delta_x;
        level->camera.target.x   += level->train.delta_x;
    }

    if (camera->y > 12) {
        level->fade_to_dinner = true;
    }

    if (level->fade_to_dinner) {
        level->staircase_fade += 0.1f * dt;
        if (level->staircase_fade >= 1) {
            chapter_5_goto_scene(game, CHAPTER_5_SCENE_DINNER_PARTY);
            level->staircase_fade = 1;
        }
    }

    if (can_move)
        chapter_5_update_camera_look(&level->camera, dt);
}

void gallery_check_quotes(Game *game, float dt) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    if (level->current_quote == 0) {
        for (int i = 0; i < QUOTE_COUNT; i++) {
            Chapter_5_Quote *quote = level->quotes + i;

            if (quote->text) {
                Vector3 camera_pos = level->camera.position;
                float angle_delta  = Vector3Angle(Vector3Subtract(level->camera.target, level->camera.position),
                                                  Vector3Subtract(quote->target, quote->position));

                if (Vector3Distance(camera_pos, quote->position) < 0.5f &&
                    angle_delta < 1)
                {
                    level->current_quote = quote;
                    level->camera_target_saved = level->camera.target;
                    level->camera_position_saved = level->camera.position;

                    level->quote_count++;
                }
            }
        }
    }

    // Interpolate to the position and angle!
    Chapter_5_Quote *quote = level->current_quote;

    if (quote) {
        level->current_quote_time += 0.1f * dt;
        /*
        if (level->current_quote_time > 0.5)
            level->current_quote_time = 1;
            */

        level->camera.target = smoothstep_vector3(level->camera_target_saved,
                                                  quote->target,
                                                  min(1, 3 * level->current_quote_time));
        level->camera.position = smoothstep_vector3(level->camera_position_saved,
                                                    quote->position,
                                                    min(1, 3 * level->current_quote_time));
        level->camera.fovy = smoothstep(FOV_DEFAULT, quote->fov, level->current_quote_time);

        if (level->current_quote_time >= 1) {
            game->current             = quote->text;
            quote->text               = 0;
            level->current_quote      = 0;
            level->current_quote_time = 0;
            quote                     = 0;
        }
    } else if (game->current == 0) {
        level->camera.fovy = FOV_DEFAULT;

        if (IsKeyDown(KEY_F))
            level->camera.fovy = FOV_DEFAULT/2;
        else
            level->camera.fovy = FOV_DEFAULT;
    }
}

void chapter_5_update_player_desert(Game *game, float dt) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    if (game->current) return;

    if (level->current_quote == 0) {
        Vector3 stored_camera_pos = level->camera.position;
        update_camera_3d(&level->camera, 4, dt);

        Vector3 velocity = Vector3Subtract(level->camera.position, stored_camera_pos);
        level->camera.position = stored_camera_pos;

        if (level->current_scene == CHAPTER_5_SCENE_GALLERY)
            apply_3d_velocity(&level->camera, level->camera_height, level->scenes[5], velocity, false);
        else if (level->current_scene == CHAPTER_5_SCENE_SEASIDE)
            apply_3d_velocity(&level->camera, level->camera_height, level->scenes[6], velocity, false);
        else
            apply_3d_velocity(&level->camera, level->camera_height, level->scenes[4], velocity, false);

        chapter_5_update_camera_look(&level->camera, dt);
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

void chapter_5_update_player_dinner_party(Game *game, float dt) {
    Level_Chapter_5 *level = (Level_Chapter_5 *)game->level;

    Vector3 stored_camera_pos = level->camera.position;

    bool look = true;

    if (level->sitting_timer != -1 && level->sitting_at_table)
        level->sitting_timer += dt;

    if (!level->good) {
        if (level->sitting_timer >= 12) {
            float debug_speed_slider = 10;

            level->camera.target = lerp_vector3(level->camera.target,
                                                Vector3Add(get_chair_position(level->hope), {0,1.65f,0}),
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
            if (!level->finished_dinner_party && level->whiteness_overlay >= 1) {
                level->whiteness_overlay = 1;
                level->finished_dinner_party = true;
                add_event(game, chapter_5_finish_dinner_party, 5);
            }
        }
    }

    bool can_move = (game->current == 0);

    if (can_move && !level->sitting_at_table) {
        update_camera_3d(&level->camera, PLAYER_SPEED_3D, dt);
    }

    Vector3 velocity = Vector3Subtract(level->camera.position, stored_camera_pos);
    level->camera.position = stored_camera_pos;

    Model *scene = nullptr;

    if (level->good) {
        scene = &level->scenes[3];
    } else {
        scene = &level->scenes[2];
    }
    apply_3d_velocity(&level->camera, level->camera_height, *scene, velocity, true);

    if (look)
        chapter_5_update_camera_look(&level->camera, dt);

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

    if (level->queued_scene != CHAPTER_5_SCENE_INVALID) {
        chapter_5_scene_init(game);
        level->queued_scene = CHAPTER_5_SCENE_INVALID;
    }

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

            Vector2 camera = { level->camera.position.x, level->camera.position.z };
            level->staircase_podium_current = -1;
            level->bartender.talk_popup = false;

            if (game->current == 0) {
                for (int i = 0; i < StaticArraySize(level->staircase_podiums); i++) {
                    Chapter_5_Podium *podium = &level->staircase_podiums[i];
                    Vector2 podium_pos = *(Vector2*)&podium->position;

                    if (!podium->read && Vector2Distance(camera, podium_pos) < 2) {
                        level->staircase_podium_current = i;

                        if (is_action_pressed()) {
                            game->current = podium->text;
                            podium->read = true;
                        }

                        break;
                    }
                }

                Vector2 bartender = {level->bartender.position.x, level->bartender.position.z};
                level->bartender.talk_popup = (Vector2Distance(camera, bartender) < 4);

                if (level->bartender.talk_popup && is_action_pressed()) {
                    if (camera.y > 50) {
                        if (level->bartender.talked_behind) {
                            game->current = &game->text[17];
                        } else {
                            game->current = &game->text[16];
                        }
                        level->bartender.talked_behind = true;
                    } else {
                        if (level->bartender.talked_front) {
                            game->current = &game->text[18];
                        } else {
                            game->current = &game->text[10];
                        }
                        level->bartender.talked_front = true;
                    }
                }
            }
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
        case CHAPTER_5_SCENE_DESERT: {
            chapter_5_update_player_desert(game, dt);

            level->read_popup = false;

            Vector2 player = {level->camera.position.x, level->camera.position.z};

            if (game->current == 0) {
                for (int i = 0; i < level->podium_count; i++) {
                    Vector3 pos = level->podiums[i].position;

                    if (Vector2Distance({pos.x, pos.z}, player) < 3) {
                        if (IsKeyDown(KEY_EQUAL)) {
                            level->podiums[i].rotation += 10 * PI * dt;
                        }
                        if (IsKeyDown(KEY_MINUS)) {
                            level->podiums[i].rotation -= 10 * PI * dt;
                        }

                        level->read_popup = true;

                        if (is_action_pressed()) {
                            game->current = level->podiums[i].text;
                        }
                    }
                }
            }

            level->desert_door_popup = (Vector2Distance({5.915f, -15.37f}, player) < 3);
            if (level->desert_door_popup && is_action_pressed()) {
                chapter_5_goto_scene(game, CHAPTER_5_SCENE_GALLERY);
            }
        } break;
        case CHAPTER_5_SCENE_GALLERY: {
            chapter_5_update_player_desert(game, dt);

            gallery_check_quotes(game, dt);

            level->gallery_door_popup = Vector2Distance(level->door_position, {level->camera.position.x, level->camera.position.z}) < 3;

            if (level->gallery_door_popup && is_action_pressed()) {
                game->current = &game->text[100];
                Text *line = &game->text[100].text[2];
                line->lines[0].text[7] = '0' + level->quote_count;
            }
        } break;
        case CHAPTER_5_SCENE_SEASIDE: {
            if (level->seaside_ending) break;

            chapter_5_update_player_desert(game, dt);

            Vector2 cam = { level->camera.position.x, level->camera.position.z };
            Vector2 hope = { -16.47f, -18.84f };

            level->talk_to_hope_popup = game->current == 0 && (Vector2Distance(cam, hope) < 2) && !level->talked_to_hope;

            if (level->talk_to_hope_popup && is_action_pressed()) {
                level->talked_to_hope = true;
                game->current = &game->text[30];
            }

            if (level->talked_to_hope) {
                if (level->camera.position.z < -22) {
                    end_chapter_5_after_stop(game);
                }
            } else {
                if (level->camera.position.z < -22)
                    level->camera.position.z = -22;
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

                    DrawModelEx(level->models.body, level->clerk.position, {0,1,0}, level->clerk.body_rotation, {1,1,1}, WHITE);

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

            Vector2 player_p = { level->camera.position.x, level->camera.position.z };
            Vector2 bartender_p = { level->bartender.position.x, level->bartender.position.z  };
            float head_angle = atan2f(bartender_p.y - player_p.y, bartender_p.x - player_p.x);

            DrawModelEx(level->models.body,
                        level->bartender.position,
                        {0, 1, 0},
                        180,
                        {1,1,1},
                        WHITE);
            DrawModelEx(level->models.pyramid_head,
                        Vector3Add(level->bartender.position, {0,1.56f,0}),
                        {0, 1, 0},
                        180 - RAD2DEG * head_angle + 90,
                        {1,1,1},
                        WHITE);

            EndShaderMode();

            EndMode3D();

            if (level->staircase_podium_current != -1)
                draw_popup("READ ME\nREAD ME\nREAD ME\nREAD ME", RED, Top);

            if (level->bartender.talk_popup)
                draw_popup("DON'T TALK TO ME\nDON'T TALK TO ME\nDON'T TALK TO ME\n", RED, Top);

            DrawRectangle(0, 0, render_width, render_height, {255, 0, 0, (uint8_t)(level->staircase_fade * 255)});
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
        case CHAPTER_5_SCENE_DESERT: {
            ClearBackground(SKYBLUE);

            BeginMode3D(level->camera);

            DrawModel(level->scenes[4], {}, 1, WHITE);

            for (int i = 0; i < level->podium_count; i++) {
                DrawModelEx(level->models.podium, level->podiums[i].position, {0,1,0}, level->podiums[i].rotation, {1.3f, 1.3f, 1.3f}, WHITE);
            }

            EndMode3D();

            if (level->read_popup) {
                draw_popup("Read the Text", BLACK, Top);
            }

            if (level->desert_door_popup) {
                draw_popup("Open Door", BLACK, Top);
            }
        } break;
        case CHAPTER_5_SCENE_GALLERY: {
            ClearBackground(SKYBLUE);

            BeginMode3D(level->camera);

            DrawModel(level->scenes[5], {}, 1, WHITE);
            EndMode3D();

            if (level->gallery_door_popup) {
                draw_popup("Exit the Gallery?", BLACK, Top);
            }
        } break;
        case CHAPTER_5_SCENE_SEASIDE: {
            ClearBackground(BLACK);

            if (level->seaside_ending) break;

            BeginMode3D(level->camera);

            rlEnableColorBlend();
            DrawModel(level->scenes[6], {}, 1, WHITE);
            EndMode3D();

            if (level->talk_to_hope_popup) {
                draw_popup("Talk to Hope", GOLD, Top);
            }
        } break;
    }
}

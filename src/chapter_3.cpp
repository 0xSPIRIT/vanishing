#define MAX_DOCUMENTS   16
#define MAX_CORRECTIONS 256

enum Chapter_3_State {
    CHAPTER_3_STATE_OFFICE,
    CHAPTER_3_STATE_LUNCH,
    CHAPTER_3_STATE_WINDOW,
    CHAPTER_3_STATE_ROAD,
    CHAPTER_3_STATE_HOME,
    CHAPTER_3_STATE_HOME_INSIDE,
    CHAPTER_3_STATE_TIKTOK,
};

enum Chapter_3_Lunch_Text {
    CHAPTER_3_LUNCH_TEXT_INVALID,
    CHAPTER_3_LUNCH_TEXT_1,
    CHAPTER_3_LUNCH_TEXT_2,
    CHAPTER_3_LUNCH_TEXT_3,
};

struct Virtual_Mouse {
    Vector2 pos;
    int texture;
};

struct Word {
    int   start_index;
    int   end_index;
    char *correction; // becomes nullptr when corrected

    Rectangle rect;
};

struct Document {
    char   *string; // Recalculate length because it can be changed when correcting words

    Word    words[MAX_CORRECTIONS];
    size_t  word_count;
    size_t  error_count;

    int     num_corrections;
};

struct Chapter_3_Job_Minigame {
    bool active;

    bool draw_background;
    uint8_t window_alpha;

    float line_height;
    int   line_count;

    Color font_color;
    Color window_bg_color;

    int current_document;

    Document document_list[MAX_DOCUMENTS];
    size_t document_count;

    Font *font;

    Rectangle window_outer, window_inner, title_bar;
    int window_border_thickness;
    int window_title_bar_size;
    int font_spacing;

    Rectangle finish_button;

    float scroll_y;

    RenderTexture render_target;
};

struct Chapter_3_Phone {
    float scroll_y, scroll_y_to;
    Color c1, c2;
    int fade_alpha;

    float black_timer;
};

struct Chapter_3_Car {
    int texture;
    bool has_control;
    bool driving_off;

    Vector2 position;

    Color color;
};

struct Level_Chapter_3 {
    Chapter_3_State        state;
    Chapter_3_Job_Minigame minigame;
    Chapter_3_Phone        phone;
    Chapter_3_Car          car, other_car;

    int home_background;

    Camera2D camera;

    Virtual_Mouse virtual_mouse;

    Chapter_3_Lunch_Text current_lunch_text;

    float text_start_timer;
    float road_time;

    bool lunch_devil_effect;

    float end_steps;

    bool black_screen;

    //bool window_popup;
    bool read_sign_popup;
    bool bed_popup;

    Entity *player;

    int screens_scrolled;
};

void chapter_3_job_init(Game *game, int which_document_list);
Entity *chapter_3_make_entity(Entity_Type type, float x, float y);

void chapter_3_set_apartment_end_text(Game *game) {
    game->current = game->text + 73;
}

void chapter_3_goto_home_inside(Game *game) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;

    level->state = CHAPTER_3_STATE_HOME_INSIDE;

    //game->post_processing.type = POST_PROCESSING_PASSTHROUGH;
    game->post_processing.type = POST_PROCESSING_CRT;

    game->post_processing.crt.do_scanline_effect = true;
    game->post_processing.crt.do_warp_effect = false;
    game->post_processing.crt.abberation_intensity = 1;

    level->player = chapter_3_make_entity(ENTITY_PLAYER, 50, 50);

    array_add(&game->entities, level->player);
    level->player->pos = { 90, 100 };

    {
        Array<Entity*> *e = &game->entities;

        add_wall(e, {40, 25, 7, 135});
        add_wall(e, {45, 23, 102, 4});
        add_wall(e, {145, 27, 6, 133});
        add_wall(e, {130, 40, 15, 58});
        add_wall(e, {118, 58, 10, 20});
        add_wall(e, {54, 24, 31, 51});
        add_wall(e, {87, 27, 10, 13});
    }

    start_fade(game, FADE_IN, 120, nullptr);
}

void chapter_3_fade_out_outside_home(Game *game) {
    start_fade(game, FADE_OUT, chapter_3_goto_home_inside);
}

void chapter_3_fade_out_outside_home_delay(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    add_event(game, chapter_3_fade_out_outside_home, 2);
}

void chapter_3_goto_road(Game *game) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;

    level->minigame.active = false;

    level->state = CHAPTER_3_STATE_ROAD;
    game->post_processing.type = POST_PROCESSING_CRT;

    Post_Processing_Crt *crt = &game->post_processing.crt;
    crt->do_scanline_effect = true;
    crt->do_warp_effect = true;
    crt->scanline_alpha = 0.25;
    crt->abberation_intensity = 0.75f;
    crt->vignette_intensity = 1;
    crt->vignette_mix = 0.1f;
    //post_process_vhs_set_intensity(&game->post_processing.vhs, VHS_INTENSITY_LOW);

    start_fade(game, FADE_IN, nullptr);
}

void chapter_3_next_document(Chapter_3_Job_Minigame *minigame) {
    if (minigame->current_document+1 < minigame->document_count) {
        minigame->current_document++;
        minigame->scroll_y = 0;
    }
}

void document_register_word(Document *doc, int start_index,
                            int end_index)
{
    doc->words[doc->word_count++] = {start_index, end_index, 0, {}};
}

void document_register_error(Document *doc, int start_index,
                             int end_index, char *correction)
{
    assert(correction);
    doc->words[doc->word_count++] = {start_index, end_index, correction, {}};
    doc->error_count++;
}

// n starts at 1.
void document_register_error_nth_occurrence(Document *doc, int n, char *word, char *correction) {
    Find_Word_Result result = find_word(doc->string, word, n);

    if (result.start_index != -1 && result.end_index != -1) {
        document_register_error(doc, result.start_index, result.end_index, correction);
    }
}

void document_register_error_first_occurrence(Document *doc, char *word, char *correction) {
    document_register_error_nth_occurrence(doc, 1, word, correction);
}

Chapter_3_Job_Minigame chapter_3_make_job_minigame(Arena *level_arena,
                                                   const char **document_list,
                                                   size_t document_count)
{
    Chapter_3_Job_Minigame result = {};

    result.active = false;

    result.font_color      = BLACK;
    result.window_bg_color = WHITE;
    result.window_alpha    = 255;
    result.draw_background = true;

    result.document_count   = document_count;
    result.current_document = 0;//(int)document_count - 1;//0;

    for (int i = 0; i < document_count; i++) {
        Document *document = &result.document_list[i];
        document->string   = (char*)arena_push(level_arena, strlen(document_list[i])*2);

        strcpy(document->string, document_list[i]);
    }

    result.window_border_thickness = 2;
    result.window_title_bar_size   = 12;
    result.window_border_thickness = 2;
    result.font_spacing            = 1;

    result.window_outer = {
        render_width/10.f,
        render_height/10.f,
        8.f*render_width/10,
        8.f*render_height/10
    };

    result.window_outer = integer_rectangle(result.window_outer);
    result.window_inner = result.window_outer;

    result.window_inner.x      += result.window_border_thickness;
    result.window_inner.y      += result.window_title_bar_size;
    result.window_inner.width  -= result.window_border_thickness * 2;
    result.window_inner.height -= result.window_title_bar_size + result.window_border_thickness;

    result.title_bar        = result.window_outer;
    result.title_bar.height = result.window_title_bar_size;

    result.finish_button.width  = 40;
    result.finish_button.height = 14;

    result.finish_button.x = result.window_inner.width  - result.finish_button.width  - 3;
    result.finish_button.y = result.window_inner.height - result.finish_button.height - 3;

    result.font = &atari_font;

    result.render_target = LoadRenderTexture(result.window_inner.width,
                                             result.window_inner.height);

    // Tokenize

    for (int doc = 0; doc < result.document_count; doc++) {
        Document *document = &result.document_list[doc];

        Word word = {};

        int string_length = (int)strlen(document->string);

        for (int i = 0; i < string_length; i++) {
            if (i >= string_length-1 || (isspace(document->string[i+1]) && !isspace(document->string[i]))) {
                word.end_index = i;

                document_register_word(document, word.start_index, word.end_index);

                int idx = i;
                while (isspace(document->string[++idx]));

                word.start_index = idx;
            }
        }
    }

    return result;
}

Entity *chapter_3_make_entity(Entity_Type type, float x, float y) {
    Entity *result = allocate_entity();

    result->type = type;
    result->pos = {x, y};

    switch (type) {
        case ENTITY_PLAYER: {
            result->texture_id = 3;
        } break;
        case ENTITY_CHAP_3_CUBICLE_VERTICAL: {
            result->texture_id = 4;
        } break;
        case ENTITY_CHAP_3_CUBICLE_TOP: {
            result->texture_id = 5;
        } break;
        case ENTITY_CHAP_3_CUBICLE_CHAIR: {
            result->texture_id = 6;
            result->has_dialogue = true;
        } break;
        case ENTITY_CHAP_3_GUY: {
            result->texture_id = 8;
            result->has_dialogue = true;
        } break;
        case ENTITY_CHAP_3_GIRL: {
            result->texture_id = 9;
            result->has_dialogue = true;
        } break;
        case ENTITY_CHAP_3_LUNCH_TABLE: {
            result->texture_id = 10;
        } break;
        case ENTITY_CHAP_3_CAR: {
            result->texture_id = 23;
        } break;
        case ENTITY_PHONE: {
            result->texture_id = 26;
            result->has_dialogue = true;
        } break;
    }

    float texture_width  = entity_texture_width(result);
    float texture_height = entity_texture_height(result);

    if (type == ENTITY_CHAP_3_LUNCH_TABLE) {
        result->base_collider = { 0, 6, 51, 25 };
    } else if (type == ENTITY_CHAP_3_CUBICLE_VERTICAL) {
        result->base_collider = { 0, 11, 14, 53 };
    } else if (type == ENTITY_CHAP_3_CUBICLE_TOP) {
        result->base_collider = { 0, 11, 27, 21 };
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

void chapter_3_goto_home_outside(Game *game) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;

    game->post_processing.type = POST_PROCESSING_CRT;
    Post_Processing_Crt *crt = &game->post_processing.crt;

    crt->do_scanline_effect = true;
    crt->do_warp_effect = true;
    crt->abberation_intensity = 0.75;
    crt->vignette_intensity = 1;
    crt->vignette_mix = 0.8f;
    crt->scanline_alpha = 0.1f;
    //post_process_vhs_set_intensity(&game->post_processing.vhs, VHS_INTENSITY_LOW);

    level->home_background = 18;

    level->camera.offset = {
        0, -(float)render_height,
    };
    level->camera.zoom = 1;

    level->state = CHAPTER_3_STATE_HOME;
    for (int i = 0; i < game->entities.length; i++) {
        free_entity(game->entities.data[i]);
        array_remove(&game->entities, i--);
    }

    level->player = chapter_3_make_entity(ENTITY_PLAYER, 58, 6*render_height+95);
    array_add(&game->entities, level->player);
    array_add(&game->entities, chapter_3_make_entity(ENTITY_CHAP_3_CAR, 16, 6*render_height+112));

    array_add(&game->entities, chapter_3_make_entity(ENTITY_PHONE, 164, 320+354));

    add_wall(&game->entities, {0, 160, 63, 520});
    add_wall(&game->entities, {129, 160, 63, 520});

    start_fade(game, FADE_IN, 240, nullptr);
}

void add_cubicle(Array<Entity*> *entities, bool right, int x, int y) {
    Entity *vertical = 0, *top = 0, *chair = 0;

    int chair_x = 0;
    int chair_y = 0;

    if (right) {
        top      = chapter_3_make_entity(ENTITY_CHAP_3_CUBICLE_TOP, x, y);
        vertical = chapter_3_make_entity(ENTITY_CHAP_3_CUBICLE_VERTICAL, x + entity_texture_width(top), y);

        chair_x = x + 6 + rand()%6;
        chair_y = y + 16;
    } else {
        vertical = chapter_3_make_entity(ENTITY_CHAP_3_CUBICLE_VERTICAL, x, y);
        top      = chapter_3_make_entity(ENTITY_CHAP_3_CUBICLE_TOP, x + entity_texture_width(vertical), y);

        chair_x = x + 16 + rand()%6;
        chair_y = y + 16;
    }

    chair = chapter_3_make_entity(ENTITY_CHAP_3_CUBICLE_CHAIR, chair_x, chair_y);

    int number = rand()%2000;
    if (number == 1138) number++; // we do this ourselves manually so it can't randomly happen.

    vertical->chap_3_cubicle.number           = number;
    vertical->chap_3_cubicle.cubicle_chair    = chair;
    vertical->chap_3_cubicle.cubicle_top      = top;
    vertical->chap_3_cubicle.cubicle_vertical = vertical;

    top->chap_3_cubicle.number                = number;
    top->chap_3_cubicle.cubicle_chair         = chair;
    top->chap_3_cubicle.cubicle_top           = top;
    top->chap_3_cubicle.cubicle_vertical      = vertical;

    chair->chap_3_cubicle.number              = number;
    chair->chap_3_cubicle.cubicle_chair       = chair;
    chair->chap_3_cubicle.cubicle_top         = top;
    chair->chap_3_cubicle.cubicle_vertical    = vertical;

    array_add(entities, top);
    array_add(entities, vertical);
    array_add(entities, chair);
}

void chapter_3_goto_job_minigame(void *game_ptr) {
    Game *game = (Game *)game_ptr;
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;
    level->minigame.active = true;
    DisableCursor();
    level->virtual_mouse.pos = { render_width/2.f, render_height/2.f };

    game->post_processing.type = POST_PROCESSING_CRT;

    game->post_processing.crt.do_scanline_effect = true;
    game->post_processing.crt.scanline_alpha = 0.25f;
    game->post_processing.crt.do_warp_effect = false;
    //game->post_processing.crt.abberation_intensity = 0.75f;

    // abberation intensity is handled in update
    game->post_processing.crt.vignette_intensity = 1;
    game->post_processing.crt.vignette_mix = 0.75f;
    game->post_processing.crt.scanline_alpha = 0.1f;
}

void chapter_3_goto_window_text(Game *game) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;

    game->post_processing.type = POST_PROCESSING_PASSTHROUGH;

    level->state = CHAPTER_3_STATE_WINDOW;
    chapter_3_job_init(game, 3);

    chapter_3_goto_job_minigame(game);

    level->black_screen = false;
}

Entity *add_lunch_table(Array<Entity*> *entities, int x, int y, int num_people) {
    Entity *table = chapter_3_make_entity(ENTITY_CHAP_3_LUNCH_TABLE, x, y);
    array_add(entities, table);

    Vector2 table_center = {
        table->pos.x + entity_texture_width(table)  / 2 - 6,
        table->pos.y + entity_texture_height(table) / 2 - 15
    };

    float angle = (float)fmod(x, 2 * PI);

    for (int i = 0; i < num_people; i++) {
        float radius = 26;
        Vector2 person = Vector2Add(table_center, {1.25f * cosf(angle) * radius, 0.7f * sinf(angle) * radius});

        Entity_Type type = (i%2) ? ENTITY_CHAP_3_GUY : ENTITY_CHAP_3_GIRL;

        Entity *e = chapter_3_make_entity(type, person.x, person.y);

        e->chap_3_circler.radius_x = 1.25f * radius;
        e->chap_3_circler.radius_y = 0.70f * radius;
        e->chap_3_circler.center_position = table_center;
        e->chap_3_circler.current_angle = angle;

        array_add(entities, e);

        angle += 2 * PI / num_people;
    }

    return table;
}

void chapter_3_goto_lunch_room(Game *game, Chapter_3_Lunch_Text lunch_text) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;

    level->current_lunch_text = lunch_text;

    game->post_processing.type = POST_PROCESSING_CRT;
    game->post_processing.crt.do_scanline_effect = false;
    game->post_processing.crt.do_warp_effect = false;
    game->post_processing.crt.abberation_intensity = 0.5f;
    game->post_processing.crt.vignette_intensity = 1;
    game->post_processing.crt.vignette_mix = 0.5f;

    level->state = CHAPTER_3_STATE_LUNCH;
    level->minigame.active = false;

    for (int i = 0; i < game->entities.length; i++) {
        Entity *e = game->entities.data[i];
        if (e->type != ENTITY_PLAYER) {
            free_entity(e);
            array_remove(&game->entities, i--);
        }
    }

    Array<Entity*> *entities = &game->entities;

    int table_1_count = 8;
    int table_2_count = 4;

    if (lunch_text == CHAPTER_3_LUNCH_TEXT_2) {
        table_1_count = 4;
        table_2_count = 3;
    }
    if (lunch_text == CHAPTER_3_LUNCH_TEXT_3) {
        table_1_count = 3;
        table_2_count = 2;
    }

    add_lunch_table(entities,  35,  35, table_1_count);
    add_lunch_table(entities, 100, 100, table_2_count);

    add_wall(entities, {0, 0, 192, 11});
    add_wall(entities, {0, 160, 200, 4});

    level->player->pos.x = 129;
    level->player->pos.y = 88;

    level->text_start_timer = 2;
}

void chapter_3_init_outside(Game *game) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;

    add_wall(&game->entities, {0, 0, 6, 94});
    add_wall(&game->entities, {95, 0, 3, 92});
    add_wall(&game->entities, {185, 0, 3, 95});

    int x = rand_bool(0.5) ? 50 : 160;
    int y = rand_bool(0.5) ? 50 : 30;

    if (level->screens_scrolled < 8) {
        if (rand_bool(0.5)) {
            Entity *guy = chapter_3_make_entity(ENTITY_CHAP_3_GUY, x, y);
            array_add(&game->entities, guy);
        } else {
            Entity *girl = chapter_3_make_entity(ENTITY_CHAP_3_GIRL, x, y);
            array_add(&game->entities, girl);
        }
    }
}

void chapter_3_job_init(Game *game, int which_document_list) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;

    const char *documents[MAX_DOCUMENTS] = {};

    int count = 0;

    switch (which_document_list) {
        case 0: {
            documents[count++] =
                "Introduction\n----------------\n\nOn your keyboard use UP and DOWN or SCROLL WHEEL to scroll, or on your controller use RIGHT STICK to scroll.\n\nThese are excerpts from documents throughout our company. Your job is to read the document thoroughly and click on any errors to correct them. After this, they're sent to the next editor in our document assurance unit. We mustn't look unprefesional!";
            documents[count++] =
                "Memo\n-------\n\nDear Team,\n  This is a frendly reminder about our upcoming employee dinner party scheduled for June 20th.\n  Due to the influx of new employees, it'll be a great place to get setled into the company culture!\n  Please ensure you RSVP by June 10th to help with planing the catering arrangements.\n\nBest Regards,\nJane Smith\n(HR Manager)";
            documents[count++] =
                "Email\n-------\n\nDear Tech Solutions Team,\n  We're experiencing a persistant issue with our data management system and is causing noticable reductions in employees' eficiences and qualities of life.\n  We are reaching out to you in hopes of finding a solution that can retsore functionality to our operation. Your expertise and guidance in this matter would be invaluable to us.\n\nRegards,\nMichael (Project Manager)";
            documents[count++] =
                "Memo\n-------\n\nNotice for all employees:\n  Please be adviesd that the copier machien is not wokring. We bought a replacemnet and it is arriving next Monday.";
            documents[count++] =
                "Email\n-------\n\nDear Peggy,\n  We met a few weeks ago at the business meeting in Sydney; we had dinner that evening. I'm following up on potential collaboration opportunities between our branches that may be beneficial to us both.\n  If there are any barriers or concerns, please let me know.\n\nWarm regards,\nHunter (Business Development Manager, Nebraska)";

            level->minigame = chapter_3_make_job_minigame(&game->level_arena, documents, count);

            Document *d0 = &level->minigame.document_list[0];
            document_register_error_first_occurrence(d0, "unprefesional!", "unprofessional.");

            Document *d1 = &level->minigame.document_list[1];
            document_register_error_first_occurrence(d1, "frendly", "friendly");
            document_register_error_first_occurrence(d1, "setled",  "settled");
            document_register_error_first_occurrence(d1, "planing", "planning");

            Document *d2 = &level->minigame.document_list[2];
            document_register_error_first_occurrence(d2, "persistant", "persistent");
            document_register_error_first_occurrence(d2, "noticable",  "noticeable");
            document_register_error_first_occurrence(d2, "eficiences", "efficiencies");
            document_register_error_first_occurrence(d2, "retsore",    "restore");

            Document *d3 = &level->minigame.document_list[3];
            document_register_error_first_occurrence(d3, "adviesd", "advised");
            document_register_error_first_occurrence(d3, "machien", "machine");
            document_register_error_first_occurrence(d3, "wokring.", "working.");
            document_register_error_first_occurrence(d3, "replacemnet", "replacement");
        } break;
        case 1: {
            documents[count++] =
                "Memo\n-------\n\nDear Team,\n  Please be noted that the cofee machine is no longer wokring. Joanne brought some instant coffee and left it in the cuboard for anyone whod like.\n\nRegards,\nMichael\n(Project Manager)";
            documents[count++] =
                "Email\n-------\n\nDear Tech Solutions Team,\n  Unfortunately, despite your team's efforts, our data management system still has many of the same issues, rendering our employees extremely inefficient.\n  I request that you return and complete the job that you were paid for.\n\nRegards,\nMichael (Project Manager)";
            documents[count++] =
                "Memo\n-------\n\nDear Team,\n  Kindly note that the mandatory anonymous employee experience survey is due by July 1st.\n  Please be brutally honest, we'd love the feedback!\n\nRegards,\nMichael (Project Manager)";
            documents[count++] =
                "Memo\n-------\n\nDear Team,\n  Over the past three and a half years, we have made great progress as a company under my leadership.\n  Today, we have a foothold on our market in Earth.\n  I know none of this could have been done without you, my trusted employees. It has been an honour to serve as your CEO, but I believe it is in the best interest of thecompany for me to stand down, and focus on the remainder of my position, until August 22nd.\n  Just remember, there's nothing this company can't do!\n\nLove,\nJohn Briden (CEO)";
            documents[count++] =
                "Email\n-------\n\nDear Peggy,\n  I received your email, and I'm delighted to hear that you're excited about a collaboration between our two branches. I'm free for lunch on Saturday June 8th, so we can discuss a proposal. Is that fine with your schedule?\n\nRegards,\nHunter (Business Development Manager, Nebraska)";
            documents[count++] = 
                "\"Don't throw away your suffering. Touch your suffering. Face it directly, and your joy will become deeper. You know that suffering and joy are both impermanent.\"";
                // But remember, "This work was strictly voluntary, but any animal who absented himself from it would have his rations reduced by half." (George Orwell, Animal Farm)

            level->minigame = chapter_3_make_job_minigame(&game->level_arena, documents, count);

            Document *d0 = &level->minigame.document_list[0];
            document_register_error_first_occurrence(d0, "cofee", "coffee");
            document_register_error_first_occurrence(d0, "wokring.", "working.");
            document_register_error_first_occurrence(d0, "cuboard", "cupboard");
            document_register_error_first_occurrence(d0, "whod", "who'd");

            Document *d1 = &level->minigame.document_list[1];
            document_register_error_first_occurrence(d1, "request", "demand");

            Document *d2 = &level->minigame.document_list[2];
            //document_register_error_first_occurrence(d2, "Kindly", "Please");
            document_register_error_first_occurrence(d2, "note", "forget");
            //document_register_error_first_occurrence(d2, "that", "remember");
            document_register_error_first_occurrence(d2, "anonymous", "pointless");
            document_register_error_first_occurrence(d2, "Please", "Don't");
            document_register_error_first_occurrence(d2, "love", "hate");

            Document *d3 = &level->minigame.document_list[3];
            document_register_error_first_occurrence(d3, "great", "some");
            document_register_error_first_occurrence(d3, "Earth.", "Nebraska.");
            document_register_error_first_occurrence(d3, "trusted", "underpaid");
            document_register_error_first_occurrence(d3, "honour", "experience");
            document_register_error_first_occurrence(d3, "thecompany", "myself");
            document_register_error_first_occurrence(d3, "nothing", "much");
            document_register_error_first_occurrence(d3, "Love,", "Bye,");

            Document *d5 = &level->minigame.document_list[5];
            document_register_error_nth_occurrence(d5, 1, "\"Don't",    "But");
            document_register_error_nth_occurrence(d5, 1, "throw",      "remember,");
            document_register_error_nth_occurrence(d5, 1, "away",       "\"This");
            document_register_error_nth_occurrence(d5, 1, "your",       "work");
            document_register_error_nth_occurrence(d5, 1, "suffering.", "was,");

            document_register_error_nth_occurrence(d5, 1, "Touch",      "strictly");
            document_register_error_nth_occurrence(d5, 2, "your",       "voluntary,");
            document_register_error_nth_occurrence(d5, 2, "suffering.", "but");

            document_register_error_nth_occurrence(d5, 1, "Face",       "any");
            document_register_error_nth_occurrence(d5, 1, "it",         "animal");
            document_register_error_nth_occurrence(d5, 1, "directly,",  "who");
            document_register_error_nth_occurrence(d5, 1, "and",        "absented");
            document_register_error_nth_occurrence(d5, 3, "your",    "himself");
            document_register_error_nth_occurrence(d5, 1, "joy",        "from");
            document_register_error_nth_occurrence(d5, 1, "will",       "it");
            document_register_error_nth_occurrence(d5, 1, "become",     "would");
            document_register_error_nth_occurrence(d5, 1, "deeper.",    "have");
            document_register_error_nth_occurrence(d5, 1, "You",        "his");
            document_register_error_nth_occurrence(d5, 1, "know",       "rations");
            document_register_error_nth_occurrence(d5, 1, "that",       "reduced");
            document_register_error_nth_occurrence(d5, 1, "suffering",  "by");
            document_register_error_nth_occurrence(d5, 2, "and",        "half.\"");
            document_register_error_nth_occurrence(d5, 2, "joy",        "(George");
            document_register_error_nth_occurrence(d5, 1, "are",        "Orwell,");
            document_register_error_nth_occurrence(d5, 1, "both",       "Animal");
            document_register_error_nth_occurrence(d5, 1, "impermanent.\"", "Farm)");
        } break;
        case 2: {
            documents[count++] =
                "Email\n-------\n\nDear Team,\n  I hope everyone enojyed the dinner party on Thursday! It seemed to have a great atmosphere, and good for the new employes to familiarize themselfs with the company culture.\n\nBest Regards,\nJane Smith\n(HR Manager)";
            documents[count++] =
                "Email\n-------\n\nPeggy,\n  The dinner was great. I suppose we don't need a reason to email on this thing anymore, but something compelled me to.\n  Anyways, have a nice night, and I hope we see each other again :)\n\nRegards,\nHunter";
            documents[count++] =
                "Dear Team, It's assumed that you all are aware of the new policy on bathrooms...";

            level->minigame = chapter_3_make_job_minigame(&game->level_arena, documents, count);

            Document *d0 = &level->minigame.document_list[0];
            document_register_error_first_occurrence(d0, "enojyed",   "enjoyed");
            document_register_error_first_occurrence(d0, "employes",  "employees");
            document_register_error_first_occurrence(d0, "themselfs", "themselves");

            Document *d1 = &level->minigame.document_list[1];
            document_register_error_first_occurrence(d1, "great.",    "amazing.");
            document_register_error_first_occurrence(d1, "Regards,",  "Love,");

            Document *d2 = &level->minigame.document_list[2];
            document_register_error_first_occurrence(d2, "Dear",         "\"One");
            document_register_error_first_occurrence(d2, "Team,",        "does");
            document_register_error_first_occurrence(d2, "It's",         "not");
            document_register_error_first_occurrence(d2, "assumed",      "discover");
            document_register_error_first_occurrence(d2, "that",         "the");
            document_register_error_first_occurrence(d2, "you",          "absurd");
            document_register_error_first_occurrence(d2, "all",          "without");
            document_register_error_first_occurrence(d2, "are",          "being");
            document_register_error_first_occurrence(d2, "aware",        "tempted");
            document_register_error_first_occurrence(d2, "of",           "to");
            document_register_error_first_occurrence(d2, "the",          "write");
            document_register_error_first_occurrence(d2, "new",          "a");
            document_register_error_first_occurrence(d2, "policy",       "manual");
            document_register_error_first_occurrence(d2, "on",           "of");
            document_register_error_first_occurrence(d2, "bathrooms...", "happiness.\" [2]");
        } break;
        case 3: {
            documents[count++] =
                "Chase stared out a window.";
            documents[count++] =
                "Although his eyes were fixated on an insignificant patch of grass outside, his mind wandered someplace else.";
            documents[count++] =
                "\"How is it that these people are so happy?\", he thought.";
            documents[count++] =
                "\"How can they find meaning in something so meaningless?\"";
            documents[count++] =
                "He thought of them as inferior to himself, placing himself on a throne so high the gap was insurmountable; a veil of diamond.";
               //He thought of them as superior to himself. They had something so pure, he could only stare in confusion, impossible to have himself.
            documents[count++] =
                "Oh, they created their own meaning, found some special place in our universe for themselves.";
                //"\"It doesn't make sense; they had to have made a deal with the devil himself!\"";
            documents[count++] =
                "Chase drove home that afternoon, shook.\n\nThe mere weeks of working at that place was enough to sicken him.";

            level->minigame = chapter_3_make_job_minigame(&game->level_arena, documents, count);

            level->minigame.draw_background = false;
            level->minigame.window_alpha    = 64;
            level->minigame.font_color      = WHITE;

            Document *d4 = &level->minigame.document_list[4];
            document_register_error_nth_occurrence(d4, 1, "inferior",        "superior");
            document_register_error_nth_occurrence(d4, 1, "himself,",        "himself.");
            document_register_error_nth_occurrence(d4, 1, "placing",         "They");
            document_register_error_nth_occurrence(d4, 1, "himself",         "had");
            document_register_error_nth_occurrence(d4, 1, "on",              "something");
            document_register_error_nth_occurrence(d4, 1, "a",               "so");
            document_register_error_nth_occurrence(d4, 1, "throne",          "pure,");
            document_register_error_nth_occurrence(d4, 1, "so",              "he");
            document_register_error_nth_occurrence(d4, 1, "high",            "could");
            document_register_error_nth_occurrence(d4, 1, "the",             "only");
            document_register_error_nth_occurrence(d4, 1, "gap",             "stare");
            document_register_error_nth_occurrence(d4, 1, "was",             "in");
            document_register_error_nth_occurrence(d4, 1, "insurmountable;", "confusion,");
            document_register_error_nth_occurrence(d4, 2, "a",               "impossible");
            document_register_error_nth_occurrence(d4, 1, "veil",            "to");
            document_register_error_nth_occurrence(d4, 2, "of",              "have");
            document_register_error_nth_occurrence(d4, 1, "diamond.",        "himself.");

            Document *d5 = &level->minigame.document_list[5];
            document_register_error_first_occurrence(d5, "Oh,", "\"It");
            document_register_error_first_occurrence(d5, "they", "doesn't");
            document_register_error_first_occurrence(d5, "created", "make");
            document_register_error_first_occurrence(d5, "their", "sense;");
            document_register_error_first_occurrence(d5, "own", "they");
            document_register_error_first_occurrence(d5, "meaning,", "had");
            document_register_error_first_occurrence(d5, "found", "to");
            document_register_error_first_occurrence(d5, "some", "have");
            document_register_error_first_occurrence(d5, "special", "made");
            document_register_error_first_occurrence(d5, "place", "a");
            document_register_error_first_occurrence(d5, "in", "deal");
            document_register_error_first_occurrence(d5, "our", "with");
            document_register_error_first_occurrence(d5, "universe", "the");
            document_register_error_first_occurrence(d5, "for", "devil");
            document_register_error_first_occurrence(d5, "themselves.", "himself!\"");

        } break;
    }
}

void chapter_3_init(Game *game) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;

    game->post_processing.type = POST_PROCESSING_CRT;
    game->post_processing.crt.do_scanline_effect = false;
    game->post_processing.crt.scanline_alpha = 0.07f;
    game->post_processing.crt.do_warp_effect = false;
    game->post_processing.crt.abberation_intensity = 0.5f;
    game->post_processing.crt.vignette_intensity = 1;
    game->post_processing.crt.vignette_mix = 0.75f;

    //level->state = CHAPTER_3_STATE_HOME_INSIDE;
    //level->state = CHAPTER_3_STATE_ROAD;

    Texture2D *textures = atari_assets.textures;
    textures[0]  = load_texture(RES_DIR "art/desktop_wallpaper.png");
    textures[1]  = load_texture(RES_DIR "art/window_controls.png");
    textures[2]  = load_texture(RES_DIR "art/window_title.png");
    textures[3]  = load_texture(RES_DIR "art/player.png");
    textures[4]  = load_texture(RES_DIR "art/cubicle_vertical_part.png");
    textures[5]  = load_texture(RES_DIR "art/cubicle_top_part.png");
    textures[6]  = load_texture(RES_DIR "art/office_chair.png");
    textures[7]  = load_texture(RES_DIR "art/office_outside.png");
    textures[8]  = load_texture(RES_DIR "art/office_guy.png");
    textures[9]  = load_texture(RES_DIR "art/office_girl.png");
    textures[10] = load_texture(RES_DIR "art/kitchen_table.png");
    textures[11] = load_texture(RES_DIR "art/lunch_room.png");
    textures[12] = load_texture(RES_DIR "art/devil_table.png");
    textures[13] = load_texture(RES_DIR "art/guy_blue.png");
    textures[14] = load_texture(RES_DIR "art/girl.png");
    textures[15] = load_texture(RES_DIR "art/player_white.png");
    textures[16] = load_texture(RES_DIR "art/open_window.png");
    textures[17] = load_texture(RES_DIR "art/road.png");
    textures[18] = load_texture(RES_DIR "art/home_outside.png");
    textures[19] = load_texture(RES_DIR "art/apartment2.png");
    textures[20] = load_texture(RES_DIR "art/smartphone.png");
    textures[21] = load_texture(RES_DIR "art/arrow_white.png");
    textures[22] = load_texture(RES_DIR "art/cursor.png");
    textures[23] = load_texture(RES_DIR "art/car.png");
    textures[24] = load_texture(RES_DIR "art/tree1.png");
    textures[25] = load_texture(RES_DIR "art/tree2.png");
    textures[26] = load_texture(RES_DIR "art/phone.png");

    level->car.texture = 23;
    level->car.position = { (float)-textures[23].width, render_height/2.f + 20 };
    level->car.color = WHITE;

    level->other_car.texture = 23;
    level->other_car.color = RED;
    level->other_car.position = { (float)render_width, render_height/2.f - 5 };

    level->camera.zoom = 1;

    game->entities = make_array<Entity*>(64);

    float speed = 30;

    atari_text_list_init(&game->text[0],
                         0,
                         "This isn't your desk.",
                         speed,
                         nullptr);

    String choices[] = { const_string("Yes"), const_string("No") };

    Text_List *next[] = { nullptr, nullptr };
    void (*hooks[])(void*) = { chapter_3_goto_job_minigame, nullptr };

    atari_choice_text_list_init(&game->text[1],
                                0,
                                "Sit at your desk?",
                                choices,
                                next,
                                hooks,
                                2);

    atari_text_list_init(&game->text[2],
                         "Jake", // Jake, Mike, Amy, --
                         "Morning, Chase.\rWeather's great today,\nisn't it?",
                         speed,
                         nullptr);

    atari_text_list_init(&game->text[3],
                         0,
                         "You shouldn't be here.",
                         10,
                         nullptr);

    speed = 30;
    
    // Lunch Text 1

    atari_text_list_init(&game->text[4],
                         "Jake",
                         "So then I decided to switch\nto whole wheat.",
                         speed,
                         &game->text[5]);
    atari_text_list_init(&game->text[5],
                         "Amy",
                         "I just buy whatever's\non sale.",
                         speed,
                         &game->text[6]);
    atari_text_list_init(&game->text[6],
                         "Mike",
                         "Brioche bread is the best\nthough. Especially for\nsandwiches.",
                         speed,
                         &game->text[7]);
    atari_text_list_init(&game->text[7],
                         "Amy",
                         "Oh! Did I tell you guys\nabout this morning?",
                         speed,
                         &game->text[8]);
    atari_text_list_init(&game->text[8],
                         "Mike",
                         "What happened?",
                         speed,
                         &game->text[9]);
    atari_text_list_init(&game->text[9],
                         "Amy",
                         "I got stuck in traffic\nfor nearly an hour\non the way to work.",
                         speed,
                         &game->text[10]);
    atari_text_list_init(&game->text[10],
                         "Mike",
                         "Was there an accident\nor something?",
                         speed,
                         &game->text[11]);
    atari_text_list_init(&game->text[11],
                         "Amy",
                         "I think they had some\nconstruction on and they\nhad to close a lane.",
                         speed,
                         &game->text[12]);
    atari_text_list_init(&game->text[12],
                         "Jake",
                         "By the way, did y'all\nhear that John is\nretiring?",
                         speed,
                         &game->text[13]);
    atari_text_list_init(&game->text[13],
                         "Clarice",
                         "That's old news, Jake.\rY'know,\rI was thinking",
                         speed,
                         nullptr);

    // Lunch Text 2

    atari_text_list_init(&game->text[15],
                         "Amy",
                         "So then I decided to switch\nto whole wheat.",
                         speed,
                         &game->text[16]);
    atari_text_list_init(&game->text[16],
                         "Jake",
                         "I just buy whatever's\non sale.",
                         speed,
                         &game->text[17]);
    atari_text_list_init(&game->text[17],
                         "Clarice",
                         "Brioche bread is the best\nthough. Especially for\nsandwiches.",
                         speed,
                         &game->text[18]);
    atari_text_list_init(&game->text[18],
                         "Jake",
                         "Oh! Did I tell you guys\nabout this morning?",
                         speed,
                         &game->text[19]);
    atari_text_list_init(&game->text[19],
                         "Amy",
                         "What happened?",
                         speed,
                         &game->text[20]);
    atari_text_list_init(&game->text[20],
                         "Jake",
                         "I got stuck in traffic\nfor nearly an hour\non the way to work.",
                         speed,
                         &game->text[21]);
    atari_text_list_init(&game->text[21],
                         "Amy",
                         "Oh no,\rwas there an accident\nor something?",
                         speed,
                         &game->text[22]);
    atari_text_list_init(&game->text[22],
                         "Jake",
                         "I think they had some\nconstruction on and they\nhad to close a lane.",
                         speed,
                         &game->text[23]);
    atari_text_list_init(&game->text[23],
                         "Clarice",
                         "By the way,\rdid y'all see Jenny and\nMark ye-",
                         speed,
                         &game->text[24]);
    atari_text_list_init(&game->text[24],
                         "Chase",
                         "Wait a minute.",
                         speed,
                         &game->text[25]);
    atari_text_list_init(&game->text[25],
                         "Chase",
                         "...\rWasn't there another\nperson here yesterday?",
                         speed,
                         &game->text[26]);
    atari_text_list_init(&game->text[26],
                         "Amy",
                         "What are you talking\nabout, Chase?",
                         speed,
                         &game->text[27]);
    atari_text_list_init(&game->text[27],
                         "Chase",
                         "Mike, right?\rIs he sick or something?\rUsually he's here.",
                         speed,
                         &game->text[28]);
    atari_text_list_init(&game->text[28],
                         "Amy",
                         "...\rWho's Mike?",
                         speed,
                         nullptr);

    // Lunch text 3
    atari_text_list_init(&game->text[30],
                         "Person",
                         "So then I decided to switch\nto whole wheat.",
                         speed,
                         &game->text[31]);
    atari_text_list_init(&game->text[31],
                         "Other Person",
                         "I got stuck in traffic.",
                         speed,
                         &game->text[32]);
    atari_text_list_init(&game->text[32],
                         "Guy",
                         "Ohhhh myyy godd\nDan and Melissa are\ntogether now!",
                         speed,
                         &game->text[33]);
    atari_text_list_init(&game->text[33],
                         "Girl",
                         "John is such an ass,\nisn't he?\nI heard he's cheating with Jackie.",
                         speed,
                         &game->text[34]);
    atari_text_list_init(&game->text[34],
                         "Some Guy",
                         "Let's go out for drinks later.\nthere's a match going on",
                         speed,
                         &game->text[35]);
    atari_text_list_init(&game->text[35],
                         "Person #2",
                         "So who are you guys\nlistening to right now?",
                         speed,
                         &game->text[36]);
    atari_text_list_init(&game->text[36],
                         "Person #9",
                         "So this one time we're going out with",
                         speed,
                         &game->text[37]);
    atari_text_list_init(&game->text[37],
                         "Girl",
                         "My commute to work is\nso longgggg!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
                         speed,
                         &game->text[58]);
    // here
    atari_text_list_init(&game->text[58],
                         "Sherane",
                         "It's so hot outside!!!!!!!!!!!!!!!!!!!\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
                         speed,
                         &game->text[38]);

    atari_text_list_init(&game->text[38],
                         "Chase",
                         "...",
                         speed,
                         &game->text[39]);
    atari_text_list_init(&game->text[39],
                         "Chase",
                         "Y'know...\rEvery other night I have\nthis recurring dream.",
                         speed,
                         &game->text[40]);
    atari_text_list_init(&game->text[40],
                         "Chase",
                         "I'm stuck in this prison\nwith no way out-",
                         speed,
                         &game->text[41]);
    atari_text_list_init(&game->text[41],
                         "Chase",
                         "It's a completely white\nvoid, surrounded by these\nlong, solid diamond bars.",
                         speed,
                         &game->text[42]);
    atari_text_list_init(&game->text[42],
                         "Chase",
                         "And I'm completely alone.",
                         speed,
                         &game->text[43]);
    atari_text_list_init(&game->text[43],
                         "Chase",
                         "There were these things\non the ground, like huge\ncapsules with liquid inside.",
                         speed,
                         &game->text[44]);
    atari_text_list_init(&game->text[44],
                         "Chase",
                         "I remember picking one,\nand there was writing\netched onto it;",
                         speed,
                         &game->text[45]);
    atari_text_list_init(&game->text[45],
                         "Chase",
                         "But I didn't read it-\rI was too busy trying\nto find a way out.",
                         speed,
                         &game->text[46]);
    atari_text_list_init(&game->text[46],
                         "Chase",
                         "So, I went up to the bars\nsurrounding the entire\nplace,",
                         speed,
                         &game->text[47]);
    atari_text_list_init(&game->text[47],
                         "Chase",
                         "and I started striking it\nwith my bare hands.",
                         speed,
                         &game->text[48]);
    atari_text_list_init(&game->text[48],
                         "Chase",
                         "The pain was\nunimaginable.",
                         speed,
                         &game->text[49]);
    atari_text_list_init(&game->text[49],
                         "Chase",
                         "After a thousand years,\rI finally made enough of\na dent so I could pass.",
                         speed,
                         &game->text[50]);
    atari_text_list_init(&game->text[50],
                         "Chase",
                         "But, as the tip of my feet\nbarely touched the ground\non the oustide,",
                         speed,
                         &game->text[51]);
    atari_text_list_init(&game->text[51],
                         "Chase",
                         "I dropped dead,\rcollapsing onto the\nground.",
                         speed,
                         &game->text[52]);
    atari_text_list_init(&game->text[52],
                         "Chase",
                         "Infinite darkness.\r...\rAnd then I wake up.",
                         speed,
                         &game->text[53]);
    atari_text_list_init(&game->text[53],
                         "Amy",
                         "... Sorry, what were you\nsaying, Chase?\rYou were mumbling.",
                         speed,
                         &game->text[54]);
    atari_text_list_init(&game->text[54],
                         "Chase",
                         "...",
                         speed,
                         &game->text[55]);
    atari_text_list_init(&game->text[55],
                         "Amy",
                         "Anyways, like I was\nsaying, I wonder if the\nmeeting is still on?",
                         speed,
                         &game->text[56]);
    atari_text_list_init(&game->text[56],
                         "Chase",
                         "...\r...\r...",
                         speed,
                         &game->text[57]);
    atari_text_list_init(&game->text[57],
                         "Chase",
                         "I need to get out\nof here.\rNow.",
                         speed,
                         nullptr);

    for (int i = 4; i <= 38; i++) {
        game->text[i].location = Top;
    }
    game->text[53].location = Top;
    game->text[55].location = Top;
    game->text[58].location = Top;

    atari_text_list_init(&game->text[59],
                         0,
                         "*ring ring*\r*ring ring*\r... *click*",
                         speed,
                         &game->text[60]);
    atari_text_list_init(&game->text[60],
                         0,
                         "*static*\r...\r...",
                         speed,
                         &game->text[61]);
    atari_text_list_init(&game->text[61],
                         0,
                         "He hears a faint voice\nin the speaker:",
                         speed,
                         &game->text[62]);
    atari_text_list_init(&game->text[62],
                         "Voice",
"\"The gods had condemned\nSisyphus to ceaselessly\nrolling a rock to the top",//of a mountain, whence",// the stone would fall back of its own weight. They had thought with some reason that there is no more dreadful punishment than futile and hopeless labor",
                         speed,
                         &game->text[63]);
    atari_text_list_init(&game->text[63],
                         "Voice",
                         "of a mountain, whence the\nstone would fall back of\nits own weight.",
                         speed,
                         &game->text[64]);
    atari_text_list_init(&game->text[64],
                         "Voice",
                         "They had thought with\nsome reason that there is\nno more dreadful",
                         speed,
                         &game->text[65]);
    atari_text_list_init(&game->text[65],
                         "Voice",
                         "punishment than futile\nand hopeless labour.\"",
                         speed,
                         &game->text[66]);
    atari_text_list_init(&game->text[66],
                         0,
                         "*static*\r...\r...",
                         speed,
                         &game->text[67]);
    atari_text_list_init(&game->text[67],
                         0,
                         "He hears a faint voice\nin the speaker:",
                         speed,
                         &game->text[68]);
    atari_text_list_init(&game->text[68],
                         "Voice",
                         "\"Sisyphus, proletarian of\nthe gods, powerless and\nrebellious,", 
                         speed,
                         &game->text[69]);
    atari_text_list_init(&game->text[69],
                         "Voice",
                         "knows the whole extent\nof his wretched condition:", 
                         speed,
                         &game->text[70]);
    atari_text_list_init(&game->text[70],
                         "Voice",
                         "it is what he thinks of\nduring his descent.\"", 
                         speed,
                         &game->text[71]);
    atari_text_list_init(&game->text[71],
                         0,
                         "*static*\r...\r...", 
                         speed,
                         &game->text[72]);
    atari_text_list_init(&game->text[72],
                         0,
                         "...\r...\r... That seems to be all.", 
                         speed,
                         nullptr);

    // If the descent is thus sometimes performed in sorrow, it can also take place in joy.
    // ...
    // These are our nights of Gethsemane. But crushing truths perish from
    // being acknowledged. Thus, Oedipus at the outset obeys fate without knowing it. But from the moment he
    // knows, his tragedy begins. Yet at the same moment, blind and desperate, he realizes that the only bond
    // linking him to the world is the cool hand of a girl.

    atari_mid_text_list_init(&game->text[73],
                             "\"If the descent is thus\nsometimes performed in\nsorrow, it can also take\nplace in joy.\"", 
                             &game->text[74]);
    atari_mid_text_list_init(&game->text[74],
                             "...", 
                             &game->text[75]);
    atari_mid_text_list_init(&game->text[75],
                             "\"These are our nights of\nGethsemane.\rBut crushing truths\nperish from being\nacknowledged.", 
                             &game->text[76]);
    atari_mid_text_list_init(&game->text[76],
                             "Thus, Oedipus at the\noutset obeys fate\nwithout knowing it.\rBut from the moment he\nknows, his tragedy\nbegins.", 
                             &game->text[77]);
    atari_mid_text_list_init(&game->text[77],
                             "Yet at the same moment,\rblind and desperate,\rhe realizes that the\nonly bond linking him\nto the world is the cool\nhand of a girl.\" [2]", 
                             nullptr);
    game->text[77].callbacks[0] = chapter_3_fade_out_outside_home_delay;

    atari_text_list_init(&game->text[80],
                         0,
                         "Apartment Complex 2B,\rA Corp.",
                         speed,
                         nullptr);

    level->player = chapter_3_make_entity(ENTITY_PLAYER, 66, 96);
    array_add(&game->entities, level->player);

    level->virtual_mouse.texture = 22;

    chapter_3_job_init(game, 0);

    //chapter_3_goto_job_minigame(game);

    //chapter_3_goto_lunch_room(game, CHAPTER_3_LUNCH_TEXT_3);
    //chapter_3_goto_window_text(game);
    //level->minigame.active = true;

    //chapter_3_init_outside(game);
    //chapter_3_goto_home_inside(game);
    //chapter_3_goto_home_outside(game);

    chapter_3_goto_road(game);
}

void job_minigame_run(Game *game, Chapter_3_Job_Minigame *minigame,
                      float dt, RenderTexture *current_render_target)
{
    assert(minigame->active);

    (void)dt;

    // Tick
    if (key_down_pressed()) minigame->scroll_y -= minigame->line_height;
    if (key_up_pressed())   minigame->scroll_y += minigame->line_height;

    Vector2 wheel = GetMouseWheelMoveV();
    int scroll_speed_px = minigame->line_height;
    wheel.y = Clamp(wheel.y, -1, 1);
    minigame->scroll_y += scroll_speed_px * wheel.y;

    minigame->scroll_y = min(minigame->scroll_y, 0);

    int bottom_pad = 3*minigame->line_height;
    int cap = -minigame->line_count * minigame->line_height + minigame->window_inner.height - bottom_pad;

    cap = min(cap, 0);

    if (minigame->scroll_y < cap) minigame->scroll_y = cap;
    
    // Draw
    Font *font = minigame->font;

    if (minigame->draw_background) {
        ClearBackground(BEIGE);
        DrawTexture(atari_assets.textures[0], 0, 0, WHITE);
    }

    uint8_t window_alpha = minigame->window_alpha;

    Color window_bg_color = minigame->window_bg_color;
    window_bg_color.a = window_alpha;

    Color border_color = minigame->font_color;
    border_color.a = window_alpha;

    DrawRectangleRec(minigame->window_outer, window_bg_color);
    DrawRectangleLinesEx(minigame->window_outer, minigame->window_border_thickness, border_color);
    DrawRectangleLinesEx(minigame->title_bar, 1, border_color);

    Texture *controls = &atari_assets.textures[1];
    DrawTexture(*controls, minigame->title_bar.x + minigame->title_bar.width - controls->width, minigame->title_bar.y, window_bg_color);
    if (minigame->draw_background) {
        Texture *window_title = &atari_assets.textures[2];
        DrawTexture(*window_title, minigame->title_bar.x, minigame->title_bar.y, window_bg_color);
    }

    // Draw document
    BeginTextureMode(minigame->render_target);

    ClearBackground({});

    String current_line = {};
    Vector2 default_size = MeasureTextEx(*font, "T", font->baseSize, minigame->font_spacing);
    size_t previous_word_index = 0;
    size_t line_start_index = 0;
    float previous_height = 0;

    minigame->line_height = default_size.y;

    int pad = 3;
    Vector2 pos = {
        pad,
        pad + (int)minigame->scroll_y
    };

    auto add_line = [&]() -> void {
        Vector2 size = default_size;

        if (current_line.text[0]) {
            size = MeasureTextEx(*font, current_line.text, font->baseSize, minigame->font_spacing);
            DrawTextEx(*font, current_line.text, pos, font->baseSize, minigame->font_spacing, minigame->font_color);
        }

        current_line = {};

        pos.y += size.y;

        minigame->line_count++;
    };

    minigame->line_count = 0;

    Document *document = &minigame->document_list[minigame->current_document];

    Word *word_on  = 0;

    auto set_word_up = [&]() {
        size_t length = word_on->end_index - word_on->start_index + 1;

        char word_before[128] = {};
        assert(word_on->start_index >= line_start_index);

        strncpy(word_before, current_line.text, word_on->start_index - line_start_index);

        char word[128] = {};

        assert(word_on->start_index >= line_start_index);
        strncpy(word, current_line.text + word_on->start_index - line_start_index, length);

        Vector2 before_size = MeasureTextEx(*font,
                                            word_before,
                                            font->baseSize,
                                            minigame->font_spacing);
        Vector2 word_size   = MeasureTextEx(*font,
                                            word,
                                            font->baseSize,
                                            minigame->font_spacing);

        word_on->rect.x      = pos.x + before_size.x + 1; 
        word_on->rect.y      = pos.y;
        word_on->rect.width  = word_size.x;
        word_on->rect.height = word_size.y;

        word_on = 0;
    };

    size_t string_length = strlen(document->string);

    for (size_t i = 0; i < string_length; i++) {
        char c = document->string[i];

        if (c != '\n')
            string_concatenate(&current_line, c);

        // Compare to words
        if (word_on == nullptr) {
            for (size_t j = 0; j < document->word_count; j++) {
                Word *word = &document->words[j];

                if (word->start_index == i) {
                    word_on = word;
                    if (word->start_index == word->end_index)
                        set_word_up();
                }
            }
        } else {
            if (i == word_on->end_index) {
                set_word_up();
            }
        }

        if (c == '\n' || c == ' ' || i == strlen(document->string)-1) {
            Vector2 size = MeasureTextEx(*font, current_line.text, font->baseSize, minigame->font_spacing);

            previous_height = size.y;

            if (size.x > minigame->window_inner.width - 2) {
                if (line_start_index >= previous_word_index) break;

                // Go back to the previous word and chop it off there.
                current_line.length = previous_word_index - line_start_index;
                current_line.text[current_line.length] = 0;
                i = previous_word_index;

                line_start_index = i+1;
                add_line();
            } else {
                previous_word_index = i;
            }
        }

        // We must not use c here because i may have changed.
        if (document->string[i] == '\n') {
            line_start_index = i+1;
            add_line();
        }
    }

    if (current_line.length) {
        add_line();
    }

    Vector2 mouse;
    {
        Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;
        mouse = level->virtual_mouse.pos;
    }

    Rectangle visible_finish_button_rect = {
        minigame->window_inner.x + minigame->finish_button.x,
        minigame->window_inner.y + minigame->finish_button.y,
        minigame->finish_button.width,
        minigame->finish_button.height
    };

    for (int i = 0; i < document->word_count; i++) {
        Rectangle rect = document->words[i].rect;
        rect.x += minigame->window_inner.x;
        rect.y += minigame->window_inner.y;

        if (CheckCollisionPointRec(mouse, rect) && !CheckCollisionPointRec(mouse, visible_finish_button_rect)) {
            Color color;
            Word *word = &document->words[i];

            if (word->correction) {
                color = { 255, 0, 0, (uint8_t)(150 + 8 * sinf(8*GetTime())) };
            } else {
                color = { 255, 0, 0, 64  };
            }

            DrawRectangleRec(document->words[i].rect, color);

            if (is_action_pressed() && word->correction) {
                int correct_length = (int)strlen(word->correction);
                int incorrect_length = word->end_index - word->start_index + 1;
                int delta = correct_length - incorrect_length;

                int length = (int)strlen(document->string);

                // Push or remove chars based on the delta.
                if (delta < 0) {
                    for (int j = word->start_index; j < length; j++) {
                        document->string[j] = document->string[j-delta];
                    }
                } else if (delta > 0) {
                    for (int j = length-1; j >= word->start_index; j--) {
                        document->string[j+delta] = document->string[j];
                    }
                }

                // Shift the indices in the words from this
                // word onwards based on delta.
                if (delta != 0) {
                    for (size_t j = 0; j < document->word_count; j++) {
                        Word *word_2 = &document->words[j];

                        if (word_2->start_index > word->start_index) {
                            word_2->start_index += delta;
                            word_2->end_index   += delta;
                        }
                    }
                }

                word->end_index = word->start_index + correct_length - 1;

                strncpy(document->string + word->start_index, word->correction, correct_length);

                document->num_corrections++;

                word->correction = nullptr;
            }
        }
    }

    Color color = RED;
    bool can_finish = (document->num_corrections == document->error_count);

    float percentage;

    if (document->error_count)
        percentage = (float)document->num_corrections / document->error_count;
    else
        percentage = 1;

    Rectangle finish_button_progress = {
        minigame->finish_button.x,
        minigame->finish_button.y,
        minigame->finish_button.width * percentage,
        minigame->finish_button.height
    };

    bool complete_lot = (minigame->current_document == minigame->document_count - 1);

    auto click_next = [&]() -> void {
        if (complete_lot) {
            Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;
            int new_text = level->current_lunch_text + 1;

            if (new_text <= CHAPTER_3_LUNCH_TEXT_3) {
                minigame->active = false;

                chapter_3_goto_lunch_room(game, (Chapter_3_Lunch_Text) new_text);
            } else if (game->fader.direction == FADE_INVALID) {
                start_fade(game, FADE_OUT, chapter_3_goto_road);
            }
        } else {
            chapter_3_next_document(minigame);
        }
    };

    if (can_finish && CheckCollisionPointRec(mouse, visible_finish_button_rect)) {
        color = BLACK;
        if (is_action_pressed()) {
            click_next();
        }
    }
    
#ifdef DEBUG
    if (IsKeyPressed(KEY_PAGE_DOWN)) {
        click_next();
    }
#endif

    DrawRectangleRec(minigame->finish_button, color);
    DrawRectangleRec(finish_button_progress, GREEN);

    if (CheckCollisionPointRec(mouse, visible_finish_button_rect)) {
        DrawRectangleRec(minigame->finish_button, {0,0,0,64});
    }

    const char *text = complete_lot ? "Finish" : "Next";

    Vector2 size = MeasureTextEx(*font, text, font->baseSize, 0);
    DrawTextEx(*minigame->font, text, {minigame->finish_button.x + minigame->finish_button.width/2 - size.x/2, minigame->finish_button.y + minigame->finish_button.height/2 - size.y/2}, font->baseSize, 0, WHITE);

    EndTextureMode();
    BeginTextureMode(*current_render_target);

    int width = minigame->window_inner.width;
    int height = minigame->window_inner.height;

    DrawTexturePro(minigame->render_target.texture,
                   {0, 0, (float)width, -(float)height},
                   minigame->window_inner,
                   {0, 0},
                   0,
                   WHITE);

    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;
    Texture *t = &atari_assets.textures[level->virtual_mouse.texture];
    DrawTextureV(*t, int_vec2(level->virtual_mouse.pos), WHITE);
}

// tick
void chapter_3_entity_update(Entity *entity, Game *game, float dt) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;

    bool open_dialogue = can_open_dialogue(game, entity, level->player);

    switch (entity->type) {
        case ENTITY_PLAYER: {
            // player tick player update player

            if (game->current)
                return;

            int dir_x = input_movement_x_axis_int(dt);//key_right() - key_left();
            int dir_y = input_movement_y_axis_int(dt);//key_down()  - key_up();

            float player_speed = 60;

            if (level->state == CHAPTER_3_STATE_HOME)
                player_speed = 30;

#ifdef DEBUG
            if (IsKeyDown(KEY_P))
                player_speed *= 5;
#endif

            if (level->state == CHAPTER_3_STATE_HOME ||
                level->state == CHAPTER_3_STATE_HOME_INSIDE)
            {
                Vector2 velocity = { dir_x * player_speed * dt, dir_y * player_speed * dt };
                apply_velocity(entity, velocity, &game->entities);
                
                entity->pos.x = Clamp(entity->pos.x, 0, render_width - entity_texture_width(entity));

                if (level->state == CHAPTER_3_STATE_HOME_INSIDE) {
                    entity->pos.y = Clamp(entity->pos.y, 0, render_height - entity_texture_height(entity));
                } else {
                    int max = atari_assets.textures[level->home_background].height - entity_texture_height(entity);
                    if (entity->pos.y > max) {
                        entity->pos.y = max;
                    }
                }
                break;
            }

            if (game->current != nullptr || level->text_start_timer > 0)
                dir_x = dir_y = 0;

            Vector2 velocity = { dir_x * player_speed * dt, dir_y * player_speed * dt };

            Vector2 before_pos = entity->pos;
            apply_velocity(entity, velocity, &game->entities);


            if (level->state == CHAPTER_3_STATE_LUNCH && level->current_lunch_text == CHAPTER_3_LUNCH_TEXT_3) {
                level->lunch_devil_effect = dir_x || dir_y;

                Rectangle player_rectangle = {
                    entity->pos.x + entity->base_collider.x,
                    entity->pos.y + entity->base_collider.y,
                    entity->base_collider.width,
                    entity->base_collider.height
                };

                level->end_steps += Vector2Distance(entity->pos, before_pos);

                if (level->end_steps >= 1000 && !level->black_screen) {
                    add_event(game, chapter_3_goto_window_text, 3);
                    level->black_screen = true;
                }

                /*
                level->window_popup = !level->lunch_devil_effect && CheckCollisionRecs(player_rectangle, {0, 0, 192, 20});
                if (level->window_popup && is_action_pressed()) {
                    level->state = CHAPTER_3_STATE_WINDOW;
                    chapter_3_job_init(game, 3);
                    level->minigame.active = true;
                }
                */
            }

            int width = entity_texture_width(entity);
            int height = entity_texture_height(entity);

            Direction scrolled_dir = DIRECTION_INVALID;

            if (entity->pos.x + width < 0) {
                entity->pos.x = render_width;
                scrolled_dir = DIRECTION_LEFT;
            }
            if (entity->pos.x > render_width) {
                entity->pos.x = -width;
                scrolled_dir = DIRECTION_RIGHT;
            }
            if (entity->pos.y + height < 0) {
                entity->pos.y = render_height;
                scrolled_dir = DIRECTION_UP;
            }
            if (entity->pos.y > render_height) {
                entity->pos.y = -height;
                scrolled_dir = DIRECTION_DOWN;
            }

            bool can_scroll = true;

            if (level->state == CHAPTER_3_STATE_LUNCH && level->current_lunch_text == CHAPTER_3_LUNCH_TEXT_2 && scrolled_dir != DIRECTION_INVALID) {
                chapter_3_job_init(game, 2);
                //level->minigame.active = true;
                chapter_3_goto_job_minigame(game);
                level->state = CHAPTER_3_STATE_OFFICE;
            } else if (level->state == CHAPTER_3_STATE_LUNCH && level->current_lunch_text == CHAPTER_3_LUNCH_TEXT_3 && scrolled_dir != DIRECTION_INVALID) {
                // Remove the tables and re-add them with slight random variation,
                Array<Entity*> *entities = &game->entities;

                for (int i = 0; i < entities->length; i++) {
                    Entity *e = entities->data[i];

                    if (e->type == ENTITY_CHAP_3_LUNCH_TABLE ||
                        e->type == ENTITY_CHAP_3_GUY ||
                        e->type == ENTITY_CHAP_3_GIRL)
                    {
                        free_entity(e);
                        array_remove(entities, i--);
                    }
                }

                int table_1_count = 3 + rand_range(-2, 3);
                int table_2_count = 3 + rand_range(-2, 3);

                add_lunch_table(entities,  35 + rand_range(-5, 5),  35 + rand_range(-5, 5), table_1_count);
                add_lunch_table(entities, 100 + rand_range(-5, 5), 100 + rand_range(-5, 5), table_2_count);
            } else if (level->screens_scrolled <= 8 && scrolled_dir != DIRECTION_UP) {
                can_scroll = false;
            } else if (level->state == CHAPTER_3_STATE_OFFICE && scrolled_dir != DIRECTION_INVALID && can_scroll) {
                level->screens_scrolled++;

                // Remove the cubicles and re-add them.
                for (int i = 0; i < game->entities.length; i++) {
                    Entity *e = game->entities.data[i];
                    Entity_Type type = e->type;

                    if (type == ENTITY_CHAP_3_CUBICLE_VERTICAL ||
                        type == ENTITY_CHAP_3_CUBICLE_TOP ||
                        type == ENTITY_CHAP_3_CUBICLE_CHAIR ||
                        type == ENTITY_WALL || 
                        type == ENTITY_CHAP_3_GUY ||
                        type == ENTITY_CHAP_3_GIRL)
                    {
                        free_entity(e);
                        array_remove(&game->entities, i--);
                    }
                }

                if (level->screens_scrolled <= 8 && level->screens_scrolled % 2 == 0) {
                    chapter_3_init_outside(game);
                }

                if (level->screens_scrolled > 8) {
                    add_cubicle(&game->entities, rand_bool(0.9),  10+rand_range(-3,3), 10+rand_range(-3,3));
                    add_cubicle(&game->entities, rand_bool(0.9),  10+rand_range(-3,3), 90+rand_range(-3,3));

                    add_cubicle(&game->entities, rand_bool(0.1), 140+rand_range(-3,3), 10+rand_range(-3,3));
                    add_cubicle(&game->entities, rand_bool(0.1), 140+rand_range(-3,3), 90+rand_range(-3,3));
                }

                if (level->screens_scrolled >= 12) {
                    int which = rand()%4;

                    int current = 0;

                    for (size_t i = 0; i < game->entities.length; i++) {
                        Entity *e = game->entities.data[i];

                        if (e->type == ENTITY_CHAP_3_CUBICLE_VERTICAL) {
                            if (current == which) {
                                e->chap_3_cubicle.number = 1138;

                                Entity *cubicle_top      = e->chap_3_cubicle.cubicle_top;
                                Entity *cubicle_vertical = e->chap_3_cubicle.cubicle_vertical;
                                Entity *cubicle_chair    = e->chap_3_cubicle.cubicle_chair;

                                cubicle_top     ->chap_3_cubicle.number = 1138;
                                cubicle_vertical->chap_3_cubicle.number = 1138;
                                cubicle_chair   ->chap_3_cubicle.number = 1138;
                            }

                            current++;
                        }
                    }
                }
            }
        } break;
        case ENTITY_PHONE: {
            if (is_action_pressed() && open_dialogue) {
                game->current = &game->text[59];
                entity->has_dialogue = false;
            }
        } break;
        case ENTITY_CHAP_3_CUBICLE_CHAIR: {
            int number = entity->chap_3_cubicle.number;

            if (is_action_pressed() && open_dialogue) {
                if (number == 666) {
                    game->current = &game->text[3];
                } else if (number == 1138) {
                    game->current = &game->text[1];
                } else {
                    game->current = &game->text[0];
                }
            }
        } break;
        case ENTITY_CHAP_3_GUY: case ENTITY_CHAP_3_GIRL: {
            if (level->state == CHAPTER_3_STATE_LUNCH && level->lunch_devil_effect) {
                Chapter_3_Circler *data = &entity->chap_3_circler;

                entity->pos.x = data->center_position.x + data->radius_x * cosf(data->current_angle);
                entity->pos.y = data->center_position.y + data->radius_y * sinf(data->current_angle);

                data->current_angle -= 1 * PI * dt;
            } else {
                if (is_action_pressed() && open_dialogue) {
                    game->current = &game->text[2];

                    const char *male_speakers[]   = { "Jake", "Mike", "Rory", "Mickey" };
                    const char *female_speakers[] = { "Amy", "Joanne", "Clarice", "Melody" };

                    char *speaker = game->current->speaker.text;

                    if (entity->type == ENTITY_CHAP_3_GUY) {
                        int index = rand() % StaticArraySize(male_speakers);

                        // Mike is dead
                        if (level->current_lunch_text == CHAPTER_3_LUNCH_TEXT_2 && index == 1)
                            index--;

                        strcpy(speaker, male_speakers[index]);
                    } else {
                        int index = rand() % StaticArraySize(female_speakers);
                        strcpy(speaker, female_speakers[index]);
                    }

                    game->current->speaker.length = strlen(speaker);
                }
            }
        } break;
    }
}

void chapter_3_entity_draw(Entity *entity, Game *game) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;

    switch (entity->type) {
        case ENTITY_CHAP_3_CUBICLE_VERTICAL: {
            default_entity_draw(entity);

            Color text_color = WHITE;

            if (entity->chap_3_cubicle.number == 1138) {
                text_color = GOLD;
            }
            if (entity->chap_3_cubicle.number == 666) {
                text_color = RED;
            }

            Font *font = &atari_font;
            char numbers[12] = {};
            sprintf(numbers, "%d", entity->chap_3_cubicle.number);

            size_t length = strlen(numbers);

            float char_height = font->baseSize - 3;

            Vector2 pos = {};
            pos.y = entity->pos.y + 36 - length * char_height / 2;

            for (size_t i = 0; i < length; i++) {
                char ch = numbers[i];

                const char text[] = {ch, 0};

                Vector2 size = MeasureTextEx(*font, text, font->baseSize, 0);

                pos.x = entity->pos.x + 7 - size.x/2;

                Vector2 integer_pos = { (int)pos.x, (int)pos.y };
                DrawTextEx(*font, text, integer_pos, font->baseSize, 0, text_color);

                pos.y += char_height;
            }

        } break;
        case ENTITY_CHAP_3_GUY: {
            if (level->lunch_devil_effect) {
                entity->texture_id = 13;
            } else {
                entity->texture_id = 8;
            }
            default_entity_draw(entity);
        } break;
        case ENTITY_CHAP_3_GIRL: {
            if (level->lunch_devil_effect) {
                entity->texture_id = 14;
            } else {
                entity->texture_id = 9;
            }
            default_entity_draw(entity);
        } break;
        case ENTITY_CHAP_3_LUNCH_TABLE: {
            if (level->lunch_devil_effect) {
                entity->texture_id = 12;
            } else {
                entity->texture_id = 10;
            }
            default_entity_draw(entity);
        } break;
        case ENTITY_PLAYER: {
            if (level->lunch_devil_effect) {
                entity->texture_id = 15;
            } else {
                entity->texture_id = 3;
            }
            default_entity_draw(entity);
        } break;
        default: {
            default_entity_draw(entity);
        } break;
    }
}

void chapter_3_deinit(Game *game) {
    (void)game;
}

void chapter_3_update(Game *game, float dt) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;

    if (level->state == CHAPTER_3_STATE_HOME) {
        //int player_x = level->player->pos.x + entity_texture_width(level->player) / 2;
        int player_y = level->player->pos.y + entity_texture_height(level->player) / 2;

        int screen_y = player_y / render_height;

        level->camera.offset.x = 0;
        level->camera.offset.y = -screen_y * render_height;

        if (level->player->pos.y + entity_texture_height(level->player)/2 < render_height) {
            game->post_processing.crt.do_scanline_effect = 0;
            game->post_processing.crt.do_warp_effect = 0;
        } else {
            game->post_processing.crt.do_scanline_effect = 1;
            game->post_processing.crt.do_warp_effect = 1;
        }

        static bool started_event = false;
        if (!started_event && level->player->pos.y + entity_texture_height(level->player) < 0) {
            add_event(game, chapter_3_set_apartment_end_text, 5);
            started_event = true;

            for (int i = 0; i < game->entities.length; i++) {
                free_entity(game->entities.data[i]);
                array_remove(&game->entities, i--);
            }
        }

#ifdef DEBUG
        if (IsKeyPressed(KEY_EQUAL))
            game->current = game->text + 73;
#endif
    }

    {
        level->virtual_mouse.pos = Vector2Add(level->virtual_mouse.pos, get_mouse_delta());

        const float sensitivity = 4;

        float cont_x = sensitivity * 60 * dt * GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        float cont_y = sensitivity * 60 * dt * GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

        Vector2 look = input_movement_cont_look_axis(dt);

        level->minigame.scroll_y -= sensitivity * look.y;

        float len = sqrtf(cont_x * cont_x + cont_y * cont_y);
        if (len > sensitivity) {
            cont_x = sensitivity * cont_x / len;
            cont_y = sensitivity * cont_y / len;
        }

        Virtual_Mouse *mouse = &level->virtual_mouse;

        mouse->pos.x += cont_x;
        mouse->pos.y += cont_y;

        mouse->pos.x = Clamp(mouse->pos.x, 0, render_width-1);
        mouse->pos.y = Clamp(mouse->pos.y, 0, render_height-1);
    }

    if (level->state == CHAPTER_3_STATE_LUNCH) {
        if (level->current_lunch_text == CHAPTER_3_LUNCH_TEXT_1 && game->current) {
            if (!game->current->next[0]) {
                if (is_text_list_at_end(game->current)) {
                    level->state = CHAPTER_3_STATE_OFFICE;
                    game->current = 0;

                    chapter_3_job_init(game, (int)level->current_lunch_text);
                    chapter_3_goto_job_minigame(game);
                    //level->minigame.active = true;
                }
            }
        }

        if (level->text_start_timer > 0) {
            level->text_start_timer -= dt;
            if (level->text_start_timer < 0) {
                level->text_start_timer = 0;
            }
        }

        if (level->text_start_timer == 0) {
            switch (level->current_lunch_text) {
                case CHAPTER_3_LUNCH_TEXT_1: {
                    game->current = &game->text[4];
                } break;
                case CHAPTER_3_LUNCH_TEXT_2: {
                    game->current = &game->text[15];
                } break;
                case CHAPTER_3_LUNCH_TEXT_3: {
                    game->current = &game->text[30];
                } break;
            }

            level->text_start_timer = -1;
        }
    }

    if (level->state != CHAPTER_3_STATE_WINDOW) {
        if (!level->minigame.active) {
            size_t entity_count = game->entities.length;

            for (size_t i = 0; i < entity_count; i++) {
                Entity *e = game->entities.data[i];
                chapter_3_entity_update(e, game, dt);
            }
        }
    }
}

void chapter_3_draw(Game *game, float dt) {
    game->textbox_alpha = 225;

    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;
    Chapter_3_Job_Minigame *minigame = &level->minigame;

    if (level->state == CHAPTER_3_STATE_LUNCH) {
        if (level->lunch_devil_effect) {
            game->post_processing.type = POST_PROCESSING_VHS;
            post_process_vhs_set_intensity(&game->post_processing.vhs, VHS_INTENSITY_MEDIUM_HIGH);
            game->post_processing.vhs.vignette_mix = 1;
        } else {
            game->post_processing.type = POST_PROCESSING_CRT;
        }
    }

    switch (level->state) {
        case CHAPTER_3_STATE_OFFICE: {
            if (minigame->active) {
                float t = GetTime() * 1;
                game->post_processing.crt.abberation_intensity = 0.5 + 0.5f * sin(t) * sin(t);

                job_minigame_run(game, minigame, dt, &game->atari_render_target);
            } else {
                if (level->screens_scrolled <= 8) {
                    Color c = {0, 0, 0, 255};
                    Color pink = { 0xFF, 0x80, 0xFF, 0xFF };

                    double interp[] = { 185./255., 70./255., 34./255., 8./255. };
                    int index = (level->screens_scrolled - 1) / 2;

                    c = lerp_color(c, pink, interp[index]);

                    ClearBackground(c);
                } else {
                    ClearBackground(GRAY);
                }

                if (level->screens_scrolled <= 8 &&
                    level->screens_scrolled % 2 == 0)
                {
                    DrawTexture(atari_assets.textures[7], 0, 0, WHITE);
                }

                BeginMode2D(level->camera);
                sort_entities(&game->entities);

                for (int i = 0; i < game->entities.length; i++) {
                    chapter_3_entity_draw(game->entities.data[i], game);
                }

                EndMode2D();
            }
        } break;
        case CHAPTER_3_STATE_LUNCH: {
            if (level->black_screen) {
                ClearBackground(BLACK);
                break;
            }

            if (level->lunch_devil_effect) {
                ClearBackground(BLACK);
            } else {
                ClearBackground(GRAY);

                DrawTexture(atari_assets.textures[11], 0, 0, WHITE);
            }

            BeginMode2D(level->camera);
            sort_entities(&game->entities);

            for (int i = 0; i < game->entities.length; i++) {
                chapter_3_entity_draw(game->entities.data[i], game);
            }

            /*
            if (level->window_popup) {
                draw_popup("Stare outside.");
            }
            */

            EndMode2D();
        } break;
        case CHAPTER_3_STATE_WINDOW: {
            ClearBackground(BLACK);

            game->textbox_alpha = 255;

            DrawTexture(atari_assets.textures[16], 0, 0, WHITE);
            job_minigame_run(game, minigame, dt, &game->atari_render_target);
        } break;
        case CHAPTER_3_STATE_ROAD: {
            ClearBackground(BLACK);
            game->textbox_alpha = 255;

            level->road_time += dt;

            if (level->road_time > 15) {
                level->car.driving_off = true;

                if (level->car.position.x > render_width + 25) {
                    if (game->fader.direction == FADE_INVALID) {
                        start_fade(game, FADE_OUT, chapter_3_goto_home_outside);
                    }
                }
            }

            if (level->car.driving_off) {
                level->car.position.x += 30 * dt;
            } else if (level->car.has_control) {
                if (key_right()) {
                    level->car.position.x += dt * 30;
                }
                if (key_left()) {
                    level->car.position.x -= dt * 30;
                }

                if (key_down()) {
                    level->car.position.y += dt * 30;
                }
                if (key_up()) {
                    level->car.position.y -= dt * 30;
                }

                if (level->car.position.x < 20)
                    level->car.position.x = 20;
                if (level->car.position.x > render_width - 80)
                    level->car.position.x = render_width - 80;
            } else {
                level->car.position.x += 30 * dt;
                if (level->car.position.x >= render_width/2 - 33)
                    level->car.has_control = true;
            }

            level->other_car.position.x -= 400 * dt;
            if (level->other_car.position.x < -100) {
                level->other_car.position.x = render_width+300 + rand()%200;
                level->other_car.color = { (uint8_t)(rand()%255), (uint8_t)(rand()%255), (uint8_t)(rand()%255), (uint8_t)255 };
            }

            if (level->car.position.y < 94)
                level->car.position.y = 94;
            if (level->car.position.y > 123 - atari_assets.textures[level->car.texture].height)
                level->car.position.y = 123 - atari_assets.textures[level->car.texture].height;

            static float offset = 0;

            offset -= 240 * dt;

            int num_lines = 5;
            int line_width = render_width/num_lines;

            /*
            if (level->road_time >= 3) {
            }
            */

            DrawTexture(atari_assets.textures[17], 0, 0, WHITE);

            int num_trees = 3;

            for (int i = 0; i < num_trees; i++) {
                int diff = 100;
                int x = offset + i * diff;

                while (x + diff <= 0) {
                    x += render_width + diff;
                }

                DrawTexture(atari_assets.textures[24],
                            x,
                            10 + sin(i) * 2,
                            WHITE);
            }

            for (int i = 0; i < num_lines; i++) {
                int x = offset + i * (line_width*2);

                while (x + line_width <= 0)
                    x += render_width + line_width;

                DrawLine(x,
                         100,
                         x + line_width,
                         100,
                         WHITE);
            }

            // draw the other car
            int width = atari_assets.textures[level->car.texture].width;
            int height = atari_assets.textures[level->car.texture].height;

            DrawTextureRec(atari_assets.textures[level->other_car.texture],
                           { 0, 0, (float)-width, (float)height },
                           {level->other_car.position.x, level->other_car.position.y},
                           level->other_car.color);

            // draw the car
            DrawTexture(atari_assets.textures[level->car.texture],
                        level->car.position.x,
                        level->car.position.y,
                        level->car.color);


            for (int i = 0; i < num_trees; i++) {
                int diff = 70;
                int x = offset + i * diff;

                while (x + diff <= 0)
                    x += render_width + diff;

                DrawTexture(atari_assets.textures[24],
                            x,
                            110 + sin(i) * 10,
                            WHITE);
            }
        } break;
        case CHAPTER_3_STATE_HOME: {
            Rectangle player = level->player->base_collider;
            player.x += level->player->pos.x;
            player.y += level->player->pos.y;

            level->read_sign_popup = CheckCollisionRecs(player, {0, 679, 62, 25});

            if (level->read_sign_popup && is_action_pressed()) {
                game->current = game->text + 80;
            }

            ClearBackground(BLACK);
            BeginMode2D(level->camera);

            DrawTexture(atari_assets.textures[level->home_background], 0, 0, WHITE);

            sort_entities(&game->entities);
            for (int i = 0; i < game->entities.length; i++) {
                chapter_3_entity_draw(game->entities.data[i], game);
            }

            EndMode2D();

            game->textbox_alpha = 255;
        } break;
        case CHAPTER_3_STATE_HOME_INSIDE: {
            ClearBackground(BLACK);
            DrawTexture(atari_assets.textures[19], 0, 0, WHITE);

            Rectangle player = level->player->base_collider;
            player.x += level->player->pos.x;
            player.y += level->player->pos.y;

            Rectangle bed_interact = { 84, 40, 12, 31 };

            level->bed_popup = CheckCollisionRecs(player, bed_interact);
            if (is_action_pressed() && level->bed_popup) {
                level->state = CHAPTER_3_STATE_TIKTOK;
            }

            for (int i = 0; i < game->entities.length; i++) {
                chapter_3_entity_draw(game->entities.data[i], game);
            }

            if (level->bed_popup) {
                draw_popup("Go to bed.");
            }
        } break;
        case CHAPTER_3_STATE_TIKTOK: {
            ClearBackground(BLACK);

            Chapter_3_Phone *phone = &level->phone;

            if (phone->black_timer > 0) {
                phone->black_timer -= dt;
                if (phone->black_timer <= 0) {
                    atari_queue_deinit_and_goto_intro(game);
                }
            } else {
                if (phone->c1.a == 0) {
                    phone->c1 = {(uint8_t)rand(), (uint8_t)rand(), (uint8_t)rand(), 255};
                    phone->c2 = {(uint8_t)rand(), (uint8_t)rand(), (uint8_t)rand(), 255};
                }

                Rectangle phone_screen = {65, 30 - phone->scroll_y, 59, 99};
                Rectangle phone_screen_down = phone_screen;
                phone_screen_down.y += phone_screen.height;

                DrawRectangleRec(phone_screen, phone->c1);
                DrawRectangleRec(phone_screen_down, phone->c2);

                if (key_down_pressed()) {
                    phone->scroll_y_to = phone_screen.height;
                }

                // TODO: Change this to framerate indepdendent lerp
                phone->scroll_y = Lerp(phone->scroll_y, phone->scroll_y_to, 0.2f);

                float epsilon = 1;
                if (phone->scroll_y != phone->scroll_y_to && fabs(phone->scroll_y - phone->scroll_y_to) < epsilon) {
                    phone->scroll_y_to = 0;
                    phone->scroll_y    = 0;

                    phone->c1 = phone->c2;
                    phone->c2 = {(uint8_t)rand(), (uint8_t)rand(), (uint8_t)rand(), 255};
                    phone->fade_alpha += 10;

                    if (phone->fade_alpha >= 240) {
                        phone->black_timer = 4;
                        phone->fade_alpha = 255;
                    }
                }

                DrawTexture(atari_assets.textures[20], 0, 0, WHITE);

                bool white = sin(GetTime()*10) > 0;
                DrawTexture(atari_assets.textures[21], 85, 101, white ? WHITE : BLACK);

                DrawRectangleRec({0,0,(float)render_width,(float)render_height}, {0,0,0,(uint8_t)phone->fade_alpha});
            }
        } break;
    }
}

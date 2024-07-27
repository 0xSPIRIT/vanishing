#define MAX_DOCUMENTS 16
#define MAX_CORRECTIONS 256

enum Chapter_3_State {
    CHAPTER_3_STATE_OFFICE,
    CHAPTER_3_STATE_LUNCH,
    CHAPTER_3_STATE_ROAD,
    CHAPTER_3_STATE_HOME
};

enum Chapter_3_Lunch_Text {
    CHAPTER_3_LUNCH_TEXT_INVALID,
    CHAPTER_3_LUNCH_TEXT_1,
    CHAPTER_3_LUNCH_TEXT_2,
    CHAPTER_3_LUNCH_TEXT_3,
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

struct Level_Chapter_3 {
    Chapter_3_State state;
    Chapter_3_Job_Minigame minigame;
    Camera2D camera;

    Chapter_3_Lunch_Text current_lunch_text;

    bool lunch_devil_effect;

    Entity *player;

    int screens_scrolled;
};

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

Chapter_3_Job_Minigame chapter_3_make_job_minigame(Arena *level_arena,
                                                   const char **document_list,
                                                   size_t document_count)
{
    Chapter_3_Job_Minigame result = {};

    result.active = false;

    result.document_count = document_count;
    result.current_document = 0;//(int)document_count - 1;

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

    result.window_inner = result.window_outer;

    result.window_inner.x      += result.window_border_thickness;
    result.window_inner.y      += result.window_title_bar_size;
    result.window_inner.width  -= result.window_border_thickness * 2;
    result.window_inner.height -= result.window_title_bar_size + result.window_border_thickness;

    result.title_bar        = result.window_outer;
    result.title_bar.height = result.window_title_bar_size;

    result.finish_button.width  = 40;
    result.finish_button.height = 14;

    result.font = &atari_font;

    result.render_target = LoadRenderTexture(result.window_inner.width,
                                             result.window_inner.height);

    // Tokenize

    for (int doc = 0; doc < result.document_count; doc++) {
        Document *document = &result.document_list[doc];

        Word word = {};

        int string_length = (int)strlen(document->string);

        for (int i = 0; i < string_length; i++) {
            if (i+1 >= string_length-1 || (isspace(document->string[i+1]) && !isspace(document->string[i]))) {
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
    Game_Atari *game = (Game_Atari *)game_ptr;
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;
    level->minigame.active = true;
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

void chapter_3_goto_lunch_room(Game_Atari *game, Chapter_3_Lunch_Text lunch_text) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;

    level->current_lunch_text = lunch_text;

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

    add_lunch_table(entities, 35, 35, 8);
    add_lunch_table(entities, 100, 100, 4);

    add_wall(entities, {0, 0, 192, 11});

    level->player->pos.x = 129;
    level->player->pos.y = 88;

    game->current = &game->text[4];
}

void chapter_3_init_outside(Game_Atari *game) {
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

void chapter_3_job_init(Game_Atari *game, int which_document_list) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;

    const char *documents[MAX_DOCUMENTS] = {};

    int count = 0;

    // The documents should start as:
    //   0: regular shmegular boring stuff, contracts,
    //      legal documents, and internal notices.
    //   1: introduce the animal farm and other quotes.
    //   2: do more stuff with completely changing words
    //      based on Chase's subconscious. "They must be
    //      dealing with the devil to have happiness like
    //      that"

    switch (which_document_list) {
        case 0: {
            documents[count++] =
                "Introduction\n----------------\n\nUse UP and DOWN to scroll.\n\nThese are excerpts from documents throughout our company. Read the document thoroughly and click on any errors to correct them- we mustn't look unprofessional!";

            documents[count++] =
                "Article 5: Termination\n\
\n\
5.1 Termination for Cause. Either party may terminate this Agreement immediately upon written notice to the other party in the event that:\n\n\
(a) The other party materially breaches any of its obligations under this Agreement and fails to cure such breach within thirty (30) days after receipt of written notice of such breach from the non-breaching party;";

            level->minigame = chapter_3_make_job_minigame(&game->level_arena, documents, count);
        } break;
        case 1: {
        } break;
        case 2: {
        } break;
    }

    /*
    switch (which_document_list) {
        case 0: {
            documents[count++] =
                "Introduction\n----------------\n\nUse UP and DOWN to scroll.\n\nThese are excerpts from documents throughout our company. Read the document thoroughly and click on any errors to correct them- we mustn't look unprofessional!";
            documents[count++] =
                "We're thrilled to announce oru new product: the thing-o-meter! It measures thigns nda whatnot.";
            documents[count++] = 
                "Here's our messagr fro the day: Appreciatw the beauty of the wrold! Be it a sunset, a walk in the park, or a conevrsation with friends. We muts thakn the Lord thta ew're aliev!";
            documents[count++] =
                "It wsa a bright cold day in April, and the clocks were strkiing thirteen. Winston Smith, his chin nussled into his brest in an efort to escape the vile wind, slipped quick through the glass doors of Victory Mansions, though not quickly enouhg to prevent a swril of gritty dust from entering long with him.";
            documents[count++] =
                "The cretures outside loked from pig to man, and form man to pig, and from pig to man again; but aready it was imposibble to say wich was wich.";
            documents[count++] = 
                "This work was stirctly volontery, but any animal who absinted himself from it wood have his rashons reduced by haf.";

            level->minigame = chapter_3_make_job_minigame(&level->arena, documents, count);

            Document *d1 = &level->minigame.document_list[1];

            document_register_error(d1, 27, 29, "our");
            document_register_error(d1, 75, 80, "things");
            document_register_error(d1, 82, 84, "and");

            Document *d2 = &level->minigame.document_list[2];

            document_register_error(d2, 11, 17, "message");
            document_register_error(d2, 19, 21, "for");
            document_register_error(d2, 32, 41, "Appreciate");
            document_register_error(d2, 61, 66, "world!");
            document_register_error(d2, 109, 120, "conversation");
            document_register_error(d2, 139, 142, "must");
            document_register_error(d2, 144, 148, "thank");
            document_register_error(d2, 159, 162, "that");
            document_register_error(d2, 164, 168, "we're");
            document_register_error(d2, 170, 174, "alive");

            Document *d3 = &level->minigame.document_list[3];

            document_register_error(d3, 3, 5, "was");
            document_register_error(d3, 55, 62, "striking");
            document_register_error(d3, 98, 104, "nuzzled");
            document_register_error(d3, 115, 119, "breast");
            document_register_error(d3, 127, 131, "effort");
            document_register_error(d3, 166, 170, "quickly");
            document_register_error(d3, 236, 241, "enough");
            document_register_error(d3, 256, 260, "swirl");
            document_register_error(d3, 291, 294, "along");

            Document *d4 = &level->minigame.document_list[4];

            document_register_error(d4, 4, 11, "creatures");
            document_register_error(d4, 21, 25, "looked");
            document_register_error(d4, 48, 51, "from");
            document_register_error(d4, 96, 101, "already");
            document_register_error(d4, 110, 119, "impossible");
            document_register_error(d4, 128, 131, "which");
            document_register_error(d4, 137, 140, "which");

            Document *d5 = &level->minigame.document_list[5];

            document_register_error(d5, 14, 21, "strictly");
            document_register_error(d5, 23, 31, "voluntary");
            document_register_error(d5, 53, 60, "absented");
            document_register_error(d5, 78, 81, "would");
            document_register_error(d5, 92, 98, "rations");
            document_register_error(d5, 111, 113, "half");
        } break;
        case 1: {
            documents[count++] = "";

            level->minigame = chapter_3_make_job_minigame(&level->arena, documents, count);
        } break;
    }
*/
}

void chapter_3_init(Game_Atari *game) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;

    level->state = CHAPTER_3_STATE_OFFICE;

    Texture2D *textures = atari_assets.textures;
    textures[0]  = load_texture("art/desktop_wallpaper.png");
    textures[1]  = load_texture("art/window_controls.png");
    textures[2]  = load_texture("art/window_title.png");
    textures[3]  = load_texture("art/player.png");
    textures[4]  = load_texture("art/cubicle_vertical_part.png");
    textures[5]  = load_texture("art/cubicle_top_part.png");
    textures[6]  = load_texture("art/office_chair.png");
    textures[7]  = load_texture("art/office_outside.png");
    textures[8]  = load_texture("art/office_guy.png");
    textures[9]  = load_texture("art/office_girl.png");
    textures[10] = load_texture("art/kitchen_table.png");
    textures[11] = load_texture("art/lunch_room.png");
    textures[12] = load_texture("art/devil_table.png");
    textures[13] = load_texture("art/guy_blue.png");
    textures[14] = load_texture("art/girl.png");
    textures[15] = load_texture("art/player_white.png");

    level->camera.zoom = 1;

    game->entities = make_array<Entity*>(512);

    float speed = 30;

    atari_text_list_init(&game->text[0],
                         0,
                         "This isn't your desk.",
                         speed,
                         nullptr);

    String choices[] = { const_string("yes"), const_string("no") };

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
                         15,
                         nullptr);

    speed = 30;

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
                         "Oh! Did I tell you guys\nabout yesterday?",
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
                         "By the way, did y'all\nsee Jenny and Mark\nyesterday?",
                         speed,
                         &game->text[13]);
    atari_text_list_init(&game->text[13],
                         "Clarice",
                         "That's old news, Jake.\rY'know,\rI was thinking",
                         speed,
                         nullptr);

    for (int i = 4; i <= 13; i++) {
        game->text[i].location = Top;
    }

    level->player = chapter_3_make_entity(ENTITY_PLAYER, 66, 96);

    array_add(&game->entities, level->player);

    chapter_3_init_outside(game);

    chapter_3_job_init(game, 0);

    //chapter_3_goto_lunch_room(game, CHAPTER_3_LUNCH_TEXT_1);
    //level->minigame.active = true;
}

void job_minigame_run(Game_Atari *game, Chapter_3_Job_Minigame *minigame,
                      float dt, RenderTexture *current_render_target)
{
    assert(minigame->active);

    // Tick
    if (key_down()) minigame->scroll_y -= 120 * dt;
    if (key_up())   minigame->scroll_y += 120 * dt;

    Vector2 wheel = GetMouseWheelMoveV();
    int scroll_speed_px = 10;
    minigame->scroll_y += scroll_speed_px * wheel.y;

    minigame->scroll_y = min(minigame->scroll_y, 0);
    
    // Draw
    ClearBackground(BEIGE);
    DrawTexture(atari_assets.textures[0], 0, 0, WHITE);

    Font *font = minigame->font;

    DrawRectangleRec(minigame->window_outer, WHITE);
    DrawRectangleLinesEx(minigame->window_outer, minigame->window_border_thickness, BLACK);
    DrawRectangleLinesEx(minigame->title_bar, 1, BLACK);

    Texture *controls = &atari_assets.textures[1];
    DrawTexture(*controls, minigame->title_bar.x + minigame->title_bar.width - controls->width, minigame->title_bar.y, WHITE);
    Texture *window_title = &atari_assets.textures[2];
    DrawTexture(*window_title, minigame->title_bar.x, minigame->title_bar.y, WHITE);

    // Draw document
    BeginTextureMode(minigame->render_target);

    ClearBackground({});

    String current_line = {};
    Vector2 default_size = MeasureTextEx(*font, "T", font->baseSize, minigame->font_spacing);
    size_t previous_word_index = 0;
    size_t line_start_index = 0;
    float previous_height = 0;

    int pad = 3;
    Vector2 pos = {
        pad,
        pad + minigame->scroll_y
    };

    auto add_line = [&]() -> void {
        Vector2 size = default_size;

        if (current_line.text[0]) {
            size = MeasureTextEx(*font, current_line.text, font->baseSize, minigame->font_spacing);
            DrawTextEx(*font, current_line.text, pos, font->baseSize, minigame->font_spacing, BLACK);
        }

        current_line = {};

        pos.y += size.y;
    };

    Document *document = &minigame->document_list[minigame->current_document];

    Word *word_on  = 0;

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
                        goto set_word_up;
                }
            }
        } else {
            if (i == word_on->end_index) {
set_word_up:
                size_t length = word_on->end_index - word_on->start_index + 1;

                char word_before[128] = {};
                strncpy(word_before, current_line.text, word_on->start_index - line_start_index);
                char word[128] = {};
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
            }
        }

        if (c == '\n' || c == ' ' || i == strlen(document->string)-1) {
            Vector2 size = MeasureTextEx(*font, current_line.text, font->baseSize, minigame->font_spacing);

            previous_height = size.y;

            if (size.x > minigame->window_inner.width) {
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

    Vector2 mouse = get_mouse();

    for (int i = 0; i < document->word_count; i++) {
        Rectangle rect = document->words[i].rect;
        rect.x += minigame->window_inner.x;
        rect.y += minigame->window_inner.y;

        if (CheckCollisionPointRec(mouse, rect)) {
            Color color = { 255, 0, 0, 128 };

            DrawRectangleRec(document->words[i].rect, color);
            if (is_action_pressed()) {
                Word *word = &document->words[i];

                if (word->correction) {
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
    }

    minigame->finish_button.x = 
        minigame->window_inner.width - minigame->finish_button.width - 5;
    minigame->finish_button.y = 
        pos.y + 5;

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

    Rectangle actual_rect = {
        minigame->window_inner.x + minigame->finish_button.x,
        minigame->window_inner.y + minigame->finish_button.y,
        minigame->finish_button.width,
        minigame->finish_button.height
    };

    if (can_finish && CheckCollisionPointRec(mouse, actual_rect)) {
        color = BLACK;
        if (is_action_pressed()) {
            if (complete_lot) {
                minigame->active = false;

                Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;
                int new_text = level->current_lunch_text + 1;
                if (new_text <= CHAPTER_3_LUNCH_TEXT_3)
                    chapter_3_goto_lunch_room(game, (Chapter_3_Lunch_Text) new_text);
            } else {
                chapter_3_next_document(minigame);
            }
        }
    }

    DrawRectangleRec(minigame->finish_button, color);
    DrawRectangleRec(finish_button_progress, GREEN);

    const char *text = complete_lot ? "Finish" : "Next";

    Vector2 size = MeasureTextEx(*font, text, font->baseSize, 0);
    DrawTextEx(*minigame->font, text, {minigame->finish_button.x + minigame->finish_button.width/2 - size.x/2, minigame->finish_button.y + minigame->finish_button.height/2 - size.y/2}, font->baseSize, 0, WHITE);

    EndTextureMode();
    BeginTextureMode(*current_render_target);

    int width = minigame->render_target.texture.width;
    int height = minigame->render_target.texture.height;

    DrawTexturePro(minigame->render_target.texture,
                   {0, 0, (float)width, -(float)height},
                   minigame->window_inner,
                   {0, 0},
                   0,
                   WHITE);
}

// tick
void chapter_3_entity_update(Entity *entity, Game_Atari *game, float dt) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;

    bool open_dialogue = can_open_dialogue(game, entity, level->player);

    switch (entity->type) {
        case ENTITY_PLAYER: {
            // player tick player update player

            int dir_x = key_right() - key_left();
            int dir_y = key_down()  - key_up();

            if (game->current != nullptr)
                dir_x = dir_y = 0;

            float player_speed = 60;

            if (IsKeyDown(KEY_P))
                player_speed *= 5;

            Vector2 velocity = { dir_x * player_speed * dt, dir_y * player_speed * dt };
            apply_velocity(entity, velocity, &game->entities);

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
            if (level->screens_scrolled <= 8 && scrolled_dir != DIRECTION_UP)
                can_scroll = false;

            if (level->state == CHAPTER_3_STATE_OFFICE && scrolled_dir != DIRECTION_INVALID && can_scroll) {
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

                if (level->screens_scrolled >= 15) {
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

void chapter_3_entity_draw(Entity *entity, Game_Atari *game) {
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

void chapter_3_deinit(Game_Atari *game) {
    (void)game;
}

void chapter_3_update(Game_Atari *game, float dt) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;

    if (level->state == CHAPTER_3_STATE_LUNCH) {
        if (game->current) {
            if (!game->current->next[0]) {
                if (is_text_list_at_end(game->current)) {
                    level->minigame.active = true;
                    level->state = CHAPTER_3_STATE_OFFICE;
                    game->current = 0;

                    chapter_3_job_init(game, 1);
                }
            }
        }

        level->lunch_devil_effect = IsKeyDown(KEY_O);
    }

    if (level->minigame.active) {
    } else {
        size_t entity_count = game->entities.length;

        for (size_t i = 0; i < entity_count; i++) {
            Entity *e = game->entities.data[i];
            chapter_3_entity_update(e, game, dt);
        }
    }
}

void chapter_3_draw(Game_Atari *game, float dt) {
    game->textbox_alpha = 200;

    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;
    Chapter_3_Job_Minigame *minigame = &level->minigame;

    switch (level->state) {
        case CHAPTER_3_STATE_OFFICE: {
            if (minigame->active) {
                job_minigame_run(game, minigame, dt, &game->render_target);
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

            EndMode2D();
        } break;
        case CHAPTER_3_STATE_ROAD: {
        } break;
        case CHAPTER_3_STATE_HOME: {
        } break;
    }
}

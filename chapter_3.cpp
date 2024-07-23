#define MAX_DOCUMENTS 16
#define MAX_CORRECTIONS 128

enum Chapter_3_Area {
    CHAPTER_3_AREA_OFFICE,
    CHAPTER_3_AREA_ROAD,
    CHAPTER_3_AREA_HOME
};

struct Word {
    int   start_index;
    int   end_index;
    char *correction; // becomes nullptr when corrected

    Rectangle rect;
};

struct Document {
    char   *string;
    size_t  string_length;

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
    Arena arena;
    Chapter_3_Area current_area;
    Chapter_3_Job_Minigame minigame;
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
    assert(strlen(correction) == (end_index - start_index + 1));
    doc->words[doc->word_count++] = {start_index, end_index, correction, {}};
    doc->error_count++;
}

Chapter_3_Job_Minigame make_chapter_3_job_minigame(const char **document_list,
                                                   size_t document_count)
{
    Chapter_3_Job_Minigame result = {};

    result.active = false;

    result.document_count = document_count;
    result.current_document = 0;

    for (int i = 0; i < document_count; i++) {
        Document *document      = &result.document_list[i];

        document->string_length = strlen(document_list[i]);
        document->string        = (char*)calloc(document->string_length+1, 1);
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

    result.finish_button.width  = 60;
    result.finish_button.height = 24;

    result.font = &atari_font;

    result.render_target = LoadRenderTexture(result.window_inner.width,
                                             result.window_inner.height);

    // Tokenize

    for (int doc = 0; doc < result.document_count; doc++) {
        Document *document = &result.document_list[doc];

        Word word = {};

        for (int i = 0; i < document->string_length; i++) {
            if (i+1 >= document->string_length-1 || isspace(document->string[i+1])) {
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

void chapter_3_init(Game_Atari *game) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;

    level->current_area = CHAPTER_3_AREA_OFFICE;

    Texture2D *textures = atari_assets.textures;
    textures[0] = load_texture("art/desktop_wallpaper.png");
    textures[1] = load_texture("art/window_controls.png");
    textures[2] = load_texture("art/window_title.png");

    const char **documents = (const char**)calloc(16, sizeof(char*));
    documents[0] = "It wsa a bright cold day in April, and the clocks were striking thirteen. Wisnton Smith, his chin nuzzled into his breast in an effort to escape the vile wind, slipped quickly through the glass doors of Victory Mansions, though not quickly enough to prevent a swirl of gritty dust from entering along with him.";

    documents[1] = 
        "Max inched towards three blocks of stone, "
        "accompanied by an array of chisels. "
        "His gaze intensified at the marble curiously, "
        "longing for something hidden, locked away. "
        "A glimmer previously dormant ignited in him.";

    level->minigame = make_chapter_3_job_minigame(documents, 2);

    Document *a = &level->minigame.document_list[0];

    document_register_error(a, 3, 5, "was");
    document_register_error(a, 74, 80, "Winston");
}

void chapter_3_deinit(Game_Atari *game) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;
    (void)level;
}

void chapter_3_update(Game_Atari *game, float dt) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;
    Chapter_3_Job_Minigame *minigame = &level->minigame;
 
    if (key_down()) minigame->scroll_y -= 120 * dt;
    if (key_up())   minigame->scroll_y += 120 * dt;
    minigame->scroll_y = min(minigame->scroll_y, 0);
}

void chapter_3_draw(Game_Atari *game) {
    Level_Chapter_3 *level = (Level_Chapter_3 *)game->level;
    Chapter_3_Job_Minigame *minigame = &level->minigame;

    ClearBackground(BEIGE);
    DrawTexture(atari_assets.textures[0], 0, 0, WHITE);

    game->textbox_alpha = 200;

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

    for (size_t i = 0; i < document->string_length; i++) {
        char c = document->string[i];

        if (c == '\n') {
            add_line();
            line_start_index = i+1;
            continue;
        }

        string_concatenate(&current_line, c);

        // Compare to words
        if (word_on == nullptr) {
            for (size_t j = 0; j < document->word_count; j++) {
                Word *word = &document->words[j];

                if (word->start_index == i) {
                    word_on = word;
                    if (word->start_index == word->end_index)
                        goto set_word_up;
                    //break;
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

        if (c == ' ' || i == document->string_length-1) {
            Vector2 size = MeasureTextEx(*font, current_line.text, font->baseSize, minigame->font_spacing);

            previous_height = size.y;

            if (size.x > minigame->window_inner.width) {
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
                    size_t length = word->end_index - word->start_index + 1;
                    assert(length == strlen(word->correction));

                    strncpy(document->string + word->start_index, word->correction, length);

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
    if (can_finish) {
        color = GREEN;
    }

    Rectangle actual_rect = {
        minigame->window_inner.x + minigame->finish_button.x,
        minigame->window_inner.y + minigame->finish_button.y,
        minigame->finish_button.width,
        minigame->finish_button.height
    };

    if (can_finish && CheckCollisionPointRec(mouse, actual_rect)) {
        color = BLACK;
        if (is_action_pressed()) {
            chapter_3_next_document(minigame);
        }
    }

    DrawRectangleRec(minigame->finish_button, color);
    Vector2 size = MeasureTextEx(*font, "Finish", font->baseSize, 0);
    DrawTextEx(*minigame->font, "Finish", {minigame->finish_button.x + minigame->finish_button.width/2 - size.x/2, minigame->finish_button.y + minigame->finish_button.height/2 - size.y/2}, font->baseSize, 0, WHITE);

    EndTextureMode();
    BeginTextureMode(game->render_target);

    int width = minigame->render_target.texture.width;
    int height = minigame->render_target.texture.height;

    DrawTexturePro(minigame->render_target.texture,
                   {0, 0, (float)width, -(float)height},
                   minigame->window_inner,
                   {0, 0},
                   0,
                   WHITE);

}

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define STRING_SIZE    1024 // We just store everything fixed size lol
#define TEXT_LIST_SIZE 32
#define TEXT_LINE_MAX  32
#define CHOICE_MAX     12
#define SCROLL_SPEED   30

#define const_string(str) { str, sizeof(str)-1 }

enum Scroll_Type {
    LetterByLetter,
    EntireLine
};

struct String {
    char text[STRING_SIZE];
    size_t length;
};

struct Text {
    Scroll_Type scroll_type;

    Sound_ID scroll_sound = SOUND_INVALID; // gets set to a default in text_init if not specified.

    bool center_text;
    bool disallow_skipping;

    Vector2 position;
    float padding;

    Font *font = &global_font;
    int font_spacing;
    Color color, backdrop_color;
    bool  background;

    float scale;

    String lines[TEXT_LINE_MAX];
    size_t line_count;
    int current_line;

    float height;

    float alpha, alpha_speed; // alpha_speed is in units/second
    float scroll_index;

    float scroll_speed = SCROLL_SPEED; // characters/second

    bool textscroll_sound_queued;

    bool finished, not_first_frame;
};

enum Render_Type {
    Bare,
    ShadowBackdrop,
    DrawTextbox,
};

enum Location {
    Top,
    Middle,
    Bottom
};

struct Text_List {
    Text_List *next[CHOICE_MAX];

    bool first_frame = true;
    bool take_keyboard_focus = true;
    bool disallow_skipping = false;

    // Render Info
    Font *font = &global_font;
    int font_spacing;
    Render_Type render_type = Render_Type::DrawTextbox;
    Scroll_Type scroll_type = Scroll_Type::LetterByLetter;
    Location location       = Location::Bottom;
    Color color             = RAYWHITE;
    Color bg_color          = BLACK;
    Color choice_color      = WHITE;
    Color choice_backdrop_color = BLACK;
    Color backdrop_color    = {0, 0, 0, 0};
    Color arrow_color       = BLACK;
    bool  background;
    float alpha_speed       = 1;
    bool center_text;
    float scale = 1;

    Sound_ID scroll_sound = SOUND_INVALID; // gets set to a default in text_init if not specified.

    float scroll_speed = SCROLL_SPEED;

    float padding;
    float textbox_height;

    String speaker;

    Text text[TEXT_LIST_SIZE];
    String choices[CHOICE_MAX];
    void (*callbacks[CHOICE_MAX])(void *);

    int text_count;
    int text_index;

    int choice_count;
    int choice_index;

    bool finished;
    bool choice;
};

void string_concatenate(String *dest, char ch) {
    if (dest->length+1 >= STRING_SIZE) return;

    dest->text[dest->length++] = ch;
    dest->text[dest->length] = 0;
}

void string_concatenate(String *dest, String src) {
    if (dest->length + src.length >= STRING_SIZE) return;

    for (size_t i = 0; i < src.length; i++) {
        dest->text[dest->length + i] = src.text[i];
    }

    dest->length += src.length;

    dest->text[dest->length] = 0;
}

bool is_newline(char c) {
    return (c == '\n') || (c == '\r') || (c == '\t');
}

void text_init(Text *text, Scroll_Type scroll_type, Vector2 pos, const char *line) {
    if (text->font == nullptr) {
        text->font = &global_font; // global
    }

    text->scroll_type = scroll_type;
    text->position = pos;

    if (text->scroll_sound == SOUND_INVALID) {
        if (scroll_type == LetterByLetter)
            text->scroll_sound = SOUND_TEXT_SCROLL;
        else
            text->scroll_sound = SOUND_EXHALE;
    }

    size_t line_length = strlen(line);
    String current_line = {};

    for (int i = 0; i < line_length; i++) {
        char ch = line[i];

        if (ch == '\n' || i == line_length-1) {
            if (i == line_length-1) {
                string_concatenate(&current_line, ch);
            }

            text->lines[text->line_count++] = current_line;
            current_line = {};
            continue;
        }

        string_concatenate(&current_line, ch);
    }

    switch (scroll_type) {
        case Scroll_Type::LetterByLetter: {
            text->alpha = 1;
            text->scroll_index = 0;
        } break;
        case Scroll_Type::EntireLine: {
            text->alpha        = 0;
            if (text->alpha_speed == 0) text->alpha_speed  = 1;
            text->current_line = (int)text->line_count-1;
            text->scroll_index = (float)text->lines[text->current_line].length-1;
        } break;
    }

    for (int i = 0; i < text->line_count; i++) {
        Vector2 size = MeasureTextEx(*text->font, text->lines[i].text, (float)text->font->baseSize, text->font_spacing);
        text->height += size.y;
    }
}

bool is_text_at_end(Text *text) {
    if (text->alpha == 1 && text->current_line == text->line_count-1 &&
        text->scroll_index == text->lines[text->current_line].length-1)
    {
        return true;
    }

    return false;
}

void text_set_to_end(Text *text) {
    text->alpha = 1;
    text->current_line = (int)(text->line_count - 1);
    text->scroll_index = (float)(text->lines[text->current_line].length - 1);
}

bool text_update_and_draw(Text *text, Vector2 offset, float dt) {
    switch (text->scroll_type) {
        case Scroll_Type::LetterByLetter: {
            do {
                size_t line_length = text->lines[text->current_line].length;

                if (text->scroll_index < line_length-1) {
                    int prev = text->scroll_index;
                    text->scroll_index += text->scroll_speed * dt;

                    if (text->textscroll_sound_queued || (int)text->scroll_index != prev) {
                        // we have scrolled one character, play the sound.
                        bool sound_available = (text->scroll_sound != SOUND_EMPTY && text->scroll_sound != SOUND_INVALID);

                        if (sound_available && !isspace(text->lines[text->current_line].text[(int)text->scroll_index])) {
                            if (!is_sound_playing(text->scroll_sound)) {
                                play_sound(text->scroll_sound);
                                text->textscroll_sound_queued = false;
                            } else {
                                text->textscroll_sound_queued = true;
                            }
                        }
                    }

                    if (text->scroll_index >= line_length-1) {
                        text->scroll_index = (float)line_length-1;
                    }
                } else if (text->current_line < text->line_count-1) {
                    text->scroll_index = 0;
                    text->current_line++;
                }

                if (is_text_at_end(text)) {
                    break;
                }
            } while (is_newline(text->lines[text->current_line].text[(int)text->scroll_index]));
        } break;
        case Scroll_Type::EntireLine: {
            if (text->scroll_sound != SOUND_EMPTY &&
                text->alpha == 0)
            {
                SetSoundPitch(game_audio.sounds[SOUND_EXHALE].sound, rand_range(0.5f, 0.7f));
                play_sound(text->scroll_sound);
            }

            text->alpha += text->alpha_speed * dt;
            text->alpha = min(text->alpha, 1);
        } break;
    }

    // Draw the actual text

    Color color = text->color;
    
    color.a = (uint8_t) (255 * text->alpha);

    float line_height = (float)text->height / text->line_count;

    Vector2 pos = text->position;
    pos = Vector2Add(pos, offset);

    for (int i = 0; i <= text->current_line; i++) {
        String line = text->lines[i];
        if (i == text->current_line)
            line.text[(int)text->scroll_index+1] = 0;

        Vector2 size = MeasureTextEx(*text->font, text->lines[i].text, (float)text->font->baseSize, text->font_spacing);
        if (text->center_text) {
            pos.x = render_width/2.f - size.x/2.f;
        }

        if (text->backdrop_color.a > 0) {
            if (text->background) {
                DrawRectangle(pos.x, pos.y, size.x+1, size.y+1, BLACK);
                //DrawRectangleLines(pos.x-1, pos.y-1, size.x+3, size.y+3, text->color);
            }

            Color backdrop = text->backdrop_color;
            backdrop.a *= text->alpha;

            DrawTextEx(*text->font, line.text, {1+(float)((int)pos.x), 1+(float)((int)pos.y)}, (float)text->font->baseSize, text->font_spacing, backdrop);
        }

        DrawTextEx(*text->font, line.text, {(float)((int)pos.x), (float)((int)pos.y)}, (float)text->font->baseSize, text->font_spacing, color);
        pos.y += line_height;
    }

#ifdef DEBUG
    if (IsKeyDown(KEY_M)) {
        text_set_to_end(text);
        text->finished = true;
        return true;
    }
#endif

    if (text->not_first_frame && !text->finished && is_action_pressed()) {
        if (is_text_at_end(text)) {
            text->finished = true;
            return true;
        }

        if (!text->disallow_skipping)
            text_set_to_end(text);
    }

    text->not_first_frame = true;
    return false;
}

// Separates Text by '\r' or '\t'
// Note: Before calling this, you should set up variables
// such as font, font_spacing, render_type, scroll_type,
// center_text, scale, and color.
void text_list_init(Text_List *list, char *speaker, char *text_string,
                    Text_List *next)
{
    if (list->font == nullptr) {
        list->font = &global_font;
    }

    if (speaker) {
        strcpy(list->speaker.text, speaker);
        list->speaker.length = strlen(speaker);
    }

    list->next[0] = next;
    list->padding = 20 * list->scale;

    list->textbox_height = render_height/3.f;

    size_t text_length = strlen(text_string);

    float cum = 0; // Accumulation of y

    char line[STRING_SIZE] = {};
    size_t line_length = 0;

    for (size_t i = 0; i < text_length; i++) {
        char ch = text_string[i];

        if (ch == '\r' || i == text_length - 1) {
            if (i == text_length - 1) {
                line[line_length++] = ch;
            }

            Text text = {};

            text.scale          = list->scale;
            text.font           = list->font;
            text.font_spacing   = list->font_spacing;
            text.center_text    = list->center_text;
            text.color          = list->color;
            text.backdrop_color = list->backdrop_color;
            text.scroll_speed   = list->scroll_speed;
            text.alpha_speed    = list->alpha_speed;
            text.background     = list->background;
            text.scroll_sound   = list->scroll_sound;
            text.disallow_skipping = list->disallow_skipping;

            Vector2 pos = { list->padding, list->padding + cum };

            text_init(&text,
                      list->scroll_type,
                      pos,
                      line);

            cum += text.height;

            list->text[list->text_count++] = text;
            text = {};

            memset(line, 0, sizeof(line));
            line_length = 0;
            continue;
        }

        line[line_length++] = ch;
    }
}

void init_text_list_default(Text_List *list, char *speaker, char *text_string, Text_List *next) {
    *list = Text_List();
    text_list_init(list, speaker, text_string, next);
}

Text_List choice_text_list_init(char *speaker,
                                char *text_string,
                                String choices[],
                                Text_List *next[],
                                int choice_count)
{
    Text_List result = {};
    
    init_text_list_default(&result, speaker, text_string, nullptr);

    result.choice = true;
    result.choice_count = choice_count;
    result.choice_index = -1;
    memcpy(result.choices, choices, sizeof(choices[0]) * choice_count);
    memcpy(result.next,    next,    sizeof(next[0]) * choice_count);

    return result;
}

Vector2 text_list_offset(Text_List *list) {
    Vector2 result = {};

    switch (list->location) {
        case Top: {} break;
        case Middle: {
            result.y = render_height / 2.f - list->textbox_height / 2.f;
        } break;
        case Bottom: {
            result.y = render_height - list->textbox_height;
        } break;
    }

    return result;
}

void reset_text(Text *text) {
    text->finished = false;
    text->not_first_frame = false;

    switch (text->scroll_type) {
        case Scroll_Type::LetterByLetter: {
            text->alpha = 1;
            text->scroll_index = 0;
            text->current_line = 0;
        } break;
        case Scroll_Type::EntireLine: {
            text->alpha        = 0;
            text->alpha_speed  = 1;
            text->current_line = (int)text->line_count-1;
            text->scroll_index = (float)text->lines[text->current_line].length-1;
        } break;
    }
}

void reset_text_list(Text_List *list) {
    list->text_index = 0;
    list->finished = false;
    list->first_frame = true;
    list->choice_index = -1;

    for (int i = 0; i < list->text_count; i++) {
        reset_text(&list->text[i]);
    }
}

bool is_text_list_at_end(Text_List *list) {
    bool result = list->text_index == list->text_count-1;
    result &= is_text_at_end(&list->text[list->text_index]);
    return result;
}

Text_List *text_list_update_and_draw(RenderTexture2D *output_target, RenderTexture2D *textbox_target, Text_List *list, void *user_data, uint8_t alpha, bool include_text_in_target, float dt) {
    BeginTextureMode(*textbox_target);

    ClearBackground({});

    Text_List *result = list;

    Vector2 offset = text_list_offset(list);

    float speaker_box_height = 0;
    float rectangle_pad = 0; // thickness of the rounded rectangle

    if (list->render_type == Render_Type::DrawTextbox) {
        float thickness = (int)(4.f * list->scale);
        if (thickness <= 0) thickness = 2; // for pixel art

        rectangle_pad = (float)thickness;

        Rectangle rectangle = {
            offset.x + rectangle_pad,
            offset.y + rectangle_pad,
            render_width         - rectangle_pad*2,
            list->textbox_height - rectangle_pad*2
        };

        // For both the main textbox and the speaker box
        int rounded_rectangle_segments = 3;
        float roundness = 0.125;

        if (thickness == 1) {
            DrawRectangleRec(rectangle, list->bg_color);
            DrawRectangleLinesEx(rectangle, 1, list->color);
        } else {
            rectangle = enlarge_rectangle(rectangle, 1);

            DrawRectangleRounded(rectangle, roundness, rounded_rectangle_segments, list->bg_color);

            rectangle = enlarge_rectangle(rectangle, -1);

            DrawRectangleRoundedLines(rectangle, roundness, thickness, rounded_rectangle_segments, list->color);
        }

        if (list->speaker.length) {
            Vector2 size = MeasureTextEx(*list->font, list->speaker.text,
                                         (float)list->font->baseSize,
                                         list->font_spacing);

            int x_offset = list->padding;
            if (thickness == 2) x_offset *= 3;

            rectangle = {
                offset.x + rectangle_pad + x_offset,
                offset.y - thickness - size.y - list->padding,
                size.x + thickness + list->padding,
                size.y + list->padding
            };

            speaker_box_height = rectangle.height;

            if (list->location == Location::Top) {
                rectangle.y = offset.y + list->textbox_height + 1;
            }

            if (thickness == 1) {
                DrawRectangleRec(rectangle, list->bg_color);
                DrawRectangleLinesEx(rectangle, 1, list->color);
            } else {
                rectangle = enlarge_rectangle(rectangle, 1);
                DrawRectangleRounded(rectangle, roundness, rounded_rectangle_segments, list->bg_color);
                rectangle = enlarge_rectangle(rectangle, -1);
                DrawRectangleRoundedLines(rectangle, roundness, thickness, rounded_rectangle_segments, list->color);
            }

            DrawTextEx(*list->font, list->speaker.text, {rectangle.x + list->padding/2, rectangle.y + list->padding/2}, (float)list->font->baseSize, list->font_spacing, list->color);
        }
    } else if (list->render_type == ShadowBackdrop) {
        Rectangle rectangle = {
            offset.x,
            offset.y,
            (float)render_width,
            list->textbox_height,
        };

        Color c = list->bg_color;
        c.a = 215;

        DrawRectangleRounded(rectangle, 0.125, 4, c);
    }

    auto finish_target = [&]() {
        EndTextureMode();

        if (output_target)
            BeginTextureMode(*output_target);

        DrawTexturePro(textbox_target->texture,
                       {0, 0, (float)render_width, -(float)render_height},
                       {0, 0, (float)render_width, (float)render_height},
                       {0, 0},
                       0,
                       {255, 255, 255, alpha});
    };

    if (!include_text_in_target) {
        finish_target();
    }

    for (int i = 0; i <= list->text_index; i++) {
        Text *text = &list->text[i];

        bool done = text_update_and_draw(text, Vector2Add(offset, {rectangle_pad + list->padding, rectangle_pad + list->padding}), dt);

        if (done) {
            // Note: done is only true for text[text_index]
            list->text_index++;
            if (list->text_index >= list->text_count) {
                list->text_index = list->text_count-1;
                if (!list->choice) {
                    list->finished = true;
                }
            }
        }
    }

    if (include_text_in_target) {
        finish_target();
    }

    if (list->choice && list->text_index == list->text_count-1) {
        Text *current_text = &list->text[list->text_index];

        if (is_text_at_end(current_text)) {
            if (key_down_pressed() || GetMouseWheelMove() < 0) {
                list->choice_index++;
                if (list->choice_index >= list->choice_count)
                    list->choice_index = 0;

                play_sound(SOUND_TEXT_SCROLL);
            }
            if (key_up_pressed() || GetMouseWheelMove() > 0) {
                list->choice_index--;
                if (list->choice_index < 0)
                    list->choice_index = list->choice_count-1;

                play_sound(SOUND_TEXT_SCROLL);
            }
            if (is_action_pressed() && list->choice_index != -1) {
                list->finished = true;
                result = list->next[list->choice_index];

                if (list == result) {
                    reset_text_list(list);
                }

                // Execute the function pointer hook if we have one.
                void (*hook)(void*) = list->callbacks[list->choice_index];
                if (hook) {
                    hook(user_data);
                }

                play_sound(SOUND_TEXT_CONFIRM);
            }

            Vector2 pos = {};
            switch (list->location) {
                case Top: {
                    pos = { list->padding*2, list->textbox_height + list->padding*2 };
                } break;
                case Middle: {
                    pos = { list->padding*2, list->textbox_height + list->padding*2 };
                } break;
                case Bottom: {
                    Vector2 size = MeasureTextEx(*list->font, list->choices[0].text,
                                                 (float)list->font->baseSize, list->font_spacing);
                    pos = { list->padding*2, -speaker_box_height - list->padding*3 - list->choice_count * size.y };
                } break;
            }
            pos = Vector2Add(pos, offset);

            for (int i = 0; i < list->choice_count; i++) {
                Color color = list->choice_color;
                Color bg = list->choice_backdrop_color;
                Color arrow_color = list->arrow_color;

                float xoff = 0;

                if (i == list->choice_index) {
                    xoff = 10;

                    // choice draw arrow

                    float size = 8;
                    float pad = 4;

                    //Vector2 v1 = { pad, pos.y - size};
                    //Vector2 v2 = { pad + v1.x, v1.y + size};
                    //Vector2 v3 = { pad + v1.x + size , v1.y };

                    Vector2 v1 = { pad, pos.y + size/4 };
                    Vector2 v2 = { pad + size, pos.y + size/2 + size/4 };
                    Vector2 v3 = { pad, pos.y + size + size/4 };

                    DrawTriangle(int_vec2(v1), int_vec2(v3), int_vec2(v2), arrow_color);
                }

                String *choice = &list->choices[i];
                Vector2 size = MeasureTextEx(*list->font, choice->text,
                                             (float)list->font->baseSize,
                                             list->font_spacing);

                //DrawRectangleRec({pos.x, pos.y, size.x, size.y}, BLACK);

                pos.x = (int)pos.x;
                pos.y = (int)pos.y;

                DrawTextEx(*list->font, choice->text, Vector2Add(pos, {xoff+1,1}), (float)list->font->baseSize, list->font_spacing, bg);
                DrawTextEx(*list->font, choice->text, Vector2Add(pos, {xoff+2,2}), (float)list->font->baseSize, list->font_spacing, bg);

                DrawTextEx(*list->font, choice->text, Vector2Add(pos, {xoff,0}), (float)list->font->baseSize, list->font_spacing, color);
                pos.y += size.y;
            }
        }
    } else if (list->finished) {
        result = list->next[0];

        void (*hook)(void*) = list->callbacks[0];
        if (hook) {
            hook(user_data);
        }
    }

    // Arrow

    Text *current_text = &list->text[list->text_index];

    if (is_text_at_end(current_text)) {
        float size = max(5, 15 * list->scale);
        float pad = size*2;

        float t = 6*(float)GetTime();

        float offset_y = sinf(t) * size/4.f;

        Vector2 v1 = { render_width - size - pad, list->textbox_height - size - pad*0.75f + offset_y };

        if (list->location == Middle) {
            v1.y = render_height - size * 2 - pad/2;
        }
        Vector2 v2 = { v1.x + size/2.f, v1.y + size};
        Vector2 v3 = { v1.x + size , v1.y };

        if (list->location != Middle) {
            v1 = Vector2Add(v1, offset);
            v2 = Vector2Add(v2, offset);
            v3 = Vector2Add(v3, offset);
        }

        DrawTriangle(v1, v2, v3, list->color);
    }
    /*

    if (list->first_frame && list->text[0].scroll_type == EntireLine) {
    }
    */

    list->first_frame = false;

    if (result != list) {
        // Reset us.
        reset_text_list(list);
    }

    return result;
}

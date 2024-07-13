#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define STRING_SIZE    128
#define TEXT_LIST_SIZE 32
#define TEXT_LINE_MAX  8
#define CHOICE_MAX     4
#define SCROLL_SPEED   30

#define const_string(str) { str, strlen(str) }

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

    bool center_text;

    Vector2 position;
    float padding;

    Font *font;
    int font_spacing;
    Color color;

    float scale;

    String lines[TEXT_LINE_MAX];
    size_t line_count;
    int current_line;

    float height;

    float alpha, alpha_speed; // alpha_speed is in units/second
    float scroll_index;

    float scroll_speed; // characters/second

    bool finished, not_first_frame;
};

enum Render_Type {
    Bare,
    DrawTextbox,
};

enum Location {
    Top,
    Middle,
    Bottom
};

struct Text_List {
    Text_List *next[CHOICE_MAX];

    // Render Info
    Font *font;
    int font_spacing;
    Render_Type render_type = Render_Type::DrawTextbox;
    Scroll_Type scroll_type = Scroll_Type::LetterByLetter;
    Location location       = Location::Bottom;
    Color color             = RAYWHITE;
    bool center_text;
    float scale = 1;

    float padding;
    float textbox_height;

    String speaker;

    Text text[TEXT_LIST_SIZE];
    String choices[CHOICE_MAX];

    int text_count;
    int text_index;

    int choice_count;
    int choice_index;

    bool finished;
    bool choice;
};

bool is_action_pressed() {
    return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

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
    text->scroll_speed = SCROLL_SPEED;

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
            text->alpha_speed  = 1;
            text->current_line = (int)text->line_count-1;
            text->scroll_index = (float)text->lines[text->current_line].length-1;
        } break;
    }

    for (int i = 0; i < text->line_count; i++) {
        Vector2 size = MeasureTextEx(*text->font, text->lines[i].text, (float)text->font->baseSize, text->font_spacing);
        text->height += size.y;
    }
}

bool text_at_end(Text *text) {
    if (text->alpha == 1 && text->current_line == text->line_count-1 &&
        text->scroll_index == text->lines[text->current_line].length-1)
    {
        return true;
    }

    return false;
}


bool text_update_and_draw(Text *text) {
    switch (text->scroll_type) {
        case Scroll_Type::LetterByLetter: {
            do {
                size_t line_length = text->lines[text->current_line].length;

                if (text->scroll_index < line_length-1) {
                    text->scroll_index += text->scroll_speed * GetFrameTime();
                    if (text->scroll_index >= line_length-1) {
                        text->scroll_index = (float)line_length-1;
                    }
                } else if (text->current_line < text->line_count-1) {
                    text->scroll_index = 0;
                    text->current_line++;
                }

                if (text_at_end(text)) {
                    break;
                }
            } while (is_newline(text->lines[text->current_line].text[(int)text->scroll_index]));
        } break;
        case Scroll_Type::EntireLine: {
            text->alpha += text->alpha_speed * GetFrameTime();
            text->alpha = min(text->alpha, 1);
        } break;
    }

    // Draw the actual text

    Color color = text->color;
    
    color.a = (uint8_t) (255 * text->alpha);

    float line_height = (float)text->height / text->line_count;

    Vector2 pos = text->position;

    for (int i = 0; i <= text->current_line; i++) {
        String line = text->lines[i];
        if (i == text->current_line)
            line.text[(int)text->scroll_index+1] = 0;

        Vector2 size = MeasureTextEx(*text->font, text->lines[i].text, (float)text->font->baseSize, text->font_spacing);
        if (text->center_text) {
            pos.x = render_width/2.f - size.x/2.f;
        }

        DrawTextEx(*text->font, line.text, pos, (float)text->font->baseSize, text->font_spacing, color);
        pos.y += line_height;
    }

    if (text->not_first_frame && !text->finished && is_action_pressed()) {
        if (text_at_end(text)) {
            text->finished = true;
            return true;
        }

        text->alpha = 1;
        text->current_line = (int)(text->line_count - 1);
        text->scroll_index = (float)(text->lines[text->current_line].length - 1);
    }

    text->not_first_frame = true;
    return false;
}

// Separates Text by '\r'
// Note: Before calling this, you should set up variables
// such as font, font_spacing, render_type, scroll_type,
// center_text, scale, and color.
void text_list_init(Text_List *list, char *speaker, char *text_string,
                    Text_List *next)
{
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

            text.scale = list->scale;
            text.font = list->font;
            text.font_spacing = list->font_spacing;
            text.center_text = list->center_text;
            text.color = list->color;

            text_init(&text,
                      list->scroll_type,
                      { list->padding, list->padding + cum },
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

Text_List make_choice_text_list(char *speaker, char *text_string,
                                String choices[], Text_List *next[],
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

Text_List *text_list_update_and_draw(Text_List *list) {
    Text_List *result = list;

    Vector2 offset = text_list_offset(list);

    if (list->render_type == Render_Type::DrawTextbox) {
        int thickness = (int)(4.f * list->scale);
        if (thickness <= 0) thickness = 1;

        float pad = (float)thickness;
        if (thickness == 1) pad = 0;

        Rectangle rectangle = {
            offset.x + pad,
            offset.y + pad,
            render_width         - pad*2,
            list->textbox_height - pad*2
        };

        if (thickness == 1) {
            DrawRectangleRec(rectangle, BLACK);
            DrawRectangleLinesEx(rectangle, 1, WHITE);
        } else {
            DrawRectangleRounded(rectangle, 0.125, 3, BLACK);
            DrawRectangleRoundedLines(rectangle, 0.125, thickness, 3, WHITE);
        }

        if (list->speaker.length) {
            Vector2 size = MeasureTextEx(*list->font, list->speaker.text,
                                         (float)list->font->baseSize,
                                         list->font_spacing);

            rectangle = {
                offset.x + pad + list->padding,
                offset.y - pad - size.y - list->padding,
                size.x + pad + list->padding,
                size.y + pad + list->padding
            };

            if (list->location == Location::Top) {
                rectangle.y = offset.y + list->textbox_height;
            }

            if (thickness == 1) {
                DrawRectangleRec(rectangle, BLACK);
                DrawRectangleLinesEx(rectangle, 1, WHITE);
            } else {
                DrawRectangleRounded(rectangle, 0.125, 3, BLACK);
                DrawRectangleRoundedLines(rectangle, 0.125, thickness, 3, WHITE);
            }

            DrawTextEx(*list->font, list->speaker.text, {rectangle.x + list->padding/2, rectangle.y + list->padding/2}, (float)list->font->baseSize, list->font_spacing, WHITE);
        }
    }

        
    for (int i = 0; i <= list->text_index; i++) {
        Text *text = &list->text[i];

        Vector2 saved_pos = text->position;
        
        text->position = Vector2Add(text->position, offset);

        bool done = text_update_and_draw(text);

        text->position = saved_pos;

        if (done) {
            // Note: done is only true for text[text_index]
            list->text_index++;
            if (list->text_index >= list->text_count) {
                list->text_index = list->text_count-1;
                if (!list->choice)
                    list->finished = true;
            }
        }
    }

    if (list->choice && list->text_index == list->text_count-1) {
        Text *current_text = &list->text[list->text_index];

        if (text_at_end(current_text)) {
            if (IsKeyPressed(KEY_DOWN)) {
                list->choice_index++;
                if (list->choice_index >= list->choice_count)
                    list->choice_index = 0;
            }
            if (IsKeyPressed(KEY_UP)) {
                list->choice_index--;
                if (list->choice_index < 0)
                    list->choice_index = list->choice_count-1;
            }
            if (is_action_pressed() && list->choice_index != -1) {
                list->finished = true;
                result = list->next[list->choice_index];
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
                    pos = { list->padding*2, -list->padding*2 - list->choice_count * size.y };
                } break;
            }
            pos = Vector2Add(pos, offset);

            for (int i = 0; i < list->choice_count; i++) {
                Color color = RAYWHITE;
                if (i == list->choice_index) color = GREEN;

                String *choice = &list->choices[i];
                Vector2 size = MeasureTextEx(*list->font, choice->text,
                                             (float)list->font->baseSize, list->font_spacing);

                DrawTextEx(*list->font, choice->text, pos, (float)list->font->baseSize, list->font_spacing, color);
                pos.y += size.y;
            }
        }
    } else if (list->finished) {
        result = list->next[0];
    }

    // Arrow

    Text *current_text = &list->text[list->text_index];
    if (current_text->alpha == 1 && current_text->current_line == current_text->line_count-1 && (int)current_text->scroll_index == current_text->lines[current_text->current_line].length-1)
    {
        float size = max(5, 20 * list->scale);
        float pad = size*2;

        float t = 6*(float)GetTime();

        Vector2 v1 = { render_width - size - pad, list->textbox_height - size - pad*0.75f + sinf(t) * size/4.f };
        Vector2 v2 = { v1.x + size/2.f, v1.y + size};
        Vector2 v3 = { v1.x + size , v1.y };

        v1 = Vector2Add(v1, offset);
        v2 = Vector2Add(v2, offset);
        v3 = Vector2Add(v3, offset);

        DrawTriangle(v1, v2, v3, WHITE);
    }

    return result;
}


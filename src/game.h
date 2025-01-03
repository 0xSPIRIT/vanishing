// This file contains a basic Game struct which has
// an array of entities.
//
// There are a few general functions that everyone
// will need when handling entities.
//

// Loaded and unloaded for each chapter,
// so you can use different texture indexes.
struct Atari_Assets {
    Texture2D textures[64];
} atari_assets;

enum Entity_Type {
    ENTITY_CACTUS,
    ENTITY_PLAYER,
    ENTITY_ROCK,
    ENTITY_FOOTSTEPS,
    ENTITY_BLOOD,
    ENTITY_NODE,
    ENTITY_PHONE,
    ENTITY_WALL,
    ENTITY_WINDOW,
    ENTITY_PRAYER_MAT,

    ENTITY_CHAP_2_DOOR,
    ENTITY_CHAP_2_TABLE,

    ENTITY_CHAP_2_JAKE,
    ENTITY_CHAP_2_ERICA,
    ENTITY_CHAP_2_MIKE,
    ENTITY_CHAP_2_JESSICA,
    ENTITY_CHAP_2_ELEANOR,

    ENTITY_CHAP_2_AMELIA,

    ENTITY_CHAP_2_LUNA,
    ENTITY_CHAP_2_ISABELLE,
    ENTITY_CHAP_2_CAMILA,
    ENTITY_CHAP_2_OLIVIA,
    ENTITY_CHAP_2_AVA,

    ENTITY_CHAP_2_RANDOM_GUY,
    ENTITY_CHAP_2_RANDOM_GIRL,
    ENTITY_CHAP_2_BOUNCER,
    ENTITY_CHAP_2_BARTENDER,
    ENTITY_CHAP_2_PENNY,

    ENTITY_CHAP_2_GIRL1,
    ENTITY_CHAP_2_BOY1,
    ENTITY_CHAP_2_GIRL2,
    ENTITY_CHAP_2_GIRL3,

    ENTITY_CHAP_2_COFFEE_TABLE,

    ENTITY_CHAP_2_COUPLE_1,
    ENTITY_CHAP_2_SINGLE_1,
    ENTITY_CHAP_2_COUPLE_2,
    ENTITY_CHAP_2_TABLEY_1,
    ENTITY_CHAP_2_TABLEY_2,

    ENTITY_CHAP_3_CUBICLE_TOP,
    ENTITY_CHAP_3_CUBICLE_VERTICAL,
    ENTITY_CHAP_3_CUBICLE_CHAIR,

    ENTITY_CHAP_3_GUY,
    ENTITY_CHAP_3_GIRL,

    ENTITY_CHAP_3_LUNCH_TABLE,

    ENTITY_CHAP_3_CAR,

    ENTITY_CHAP_4_WINDOW,
    ENTITY_CHAP_4_DEVIL,
    ENTITY_CHAP_4_MICROWAVE,
};

enum Draw_Layer {
    DRAW_LAYER_NORMAL = 0,
    DRAW_LAYER_LOW, // below everything
};

// All 2d chapters use the same Entity struct, because
// there's an Array<Entity*> in the main Game struct.
//
// You can include specialized variables by adding a new
// struct to the union.

// 2D entity:
struct Entity {
    Entity_Type type;
    Entity     *next_free;
    int         texture_id;      // Index into atari_assets.textures[]
    Vector2     pos;
    Rectangle   base_collider;   // At origin. add pos to get collision box
    float       alarm[10];       // General purpose alarms
    int         dialogue_state;
    bool        has_dialogue;
    Draw_Layer  draw_layer;

    union {
        // Chapter 1 entities
        Chapter_1_Player  chap_1_player;
        Chapter_1_Node    chap_1_node;
        Chapter_1_Phone   chap_1_phone;

        // Chapter 2 entities
        Chapter_2_Player  chap_2_player;
        Chapter_2_Door    chap_2_door;
        Chapter_2_Penny   chap_2_penny;

        // Chapter 3 entities
        Chapter_3_Cubicle chap_3_cubicle;
        Chapter_3_Circler chap_3_circler;

        // Chapter 4 entities
        Chapter_4_Window    chap_4_window;
        Chapter_4_Devil     chap_4_devil;
        Chapter_4_Microwave chap_4_microwave;
    };
};

enum Render_State {
    RENDER_STATE_ATARI,
    RENDER_STATE_3D
};

struct Event {
    void (*action)(struct Game *);
    float time;
};

enum Fade_Direction {
    FADE_IN = -1,
    FADE_INVALID = 0, // no fade currently happening
    FADE_OUT = 1,
};

struct Fader {
    Fade_Direction direction;
    float          alpha;
    float          speed; // alpha per second
    Color          color;

    void (*action)(struct Game *);
};

struct Game {
    Render_State render_state;

    RenderTexture2D atari_render_target;
    RenderTexture2D render_target_3d;
    RenderTexture2D textbox_target;

    Post_Processing post_processing;

    Arena level_arena; // memory persists for entire level, reset at game_init
    Array<Entity*> entities;

    Text_List  text[200];
    Text_List *current;

    Event events[64];
    int   num_events;

    bool include_text_in_target;

    Fader fader;

    float textbox_alpha; // 0.0f to 255.0f

    bool  queue_deinit_and_goto_intro;
    void *level; // Points to the specific chapter level struct.
};

struct Entity_Allocator {
    Entity *first_free;
    Arena  *level_arena;
};

Entity_Allocator entity_allocator;

Entity *allocate_entity(void) {
    Entity *result = entity_allocator.first_free;

    if (result) {
        entity_allocator.first_free = entity_allocator.first_free->next_free;
        memset(result, 0, sizeof(Entity));
    } else {
        result = (Entity *)arena_push(entity_allocator.level_arena, sizeof(Entity));
    }

    return result;
}

void free_entity(Entity *entity) {
    entity->next_free = entity_allocator.first_free;
    entity_allocator.first_free = entity;
}

Keyboard_Focus keyboard_focus(Game *game) {
    if (game->current != nullptr && game->current->take_keyboard_focus) {
        return KEYBOARD_FOCUS_TEXTBOX;
    } else {
        return NO_KEYBOARD_FOCUS;
    }
}

void add_event(Game *game, void (*action)(Game *), float time) {
    if (game->num_events+1 <= StaticArraySize(game->events)) {
        Event event = { action, time };
        game->events[game->num_events++] = event;
    }
}

void tick_events(Game *game, float dt) {
    for (int i = 0; i < game->num_events; i++) {
        Event *event = &game->events[i];

        event->time -= dt;
        if (event->time <= 0) {
            event->action(game);
            game->events[i--] = game->events[--game->num_events];
        }
    }
}

void start_fade(Game *game, Fade_Direction dir, float speed, void (*action)(Game *), Color color = BLACK) {
    Fader f;

    f.direction = dir;
    f.action = action;
    f.speed = speed;
    f.color = color;

    switch (dir) {
        case FADE_IN:  f.alpha = 255; break;
        case FADE_OUT: f.alpha = 0;   break;
    }

    game->fader = f;
}

void start_fade(Game *game, Fade_Direction dir, void (*action)(Game *)) {
    start_fade(game, dir, 60, action);
}

bool is_fade_active(Game *game) {
    return game->fader.direction != FADE_INVALID;
}

void update_and_draw_fade(Game *game, Fader *fader, float dt) {
    if (fader->direction == FADE_INVALID)
        return;

    fader->alpha += (int)fader->direction * fader->speed * dt;

    bool done = (fader->direction == FADE_OUT && fader->alpha >= 255) || (fader->direction == FADE_IN && fader->alpha <= 0);

    if (done) {
        fader->alpha = Clamp(fader->alpha, 0, 255);
        fader->direction = FADE_INVALID;

        if (fader->action)
            fader->action(game);
    }

    Color color = fader->color;
    color.a = (uint8_t)fader->alpha;

    DrawRectangle(0, 0, render_width, render_height, color);
}

Entity *entities_get_player(Array<Entity*> *entities) {
    Entity *result = 0;

    for (size_t i = 0; i < entities->length; i++) {
        if (entities->data[i]->type == ENTITY_PLAYER) {
            result = entities->data[i];
            return result;
        }
    }

    return result;
}

Entity *entities_find_from_type(Array<Entity*> *entities, Entity_Type search) {
    Entity *result = 0;

    for (size_t i = 0; i < entities->length; i++) {
        Entity *e = entities->data[i];

        if (e->type == search) {
            result = e;
            return result;
        }
    }

    return result;
}

// TODO: You can make this better by setting the alarm to 0
// when completed, but on the next frame set it to -1.
// So any code that has a check like `if (e->alarm[0] == 0)`
// will only go through once, because it's -1 on the next
// frame and it doesn't need to store an additional bool
// or whatever.
void entity_update_alarms(Entity *e, float dt) {
    for (size_t i = 0; i < StaticArraySize(e->alarm); i++) {
        if (e->alarm[i] > 0) {
            e->alarm[i] -= dt;
            if (e->alarm[i] <= 0)
                e->alarm[i] = 0;
        }
    }
}

void add_wall(Array<Entity*> *entities, Rectangle r) {
    Entity *wall = allocate_entity();
    
    wall->pos = { r.x, r.y };
    wall->type = ENTITY_WALL;

    wall->texture_id = -1;

    wall->base_collider.width  = r.width;
    wall->base_collider.height = r.height;

    array_add(entities, wall);
}

Texture2D *entity_get_texture(Entity *entity) {
    int id = entity->texture_id;

    if (id < 0 || id >= StaticArraySize(atari_assets.textures)) {
        if (id >= 0)
            assert(false);

        return nullptr;
    }

    return &atari_assets.textures[entity->texture_id];
}

int entity_texture_width(Entity *entity) {
    int result = 0;

    Texture2D *texture = entity_get_texture(entity);
    if (texture) {
        result = texture->width;
    }

    return result;
}

int entity_texture_height(Entity *entity) {
    int result = 0;

    Texture2D *texture = entity_get_texture(entity);
    if (texture) {
        result = texture->height;
    }

    return result;
}

bool are_entities_colliding(Entity *a, Entity *b) {
    assert(a != b);

    bool result = false;

    Rectangle a_collider = a->base_collider;
    Rectangle b_collider = b->base_collider;

    a_collider.x += a->pos.x;
    a_collider.y += a->pos.y;

    b_collider.x += b->pos.x;
    b_collider.y += b->pos.y;

    result = CheckCollisionRecs(a_collider, b_collider);

    return result;
}

bool are_entities_visibly_colliding(Entity *a, Entity *b) {
    assert(a != b);

    assert(a->texture_id != -1);
    assert(b->texture_id != -1);

    Rectangle a_rect = {
        a->pos.x, a->pos.y,
        (float)entity_texture_width(a),
        (float)entity_texture_height(a),
    };

    Rectangle b_rect = {
        b->pos.x, b->pos.y,
        (float)entity_texture_width(b),
        (float)entity_texture_height(b)
    };

    return CheckCollisionRecs(a_rect, b_rect);
}

bool is_entity_collidable(Entity *e) {
    if (e->type == ENTITY_CHAP_2_DOOR) {
        return e->chap_2_door.active;
    }

    if (e->type == ENTITY_FOOTSTEPS || e->type == ENTITY_BLOOD || e->type == ENTITY_CHAP_4_WINDOW ||
        e->type == ENTITY_PRAYER_MAT)
        return false;

    return true;
}

Entity *is_entity_colliding_with_any_collidable_entity(Entity *e, Array<Entity*> *entities) {
    for (int i = 0; i < entities->length; i++) {
        Entity *entity = entities->data[i];

        if (entity != e &&
            is_entity_collidable(entity) &&
            are_entities_colliding(e, entity))
        {
            return entity;
        }
    }

    return nullptr;
}

void apply_velocity_axis(Entity *e, Vector2 vel, Array<Entity*> *entities, float *pos_axis) {
    float length = 0;
    int dir = 0;

    if (pos_axis == &e->pos.x) {
        length = fabs(vel.x);
        dir    = sign(vel.x);
    } else if (pos_axis == &e->pos.y) {
        length = fabs(vel.y);
        dir    = sign(vel.y);
    }

    if (length == 0) return;

    float step     = 0.25;
    float distance = 0;

    if (step >= length/2) {
        step = length/4;
    }

    if (is_entity_colliding_with_any_collidable_entity(e, entities)) {
        // Just apply the velocity if you're stuck
        e->pos = Vector2Add(e->pos, vel);
        return;
    }

    while (true) {
        distance  += step;
        *pos_axis += step * dir;

        if (distance > length) {
            *pos_axis -= step * dir;
            break;
        }

        if (is_entity_colliding_with_any_collidable_entity(e, entities)) {
            *pos_axis -= step * dir;
            break;
        }
    }
}

void apply_velocity(Entity *e, Vector2 vel, Array<Entity*> *entities) {
    apply_velocity_axis(e, vel, entities, &e->pos.x);
    apply_velocity_axis(e, vel, entities, &e->pos.y);
}

bool is_player_close_to_entity(Entity *player, Entity *e, int closeness) {
    float width = entity_texture_width(e);
    float height = entity_texture_height(e);

    Rectangle a = {
        e->pos.x,
        e->pos.y,
        width,
        height
    };

    a = enlarge_rectangle(a, closeness);

    float player_width = entity_texture_width(player);
    float player_height = entity_texture_height(player);

    Rectangle player_rect = {
        player->pos.x,
        player->pos.y,
        player_width,
        player_height
    };

    bool within_region = CheckCollisionRecs(player_rect, a);

    return within_region;
}

bool can_open_dialogue(Game *game, Entity *e, Entity *player) {
    // Check if the player is close to multiple entities,
    // and if the closest entity == e, return true.

    assert(e);
    assert(player);

    if (e == player)              return false;
    if (game->current != nullptr) return false;
    if (!e->has_dialogue)         return false;
    if (!is_player_close_to_entity(e, player, 8)) return false;

    Entity *closest = 0;
    float closest_distance = render_width * render_height;

    for (size_t i = 0; i < game->entities.length; i++) {
        Entity *current = game->entities.data[i];

        if (current == player)      continue;
        if (!current->has_dialogue) continue;

        float distance_to_player = Vector2Length(Vector2Subtract(player->pos, current->pos));

        if (distance_to_player < closest_distance) {
            closest = current;
            closest_distance = distance_to_player;
        }
    }

    bool result = (closest == e);

    return result;
}

void atari_queue_deinit_and_goto_intro(Game *game) {
    game->queue_deinit_and_goto_intro = true;
}

void sort_entities(Array<Entity*> *entities) {
    // Sort the entity array based on the bottom of the texture
    size_t entity_count = entities->length;

    if (entity_count == 0) return;

    // do the DRAW_LAYER_LOW first
    int low_layer_count = 0;

    for (size_t i = 0; i < entity_count; i++) {
        if (entities->data[i]->draw_layer == DRAW_LAYER_LOW) {
            Entity *temp = entities->data[low_layer_count];

            entities->data[low_layer_count] = entities->data[i];
            entities->data[i] = temp;

            low_layer_count++;
        }
    }

    auto selection_sort = [&](size_t start, size_t end) {
        // Selection sort
        for (size_t i = start; i <= end; i++) {
            size_t lowest_index = i;

            for (size_t j = lowest_index+1; j <= end; j++) {
                Entity *current_lowest = entities->data[lowest_index];
                Entity *b = entities->data[j];

                Texture2D *current_lowest_texture = entity_get_texture(current_lowest);
                Texture2D *b_texture              = entity_get_texture(b);

                int b_height = 0;
                if (b_texture)
                    b_height = b_texture->height;
                else
                    b_height = b->base_collider.height;

                int current_lowest_height = 0;
                if (current_lowest_texture)
                    current_lowest_height = current_lowest_texture->height;
                else
                    current_lowest_height = current_lowest->base_collider.height;

                int b_bottom_y              = b->pos.y + b_height;
                int current_lowest_bottom_y = current_lowest->pos.y + current_lowest_height;

                if (b_bottom_y < current_lowest_bottom_y)
                    lowest_index = j;
            }

            // Swap i and lowest_index
            Entity *temp = entities->data[lowest_index];
            entities->data[lowest_index] = entities->data[i];
            entities->data[i] = temp;
        }
    };

    if (low_layer_count > 0)
        selection_sort(0, low_layer_count-1);

    selection_sort(low_layer_count, entity_count-1);
}

void default_entity_draw(Entity *e) {
    Texture2D *texture = entity_get_texture(e);
    if (texture) {
        DrawTexture(*texture, e->pos.x, e->pos.y, WHITE);
    }
}

void draw_popup(const char *text, Color color, Location location, int xoff, int yoff) {
    int pad = 6;

    Vector2 size = MeasureTextEx(atari_font, text, atari_font.baseSize, 1);
    Vector2 pos = {};

    if (location == Bottom) {
        pos = {
            render_width/2 - size.x/2,
            render_height - size.y - pad
        };
    } else if (location == Top) {
        pos = {
            render_width/2 - size.x/2,
            (float)pad
        };
    } else {
        pos = {
            render_width/2 - size.x/2,
            render_height/2 - size.y/2
        };
    }

    pos.x = (int)pos.x;
    pos.y = (int)pos.y;

    pos.x += xoff;
    pos.y += yoff;

    int darken = 60;
    Color dark = color;

    if (dark.r > 0 && dark.g > 0 && dark.b > 0) {
        dark.r = (uint8_t) max(0, (int)color.r - darken);
        dark.g = (uint8_t) max(0, (int)color.g - darken);
        dark.b = (uint8_t) max(0, (int)color.b - darken);

        pos.x++;
        pos.y++;

        DrawTextEx(atari_font, text, pos, atari_font.baseSize, 1, dark);

        pos.x--;
        pos.y--;
    }

    DrawTextEx(atari_font, text, pos, atari_font.baseSize, 1, color);
}

void draw_popup(const char *text, Color color, Location location) {
    draw_popup(text, color, location, 0, 0);
}

void draw_popup(const char *text, Color color) {
    draw_popup(text, color, Bottom, 0, 0);
}

void draw_popup(const char *text) {
    draw_popup(text, GOLD, Bottom, 0, 0);
}

void atari_update_and_draw_textbox(Game *game, float dt) {
    BeginTextureMode(game->textbox_target);

    RenderTexture2D *output = 0;
    switch (game->render_state) {
        case RENDER_STATE_ATARI: {
            output = &game->atari_render_target;
        } break;
        case RENDER_STATE_3D: {
            output = &game->render_target_3d;
        } break;
    }

    if (game->current)
        game->current = text_list_update_and_draw(output, &game->textbox_target, game->current, game, (uint8_t)game->textbox_alpha, game->include_text_in_target, dt);

}

void setup_text_scroll_sound(Text_List *list, char *speaker) {
    if (speaker) {
        const char *male_names[] = {
            "Bartender", "Guard", "Noah", "Mike",
            "Hunter", "Tyrell", "Trey", "Judas",
            "Lucas", "Siphor", "Tyrell", "Jason",
            "Matt", "Man",
        };

        const char *female_names[] = {
            "Eleanor", "Saira", "Jessica", "Amy",
            "Clarice", "Sherane", "Joanne", "Melody",
            "Aria", "Olivia", "Ana", "???",
            "Trisha", "Woman", "Woman 1", "Woman 2",
            "Woman 3",
        };

        if (strcmp(speaker, "Chase") == 0) {
            list->scroll_sound = SOUND_TEXT_SCROLL_CHASE;
            return;
        }

        for (int i = 0; i < StaticArraySize(male_names); i++) {
            if (strcmp(speaker, male_names[i]) == 0) {
                list->scroll_sound = SOUND_TEXT_SCROLL_MALE;
                return;
            }
        }

        for (int i = 0; i < StaticArraySize(female_names); i++) {
            if (strcmp(speaker, female_names[i]) == 0) {
                list->scroll_sound = SOUND_TEXT_SCROLL_FEMALE;
                return;
            }
        }
    }
}

// This must be called after setup_text_scroll_sound
void setup_text_color(Text_List *list, char *speaker) {
    if (speaker) {
        if (list->scroll_sound == SOUND_TEXT_SCROLL_FEMALE) {
            list->color = PINK;
        }

        if (list->scroll_sound == SOUND_TEXT_SCROLL_MALE) {
            list->color = BLUE;
        }

        if (strcmp(speaker, "???") == 0) {
            Color pink = {255, 133, 220, 255};
            list->color = pink;
        }
    
        switch (chapter) {
            case 0: {
                list->bg_color = {0, 0, 0, 127};
            } break;
            case 1: {
                list->bg_color = {0, 0, 0, 127};
            } break;
            case 2: {
                list->bg_color = {0, 0, 0, 200};
            } break;
        }
    }
}

void atari_text_list_init(Text_List *list, char *speaker,
                          char *line, float scroll_speed,
                          Text_List *next)
{
    list->font         = &atari_font;
    list->font_spacing = 1;
    list->scale        = 0.125;
    list->scroll_speed = scroll_speed;

    setup_text_scroll_sound(list, speaker);

    switch (chapter) {
        case 1: {
            list->color    = {68,40,0,255};
            list->bg_color = {232,204,124,255};
        } break;
        case 2: {
            /*
            list->color    = WHITE;
            list->bg_color = {50, 0, 0, 220};
            */
            setup_text_color(list, speaker);
        } break;
        default: {
            list->color = WHITE;
            list->bg_color = BLACK;
        } break;
    }
    list->center_text  = false;

    text_list_init(list, speaker, line, next);
}

void atari_choice_text_list_init(Text_List *list,
                                 char *speaker,
                                 char *text_string,
                                 String choices[],
                                 Text_List *next[],
                                 void (*hooks[])(void*),
                                 int choice_count)
{
    list->scroll_sound = SOUND_TEXT_SCROLL;

    atari_text_list_init(list, speaker, text_string, 30, 0);

    list->choice = true;
    list->choice_count = choice_count;
    list->choice_index = -1;

    memcpy(list->choices,
           choices,
           sizeof(choices[0]) * choice_count);
    memcpy(list->next,
           next,
           sizeof(next[0]) * choice_count);
    memcpy(list->callbacks,
           hooks,
           sizeof(hooks[0]) * choice_count);
}

void atari_mid_text_list_init(Text_List *list, char *line,
                              Text_List *next)
{
    list->font         = &atari_font;
    list->font_spacing = 2;
    list->scale        = 0.125;
    list->scroll_speed = 15;
    if (chapter == 1) 
        list->alpha_speed  = 1;
    else
        list->alpha_speed = 1;

    list->color        = BLACK;

    if (chapter == 1)
        list->backdrop_color={163, 116, 80, 255};
    else
        list->backdrop_color={180,180,180,255};

    list->center_text  = true;
    list->scroll_type  = EntireLine;
    list->render_type  = Bare;
    list->location     = Middle;
    list->take_keyboard_focus = false;

    text_list_init(list, 0, line, next);
}

void model_set_bilinear(Model *model) {
    for (int i = 0; i < model->materialCount; i++) {
        Material material = model->materials[i];

        // Loop through each texture type in the material (Diffuse, Specular, etc.)
        const int MAX_MATERIAL_MAPS = 12;
        for (int j = 0; j < MAX_MATERIAL_MAPS; j++) {
            Texture2D texture = material.maps[j].texture;

            if (texture.id > 0) {
                SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
            }
        }
    }

}

void model_set_shader(Model *model, Shader shader) {
    for (int i = 0; i < model->materialCount; i++) {
        model->materials[i].shader = shader;
    }
}

void update_camera_look(Camera3D *camera, float dt) {
    Vector2 look = input_movement_look(dt);
    CameraYaw(camera,   -look.x, false);
    CameraPitch(camera, -look.y, true, false, false);
}

void update_camera_3d(Camera3D *camera, float speed, bool allow_run, float dt) {
    float dir_x = input_movement_x_axis(dt);//key_right() - key_left();
    float dir_y = input_movement_y_axis(dt);//key_down()  - key_up();

    float run_speed = 7;

#ifdef DEBUG
    run_speed = 30;
#endif

    if (allow_run) {
        if (IsKeyDown(KEY_LEFT_SHIFT) ||
            (IsGamepadAvailable(0) && IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_2))) speed = run_speed;
        if (IsKeyDown(KEY_LEFT_ALT)) speed = 0.5f;
    }

    Vector3 saved = camera->target;
    CameraMoveForward(camera, -dir_y * speed * dt, true);
    CameraMoveRight(camera, dir_x * speed * dt, true);
    camera->target = saved;
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

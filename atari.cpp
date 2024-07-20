// This file contains a basic Game struct which has
// an array of entities.
//
// There are a few general functions that everyone
// will need when handling entities.
//
// The specific chapters are #included after these
// utility entity functions.
//
// Via switch statements, we dispatch to the specific
// chapter's code for updating and rendering the game.

// Loaded and unloaded for each chapter,
// so you can use different texture ID's.
struct Atari_Assets {
    Texture2D textures[32];
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
};

// All Atari chapters use the same Entity struct, because
// there's an Array<Entity*> in the main Game_Atari struct.
//
// You can include specialized variables by adding a new
// struct to the union.

struct Entity {
    Entity_Type type;
    int         texture_id;      // Index into atari_assets.textures[]
    Vector2     pos;
    Rectangle   base_collider;   // At origin. add pos to get collision box
    float       alarm[10];       // General purpose alarms
    int         dialogue_state;
    bool        has_dialogue;

    union {
        // Chapter 1 entities
        Chapter_1_Player  chap_1_player;
        Chapter_1_Node    chap_1_node;
        Chapter_1_Phone   chap_1_phone;

        // Chapter 2 entities
        Chapter_2_Door    chap_2_door;
        Chapter_2_Player  chap_2_player;
        Chapter_2_Penny   chap_2_penny;
    };
};

struct Game_Atari {
    RenderTexture2D render_target;
    RenderTexture2D textbox_target;

    Array<Entity*> entities;

    Text_List text[128];
    Text_List *current;

    bool queue_deinit_and_goto_intro;
    void *level; // Points to the specific chapter level struct.
};

Font atari_font;

Keyboard_Focus keyboard_focus(Game_Atari *game) {
    if (game->current != nullptr && game->current->take_keyboard_focus) {
        return KEYBOARD_FOCUS_TEXTBOX;
    } else {
        return NO_KEYBOARD_FOCUS;
    }
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

void entity_update_alarms(Entity *e, float dt) {
    for (size_t i = 0; i < StaticArraySize(e->alarm); i++) {
        if (e->alarm[i] > 0) {
            e->alarm[i] -= dt;
            if (e->alarm[i] <= 0)
                e->alarm[i] = 0;
        }
    }
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

    Rectangle a_rect = {
        a->pos.x, a->pos.y,
        a->base_collider.width,
        a->base_collider.height
    };

    Rectangle b_rect = {
        b->pos.x, b->pos.y,
        b->base_collider.width,
        b->base_collider.height
    };

    return CheckCollisionRecs(a_rect, b_rect);
}

bool is_entity_collidable(Entity *e) {
    if (e->type == ENTITY_CHAP_2_DOOR) {
        return e->chap_2_door.active;
    }

    if (e->type == ENTITY_FOOTSTEPS || e->type == ENTITY_BLOOD)
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

void apply_velocity_axis(Entity *e, Vector2 vel,
                         Array<Entity*> *entities, float *pos_axis)
{
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
    int width = entity_texture_width(e);
    int height = entity_texture_height(e);

    Rectangle a = {
        e->pos.x,
        e->pos.y,
        width,
        height
    };

    a = enlarge_rectangle(a, closeness);

    int player_width = entity_texture_width(player);
    int player_height = entity_texture_height(player);

    Rectangle player_rect = {
        player->pos.x,
        player->pos.y,
        player_width,
        player_height
    };

    bool within_region = CheckCollisionRecs(player_rect, a);

    return within_region;
}

bool can_open_dialogue(Game_Atari *game, Entity *e, Entity *player) {
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

        float distance_to_player = Vector2Length(Vector2Subtract(player->pos,
                                                                 current->pos));

        if (distance_to_player < closest_distance) {
            closest = current;
            closest_distance = distance_to_player;
        }
    }

    bool result = (closest == e);

    return result;
}

void atari_deinit(Game_Atari *game) {
    for (int i = 0; i < StaticArraySize(atari_assets.textures); i++) {
        if (atari_assets.textures[i].width) {
            UnloadTexture(atari_assets.textures[i]);
            memset(&atari_assets.textures[i], 0, sizeof(Texture2D));
        }
    }

    //memset(game->text, 0, sizeof(game->text));
    // Reset text
    for (int i = 0; i < StaticArraySize(game->text); i++) {
        game->text[i] = {};
    }
    game->current = 0;

    for (size_t i = 0; i < game->entities.length; i++) {
        Entity *e = game->entities.data[i];
        free(e);
    }

    game->entities.length = 0;

    free(game->level);
    game->level = 0;
}

void atari_queue_deinit_and_goto_intro(Game_Atari *game) {
    game->queue_deinit_and_goto_intro = true;
}

void sort_entities(Array<Entity*> *entities) {
    // Sort the entity array based on the bottom of the texture
    size_t entity_count = entities->length;

    // Selection sort
    for (int i = 0; i < entity_count; i++) {
        int lowest_index = i;

        for (int j = lowest_index+1; j < entity_count; j++) {
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
}

void atari_update_and_draw_textbox(Game_Atari *game) {
    BeginTextureMode(game->textbox_target);

    ClearBackground({0, 0, 0, 0});

    if (game->current)
        game->current = text_list_update_and_draw(game->current, game);

    EndTextureMode();
    BeginTextureMode(game->render_target);

    DrawTexturePro(game->textbox_target.texture,
                   {0, 0, (float)render_width, -(float)render_height},
                   {0, 0, (float)render_width, (float)render_height},
                   {0, 0},
                   0,
                   {255, 255, 255, 200});
}

void atari_text_list_init(Text_List *list, char *speaker,
                          char *line, float scroll_speed,
                          Text_List *next)
{
    list->font         = &atari_font;
    list->font_spacing = 1;
    list->scale        = 0.125;
    list->scroll_speed = scroll_speed;
    list->color        = BLACK;
    list->bg_color     = {186, 158, 78, 255};
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
    list->alpha_speed  = 0.5;
    list->color        = RED;
    list->center_text  = true;
    list->scroll_type  = EntireLine;
    list->render_type  = ShadowBackdrop;
    list->location     = Middle;
    list->take_keyboard_focus = false;

    text_list_init(list, 0, line, next);
}

#include "chapter_1.cpp"
#include "chapter_2.cpp"

void game_atari_init(Game_Atari *game) {
    assert(game->level == nullptr); // So we can make sure we had called deinit before

    printf("sizeof(Text_List) = %.2fMB, sizeof(game->text[]) = %.2fMB\n",
           sizeof(Text_List)  / 1024.0 / 1024.0,
           sizeof(game->text) / 1024.0 / 1024.0);

    render_width = 192;
    render_height = 160;

    atari_font = LoadFont("romulus.png");

    game->render_target  = LoadRenderTexture(render_width, render_height);
    game->textbox_target = LoadRenderTexture(render_width, render_height);

    switch (chapter) {
        case 1: {
            game->level = calloc(1, sizeof(Level_Chapter_1));
            chapter_1_level_init(game);
        } break;
        case 2: {
            game->level = calloc(1, sizeof(Level_Chapter_2));
            chapter_2_level_init(game);
        } break;
    }
}

void game_atari_run(Game_Atari *game) {
    float dt = GetFrameTime();

    switch (chapter) {
        case 1: chapter_1_update(game, dt); break;
        case 2: chapter_2_update(game, dt); break;
    }

    BeginDrawing();
    ClearBackground(BLACK);
    {
        BeginTextureMode(game->render_target);

        switch (chapter) {
            case 1: chapter_1_draw(game); break;
            case 2: chapter_2_draw(game); break;
        }

        atari_update_and_draw_textbox(game);

        EndTextureMode();
    }

    Rectangle destination = get_screen_rectangle();

    DrawTexturePro(game->render_target.texture,
                   {0, 0, (float)render_width, -(float)render_height},
                   destination,
                   {0, 0},
                   0,
                   WHITE);

    EndDrawing();

    if (game->queue_deinit_and_goto_intro) {
        atari_deinit(game);
        chapter++;
        set_game_mode(GAME_MODE_INTRO);
        game->queue_deinit_and_goto_intro = false;
    }
}
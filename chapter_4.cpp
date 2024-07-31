enum Chapter_4_State {
    CHAPTER_4_STATE_ATARI,
    CHAPTER_4_STATE_BED_1,
    CHAPTER_4_STATE_3D
};

struct Level_Chapter_4 {
    Chapter_4_State state;
    Entity *player;
};

Entity *chapter_4_make_entity  (Entity_Type type, float x, float y);
void    chapter_4_entity_update(Entity *entity, Game_Atari *game, float dt);
void    chapter_4_entity_draw  (Entity *entity, Game_Atari *game);

void chapter_4_init(Game_Atari *game) {
    Level_Chapter_4 *level = (Level_Chapter_4 *)game->level;

    Texture2D *textures = atari_assets.textures;
    textures[0] = load_texture("art/apartment2.png");
    textures[1] = load_texture("art/player.png");

    level->state = CHAPTER_4_STATE_ATARI;

    game->entities = make_array<Entity *>(20);

    level->player = chapter_4_make_entity(ENTITY_PLAYER, 84, 13);
    array_add(&game->entities, level->player);
}

void chapter_4_update(Game_Atari *game, float dt) {
    for (int i = 0; i < game->entities.length; i++) {
        Entity *e = game->entities.data[i];
        chapter_4_entity_update(e, game, dt);
    }
}

void chapter_4_draw(Game_Atari *game) {
    ClearBackground(BLACK);

    DrawTexture(atari_assets.textures[0], 0, 0, WHITE);

    for (int i = 0; i < game->entities.length; i++) {
        Entity *e = game->entities.data[i];
        chapter_4_entity_draw(e, game);
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

void chapter_4_entity_update(Entity *entity, Game_Atari *game, float dt) {
    switch (entity->type) {
        case ENTITY_PLAYER: {
            int dir_x = key_right() - key_left();
            int dir_y = key_down()  - key_up();

            const float speed = 60;

            Vector2 vel = { speed * dir_x * dt, speed * dir_y * dt };
            apply_velocity(entity, vel, &game->entities);

            entity->pos.x = Clamp(entity->pos.x, 0, render_width - entity_texture_width(entity));
            entity->pos.y = Clamp(entity->pos.y, 0, render_height - entity_texture_height(entity));
        } break;
    }
}

void chapter_4_entity_draw(Entity *entity, Game_Atari *game) {
    (void)game;
    default_entity_draw(entity);
}

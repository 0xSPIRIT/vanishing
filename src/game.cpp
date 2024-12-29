// Via switch statements, we dispatch to the specific
// chapter's code for updating and rendering the game.
//
// Functions you must implement if you're making
// a new chapter:
//
//   chapter_n_init(Game *game)
//   chapter_n_update(Game *game, float dt)
//   chapter_n_draw(Game *game)
//

#include "chapter_intro.cpp"
#include "chapter_1.cpp"
#include "chapter_2.cpp"
#include "chapter_3.cpp"
#include "chapter_4.cpp"
#include "chapter_5.cpp"
#include "chapter_6.cpp"
#include "chapter_epilogue.cpp"

void game_init(Game *game) {
    assert(game->level == nullptr); // assert we had called deinit before

    render_width  = DIM_ATARI_WIDTH;
    render_height = DIM_ATARI_HEIGHT;

    game->atari_render_target = LoadRenderTexture(render_width, render_height);
    game->render_target_3d = LoadRenderTexture(DIM_3D_WIDTH, DIM_3D_HEIGHT);
    game->textbox_target = LoadRenderTexture(render_width, render_height);

    post_process_init(&game->post_processing);

    if (!game->level_arena.buffer) {
        game->level_arena = make_arena(Megabytes(16));
    } else {
        arena_reset(&game->level_arena);
    }

    entity_allocator.first_free  = 0;
    entity_allocator.level_arena = &game->level_arena;

    uint64_t start = get_system_time();

    switch (chapter) {
        case 0: {
            game->level = arena_push(&game->level_arena, sizeof(Level_Chapter_Intro));
            chapter_intro_init(game);
        } break;
        case 1: {
            game->level = arena_push(&game->level_arena, sizeof(Level_Chapter_1));
            chapter_1_init(game);
        } break;
        case 2: {
            game->level = arena_push(&game->level_arena, sizeof(Level_Chapter_2));
            chapter_2_init(game);
        } break;
        case 3: {
            game->level = arena_push(&game->level_arena, sizeof(Level_Chapter_3));
            chapter_3_init(game);
        } break;
        case 4: {
            game->level = arena_push(&game->level_arena, sizeof(Level_Chapter_4));
            chapter_4_init(game);
        } break;
        case 5: {
            game->level = arena_push(&game->level_arena, sizeof(Level_Chapter_5));
            chapter_5_init(game);
        } break;
        case 6: {
            game->level = arena_push(&game->level_arena, sizeof(Level_Chapter_6));
            chapter_6_init(game);
        } break;
        case 7: {
            game->level = arena_push(&game->level_arena, sizeof(Level_Chapter_Epilogue));
            chapter_epilogue_init(game);
        } break;
    }

    uint64_t end = get_system_time();

    double diff_ms = 1000 * ((double)(end - start)) / global_system_timer_frequency;
    printf("Init took %.2fms\n", diff_ms);
}

void game_deinit(Game *game) {
    switch (chapter) {
        case 1: chapter_1_deinit(game);
        case 2: chapter_2_deinit(game);
        case 3: chapter_3_deinit(game);
    }

    entity_allocator.first_free  = 0;
    entity_allocator.level_arena = 0;

    for (int i = 0; i < StaticArraySize(atari_assets.textures); i++) {
        if (atari_assets.textures[i].width) {
            UnloadTexture(atari_assets.textures[i]);
            memset(&atari_assets.textures[i], 0, sizeof(Texture2D));
        }
    }

    // Reset text to the default Text
    for (int i = 0; i < StaticArraySize(game->text); i++)
        game->text[i] = {};

    game->current = nullptr;

    array_free(&game->entities);
    game->level = nullptr;
}

void game_run(Game *game) {
    float dt = get_frame_time_clamped();

    if (dt < 1.0f/144.0f) dt = 1.0f/144.0f;
    if (dt > 1.0f/30.0f)  dt = 1.0f/30.0f;

    tick_events(game, dt);

    if (!game_movie.movie) {
        switch (chapter) {
            case 0: chapter_intro_update(game, dt);    break;
            case 1: chapter_1_update(game, dt);        break;
            case 2: chapter_2_update(game, dt);        break;
            case 3: chapter_3_update(game, dt);        break;
            case 4: chapter_4_update(game, dt);        break;
            case 5: chapter_5_update(game, dt);        break;
            case 6: chapter_6_update(game, dt);        break;
            case 7: chapter_epilogue_update(game, dt); break;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        RenderTexture2D *target = 0;

        switch (game->render_state) {
            case RENDER_STATE_ATARI: {
                target = &game->atari_render_target;
            } break;
            case RENDER_STATE_3D: {
                target = &game->render_target_3d;
            } break;
            default: {
                assert(false);
            } break;
        }

        BeginTextureMode(*target);

        switch (chapter) {
            case 0: chapter_intro_draw(game);       break;
            case 1: chapter_1_draw(game);           break;
            case 2: chapter_2_draw(game, target);   break;
            case 3: chapter_3_draw(game, dt);       break;
            case 4: chapter_4_draw(game, dt);       break;
            case 5: chapter_5_draw(game);           break;
            case 6: chapter_6_draw(game);           break;
            case 7: chapter_epilogue_draw(game,dt); break;
        }

        update_and_draw_fade(game, &game->fader, dt);

        atari_update_and_draw_textbox(game, dt);

        EndTextureMode();

        // Apply post processing shader, then draw it to the screen, or
        // just pass it through if game->post_procesing.type == POST_PROCESSING_PASSTHROUGH
        post_process(&game->post_processing, &target->texture);
    } else {
        movie_run(&game_movie);
    }

#ifdef PLATFORM_WEB
    DrawFPS(10, 10);
#endif

    EndDrawing();

    if (game->queue_deinit_and_goto_intro) {
        game->queue_deinit_and_goto_intro = false;
        game_deinit(game);
        chapter++;
        set_game_mode(GAME_MODE_INTRO);
    }
}

struct Game_Intro {
    Text_List text[32];
    Text_List *current_text_list;

    RenderTexture2D render_target;
};

void intro_init_text_list(Text_List *list,
                          char *string,
                          Text_List *next)
{
    *list = Text_List();
    list->center_text = true;
    list->render_type = Render_Type::Bare;
    list->location = Location::Middle;
    list->scroll_type = Scroll_Type::EntireLine;
    list->alpha_speed = 3;

    text_list_init(list, 0, string, next);
}

void game_intro_init(Game_Intro *game) {
    render_width = 640;
    render_height = 480;

    game->render_target = LoadRenderTexture(render_width, render_height);

    switch (chapter) {
        case 1: {
            intro_init_text_list(&game->text[0], "1. Dysania", &game->text[1]);
            intro_init_text_list(&game->text[1], "Chase sat despondently at a cafe on a bright\nSaturday morning.\rHe listened to the hustle and bustle of the commuters,\rsaw their happy faces eating breakfast.", &game->text[2]);
            intro_init_text_list(&game->text[2], "Laughter erupted at a nearby table--\ra group of friends.", &game->text[3]);
            intro_init_text_list(&game->text[3], "A sinking feeling engulfed his entire body.", &game->text[4]);
            intro_init_text_list(&game->text[4], "He tried looking outside;\rtrying to feel something-- anything!", &game->text[5]);
            intro_init_text_list(&game->text[5], "But couldn't.", &game->text[6]);
            intro_init_text_list(&game->text[6], "He clenched the cross that hung around his neck.\rHe went to sleep that night\nparticularly perturbed.", &game->text[7]);
            intro_init_text_list(&game->text[7], "He twisted and turned in his bed\nas he slowly dozed off...", nullptr);
        } break;
        case 2: {
            intro_init_text_list(&game->text[0], "2. Dinner Party", 0);
        } break;
        case 3: {
            intro_init_text_list(&game->text[0], "3. Repetitions", 0);
        } break;
    }

    game->current_text_list = &game->text[0];
}

void game_intro_run(Game_Intro *game) {
    BeginDrawing();

    ClearBackground(BLACK);

    BeginTextureMode(game->render_target);

    ClearBackground({82, 75, 66, 255});
    if (game->current_text_list)
        game->current_text_list = text_list_update_and_draw(game->current_text_list);

    if (game->current_text_list == nullptr) {
        set_game_mode(GAME_MODE_ATARI);
    }

    EndTextureMode();

    Rectangle destination = get_screen_rectangle();

    DrawTexturePro(game->render_target.texture,
                   {0, 0, (float)render_width, -(float)render_height},
                   destination,
                   {0, 0},
                   0,
                   WHITE);

    EndDrawing();
}



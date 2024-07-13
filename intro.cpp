struct Game_Intro {
    Text_List text[9];
    Text_List *current_text_list;
};

void game_intro_init(Game_Intro *game) {
    init_text_list_default(&game->text[0], "Her", "Okay okay, I got another one.\rThis is gonna be a good one, right?", &game->text[1]);
    init_text_list_default(&game->text[1], "Him", "Aight let's see watchu got.", &game->text[2]);
    init_text_list_default(&game->text[2], "Her", "Uh, so, a polar bear walks into a bar.", &game->text[3]);
    init_text_list_default(&game->text[3], "Her", "And he says to the bartender,\r\"I'll have a rum ....... and coke.\"\rThe bartender asks, \"What's with the big pause?\"", &game->text[4]);
    init_text_list_default(&game->text[4], "Her", "The bear shrugs and says \"I dunno I was born with them.\"", &game->text[5]);
    init_text_list_default(&game->text[5], "Him", "Bahahah that's so bad it's crazy.\r...\r...\r... Okay Keep 'em coming.", &game->text[6]);

    Text_List *t = &game->text[6];

    t->font         = nullptr;
    t->font_spacing = 0;
    t->render_type  = Render_Type::Bare;
    t->scroll_type  = Scroll_Type::EntireLine;
    t->location     = Location::Middle;
    t->color        = RED;
    t->center_text  = true;
    t->scale        = 1;

    text_list_init(t, nullptr, "PROMETHEUS STOLE FIRE FROM THE GODS AND GAVE IT TO MAN.\rFOR THIS HE WAS CHAINED TO A ROCK AND TORTURED FOR ETERNITY.", nullptr);

    game->current_text_list = &game->text[6];
}

void game_intro_run(Game_Intro *game) {
    render_width = GetRenderWidth();
    render_height = GetRenderHeight();

    BeginDrawing();

    ClearBackground(BLACK);

    if (game->current_text_list)
        game->current_text_list = text_list_update_and_draw(game->current_text_list);

    if (game->current_text_list == nullptr)
        set_game_mode(GAME_MODE_ATARI);

    EndDrawing();
}



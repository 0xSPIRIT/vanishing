#define OPTION_COUNT 3

enum Titlescreen_Options {
    OPTION_PLAY,
    OPTION_SETTINGS,
    OPTION_EXIT
};

struct Titlescreen {
    Model scene;
    Model clerk, head;
    Camera3D camera;

    char *options[OPTION_COUNT];
    int   option_current;

    RenderTexture target;
};

void titlescreen_init(Titlescreen *tit) {
    render_width = DIM_3D_WIDTH;
    render_height = DIM_3D_HEIGHT;
    tit->target = LoadRenderTexture(render_width, render_height);

    tit->scene = load_model("models/train_station2.glb");
    tit->clerk = load_model("models/guy.glb");
    tit->head  = load_model("models/pyramid_head.glb");

    tit->camera.fovy = 50;
    tit->camera.up = { 0, 1, 0 };
    tit->camera.projection = CAMERA_PERSPECTIVE;
    tit->camera.position = {-3.18f, 1.67f, 4.24f};
    tit->camera.target =   {-49.60f, 6.83f, -13.59f};

    char *options[] = { "Play", "Options", "Exit" };
    memcpy(tit->options, options, sizeof(options));
    tit->option_current = 0;
}

void titlescreen_update_and_draw(Titlescreen *tit) {
    const Vector3 clerk_pos = { -9.4f, 0, 0 };

    static float dark_screen_timer = 0;

    if (key_up_pressed())
        tit->option_current--;
    if (key_down_pressed())
        tit->option_current++;

    if (tit->option_current < 0)
        tit->option_current = OPTION_COUNT-1;
    if (tit->option_current > OPTION_COUNT-1)
        tit->option_current = 0;

    if (is_action_pressed()) {
        switch (tit->option_current) {
            case OPTION_PLAY: {
                dark_screen_timer = 2;
            } break;
            case OPTION_SETTINGS: {
                // TODO
            } break;
            case OPTION_EXIT: {
                exit(0);
            } break;
        }
    }

    if (dark_screen_timer != 0) {
        dark_screen_timer -= GetFrameTime();

        if (dark_screen_timer <= 0) {
            set_game_mode(GAME_MODE_ATARI);
        }
    }


    Vector2 clerk_p = { clerk_pos.x, clerk_pos.z };
    Vector2 player_p = { tit->camera.position.x, tit->camera.position.z };
    float to_angle = atan2f(clerk_p.y - player_p.y, clerk_p.x - player_p.x);
    to_angle = RAD2DEG * -to_angle - 90;
    float direct_angle = to_angle;

    float time = GetTime();
    float sine = 1.5f * (sinf(time) + sinf(2 * time) + sinf(2.5f * time)); 
    to_angle += sine;

    BeginDrawing();

    ClearBackground(BLACK);

    if (dark_screen_timer == 0) {
        BeginTextureMode(tit->target);

        ClearBackground(BLACK);

        {
            BeginMode3D(tit->camera);

            DrawModel(tit->scene, {}, 1, WHITE);
            DrawModelEx(tit->clerk, clerk_pos, {0,1,0},
                        direct_angle, {1,1,1}, WHITE);
            DrawModelEx(tit->head, Vector3Add(clerk_pos, {0,1.55f,0}), {0,1,0}, to_angle, {0.85f,0.85f,0.85f}, WHITE);

            EndMode3D();
        }

        {
            auto text = [&](int off, Color a) {
                DrawTextEx(titlescreen_font,
                           "Veil",
                           {20.f+off, 36.f+off},
                           titlescreen_font.baseSize,
                           0,
                           a);
            };

            text(3, {0,0,0,255});
            text(2, {64,64,64,255});
            text(1, {128,128,128,255});
            text(0, WHITE);
        }

        {
            auto text = [&](int i, char *message, bool selected, Color c, float offset) {
                Font *font = &titlescreen_minor_font;

                float x_offs = 0;

                if (selected)
                    x_offs = 5;

                int x = int(x_offs + offset + 20);
                int y = int(offset + 5.25 * render_height / 8 + i * font->baseSize - 10);

                DrawTextEx(*font,
                           message,
                           {(float)x, (float)y},
                           font->baseSize,
                           0,
                           c);
            };

            for (int i = 0; i < OPTION_COUNT; i++) {
                char *message = tit->options[i];

                bool selected = (i == tit->option_current);

                Color color = {120,120,120,255};
                if (selected)
                    color = WHITE;

                text(i, message, selected, ColorScale(color, 0.00f), 3);
                text(i, message, selected, ColorScale(color, 0.50f), 2);
                text(i, message, selected, ColorScale(color, 0.75f), 1);
                text(i, message, selected, ColorScale(color, 1.00f), 0);
            };
        }

        EndTextureMode();

        Rectangle destination = get_screen_rectangle();
        DrawTexturePro(tit->target.texture,
                       {0, 0, (float)render_width, (float)-render_height}, // destination.x and y == 0
                       destination,
                       {0, 0},
                       0,
                       WHITE);
    }

    EndDrawing();
}

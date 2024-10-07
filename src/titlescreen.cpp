struct Titlescreen {
    Model scene;
    Model clerk, head;
    Camera3D camera;

    RenderTexture target;
};

void titlescreen_init(Titlescreen *tit) {
    render_width = DIM_3D_WIDTH;
    render_height = DIM_3D_HEIGHT;
    tit->target = LoadRenderTexture(render_width, render_height);

    tit->scene = LoadModel(RES_DIR "models/train_station2.glb");
    tit->clerk = LoadModel(RES_DIR "models/guy.glb");
    tit->head = LoadModel(RES_DIR "models/pyramid_head.glb");

    tit->camera.fovy = 50;
    tit->camera.up = { 0, 1, 0 };
    tit->camera.projection = CAMERA_PERSPECTIVE;
    tit->camera.position = {-3.18f, 1.67f, 4.24f};
    tit->camera.target =   {-49.60f, 6.83f, -13.59f};
}

void titlescreen_update_and_draw(Titlescreen *tit) {
    const Vector3 clerk_pos = { -9.4f, 0, 0 };

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
                       "Gethsemane",
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

    EndTextureMode();

    Rectangle destination = get_screen_rectangle();
    DrawTexturePro(tit->target.texture,
                   {0, 0, (float)render_width, (float)-render_height}, // destination.x and y == 0
                   destination,
                   {0, 0},
                   0,
                   WHITE);

    EndDrawing();
}

struct Game_3D {
    Camera3D camera;
    Shader shader;
    Model scene;

    float rotation; // degrees

    RenderTexture2D render_target;
};

void game_3d_init(Game_3D *game) {
    render_width = 320;
    render_height = 240;
    game->render_target = LoadRenderTexture(render_width, render_height);

    game->scene = LoadModel("models/scene.glb");

    game->rotation = 0;

    game->camera.position = {0, 2, 0};
    game->camera.target   = {1, 2, 0};
    game->camera.up       = {0, 1, 0};
    game->camera.fovy     = 62;
    game->camera.projection = CAMERA_PERSPECTIVE;

    game->shader = LoadShader("shaders/basic.vs", "shaders/basic.fs");

    game->shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(game->shader, "matModel");
    game->shader.locs[SHADER_LOC_VECTOR_VIEW]  = GetShaderLocation(game->shader, "viewPos");

    for (int i = 0; i < game->scene.materialCount; i++) {
        game->scene.materials[i].shader = game->shader;
    }

    int ambientLoc = GetShaderLocation(game->shader, "ambient");
    float data[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    SetShaderValue(game->shader, ambientLoc, data, SHADER_UNIFORM_VEC4);

    float fogDensity = 0.02f;
    int fogDensityLoc = GetShaderLocation(game->shader, "fogDensity");
    SetShaderValue(game->shader, fogDensityLoc, &fogDensity, SHADER_UNIFORM_FLOAT);

    CreateLight(LIGHT_POINT, { 0, 6, 6 }, Vector3Zero(), ORANGE, game->shader);
}

void update_camera(Camera3D *camera, Vector3 axis, float angle) {
    int dir_x = key_right() - key_left();
    int dir_y = key_down() - key_up();

    float speed = 5;
    float dt = GetFrameTime();
    
    (void)axis, angle;

    CameraMoveForward(camera, -dir_y * speed * dt, true);
    CameraMoveRight(camera, dir_x * speed * dt, true);

    Vector2 mouse = GetMouseDelta();

    const float sensitivity = 0.005f;

    mouse.x *= sensitivity;
    mouse.y *= sensitivity;

    CameraYaw(camera, -mouse.x, false);
    CameraPitch(camera, -mouse.y, false, false, false);
}

void game_3d_run(Game_3D *game) {
    Vector3 stored_camera_pos = game->camera.position;
    Vector3 stored_camera_target = game->camera.target;

    Vector3 scene_rotation_axis = {1, 0, 0};
    Vector3 model_down = Vector3RotateByAxisAngle({0, -1, 0}, scene_rotation_axis, Radians(game->rotation));
    Vector3 model_up   = Vector3RotateByAxisAngle({0, +1, 0}, scene_rotation_axis, Radians(game->rotation));

    update_camera(&game->camera, scene_rotation_axis, game->rotation);

    Ray ray = {};

    ray.position = game->camera.position;
    ray.direction = model_down;

    bool hit = false;

    for (int i = 0; i < game->scene.meshCount; i++) {
        Mesh mesh = game->scene.meshes[i];
        Matrix transform = MatrixRotate(scene_rotation_axis, Radians(game->rotation));
        RayCollision result = GetRayCollisionMesh(ray, mesh, transform);

        if (result.hit) {
            Vector3 desired_camera_position = Vector3Add(result.point, Vector3Scale(model_up, 2));

            Vector3 delta = Vector3Subtract(desired_camera_position, game->camera.position);

            game->camera.position = desired_camera_position;
            game->camera.target = Vector3Add(game->camera.target, delta);

            hit = true;
        }
    }

    if (!hit) {
        // Revert position
        game->camera.position = stored_camera_pos;
        game->camera.target = stored_camera_target;
    }

    SetShaderValue(game->shader, game->shader.locs[SHADER_LOC_VECTOR_VIEW], &game->camera.position.x, SHADER_UNIFORM_VEC3);

    BeginDrawing();

    ClearBackground(BLACK);

    BeginTextureMode(game->render_target);
    {
        ClearBackground(BLACK);

        BeginMode3D(game->camera);
        BeginShaderMode(game->shader);

        DrawModelEx(game->scene, {}, scene_rotation_axis, game->rotation, {1, 1, 1}, WHITE);

        EndShaderMode();

        EndMode3D();
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


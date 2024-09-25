enum Post_Process_Shader_Type {
    POST_PROCESSING_PASSTHROUGH,
    POST_PROCESSING_VHS
};

enum Vhs_Intensity {
    VHS_INTENSITY_LOW,
    VHS_INTENSITY_MEDIUM,
    VHS_INTENSITY_MEDIUM_HIGH,
    VHS_INTENSITY_HIGH
};

struct Post_Processing_Vhs {
    Shader shader;

    int    u_time;
    int    u_scan_intensity;
    int    u_noise_intensity;
    int    u_abberation_intensity;

    float  scan_intensity;
    float  noise_intensity;
    float  abberation_intensity;
};

struct Post_Processing {
    Post_Process_Shader_Type type;
    RenderTexture2D          target; // This is the size of the window, and is resized when a change in size is detected.
    RenderTexture2D          game_target; // This is the internal resolution.

    Post_Processing_Vhs vhs;
};

void post_process_init(Post_Processing *post) {
    post->vhs.shader = LoadShader(0, RES_DIR "shaders/vhs.fs");

    post->vhs.u_time                 = GetShaderLocation(post->vhs.shader, "time");
    post->vhs.u_scan_intensity       = GetShaderLocation(post->vhs.shader, "scan_intensity");
    post->vhs.u_noise_intensity      = GetShaderLocation(post->vhs.shader, "noise_intensity");
    post->vhs.u_abberation_intensity = GetShaderLocation(post->vhs.shader, "abberation_intensity");

    post->vhs.scan_intensity = 1;
    post->vhs.noise_intensity = 1;
    post->vhs.abberation_intensity = 1;

    post->type = POST_PROCESSING_PASSTHROUGH;
}

void post_process_vhs_set_intensity(Post_Processing_Vhs *vhs, Vhs_Intensity intensity) {
    switch (intensity) {
        case VHS_INTENSITY_LOW: {
            vhs->scan_intensity = 0.7f;
            vhs->noise_intensity = 0.3f;
            vhs->abberation_intensity = 0.7f;
        } break;
        case VHS_INTENSITY_MEDIUM: {
            vhs->scan_intensity = 1;
            vhs->noise_intensity = 0.9f;
            vhs->abberation_intensity = 1;
        } break;
        case VHS_INTENSITY_MEDIUM_HIGH: {
            vhs->scan_intensity = 2;
            vhs->noise_intensity = 1.3f;
            vhs->abberation_intensity = 1.1f;
        } break;
        case VHS_INTENSITY_HIGH: {
            vhs->scan_intensity = 3;
            vhs->noise_intensity = 3;
            vhs->abberation_intensity = 2;
        } break;
    }
}

void post_process_vhs_uniforms(Post_Processing_Vhs *vhs) {
    /*
    for (int i = 0; i < 4; i++) {
        if (IsKeyPressed(KEY_ONE + i)) {
            post_process_vhs_set_intensity(vhs, (Vhs_Intensity)i);
        }
    }
    */

    // we can modulate these values over time

    float curtime = GetTime();
    SetShaderValue(vhs->shader, vhs->u_time, &curtime, SHADER_UNIFORM_FLOAT);
    SetShaderValue(vhs->shader, vhs->u_scan_intensity, &vhs->scan_intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(vhs->shader, vhs->u_noise_intensity, &vhs->noise_intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(vhs->shader, vhs->u_abberation_intensity, &vhs->abberation_intensity, SHADER_UNIFORM_FLOAT);
}

// Apply post processing shader, then draw it to the screen.
void post_process(Post_Processing *post, Texture2D *game_texture) {
    Shader *current_shader = 0;

    switch (post->type) {
        case POST_PROCESSING_VHS: { current_shader = &post->vhs.shader; } break;
    }

    if (IsKeyPressed(KEY_K)) {
        UnloadShader(*current_shader);
        post_process_init(post);
    }

    if (post->target.texture.width != GetRenderWidth() ||
        post->target.texture.height != GetRenderHeight())
    {
        if (IsRenderTextureReady(post->target))
            UnloadRenderTexture(post->target);

        post->target = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    }

    if (post->game_target.texture.width != render_width ||
        post->game_target.texture.height != render_height)
    {
        if (IsRenderTextureReady(post->game_target))
            UnloadRenderTexture(post->game_target);

        post->game_target = LoadRenderTexture(render_width, render_height);

        printf("resized to %d, %d\n", render_width, render_height);
    }

    if (post->type == POST_PROCESSING_PASSTHROUGH) {
        Rectangle destination = get_screen_rectangle();

        DrawTexturePro(*game_texture,
                       {0, 0, (float)render_width, -(float)render_height},
                       destination,
                       {0, 0},
                       0,
                       WHITE);
    } else {
        const bool apply_shader_to_internal_resolution = true;

        switch (post->type) {
            case POST_PROCESSING_VHS: {
                post_process_vhs_uniforms(&post->vhs);
            } break;
            default: {
                assert(false);
            } break;
        }

        BeginTextureMode(post->game_target);

        if (apply_shader_to_internal_resolution) 
            BeginShaderMode(*current_shader);

        ClearBackground(BLACK);
        DrawTexturePro(*game_texture,
                       {0, 0, (float)render_width, -(float)render_height},
                       {0, 0, (float)render_width,  (float)render_height},
                       { 0, 0 },
                       0,
                       WHITE);

        if (apply_shader_to_internal_resolution) 
            EndShaderMode();

        EndTextureMode();

        BeginTextureMode(post->target);

        if (!apply_shader_to_internal_resolution) 
            BeginShaderMode(*current_shader);

        ClearBackground(BLACK);

        Rectangle destination = get_screen_rectangle();

        DrawTexturePro(post->game_target.texture,
                       {0, 0, (float)render_width, -(float)render_height},
                       destination,
                       {0, 0},
                       0,
                       WHITE);

        if (!apply_shader_to_internal_resolution) 
            EndShaderMode();

        EndTextureMode();

        // Draw the game to the screen.
        DrawTexturePro(post->target.texture,
                       {0, 0, (float)GetRenderWidth(), (float)-GetRenderHeight()}, // destination.x and y == 0
                       {0, 0, (float)GetRenderWidth(), (float)GetRenderHeight()},
                       {0, 0},
                       0,
                       WHITE);
    }
}

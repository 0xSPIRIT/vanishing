enum Post_Process_Shader_Type {
    POST_PROCESSING_PASSTHROUGH,
    POST_PROCESSING_VHS,
    POST_PROCESSING_CRT,
    POST_PROCESSING_BLOOM,
};

enum Vhs_Intensity {
    VHS_INTENSITY_LOW,
    VHS_INTENSITY_MEDIUM,
    VHS_INTENSITY_MEDIUM_HIGH,
    VHS_INTENSITY_HIGH
};

struct Post_Processing_Bloom {
    Shader shader;

    int    u_time;
    int    u_bloom_intensity;
    int    u_vignette_mix;
    int    u_window_size;

    float   bloom_intensity;
    float   vignette_mix;
};

struct Post_Processing_Vhs {
    Shader shader;

    int    u_time;
    int    u_scan_intensity;
    int    u_noise_intensity;
    int    u_abberation_intensity;
    int    u_mix_factor;
    int    u_vignette_intensity;
    int    u_vignette_mix;

    float  scan_intensity;
    float  noise_intensity;
    float  abberation_intensity;
    float  vignette_intensity;
    float  vignette_mix;
    float  mix_factor;
};

struct Post_Processing_Crt {
    Shader shader;
    int    u_time;
    int    u_do_scanline_effect;
    int    u_do_warp_effect;
    int    u_abberation_intensity;
    int    u_vignette_intensity;
    int    u_scanline_alpha;
    int    u_vignette_mix;
    int    u_noise_intensity;
    int    u_red_offset, u_green_offset, u_blue_offset;
    int    u_do_spin;
    int    u_do_wiggle;

    int    do_scanline_effect;
    int    do_warp_effect;
    int    do_spin;
    int    do_wiggle;

    float  abberation_intensity;
    float  vignette_intensity;
    float  scanline_alpha;
    float  vignette_mix;
    float  noise_intensity;
    float  red_offset, green_offset, blue_offset; // for chromatic abberation

    bool   should_spin_randomly;
    float  last_spin_time;
    float  spin_duration;
    float  time_to_spin;
};

struct Post_Processing {
    Post_Process_Shader_Type type;
    RenderTexture2D          target; // This is the size of the window, and is resized when a change in size is detected.
    RenderTexture2D          game_target; // This is the internal resolution.

    Post_Processing_Vhs   vhs;
    Post_Processing_Crt   crt;
    Post_Processing_Bloom bloom;
};

void post_process_init(Post_Processing *post) {
    post->vhs.shader = LoadShader(0, RES_DIR "shaders/vhs.fs");

    post->type = POST_PROCESSING_PASSTHROUGH;

    Post_Processing_Vhs *vhs = &post->vhs;

    vhs->u_time                 = GetShaderLocation(vhs->shader, "time");
    vhs->u_scan_intensity       = GetShaderLocation(vhs->shader, "scan_intensity");
    vhs->u_noise_intensity      = GetShaderLocation(vhs->shader, "noise_intensity");
    vhs->u_abberation_intensity = GetShaderLocation(vhs->shader, "abberation_intensity");
    vhs->u_mix_factor           = GetShaderLocation(vhs->shader, "mix_factor");
    vhs->u_vignette_intensity   = GetShaderLocation(vhs->shader, "vignette_intensity");
    vhs->u_vignette_mix         = GetShaderLocation(vhs->shader, "vignette_mix");

    vhs->scan_intensity = 1;
    vhs->noise_intensity = 1;
    vhs->abberation_intensity = 1;
    vhs->mix_factor = 1;
    vhs->vignette_intensity = 0;
    vhs->vignette_mix = 1;


    Post_Processing_Crt *crt = &post->crt;

    crt->do_scanline_effect = 1;
    crt->abberation_intensity = 1;
    crt->vignette_intensity = 1;
    crt->do_wiggle = 1;

    crt->scanline_alpha = 0.25;
    crt->vignette_mix = 1;
    crt->noise_intensity = 0.25;

    crt->red_offset   = -0.009f;
    crt->green_offset = +0.006f;
    crt->blue_offset  = -0.006f;

    crt->shader                 = LoadShader(0, RES_DIR "shaders/crt.fs");
    crt->u_time                 = GetShaderLocation(crt->shader, "time");
    crt->u_do_scanline_effect   = GetShaderLocation(crt->shader, "do_scanline_effect");
    crt->u_do_warp_effect       = GetShaderLocation(crt->shader, "do_warp_effect");
    crt->u_abberation_intensity = GetShaderLocation(crt->shader, "abberation_intensity");
    crt->u_vignette_intensity   = GetShaderLocation(crt->shader, "vignette_intensity");
    crt->u_scanline_alpha       = GetShaderLocation(crt->shader, "scanline_alpha");
    crt->u_vignette_mix         = GetShaderLocation(crt->shader, "vignette_mix");
    crt->u_noise_intensity      = GetShaderLocation(crt->shader, "noise_intensity");
    crt->u_red_offset           = GetShaderLocation(crt->shader, "redOffset");
    crt->u_green_offset         = GetShaderLocation(crt->shader, "greenOffset");
    crt->u_blue_offset          = GetShaderLocation(crt->shader, "blueOffset");
    crt->u_do_spin              = GetShaderLocation(crt->shader, "do_spin");
    crt->u_do_wiggle            = GetShaderLocation(crt->shader, "do_wiggle");

    Post_Processing_Bloom *bloom = &post->bloom;

    bloom->bloom_intensity = 4;
    bloom->vignette_mix = 0.35f;

    bloom->shader            = LoadShader(0, RES_DIR "shaders/bloom.fs");
    bloom->u_time            = GetShaderLocation(bloom->shader, "time");
    bloom->u_bloom_intensity = GetShaderLocation(bloom->shader, "bloom_intensity");
    bloom->u_vignette_mix    = GetShaderLocation(bloom->shader, "vignette_mix");
    bloom->u_window_size     = GetShaderLocation(bloom->shader, "window_size");
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
    float curtime = GetTime();

    Shader shader = vhs->shader;

    SetShaderValue(shader, vhs->u_time, &curtime, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, vhs->u_scan_intensity, &vhs->scan_intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, vhs->u_noise_intensity, &vhs->noise_intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, vhs->u_abberation_intensity, &vhs->abberation_intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, vhs->u_mix_factor, &vhs->mix_factor, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, vhs->u_vignette_intensity, &vhs->vignette_intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, vhs->u_vignette_mix, &vhs->vignette_mix, SHADER_UNIFORM_FLOAT);
}

void post_process_crt_uniforms(Post_Processing_Crt *crt) {
    float curtime = GetTime();
    SetShaderValue(crt->shader, crt->u_time, &curtime, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crt->shader, crt->u_do_scanline_effect, &crt->do_scanline_effect, SHADER_UNIFORM_INT);
    SetShaderValue(crt->shader, crt->u_do_warp_effect, &crt->do_warp_effect, SHADER_UNIFORM_INT);
    SetShaderValue(crt->shader, crt->u_abberation_intensity, &crt->abberation_intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crt->shader, crt->u_vignette_intensity, &crt->vignette_intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crt->shader, crt->u_scanline_alpha, &crt->scanline_alpha, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crt->shader, crt->u_vignette_mix, &crt->vignette_mix, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crt->shader, crt->u_noise_intensity, &crt->noise_intensity, SHADER_UNIFORM_FLOAT);

    SetShaderValue(crt->shader, crt->u_red_offset, &crt->red_offset, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crt->shader, crt->u_green_offset, &crt->green_offset, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crt->shader, crt->u_blue_offset, &crt->blue_offset, SHADER_UNIFORM_FLOAT);

    SetShaderValue(crt->shader, crt->u_do_wiggle, &crt->do_wiggle, SHADER_UNIFORM_INT);

    if (crt->should_spin_randomly) {
        if (!crt->do_spin) {
            if (curtime - crt->last_spin_time >= crt->time_to_spin) {
                crt->do_spin = true;
                crt->last_spin_time = curtime;
                crt->spin_duration = rand_range(2.0f/60.0f, 20.0f/60.0f);
            }
        } else if (curtime - crt->last_spin_time >= crt->spin_duration) {
            crt->do_spin = false;
            crt->last_spin_time = curtime;
            crt->time_to_spin = rand_range(0.2f, 5.f);
        }
    }
    SetShaderValue(crt->shader, crt->u_do_spin, &crt->do_spin, SHADER_UNIFORM_INT);
}

void post_process_bloom_uniforms(Post_Processing_Bloom *bloom) {
    float curtime = GetTime();
    Vector2 window_size = { render_width, render_height };

    SetShaderValue(bloom->shader, bloom->u_time,            &curtime,                SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloom->shader, bloom->u_bloom_intensity, &bloom->bloom_intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloom->shader, bloom->u_vignette_mix,    &bloom->vignette_mix,    SHADER_UNIFORM_FLOAT);
    SetShaderValue(bloom->shader, bloom->u_window_size,     &window_size,            SHADER_UNIFORM_VEC2);
}

// Apply post processing shader, then draw it to the screen.
void post_process(Post_Processing *post, Texture2D *game_texture) {
    Shader *current_shader = 0;

    switch (post->type) {
        case POST_PROCESSING_VHS:   current_shader = &post->vhs.shader; break;
        case POST_PROCESSING_CRT:   current_shader = &post->crt.shader; break;
        case POST_PROCESSING_BLOOM: current_shader = &post->bloom.shader; break;
    }

#ifdef DEBUG
    if (current_shader && IsKeyPressed(KEY_K)) {
        Post_Process_Shader_Type type = post->type;
        UnloadShader(*current_shader);
        post_process_init(post);
        post->type = type;
    }
#endif

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
            case POST_PROCESSING_CRT: {
                post_process_crt_uniforms(&post->crt);
            } break;
            case POST_PROCESSING_BLOOM: {
                post_process_bloom_uniforms(&post->bloom);
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

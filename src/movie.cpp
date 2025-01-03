#if defined(PLATFORM_WEB)
  #define PROFILE 1
#else
  #define PROFILE 0
#endif

void movie_handle_frame(plm_t *mpeg, plm_frame_t *frame, void *user) {
    (void)mpeg;
    Movie *movie = (Movie *)user;

#if PROFILE
    double start = GetTime();
#endif

    Image image = GenImageColor(frame->width, frame->height, BLACK);

    plm_frame_to_rgba(frame, (uint8_t*)image.data, 4 * frame->width);

    // for some reason we have to do this for emscripten
    UnloadTexture(movie->texture2);
    movie->texture2 = movie->texture;
    movie->texture = LoadTextureFromImage(image);

    UnloadImage(image);

#if PROFILE
    double end = GetTime();

    double time = end - start;

    printf("Frame took %fms\n", 1000 * time);
#endif
}

void movie_load_video(Movie *movie, const char *movie_file, const char *audio_file) {
    if (movie->plm)
        plm_destroy(movie->plm);

    movie->plm = plm_create_with_filename(movie_file);

    if (!movie->plm) {
        fprintf(stderr, "Couldn't open %s\n", movie_file);
        exit(1);
    }

    if (IsMusicValid(movie->audio))
        UnloadMusicStream(movie->audio);

    movie->audio = LoadMusicStream(audio_file);
    if (!IsMusicValid(movie->audio)) {
        fprintf(stderr, "Couldn't open %s\n", audio_file);
        exit(1);
    }

    movie->framerate = plm_get_framerate(movie->plm);
    printf("Framerate: %f\n", movie->framerate);

    plm_set_video_decode_callback(movie->plm, movie_handle_frame, movie);
    plm_set_loop(movie->plm, false);

    movie->audio.looping = false;

    PlayMusicStream(movie->audio);

    movie->last_time = -1;

    Image image = GenImageColor(plm_get_width(movie->plm),
                                plm_get_height(movie->plm),
                                BLACK);
    movie->texture = LoadTextureFromImage(image);
    UnloadImage(image);
}

void movie_free(Movie *movie) {
    if (movie->plm == nullptr)
        return;

    plm_destroy(movie->plm);
    movie->movie = MOVIE_OFF;

    movie->end_movie_callback = 0;
}

void movie_run(Movie *movie) {
#if PROFILE
    double start = GetTime();
#endif
    movie->frames++;

    if (plm_has_ended(movie->plm)) {
        movie->movie = MOVIE_OFF;
    }

    bool skip = false;

#ifdef DEBUG
    skip = IsKeyPressed(KEY_SPACE);
#endif

    if (skip || (plm_has_ended(movie->plm) && movie->end_movie_callback)) {
        movie->movie = MOVIE_OFF;

        if (movie->end_movie_callback)
            movie->end_movie_callback(&game_atari);

        ClearBackground(BLACK);
        return;
    }

    UpdateMusicStream(movie->audio);

    ClearBackground(BLACK);

    Rectangle src = {
        0, 0,
        (float)movie->texture.width, (float)movie->texture.height
    };
    Rectangle dst = get_screen_rectangle();

    DrawTexturePro(movie->texture, src, dst, {}, 0, WHITE);

    double current_time = GetTime();

    if (movie->last_time == -1)
        movie->last_time = current_time;

    double elapsed_time = current_time - movie->last_time;
    movie->last_time = current_time;

    plm_decode(movie->plm, elapsed_time);

    // Sync audio and video
    float movie_time = plm_get_time(movie->plm);

    if (fabs(movie_time - GetMusicTimePlayed(movie->audio)) > 0.05) {
        SeekMusicStream(movie->audio, movie_time);
    }

    if (movie->movie == MOVIE_OFF) {
        movie_free(movie);
    }

#if PROFILE
    double end = GetTime();
    double taken = end - start;

    printf("movie_run  took %fms\n", 1000 * taken);
#endif
}

void movie_init(Movie *movie, int which_movie) {
    const char *movie_name = 0;

    movie->movie = which_movie;

    switch (movie->movie) {
        case MOVIE_DRACULA:   movie_name = "dracula"; break;
        case MOVIE_PICNIC:    movie_name = "picnic";  break;
        case MOVIE_EMPTINESS: movie_name = "empty";   break;
    }

    assert(movie_name);

    const char *video = TextFormat(RES_DIR "movie/%s.mpeg", movie_name);
    const char *audio = TextFormat(RES_DIR "movie/%s.mp3",  movie_name);

    movie_load_video(movie, video, audio);

    if (movie->movie == MOVIE_DRACULA)
        SetMusicVolume(movie->audio, 0.45f);
}

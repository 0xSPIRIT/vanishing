enum Channel {
    CHANNEL_GUI,
    CHANNEL_WORLD,
};

float channel_volumes[] = {
    1.f, // CHANNEL_GUI
    1.f, // CHANNEL_WORLD
};

enum Music_ID {
    MUSIC_DESERT_AMBIENCE,
    MUSIC_VHS_BAD,
    MUSIC_VHS_BAD_2,
    MUSIC_VHS_BAD_3,
    MUSIC_VHS_BAD_4,
    MUSIC_NOISE,
    MUSIC_GLITCH,
    MUSIC_NIGHT_AMBIENCE,
    MUSIC_COUNT
};

enum Sound_ID {
    SOUND_INVALID = -1,
    SOUND_EMPTY,

    SOUND_TEXT_SCROLL,
    SOUND_TEXT_SCROLL_LOW,
    SOUND_TEXT_SCROLL_BAD,
    SOUND_TEXT_CONFIRM,
    SOUND_TEXT_SCROLL_MALE,
    SOUND_TEXT_SCROLL_FEMALE,
    SOUND_TEXT_SCROLL_CHASE,

    SOUND_SCREAM,

    // bitcrushed (chapter 1)
    SOUND_SAND_FOOTSTEP_1,
    SOUND_SAND_FOOTSTEP_2,
    SOUND_SAND_FOOTSTEP_3,
    SOUND_SAND_FOOTSTEP_4,

    // non bitcrushed (chapter 5)
    SOUND_REAL_SAND_STEP_1,
    SOUND_REAL_SAND_STEP_2,
    SOUND_REAL_SAND_STEP_3,
    SOUND_REAL_SAND_STEP_4,
    SOUND_REAL_SAND_STEP_5,
    SOUND_REAL_SAND_STEP_6,

    SOUND_KNOCKING_1,
    SOUND_KNOCKING_2,
    SOUND_KNOCKING_3,
    SOUND_KNOCKING_4,
    SOUND_KNOCKING_WEIRD,

    SOUND_OPEN_WINDOW,
    SOUND_CLOSE_WINDOW,
    SOUND_CREAKING,

    SOUND_ATARI_BASS_EFFECT,

    SOUND_EXHALE,

    SOUND_COUNT
};

struct Game_Sound {
    Sound   sound;
    Channel channel;
    float   volume;
    bool    playing;
};

struct Game_Audio {
    Game_Sound sounds[SOUND_COUNT];
    Music      musics[MUSIC_COUNT];

    float      volume_a, volume_b;

    int        current_music, secondary_music;
};
Game_Audio game_audio;

Game_Sound load_sound(Channel channel, const char *file, float volume=1) {
    Game_Sound result;

    const char *fp = TextFormat("%s%s", RES_DIR "audio/", file);

    result.sound   = LoadSound(fp);
    result.channel = channel;
    result.playing = false;
    result.volume  = volume;

    assert(IsSoundValid(result.sound));

    return result;
}

inline Music current_music() {
    return game_audio.musics[game_audio.current_music];
}

void set_music_volume(Music_ID music, float volume) {
    SetMusicVolume(game_audio.musics[music], volume);
}

void set_music_pitch(Music_ID music, float pitch) {
    SetMusicPitch(game_audio.musics[music], pitch);
}

void play_music(Music_ID music, bool crossfade = false) {
    if (!crossfade) {
        if (game_audio.current_music != -1) {
            StopMusicStream(current_music());
        }

        game_audio.current_music = music;
        PlayMusicStream(current_music());
    } else {
        if (game_audio.volume_a == -1 ||
            game_audio.volume_b == -1)
        {
            game_audio.volume_a = 1;
            game_audio.volume_b = 0;
        }

        if (game_audio.current_music != -1)
            SetMusicVolume(current_music(),          game_audio.volume_a);
        SetMusicVolume(game_audio.musics[music], game_audio.volume_b);

        game_audio.secondary_music = music;

        PlayMusicStream(game_audio.musics[music]);
    }
}

void stop_music() {
    if (game_audio.current_music != -1)
        StopMusicStream(current_music());

    game_audio.current_music = -1;
}

Music load_music(const char *file) {
    const char *fp = TextFormat("%s%s", RES_DIR "audio/", file);
    Music result = LoadMusicStream(fp);
    result.looping = true;

    return result;
}

bool is_music_playing(Music_ID music) {
    return IsMusicStreamPlaying(game_audio.musics[music]);
}

void play_sound(Sound_ID sound_id, Channel channel_override, float pan) {
    Game_Sound *sound = game_audio.sounds + sound_id;

    sound->channel = channel_override;

    float volume = sound->volume * channel_volumes[sound->channel];
    SetSoundVolume(sound->sound, volume);

    SetSoundPan(sound->sound, pan);

    PlaySound(sound->sound);
}

void play_sound(Sound_ID sound_id) {
    play_sound(sound_id, game_audio.sounds[sound_id].channel, 0.5);
}

bool is_sound_playing(Sound_ID sound_id) {
    return IsSoundPlaying(game_audio.sounds[sound_id].sound);
}

// We load every sound for the entire game here.
// We don't call UnloadSound since it's freed
// anyways by the OS on exit.
void game_audio_init() {
    Game_Audio *audio = &game_audio;

    audio->current_music = -1;

    audio->musics[MUSIC_DESERT_AMBIENCE] = load_music("NULL DESERT.ogg");
    audio->musics[MUSIC_VHS_BAD]         = load_music("vhs_bad.ogg");
    audio->musics[MUSIC_VHS_BAD_2]       = load_music("vhs_bad_2.ogg");
    audio->musics[MUSIC_VHS_BAD_3]       = load_music("vhs_bad_3.ogg");
    audio->musics[MUSIC_VHS_BAD_4]       = load_music("vhs_bad_4.wav");
    audio->musics[MUSIC_NOISE]           = load_music("whitenoise.ogg");
    audio->musics[MUSIC_GLITCH]          = load_music("glitch.ogg");
    audio->musics[MUSIC_NIGHT_AMBIENCE]  = load_music("nightambience.ogg");

    SetMusicVolume(audio->musics[MUSIC_VHS_BAD_4], 0.2f);
    SetMusicVolume(audio->musics[MUSIC_NOISE], 0.2f);
    SetMusicVolume(audio->musics[MUSIC_GLITCH], 1);

    float volume = 0.05f;

    audio->sounds[SOUND_TEXT_SCROLL]        = load_sound(CHANNEL_GUI, "textblip.wav",          volume * 3);
    audio->sounds[SOUND_TEXT_SCROLL_CHASE]  = load_sound(CHANNEL_GUI, "textscroll_chase.wav",  volume * 3);
    audio->sounds[SOUND_TEXT_SCROLL_FEMALE] = load_sound(CHANNEL_GUI, "textscroll_female.wav", volume * 2);
    audio->sounds[SOUND_TEXT_SCROLL_MALE]   = load_sound(CHANNEL_GUI, "textscroll_guy.wav",    volume * 3);

    audio->sounds[SOUND_TEXT_SCROLL_LOW] = load_sound(CHANNEL_GUI,   "textblip_low.wav", 0.85f);
    audio->sounds[SOUND_TEXT_SCROLL_BAD] = load_sound(CHANNEL_GUI,   "textblip_bad.wav");
    audio->sounds[SOUND_TEXT_CONFIRM]    = load_sound(CHANNEL_GUI,   "confirm.ogg");

    float footstep_volume = 0.2f;
    audio->sounds[SOUND_SAND_FOOTSTEP_1] = load_sound(CHANNEL_WORLD, "footstep1.ogg", footstep_volume);
    audio->sounds[SOUND_SAND_FOOTSTEP_2] = load_sound(CHANNEL_WORLD, "footstep2.ogg", footstep_volume);
    audio->sounds[SOUND_SAND_FOOTSTEP_3] = load_sound(CHANNEL_WORLD, "footstep3.ogg", footstep_volume);
    audio->sounds[SOUND_SAND_FOOTSTEP_4] = load_sound(CHANNEL_WORLD, "footstep4.ogg", footstep_volume);

    for (int i = 1; i <= 6; i++) {
        int index = SOUND_REAL_SAND_STEP_1 - 1 + i;
        audio->sounds[index] = load_sound(CHANNEL_WORLD, TextFormat("sand_%d.ogg", i), footstep_volume);
    }

    audio->sounds[SOUND_ATARI_BASS_EFFECT] = load_sound(CHANNEL_WORLD, "atari_effect.ogg");

    audio->sounds[SOUND_EXHALE] = load_sound(CHANNEL_WORLD, "exhale.ogg");

    audio->sounds[SOUND_KNOCKING_1] = load_sound(CHANNEL_WORLD, "knocking1.ogg");
    audio->sounds[SOUND_KNOCKING_2] = load_sound(CHANNEL_WORLD, "knocking2.ogg");
    audio->sounds[SOUND_KNOCKING_3] = load_sound(CHANNEL_WORLD, "knocking3.ogg");
    audio->sounds[SOUND_KNOCKING_4] = load_sound(CHANNEL_WORLD, "knocking4.ogg");
    audio->sounds[SOUND_KNOCKING_WEIRD] = load_sound(CHANNEL_WORLD, "knocking_weird.ogg", 0.4f);

    audio->sounds[SOUND_OPEN_WINDOW]  = load_sound(CHANNEL_WORLD, "open_window.ogg");
    audio->sounds[SOUND_CLOSE_WINDOW] = load_sound(CHANNEL_WORLD, "close_window.ogg");
    audio->sounds[SOUND_CREAKING]     = load_sound(CHANNEL_WORLD, "creaking.ogg", 0.4f);

    audio->sounds[SOUND_SCREAM]       = load_sound(CHANNEL_WORLD, "scream.ogg", 0.4f);

    audio->current_music = -1;
    audio->volume_a = audio->volume_b = -1;
}

void game_audio_update(float dt) {
    Game_Audio *audio = &game_audio;

    if (audio->current_music != -1) {
        UpdateMusicStream(audio->musics[audio->current_music]);

        float fade_speed = 5;

        if (audio->volume_a != -1 || audio->volume_b != -1) {
            if (audio->volume_a > 0) {
                audio->volume_a -= fade_speed * dt;

                if (audio->volume_a < 0) {
                    audio->volume_a = -1;
                    audio->volume_b = 0;

                    if (game_audio.current_music != -1)
                        StopMusicStream(current_music());

                    game_audio.current_music = audio->secondary_music;

                    if (game_audio.current_music != -1)
                        SetMusicVolume(current_music(), audio->volume_b);

                    PlayMusicStream(current_music());
                }

                SetMusicVolume(current_music(), audio->volume_a);
            }
            else if (audio->volume_b >= 0) {
                audio->volume_b += fade_speed * dt;

                bool done = false;

                if (audio->volume_b > 1) {
                    audio->volume_b = 1;
                    done = true;
                }

                SetMusicVolume(current_music(), audio->volume_b);

                if (done) {
                    audio->volume_b = -1;
                }
            }
        }
    }
}

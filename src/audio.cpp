enum Channel {
    CHANNEL_GUI,
    CHANNEL_WORLD,
};

float channel_volumes[] = {
    0.75f, // CHANNEL_GUI
    1.f, // CHANNEL_WORLD
};

enum Music_ID {
    MUSIC_DESERT_AMBIENCE,
    MUSIC_VHS_BAD,
    MUSIC_NOISE,
    MUSIC_GLITCH,
    MUSIC_COUNT
};

enum Sound_ID {
    SOUND_INVALID = 0,

    SOUND_TEXT_SCROLL,
    SOUND_TEXT_CONFIRM,

    SOUND_SAND_FOOTSTEP_1,
    SOUND_SAND_FOOTSTEP_2,
    SOUND_SAND_FOOTSTEP_3,
    SOUND_SAND_FOOTSTEP_4,

    SOUND_KNOCKING_1,
    SOUND_KNOCKING_2,
    SOUND_KNOCKING_3,
    SOUND_KNOCKING_4,
    SOUND_KNOCKING_WEIRD,

    SOUND_OPEN_WINDOW,
    SOUND_CLOSE_WINDOW,

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

    return result;
}

inline Music current_music() {
    return game_audio.musics[game_audio.current_music];
}

void set_music_volume(Music_ID music, float volume) {
    SetMusicVolume(game_audio.musics[music], volume);
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
    StopMusicStream(current_music());
    game_audio.current_music = -1;
}

Music load_music(const char *file) {
    const char *fp = TextFormat("%s%s", RES_DIR "audio/", file);
    Music result = LoadMusicStream(fp);

    return result;
}

void play_sound(Sound_ID sound_id, Channel channel_override) {
    Game_Sound *sound = game_audio.sounds + sound_id;

    sound->channel = channel_override;

    float volume = sound->volume * channel_volumes[sound->channel];
    SetSoundVolume(sound->sound, volume);

    PlaySound(sound->sound);
}

void play_sound(Sound_ID sound_id) {
    play_sound(sound_id, game_audio.sounds[sound_id].channel);
}

bool is_sound_playing(Sound_ID sound_id) {
    return IsSoundPlaying(game_audio.sounds[sound_id].sound);
}

// We load every sound for the entire game here.
// We don't call UnloadSound since it's freed
// anyways by the OS on exit.
void game_audio_init() {
    Game_Audio *audio = &game_audio;

    audio->musics[MUSIC_DESERT_AMBIENCE] = load_music("NULL DESERT.ogg");
    audio->musics[MUSIC_VHS_BAD]         = load_music("vhs_bad.ogg");
    audio->musics[MUSIC_NOISE]           = load_music("whitenoise.ogg");
    audio->musics[MUSIC_GLITCH]          = load_music("glitch.ogg");
    SetMusicVolume(audio->musics[MUSIC_NOISE], 0.2f);
    SetMusicVolume(audio->musics[MUSIC_GLITCH], 1);

    audio->sounds[SOUND_TEXT_SCROLL]     = load_sound(CHANNEL_GUI,   "textscroll1.ogg");
    audio->sounds[SOUND_TEXT_CONFIRM]    = load_sound(CHANNEL_GUI,   "confirm.ogg");

    float footstep_volume = 0.12f;
    audio->sounds[SOUND_SAND_FOOTSTEP_1] = load_sound(CHANNEL_WORLD, "footstep1.ogg", footstep_volume);
    audio->sounds[SOUND_SAND_FOOTSTEP_2] = load_sound(CHANNEL_WORLD, "footstep2.ogg", footstep_volume);
    audio->sounds[SOUND_SAND_FOOTSTEP_3] = load_sound(CHANNEL_WORLD, "footstep3.ogg", footstep_volume);
    audio->sounds[SOUND_SAND_FOOTSTEP_4] = load_sound(CHANNEL_WORLD, "footstep4.ogg", footstep_volume);

    audio->sounds[SOUND_ATARI_BASS_EFFECT] = load_sound(CHANNEL_WORLD, "atari_effect.ogg");

    audio->sounds[SOUND_EXHALE] = load_sound(CHANNEL_WORLD, "exhale.ogg");

    audio->sounds[SOUND_KNOCKING_1] = load_sound(CHANNEL_WORLD, "knocking1.ogg");
    audio->sounds[SOUND_KNOCKING_2] = load_sound(CHANNEL_WORLD, "knocking2.ogg");
    audio->sounds[SOUND_KNOCKING_3] = load_sound(CHANNEL_WORLD, "knocking3.ogg");
    audio->sounds[SOUND_KNOCKING_4] = load_sound(CHANNEL_WORLD, "knocking4.ogg");
    audio->sounds[SOUND_KNOCKING_WEIRD] = load_sound(CHANNEL_WORLD, "knocking_weird.ogg");

    audio->sounds[SOUND_OPEN_WINDOW]  = load_sound(CHANNEL_WORLD, "open_window.ogg");
    audio->sounds[SOUND_CLOSE_WINDOW] = load_sound(CHANNEL_WORLD, "close_window.ogg");

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

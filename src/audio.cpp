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
    MUSIC_COUNT
};

enum Sound_ID {
    SOUND_TEXT_SCROLL,

    SOUND_SAND_FOOTSTEP_1,
    SOUND_SAND_FOOTSTEP_2,
    SOUND_SAND_FOOTSTEP_3,
    SOUND_SAND_FOOTSTEP_4,

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

void play_music(Music_ID music, bool crossfade = false) {
    if (!crossfade || game_audio.current_music == -1) {
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

    audio->sounds[SOUND_TEXT_SCROLL]     = load_sound(CHANNEL_GUI,   "textscroll1.ogg");

    float footstep_volume = 0.12f;
    audio->sounds[SOUND_SAND_FOOTSTEP_1] = load_sound(CHANNEL_WORLD, "footstep1.ogg", footstep_volume);
    audio->sounds[SOUND_SAND_FOOTSTEP_2] = load_sound(CHANNEL_WORLD, "footstep2.ogg", footstep_volume);
    audio->sounds[SOUND_SAND_FOOTSTEP_3] = load_sound(CHANNEL_WORLD, "footstep3.ogg", footstep_volume);
    audio->sounds[SOUND_SAND_FOOTSTEP_4] = load_sound(CHANNEL_WORLD, "footstep4.ogg", footstep_volume);

    audio->sounds[SOUND_ATARI_BASS_EFFECT] = load_sound(CHANNEL_WORLD, "atari_effect.ogg");

    audio->sounds[SOUND_EXHALE] = load_sound(CHANNEL_WORLD, "exhale.ogg");

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

                    StopMusicStream(current_music());

                    game_audio.current_music = audio->secondary_music;

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

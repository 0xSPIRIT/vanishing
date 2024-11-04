enum Channel {
    CHANNEL_GUI,
    CHANNEL_WORLD,
    CHANNEL_MUSIC,
};

float channel_volumes[] = {
    0.5f, // CHANNEL_GUI
    0.5f, // CHANNEL_WORLD
    0.7f  // CHANNEL_MUSIC
};

enum Sound_ID {
    SOUND_TEXT_SCROLL,
    SOUND_VHS,
    SOUND_COUNT
};

struct Game_Sound {
    Sound   sound;
    Channel channel;
    bool    looping;
    bool    playing;
};

struct Game_Audio {
    Game_Sound sounds[SOUND_COUNT];
    //Music      tracks[MUSIC_COUNT];

    Sound_ID bg_music;
};
Game_Audio game_audio;

Game_Sound game_sound(Channel channel, const char *file) {
    Game_Sound result;

    const char *fp = TextFormat("%s%s", RES_DIR "audio/", file);

    result.sound   = LoadSound(fp);
    result.channel = channel;
    result.looping = false;
    result.playing = false;

    return result;
}

void play_sound(Sound_ID sound_id, Channel channel_override, bool looping) {
    Game_Sound *sound = game_audio.sounds + sound_id;

    sound->channel = channel_override;
    sound->looping = looping;

    SetSoundVolume(sound->sound, channel_volumes[sound->channel]);

    PlaySound(sound->sound);
}

void play_sound(Sound_ID sound_id) {
    play_sound(sound_id, game_audio.sounds[sound_id].channel, false);
}

void play_sound(Sound_ID sound_id, bool looping) {
    play_sound(sound_id, game_audio.sounds[sound_id].channel, looping);
}

bool is_sound_playing(Sound_ID sound_id) {
    return IsSoundPlaying(game_audio.sounds[sound_id].sound);
}
// We load every sound for the entire game here.
// We don't call UnloadSound since it's freed
// anyways by the OS on exit.
void game_audio_init() {
    Game_Audio *audio = &game_audio;

    audio->sounds[SOUND_TEXT_SCROLL] = game_sound(CHANNEL_GUI,   "textscroll1.ogg");
    audio->sounds[SOUND_VHS]         = game_sound(CHANNEL_MUSIC, "vhs.ogg");

    audio->bg_music = SOUND_VHS;

    //play_sound(audio->bg_music, true);
}

void game_audio_update() {
}

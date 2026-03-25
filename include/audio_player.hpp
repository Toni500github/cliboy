#pragma once

#include "miniaudio.h"

namespace TetrisSounds {
    constexpr const char* BGM        = "tetris.wav";
    //constexpr const char* LINE_CLEAR = "sfx_line_clear.mp3";
}

namespace MenuSounds {
    constexpr const char* NAVIGATE = "sfx_item_select.mp3";
}

class AudioPlayer
{
public:
    AudioPlayer() = default;
    ~AudioPlayer();

    // Non-copyable for owning ma_engine/ma_sound resources
    AudioPlayer(const AudioPlayer&)            = delete;
    AudioPlayer& operator=(const AudioPlayer&) = delete;

    bool begin();

    // Background music - loops continuously, one track at a time
    void playMusic(const char* path);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    bool isMusicPlaying() const;

    // SFX - plays once, does not interrupt music
    void playSfx(const char* path);
    void stopSfx();

    // Volume - [0.0, 1.0]
    void setMusicVolume(float volume);
    void setSfxVolume(float volume);

private:
    void unloadMusic();
    void unloadSfx();

    ma_engine m_engine{};
    ma_sound  m_music{};
    ma_sound  m_sfx{};

    bool  m_engine_ready  = false;
    bool  m_music_loaded  = false;
    bool  m_sfx_loaded    = false;
    float m_music_volume  = 1.0f;
    float m_sfx_volume    = 1.0f;
};

extern AudioPlayer playback;

#pragma once

#include <string>

#include "miniaudio.h"

namespace TetrisSounds
{
constexpr const char* BGM        = "tetris.mp3";
constexpr const char* LINE_CLEAR = "sfx_tetris_clear_line.wav";
}  // namespace TetrisSounds

namespace SnakeSounds
{
constexpr const char* FOOD = "sfx_snake_food.mp3";
}

namespace Game2048Sounds
{
constexpr const char* BGM = "2048.mp3";
}

namespace WordleSounds
{
constexpr const char* BGM = "wordle.mp3";
}

namespace MinesweeperSounds
{
constexpr const char* BGM = "minesweeper.mp3";
}

namespace MenuSounds
{
constexpr const char* BGM = "bg_music.mp3";
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

    // Errors
    bool        hasError() const { return !m_last_error.empty(); }
    std::string takeError()
    {
        std::string e = m_last_error;
        m_last_error.clear();
        return e;
    }

private:
    ma_engine m_engine{};
    ma_sound  m_music{};
    ma_sound  m_sfx{};

    std::string m_last_error;
    std::string m_current_music;

    bool  m_engine_ready = false;
    bool  m_music_loaded = false;
    bool  m_sfx_loaded   = false;
    float m_music_volume = 1.0f;
    float m_sfx_volume   = 1.0f;

    void unloadMusic();
    void unloadSfx();
};

extern AudioPlayer playback;

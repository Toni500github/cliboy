#include "audio_player.hpp"

#include <cstdio>
#include <string>

#include "settings.hpp"

AudioPlayer::~AudioPlayer()
{
    // Sounds must be uninitialized before the engine they belong to
    unloadMusic();
    unloadSfx();

    if (m_engine_ready)
        ma_engine_uninit(&m_engine);
}

bool AudioPlayer::begin()
{
    ma_result result = ma_engine_init(nullptr, &m_engine);
    if (result != MA_SUCCESS)
    {
        fprintf(stderr, "[audio] Failed to init engine: %s\n", ma_result_description(result));
        return false;
    }

    m_engine_ready = true;
    return true;
}

// -------------------------------------
// Music
// -------------------------------------

void AudioPlayer::playMusic(const char* audio)
{
    if (!m_engine_ready)
        return;

    const std::string path = settings.general.assets_path + "/audios/" + audio;

    // Already playing this exact track — do nothing
    if (m_music_loaded && m_current_music == path && ma_sound_is_playing(&m_music))
        return;

    unloadMusic();
    m_current_music = path;

    ma_result result =
        ma_sound_init_from_file(&m_engine, path.c_str(), MA_SOUND_FLAG_STREAM, nullptr, nullptr, &m_music);

    if (result != MA_SUCCESS)
    {
        fprintf(stderr, "[audio] Failed to load music '%s': %s\n", path.c_str(), ma_result_description(result));
        return;
    }

    m_music_loaded = true;
    ma_sound_set_looping(&m_music, MA_TRUE);
    ma_sound_set_volume(&m_music, m_music_volume);
    ma_sound_start(&m_music);
}

void AudioPlayer::stopMusic()
{
    if (!m_music_loaded)
        return;

    ma_sound_stop(&m_music);
    ma_sound_seek_to_pcm_frame(&m_music, 0);
}

void AudioPlayer::pauseMusic()
{
    if (m_music_loaded)
        ma_sound_stop(&m_music);  // miniaudio pause = stop without seek
}

void AudioPlayer::resumeMusic()
{
    if (m_music_loaded && !ma_sound_is_playing(&m_music))
        ma_sound_start(&m_music);
}

bool AudioPlayer::isMusicPlaying() const
{
    return m_music_loaded && ma_sound_is_playing(&m_music);
}

void AudioPlayer::setMusicVolume(float volume)
{
    m_music_volume = volume;
    if (m_music_loaded)
        ma_sound_set_volume(&m_music, volume);
}

// -------------------------------------
// SFX
// -------------------------------------

void AudioPlayer::playSfx(const char* audio)
{
    if (!m_engine_ready)
        return;

    unloadSfx();

    const std::string path = settings.general.assets_path + "/audios/" + audio;

    ma_result result = ma_sound_init_from_file(&m_engine, path.c_str(), MA_SOUND_FLAG_DECODE, nullptr, nullptr, &m_sfx);

    if (result != MA_SUCCESS)
    {
        fprintf(stderr, "[audio] Failed to load sfx '%s': %s\n", path.c_str(), ma_result_description(result));
        return;
    }

    m_sfx_loaded = true;
    ma_sound_set_looping(&m_sfx, MA_FALSE);
    ma_sound_set_volume(&m_sfx, m_sfx_volume);
    ma_sound_start(&m_sfx);
}

void AudioPlayer::stopSfx()
{
    if (m_sfx_loaded)
        ma_sound_stop(&m_sfx);
}

void AudioPlayer::setSfxVolume(float volume)
{
    m_sfx_volume = volume;
    if (m_sfx_loaded)
        ma_sound_set_volume(&m_sfx, volume);
}

void AudioPlayer::unloadMusic()
{
    if (m_music_loaded)
    {
        ma_sound_uninit(&m_music);
        m_music_loaded = false;
    }
}

void AudioPlayer::unloadSfx()
{
    if (m_sfx_loaded)
    {
        ma_sound_uninit(&m_sfx);
        m_sfx_loaded = false;
    }
}

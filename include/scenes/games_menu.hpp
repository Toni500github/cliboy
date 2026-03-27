#pragma once

#include "audio_player.hpp"
#include "scenes.hpp"

class GamesMenuScene : public Scene
{
public:
    Result<> on_begin() override
    {
        playback.playMusic(MenuSounds::BGM);
        set_footer("Arrow Keys: Navigate | Enter: Play | ESC: Back");
        return Ok();
    }

    void        render() override;
    void        end(SceneResult next_scene) override;
    SceneResult handle_input(uint32_t key) override;

private:
    int                  m_selected_game = 0;
    static constexpr int GAME_COUNT      = static_cast<int>(ScenesGame::COUNT);
};

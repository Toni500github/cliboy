#pragma once

#include "audio_player.hpp"
#include "scenes.hpp"

class GamesMenuScene : public Scene
{
public:
    Result<> onBegin() override
    {
        playback.playMusic(MenuSounds::BGM);
        setFooter("Arrow Keys: Navigate | Enter: Play | ESC: Back");
        return Ok();
    }

    void        render() override;
    void        end(SceneResult next_scene) override;
    SceneResult sceneID() const override { return Scenes::GamesMenu; }
    SceneResult handleInput(uint32_t key) override;

private:
    int                  m_selected_game = 0;
    static constexpr int GAME_COUNT      = static_cast<int>(ScenesGame::COUNT);
};

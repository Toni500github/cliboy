#pragma once

#include "audio_player.hpp"
#include "scenes.hpp"

class MainMenuScene : public Scene
{
public:
    Result<> onBegin() override
    {
        playback.playMusic(MenuSounds::BGM);
        setFooter("Arrow Keys: Navigate | Enter: Select | ESC: Exit");
        return Ok();
    }

    void        end(SceneResult) override {}
    void        render() override;
    SceneResult sceneID() const override { return Scenes::MainMenu; }
    SceneResult handleInput(uint32_t key) override;

private:
    int                  m_selected_item = 0;
    static constexpr int MENU_ITEM_COUNT = static_cast<int>(Scenes::COUNT);
};

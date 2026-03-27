#pragma once

#include "audio_player.hpp"
#include "scenes.hpp"

class MainMenuScene : public Scene
{
public:
    Result<> on_begin() override
    {
        playback.playMusic(MenuSounds::BGM);
        set_footer("Arrow Keys: Navigate | Enter: Select | ESC: Exit");
        return Ok();
    }

    void        end(SceneResult) override {}
    void        render() override;
    SceneResult handle_input(uint32_t key) override;

private:
    int                  m_selected_item = 0;
    static constexpr int MENU_ITEM_COUNT = static_cast<int>(Scenes::COUNT);
};

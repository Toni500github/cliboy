#pragma once

#include "scenes.hpp"

class MainMenuScene : public Scene
{
public:
    Result<> on_begin() override
    {
        set_footer("Arrow Keys: Navigate | Enter: Select | ESC: Exit");
        return Ok();
    }

    void        render() override;
    SceneResult handle_input(uint32_t key) override;

private:
    int                  m_selected_item = 0;
    static constexpr int MENU_ITEM_COUNT = static_cast<int>(Scenes::COUNT);
};

#pragma once

#include "scenes.hpp"

class GamesMenuScene : public Scene
{
public:
    Result<> on_begin() override
    {
        set_footer("Arrow Keys: Navigate | Enter: Play | ESC: Back");
        return Ok();
    }

    void        render() override;
    SceneResult handle_input(uint32_t key) override;

private:
    int                  m_selected_game = 0;
    static constexpr int GAME_COUNT      = static_cast<int>(ScenesGame::COUNT);
};

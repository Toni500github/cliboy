#pragma once

#include "scenes.hpp"

class TTTGame : public Scene
{
public:
    Result<> on_begin() override
    {
        set_footer("Arrow: Navigation | Enter: Place | ESC: Back");
        return Ok();
    }
    void        render() override;
    SceneResult handle_input(uint32_t key) override;
};

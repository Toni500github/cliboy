#pragma once

#include "scenes.hpp"

class RpsGame : public Scene
{
public:
    Result<> on_begin() override
    {
        set_footer("Rock: r | Paper: p | Scissors: s | Enter: Select | ESC: Back");
        return Ok();
    }

    void        render() override;
    SceneResult handle_input(uint32_t key) override;
};

#pragma once

#include "scenes.hpp"

class CreditsScene : public Scene
{
public:
    Result<> on_begin() override
    {
        set_footer("ESC: Back");
        return Ok();
    }

    void        render() override;
    void        end(SceneResult) override {}
    SceneResult handle_input(uint32_t key) override;
};

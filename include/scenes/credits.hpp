#pragma once

#include "scenes.hpp"

class CreditsScene : public Scene
{
public:
    Result<> onBegin() override
    {
        setFooter("ESC: Back");
        return Ok();
    }

    void        render() override;
    void        end(SceneResult) override {}
    SceneResult sceneID() const override { return Scenes::Credits; }
    SceneResult handleInput(uint32_t key) override;
};

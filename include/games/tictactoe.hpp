#pragma once

#include "scenes.hpp"

class TTTScene : public Scene
{
public:
    TTTScene();
    void        render() override;
    SceneResult handle_input(uint32_t key) override;
};

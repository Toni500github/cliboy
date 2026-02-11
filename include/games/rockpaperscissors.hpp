#pragma once

#include "scenes.hpp"

class RpsGame : public Scene
{
public:
    void        render() override;
    SceneResult handle_input(uint32_t key) override;
};

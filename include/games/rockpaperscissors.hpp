#pragma once

#include "scenes.hpp"

class RpsScene : public Scene
{
public:
    void        render() override;
    SceneResult handle_input(uint32_t key) override;
};

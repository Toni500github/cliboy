#pragma once

#include "scenes.hpp"

class WordleGame : public Scene
{
public:
    WordleGame();
    void        render() override;
    SceneResult handle_input(uint32_t key) override;
};

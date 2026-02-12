#pragma once

#include "scenes.hpp"

class WordleGame : public Scene
{
public:
    Result<>    on_begin() override;
    void        render() override;
    SceneResult handle_input(uint32_t key) override;
};

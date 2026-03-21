#pragma once

#include "scenes.hpp"

enum class Winner
{
    CPU,
    DRAW,
    PLAYER
};

enum class Moves
{
    None,
    Rock,
    Paper,
    Scissors,
    Done
};

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

private:
    Moves m_player_move = Moves::None;
    bool  m_selected    = false;

    static std::string get_move_str(Moves move);
    static Moves       get_cpu_move();
    static Winner      calculate_winner(Moves cpu_move, Moves player_move);

    void print_countdown(uint8_t n);
    void print_player_move(Moves player_move);
    void print_moves(Moves computer_move, Moves player_move);
    void print_winner(Winner winner);
};

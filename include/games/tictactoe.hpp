#pragma once

#include <type_traits>

#include "scenes.hpp"

// I'm not singing for your XO... I'm singing cuz it's oveeeerrrrr
enum class Player
{
    None = 0,
    X    = 'X',
    O    = 'O',
};

class TTTGame : public Scene
{
public:
    Result<> on_begin() override
    {
        reset_game();
        set_footer("Arrow: Navigation | Enter: Place | ESC: Back");
        return Ok();
    }
    void        render() override;
    SceneResult handle_input(uint32_t key) override;

private:
    char   m_board[3][3]    = { { ' ', ' ', ' ' }, { ' ', ' ', ' ' }, { ' ', ' ', ' ' } };
    bool   m_choose_pos     = false;
    Player m_current_player = Player::X;
    int    m_old_pos_x{}, m_cursor_x{};
    int    m_old_pos_y{}, m_cursor_y{};
    int    m_moves = 0;

    void draw_piece(int row, int col, char piece);
    void draw_game_screen();
    void draw_winner(Player winner);

    bool   is_board_full();
    void   animate_line(int x0, int y0, int x1, int y1);
    Player check_winner();
    void   reset_game();

    template <typename Func>
    auto iterate_board(Func&& fun)
    {
        using Ret = std::invoke_result_t<Func, char&, int, int>;

        if constexpr (std::is_same_v<Ret, bool>)
        {
            for (int r = 0; r < 3; ++r)
                for (int c = 0; c < 3; ++c)
                    if (fun(m_board[r][c], r, c))
                        return true;
            return false;
        }
        else
        {
            for (int r = 0; r < 3; ++r)
                for (int c = 0; c < 3; ++c)
                    fun(m_board[r][c], r, c);
        }
    }
};

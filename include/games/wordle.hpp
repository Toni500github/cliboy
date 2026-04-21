#pragma once

#include <vector>

#include "game.hpp"

enum class TileState
{
    Empty,
    Absent,   // gray
    Present,  // yellow
    Correct   // green
};

struct Tile
{
    char      ch    = ' ';
    TileState state = TileState::Empty;
};

using RowStates    = std::array<TileState, 5>;
using WordleStates = std::array<std::array<Tile, 5>, 6>;

class WordleGame : public BaseGame
{
public:
    SceneResult handle_input(uint32_t key) override;

protected:
    void        init_game() override;
    void        render_game() override;
    Result<>    on_game_begin() override;
    SceneResult scene_id() const override { return ScenesGame::Wordle; }

private:
    std::string              m_buf;
    std::string              m_guess;
    std::string              m_invalid_word;
    std::vector<std::string> m_words_list;
    bool                     m_is_selected{};
    bool                     m_is_correct{};
    bool                     m_is_invalid{};
    WordleStates             m_grid{};
    int                      m_row{};

    static uintattr_t bg_for(TileState s);
    static uintattr_t fg_for(TileState s);
    static bool       is_correct(const RowStates& row);

    std::string get_random_guess();
    RowStates   get_states(const std::string& str);
    bool        is_valid(const std::string& word) const;
    void        draw_wordle_grid(const WordleStates& grid);
    void        draw_not_valid(const std::string& word);
    void        update_game();
    void        draw_end_game(bool won);
    void        run_end_game_sequence(bool won);
};

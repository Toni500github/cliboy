#pragma once

#include <vector>

#include "game.hpp"

static constexpr int kWordLen = 5;
static constexpr int kMaxRows = 6;

static constexpr const char* kFooterText =
    "Try to guess the word. Each letter color:\n"
    "Black: Absent | Yellow: Present | Green: Correct";

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

using RowStates    = std::array<TileState, kWordLen>;
using WordleStates = std::array<std::array<Tile, kWordLen>, kMaxRows>;
using LetterStates = std::array<TileState, 26>;  // indexed by letter - 'A'

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
    static constexpr int cell_w = 5;
    static constexpr int cell_h = 3;
    static constexpr int gap_x  = 1;
    static constexpr int gap_y  = 1;

    static constexpr int kb_cell_w     = 3;
    static constexpr int kb_cell_h     = 3;
    static constexpr int kb_gap_x      = 1;
    static constexpr int kb_gap_y      = 1;
    static constexpr int grid_kb_gap   = 8;
    static constexpr int kb_rows_count = 3;

    // Derived heights (used by both draw functions to agree on layout)
    static constexpr int grid_h  = kMaxRows * cell_h + (kMaxRows - 1) * gap_y;
    static constexpr int kb_h    = kb_rows_count * kb_cell_h + (kb_rows_count - 1) * kb_gap_y;
    static constexpr int total_h = grid_h + grid_kb_gap + kb_h;

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
    void        draw_keyboard(const WordleStates& letter_states);
    void        update_game();
    void        draw_end_game(bool won);
    void        run_end_game_sequence(bool won);
};

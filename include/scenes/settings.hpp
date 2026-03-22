#pragma once

#include "scenes.hpp"

class SettingsScene : public Scene
{
public:
    void        render() override;
    SceneResult handle_input(uint32_t key) override;

private:
    size_t      m_selected_item = 0;
    size_t      m_scroll_offset = 0;  // index of the first rendered entry
    bool        m_editing       = false;
    std::string m_edit_buffer;

    // Adjusts m_scroll_offset so that m_selected_item is always in the
    // visible portion of the list.
    void ensure_visible();
};

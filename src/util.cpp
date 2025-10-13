#include "util.hpp"

#include <thread>

#include "scenes.hpp"
#include "terminal_display.hpp"

uint64_t button_state      = 0;
uint64_t prev_button_state = 0;
uint64_t button_pressed    = 0;  // New presses only

void settings_update_key_button(int choice)
{
    struct ncinput ni;

    display.clearDisplay();
    display.centerText(5, "Press any key to assign...");
    display.display();

    // Wait for a single keypress
    uint32_t id = notcurses_get(display.getNC(), nullptr, &ni);

    if (id != (uint32_t)-1 && id != NCKEY_ENTER && id != NCKEY_ESC)
    {
        switch (choice)
        {
            case SCENE_SETTINGS_KEY_UP:    settings.ch_up = id; break;
            case SCENE_SETTINGS_KEY_DOWN:  settings.ch_down = id; break;
            case SCENE_SETTINGS_KEY_LEFT:  settings.ch_left = id; break;
            case SCENE_SETTINGS_KEY_RIGHT: settings.ch_right = id; break;
        }

        display.clearDisplay();
        display.centerText(5, "Assigned: {0:c} (0x{0:x})", id);
        display.display();
        std::this_thread::sleep_for(1s);
    }
}

void update_button()
{
    prev_button_state = button_state;
    button_state      = 0;

    struct ncinput ni;
    uint32_t       id;

    // non-blocking poll (returns 0 if no input available immediately)
    while ((id = notcurses_get_nblock(display.getNC(), &ni)) != 0)
    {
        if (id == (uint32_t)-1)
            break;  // error

        if (id == NCKEY_ESC)
            button_state |= KEY_QUIT;

        const int& idi = static_cast<int>(id);

        if (idi == settings.ch_down || id == NCKEY_DOWN)
            button_state |= KEY_DOWN_BIT;

        else if (idi == settings.ch_left || id == NCKEY_LEFT)
            button_state |= KEY_LEFT_BIT;

        else if (idi == settings.ch_right || id == NCKEY_RIGHT)
            button_state |= KEY_RIGHT_BIT;

        else if (idi == settings.ch_up || id == NCKEY_UP)
            button_state |= KEY_UP_BIT;

        else if (id == NCKEY_ENTER)
            button_state |= KEY_SELECTED;
    }

    // calculate edge transitions
    button_pressed = button_state & ~prev_button_state;  // bits that went from 0->1
}

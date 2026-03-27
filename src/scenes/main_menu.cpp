#include "scenes/main_menu.hpp"

#include "terminal_display.hpp"

void MainMenuScene::render()
{
    if (!playback.isMusicPlaying())
        playback.playMusic(MenuSounds::BGM);

    display.clearDisplay();

    // Colored title
    display.setTextColor(TB_CYAN | TB_BOLD);
    display.setFont(FigletType::FullWidth, "Big Money-nw");
    display.centerText(display.pctY(0.08f), "Cli-Boy");
    display.resetFont();
    display.resetColors();

    // Tagline below the logo
    display.setTextColor(TB_CYAN);
    display.centerText(display.pctY(0.30f), "~ Terminal Games Collection ~");
    display.resetColors();

    // Thin separator line
    display.setTextColor(TB_CYAN);
    display.drawLine(display.pctX(0.20f),
                     display.pctY(0.36f),
                     display.pctX(0.80f),
                     display.pctY(0.36f),
                     settings.general.utf8 ? U'─' : '-');
    display.resetColors();

    // Menu items display
    const char* menu_items[] = { "Games", "Settings", "Credits" };
    int         start_y      = display.pctY(0.50f) - (MENU_ITEM_COUNT - 1) * 5 / 2;

    display.setFont(FigletType::FullWidth, "Small Slant");
    for (int i = 0; i < MENU_ITEM_COUNT; i++)
    {
        int y = start_y + i * 5;
        if (i == m_selected_item)
        {
            display.setTextColor(TB_YELLOW | TB_BOLD);
            display.centerText(y, "> {} <", menu_items[i]);
        }
        else
        {
            display.setTextColor(TB_WHITE);
            display.centerText(y, "  {}  ", menu_items[i]);
        }
        display.resetColors();
    }
    display.resetFont();

    // Version watermark (bottom-right corner)
    display.setTextColor(TB_CYAN);
    display.setCursor(display.getWidth() - 10, display.getHeight() - 2);
    display.print("v" VERSION);
    display.resetColors();

    display.display();
}

SceneResult MainMenuScene::handle_input(uint32_t key)
{
    switch (key)
    {
        case TB_KEY_ESC: return Scenes::Exit;

        case TB_KEY_ARROW_UP:   m_selected_item = (m_selected_item - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT; break;
        case TB_KEY_ARROW_DOWN: m_selected_item = (m_selected_item + 1) % MENU_ITEM_COUNT; break;

        case '\n':
        case TB_KEY_ENTER: return static_cast<Scenes>(m_selected_item);
    }

    return Scenes::MainMenu;
}

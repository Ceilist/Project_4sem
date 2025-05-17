#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace AppConstants {
    // Оконные константы
    const unsigned int WINDOW_WIDTH = 1200;
    const unsigned int WINDOW_HEIGHT = 800;
    // Имя для заголовка окна

    const std::string WINDOW_TITLE_BASE = "Interactive Ray Optics";

    // Константы симуляции
    const float MAX_RAY_LENGTH = 2000.f;
    const float ROTATION_SPEED = 0.05f;
    const float PARAM_ADJUST_SPEED = 10.0f;
    const int SOURCE_PARAM_ADJUST_SPEED = 1;
    const float MIN_ELEMENT_PLACEMENT_DISTANCE = 5.0f;

    // Константы UI и выбора
    const float ELEMENT_SELECT_TOLERANCE = 8.0f;
    const float HANDLE_SELECT_TOLERANCE = 8.0f;
    const unsigned int FONT_SIZE_UI = 14;

    // Цвета
    const sf::Color COLOR_BACKGROUND = sf::Color(30, 30, 50);
    const sf::Color COLOR_HELP_TEXT = sf::Color::White;
    const sf::Color COLOR_HANDLE_MOVE = sf::Color(100, 100, 255);
    const sf::Color COLOR_HANDLE_RESIZE = sf::Color(100, 255, 100);

    // Цвета для текстового ввода
    const sf::Color COLOR_INPUT_TEXT_FG = sf::Color::Black;
    const sf::Color COLOR_INPUT_TEXT_BG = sf::Color(255, 255, 255, 230);
    const sf::Color COLOR_INPUT_TEXT_OUTLINE = sf::Color::Black;

    // Цвета для предпросмотра размещения
    const sf::Color COLOR_PLACEMENT_PREVIEW_MIRROR = sf::Color::White;
    const sf::Color COLOR_PLACEMENT_PREVIEW_LENS = sf::Color(100, 100, 255);
    const sf::Color COLOR_PLACEMENT_PREVIEW_SPHERICAL_MIRROR = sf::Color(192, 192, 192);

    // Константы для UI редактирования текста
    const float TEXT_INPUT_FIELD_MIN_WIDTH = 10.f;
    const float TEXT_INPUT_PADDING_HORIZONTAL_BG = 5.f; // Отступ слева/справа для фона от всего блока
    const float TEXT_INPUT_PADDING_VERTICAL_BG = 4.f;   // Отступ сверху/снизу для фона от текста
    const float TEXT_INPUT_SPACING_PROMPT_TO_TEXT = 5.f; // Расстояние между prompt ("F=") и самим полем ввода


    const std::vector<std::string> FONT_PATHS = {
        "res/arial.ttf", "arial.ttf"
    };

} // namespace AppConstants

#endif // CONSTANTS_HPP

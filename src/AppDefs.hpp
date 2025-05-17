#ifndef APPDEFS_HPP
#define APPDEFS_HPP

#include <vector>
#include <SFML/Graphics.hpp>

// Состояния приложения (режимы работы)
enum class Mode
{
    IDLE,             // Ожидание действий пользователя (элемент может быть выбран)
    PLACING_START,    // Готовность начать размещение (первый клик - центр/начальная точка)
    PLACING_END,      // Ожидание второго клика для завершения размещения (радиус/конечная точка)
    DRAGGING_ELEMENT, // Перетаскивание элемента или его ручки
    EDITING_PARAMETER // Режим ввода текста для параметра
};


enum class HandleType
{
    NONE = -1,
    MOVE = 0,
    RESIZE_P1 = 1,
    RESIZE_P2 = 2

};

using RayPath = std::vector<sf::Vertex>;

#endif // APPDEFS_HPP
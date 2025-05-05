#ifndef HEADER_GUARD_OPTICAL_ELEMENT_HPP
#define HEADER_GUARD_OPTICAL_ELEMENT_HPP

#include <SFML/Graphics.hpp>
#include <optional>
#include <cmath>
#include "VectorMath.hpp"

// Структура, представляющая луч
struct Ray
{
    sf::Vector2f origin;                 // Точка испускания луча
    sf::Vector2f direction;              // Нормализованный вектор направления
    int bounces_left = 5;                // Максимальное количество взаимодействий (отскоков/преломлений)
    sf::Color color = sf::Color::Yellow; // Цвет луча по умолчанию
};

// Структура для хранения результата взаимодействия луча с элементом
struct RayAction
{
    std::optional<Ray> outgoingRay; // Исходящий луч (если взаимодействие произошло)
    sf::Vector2f interactionPoint;  // Точка взаимодействия

    inline RayAction() : outgoingRay(std::nullopt) {}

    inline RayAction(sf::Vector2f p, Ray r) : interactionPoint(p), outgoingRay(std::make_optional(r)) {}
};

// Базовый класс для всех оптических элементов
class OpticalElement
{
public:
    virtual ~OpticalElement() = default;

    // Отрисовка элемента
    virtual void draw(sf::RenderTarget &target) const = 0;

    // Перечисление типов элементов
    enum class Type
    {
        NONE,
        SOURCE,
        MIRROR,
        LENS,
        SPHERICAL_MIRROR
    };
    // Получение типа конкретного элемента
    virtual Type getType() const = 0;
};

#endif // HEADER_GUARD_OPTICAL_ELEMENT_HPP
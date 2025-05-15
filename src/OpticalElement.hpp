#ifndef HEADER_GUARD_OPTICAL_ELEMENT_HPP
#define HEADER_GUARD_OPTICAL_ELEMENT_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <cmath>
#include <memory>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
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
    // Поиск точки пересечения луча с элементом
    virtual VectorMath::IntersectionResult findIntersection(const Ray &ray) const
    {
        return {};
    }
    // Расчет взаимодействия луча (отражение/преломление) в точке пересечения
    virtual RayAction interact(const Ray &incomingRay, const sf::Vector2f &intersectionPoint) const { return RayAction(); }

    // Проверка, находится ли точка рядом с элементом (для выбора мышью)
    virtual bool isPointNear(const sf::Vector2f &point, float tolerance = 5.0f) const = 0;
    // Перемещение элемента на заданный вектор
    virtual void move(const sf::Vector2f &delta) = 0;
    // Получение центральной точки элемента
    virtual sf::Vector2f getCenter() const = 0;

    // Получение позиций ручек управления (по умолчанию только центр)
    virtual std::vector<sf::Vector2f> getHandles() const { return {getCenter()}; }
    // Определение, какая ручка находится в данной точке
    virtual int getHandleAtPoint(const sf::Vector2f &point, float tolerance = 8.0f) const
    {
        if (VectorMath::distance(point, getCenter()) < tolerance)
            return 0; // Ручка 0 - центр
        return -1;    // Нет ручки
    }
    // Установка новой позиции для ручки с указанным индексом
    virtual void setHandlePosition(int handleIndex, sf::Vector2f newPos, const sf::Vector2f &lastPos) {}
    // Отрисовка ручек управления
    virtual void drawHandles(sf::RenderTarget &target, sf::Color moveColor, sf::Color resizeColor) const {}

    // Поворот элемента на заданный угол (в радианах)
    virtual void rotate(float angleDelta) {}
    // Установка абсолютного угла элемента (для линейных или startAngle для круговых)
    virtual void setAngle(float newAngle) {}
    // Получение текущего угла (для линейных элементов)
    virtual float getAngle() const { return 0.f; }

    // Изменение параметра элемента (F, R, N) на заданную величину
    virtual void adjustParameter(float delta) {}
    // Получение строкового представления параметра для отображения
    virtual std::string getParameterString() const { return ""; }
    // Получение границ области отображения текста параметра
    virtual sf::FloatRect getParameterBounds() const { return {}; }
    // Установка параметра элемента из строки (ввод пользователя)
    virtual void setParameterFromString(const std::string &s) {}
    // Установка шрифта для текста параметра
    virtual void setFont(const sf::Font &font) const {}

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
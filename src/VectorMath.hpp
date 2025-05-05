// header guard
#ifndef VECTORMATH_HPP
#define VECTORMATH_HPP

#include <SFML/System/Vector2.hpp> // 2D вектор
#define _USE_MATH_DEFINES          // Для числа пи (M_PI)
#include <cmath>

// Небольшое значение для сравнения float-ов
const float EPSILON = 1e-5f;

namespace VectorMath
{

    inline float length(const sf::Vector2f &v)
    {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }

    inline sf::Vector2f normalize(const sf::Vector2f &v)
    {
        float l = length(v);
        if (l < EPSILON)
        {
            return sf::Vector2f(0.f, 0.f);
        }
        return v / l;
    }

    inline float dot(const sf::Vector2f &a, const sf::Vector2f &b)
    {
        return a.x * b.x + a.y * b.y;
    }

    inline float distance(const sf::Vector2f &p1, const sf::Vector2f &p2)
    {
        return length(p2 - p1);
    }

}

#endif // VECTORMATH_HPP
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

    inline sf::Vector2f reflect(const sf::Vector2f &v, const sf::Vector2f &n)
    {
        return v - 2.f * dot(v, n) * n;
    }

    // Структура для результата пересечения луча с отрезком
    struct IntersectionResult
    {
        sf::Vector2f point;                                 // Точка пересечения
        float distance = std::numeric_limits<float>::max(); // Расстояние от начала луча до точки
        bool intersects = false;                            // Флаг, было ли пересечение
    };

    // Проверяет пересечение лучас отрезком
    inline IntersectionResult raySegmentIntersection(const sf::Vector2f &rayOrigin, const sf::Vector2f &rayDir, const sf::Vector2f &segP1, const sf::Vector2f &segP2)
    {
        IntersectionResult result;
        sf::Vector2f v1 = rayOrigin - segP1;
        sf::Vector2f v2 = segP2 - segP1;      // Вектор отрезка
        sf::Vector2f v3(-rayDir.y, rayDir.x); // Перпендикуляр к направлению луча

        float dot_v2_v3 = dot(v2, v3);

        // Если знаменатель близок к нулю, луч и отрезок параллельны
        if (std::abs(dot_v2_v3) < EPSILON)
        {
            return result;
        }

        // t1 - параметр вдоль луча до пересечения
        float t1 = (v2.x * v1.y - v2.y * v1.x) / dot_v2_v3;
        // t2 - параметр вдоль отрезка (0 <= t2 <= 1 для пересечения)
        float t2 = dot(v1, v3) / dot_v2_v3;

        // Проверяем, что пересечение находится впереди по лучу (t1 > 0)
        // и в пределах отрезка (0 <= t2 <= 1) с небольшими допусками
        if (t1 >= EPSILON && t2 >= -EPSILON && t2 <= 1.0f + EPSILON)
        {
            result.point = rayOrigin + t1 * rayDir;
            result.distance = t1;
            result.intersects = true;
        }

        return result;
    }

    // Вычисляет расстояние от точки до отрезка
    inline float distancePointSegment(const sf::Vector2f &p, const sf::Vector2f &a, const sf::Vector2f &b)
    {
        sf::Vector2f ab = b - a;
        sf::Vector2f ap = p - a;

        float lenSq_ab = dot(ab, ab);
        // Если отрезок вырожден в точку
        if (lenSq_ab < EPSILON * EPSILON)
        {
            return distance(p, a);
        }

        // Проекция вектора ap на вектор ab, нормализованная на длину ab
        float t = dot(ap, ab) / lenSq_ab;

        // Ограничиваем t диапазоном [0, 1], чтобы проекция оставалась на отрезке
        t = std::max(0.f, std::min(1.f, t));

        // Находим точку проекции на отрезке
        sf::Vector2f projection = a + t * ab;

        return distance(p, projection);
    }

    // Нормализует угол в диапазон [-PI, PI)
    inline float normalizeAngle(float angle)
    {
        while (angle >= M_PI)
            angle -= 2.f * M_PI;
        while (angle < -M_PI)
            angle += 2.f * M_PI;
        return angle;
    }

}

#endif // VECTORMATH_HPP
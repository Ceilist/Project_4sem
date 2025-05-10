#ifndef HEADER_GUARD_POINT_SOURCE_HPP
#define HEADER_GUARD_POINT_SOURCE_HPP

#include "OpticalElement.hpp"

class PointSource : public OpticalElement
{
public:
    sf::Vector2f position; // Позиция источника
    int numRays;           // Количество испускаемых лучей
    sf::Color color;       // Цвет лучей
    float startAngle;      // Угол начала сектора испускания (в радианах)
    float spanAngle;       // Угловой размер сектора испускания (в радианах)

    PointSource(sf::Vector2f pos, int rays = 30, sf::Color c = sf::Color::Yellow, float start = 0.f, float span = 2.f * M_PI)
        : position(pos), numRays(std::max(1, rays)), color(c),
          startAngle(VectorMath::normalizeAngle(start)),
          spanAngle(std::clamp(span, 0.f, static_cast<float>(2.0 * M_PI)))
    {
    }

    Type getType() const override { return Type::SOURCE; }

    // Генерация исходящих лучей в заданном угловом диапазоне
    std::vector<Ray> emitRays() const
    {
        std::vector<Ray> rays;
        if (numRays <= 0 || spanAngle <= EPSILON)
            return rays;

        rays.reserve(numRays);
        float angleStep = 0;
        bool fullCircle = (spanAngle >= 2.f * M_PI - EPSILON); // Проверяем, близок ли угол к полному кругу

        if (fullCircle)
        {
            angleStep = 2.f * M_PI / numRays;
        }
        else if (numRays > 1)
        {
            angleStep = spanAngle / (numRays - 1);
        }

        for (int i = 0; i < numRays; ++i)
        {
            float angle = 0;
            if (fullCircle)
            {
                angle = startAngle + i * angleStep;
            }
            else
            {
                if (numRays == 1)
                {
                    angle = startAngle + spanAngle / 2.f;
                }
                else
                {
                    angle = startAngle + i * angleStep;
                }
            }

            sf::Vector2f dir = VectorMath::normalize(sf::Vector2f(std::cos(angle), std::sin(angle)));
            rays.push_back({position, dir, Ray().bounces_left, color});
        }
        return rays;
    }

    bool isPointNear(const sf::Vector2f &point, float tolerance = 8.0f) const override
    {
        return VectorMath::distance(point, position) <= tolerance;
    }
    void move(const sf::Vector2f &delta) override
    {
        position += delta;
    }
    sf::Vector2f getCenter() const override { return position; }
};

#endif // HEADER_GUARD_POINT_SOURCE_HPP
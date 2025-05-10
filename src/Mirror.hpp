#ifndef HEADER_GUARD_MIRROR_HPP
#define HEADER_GUARD_MIRROR_HPP

#include "OpticalElement.hpp"

class Mirror : public OpticalElement
{
public:
    sf::Vector2f center; // Центр отрезка зеркала
    float length;        // Длина зеркала
    float angle;         // Угол наклона зеркала (в радианах)
     sf::Color color;     // Цвет зеркала

    Mirror(sf::Vector2f c, float l, float a = 0.f, sf::Color col = sf::Color::White) : center(c), length(l), angle(a), color(col) {}
    Mirror(sf::Vector2f p1, sf::Vector2f p2, sf::Color c = sf::Color::White) : color(c)
    {
        center = (p1 + p2) / 2.f;
        sf::Vector2f delta = p2 - p1;
        length = VectorMath::length(delta);
        angle = std::atan2(delta.y, delta.x);
    }

    Type getType() const override { return Type::MIRROR; }

    // Получение координат конечных точек зеркала
    sf::Vector2f getP1() const
    {
        sf::Vector2f dir(std::cos(angle), std::sin(angle));
        return center - dir * (length / 2.f);
    }
    sf::Vector2f getP2() const
    {
        sf::Vector2f dir(std::cos(angle), std::sin(angle));
        return center + dir * (length / 2.f);
    }

    VectorMath::IntersectionResult findIntersection(const Ray &ray) const override { return VectorMath::raySegmentIntersection(ray.origin, ray.direction, getP1(), getP2()); }

    RayAction interact(const Ray &incomingRay, const sf::Vector2f &intersectionPoint) const override
    {
        sf::Vector2f p1 = getP1();
        sf::Vector2f p2 = getP2();
        sf::Vector2f segmentVec = p2 - p1;
        // Нормаль перпендикулярна отрезку
        sf::Vector2f normal = VectorMath::normalize(sf::Vector2f(-segmentVec.y, segmentVec.x));
        // Убедимся, что нормаль направлена против луча
        if (VectorMath::dot(normal, incomingRay.direction) > 0)
            normal = -normal;
        sf::Vector2f reflectedDir = VectorMath::reflect(incomingRay.direction, normal); // Отражаем луч
        // RayAction result(intersectionPoint, Ray{intersectionPoint + reflectedDir * EPSILON * 10.f, reflectedDir, incomingRay.bounces_left - 1, incomingRay.color});
        return RayAction(intersectionPoint, Ray{intersectionPoint + reflectedDir * EPSILON * 10.f, reflectedDir, incomingRay.bounces_left - 1, incomingRay.color});
    }

    bool isPointNear(const sf::Vector2f &point, float tolerance = 5.0f) const override
    {
        return VectorMath::distancePointSegment(point, getP1(), getP2()) <= tolerance;
    }
    void move(const sf::Vector2f &delta) override
    {
        center += delta;
    }
    sf::Vector2f getCenter() const override
    {
        return center;
    }

};

#endif // HEADER_GUARD_MIRROR_HPP
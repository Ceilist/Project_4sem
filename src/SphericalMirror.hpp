#ifndef HEADER_GUARD_SPHERICAL_MIRROR_HPP
#define HEADER_GUARD_SPHERICAL_MIRROR_HPP

#include "OpticalElement.hpp"

class SphericalMirror : public OpticalElement
{
public:
    sf::Vector2f center;                       // Центр кривизны
    float radius;                              // Радиус кривизны
    float startAngle;                          // Угол начала дуги (в радианах)
    float spanAngle;                           // Угловой размер дуги (в радианах, >0)
    sf::Color color;                           // Цвет зеркала
    mutable sf::Text radiusText;               // Текст "R = ..."
    mutable sf::Font const *fontPtr = nullptr; // Указатель на шрифт
    float thickness = 4.0f;                    // Визуальная толщина зеркала

    SphericalMirror(sf::Vector2f c, float r, float start, float span, sf::Color col = sf::Color(192, 192, 192))
        : center(c), radius(r), startAngle(VectorMath::normalizeAngle(start)), spanAngle(std::max(0.f, span)), color(col)
    {
        if (std::abs(radius) < 1.f)
            radius = (radius >= 0) ? 1.f : -1.f;
        if (spanAngle > 2.f * M_PI)
            spanAngle = 2.f * M_PI;
        setupText();
    }

    void setupText() const
    {
        radiusText.setCharacterSize(14);
        radiusText.setFillColor(sf::Color::White);
        updateParameterString();
    }

    Type getType() const override
    {
        return Type::SPHERICAL_MIRROR;
    }

    // Получение координат конечных точек дуги
    sf::Vector2f getP1() const
    {
        return center + std::abs(radius) * sf::Vector2f(std::cos(startAngle), std::sin(startAngle));
    }
    sf::Vector2f getP2() const
    {
        float endAngle = startAngle + spanAngle;
        return center + std::abs(radius) * sf::Vector2f(std::cos(endAngle), std::sin(endAngle));
    }
    // Получение нормали в точке на поверхности (направлена к центру кривизны)
    sf::Vector2f getNormalAt(const sf::Vector2f &pointOnSurface) const
    {
        return VectorMath::normalize(center - pointOnSurface);
    }

    VectorMath::IntersectionResult findIntersection(const Ray &ray) const override
    {
        VectorMath::IntersectionResult result;
        VectorMath::CircleIntersection circleResult = VectorMath::rayCircleIntersection(ray.origin, ray.direction, center, std::abs(radius));
        float closestDist = std::numeric_limits<float>::max();
        for (int i = 0; i < circleResult.hitCount; ++i)
        {
            float t = circleResult.t[i];
            if (t > EPSILON)
            { // Только пересечения впереди луча
                sf::Vector2f intersectPoint = ray.origin + t * ray.direction;
                float pointAngle = std::atan2(intersectPoint.y - center.y, intersectPoint.x - center.x);
                if (VectorMath::isAngleBetween(pointAngle, startAngle, spanAngle))
                {
                    if (t < closestDist)
                    {
                        closestDist = t;
                        result.point = intersectPoint;
                        result.distance = t;
                        result.intersects = true;
                    }
                }
            }
        }
        return result;
    }

    RayAction interact(const Ray &incomingRay, const sf::Vector2f &intersectionPoint) const override
    {
        sf::Vector2f normal = getNormalAt(intersectionPoint);
        sf::Vector2f reflectedDir = VectorMath::reflect(incomingRay.direction, normal);
        return RayAction(
            intersectionPoint,
            Ray{intersectionPoint + reflectedDir * EPSILON * 10.f, reflectedDir, incomingRay.bounces_left - 1, incomingRay.color});
    }

    bool isPointNear(const sf::Vector2f &point, float tolerance = 5.0f) const override
    {
        float distToCenter = VectorMath::distance(point, center);
        float r = std::abs(radius);
        if (std::abs(distToCenter - r) <= tolerance + thickness / 2.0f)
        {
            float pointAngle = std::atan2(point.y - center.y, point.x - center.x);
            return VectorMath::isAngleBetween(pointAngle, startAngle, spanAngle);
        }
        return false;
    }
    void move(const sf::Vector2f &delta) override
    {
        center += delta;
        updateParameterString();
    }
    sf::Vector2f getCenter() const override
    {
        return center;
    }
    std::vector<sf::Vector2f> getHandles() const override
    {
        return {center, getP1(), getP2()};
    } // Центр, начало, конец дуги
    int getHandleAtPoint(const sf::Vector2f &point, float tolerance = 8.0f) const override
    {
        auto handles = getHandles();
        if (VectorMath::distance(point, handles[1]) <= tolerance)
            return 1;
        if (VectorMath::distance(point, handles[2]) <= tolerance)
            return 2;
        if (VectorMath::distance(point, handles[0]) <= tolerance)
            return 0;
        if (isPointNear(point, tolerance))
            return 0;
        return -1;
    }
    void setHandlePosition(int handleIndex, sf::Vector2f newPos, const sf::Vector2f &lastPos) override
    {
        if (handleIndex == 0)
        {
            move(newPos - lastPos);
        }
        else if (handleIndex == 1)
        { // Ручка начала дуги
            sf::Vector2f delta = newPos - center;
            float newAngle = std::atan2(delta.y, delta.x);
            float oldEndAngle = VectorMath::normalizeAngle(startAngle + spanAngle);
            float newSpan = VectorMath::normalizeAngle(oldEndAngle - newAngle);
            if (newSpan <= 0)
                newSpan += 2.f * M_PI;
            startAngle = VectorMath::normalizeAngle(newAngle);
            spanAngle = std::max(0.f, std::min(newSpan, static_cast<float>(2.0 * M_PI)));
            updateParameterString();
        }
        else if (handleIndex == 2)
        { // Ручка конца дуги
            sf::Vector2f delta = newPos - center;
            float newEndAngle = std::atan2(delta.y, delta.x);
            float newSpan = VectorMath::normalizeAngle(newEndAngle - startAngle);
            if (newSpan <= 0)
                newSpan += 2.f * M_PI;
            spanAngle = std::max(0.f, std::min(newSpan, static_cast<float>(2.0 * M_PI)));
            updateParameterString();
        }
    }

    void rotate(float angleDelta) override
    {
        startAngle = VectorMath::normalizeAngle(startAngle + angleDelta);
        updateParameterString();
    }
    void setAngle(float newAngle) override
    {
        startAngle = VectorMath::normalizeAngle(newAngle);
        updateParameterString();
    }

    void adjustParameter(float delta) override
    {
        float oldSign = (radius >= 0) ? 1.f : -1.f;
        radius += delta;
        if (std::abs(radius) < 1.0f)
        {
            radius = oldSign * 1.0f;
        }
        updateParameterString();
    }

    void updateParameterString() const
    {
        if (!fontPtr)
            return;
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << radius;
        std::string rStr = ss.str();
        if (rStr.length() > 2 && rStr.substr(rStr.length() - 2) == ".0")
        {
            rStr = rStr.substr(0, rStr.length() - 2);
        }
        radiusText.setString("R = " + rStr);
        sf::Vector2f textPos = center + sf::Vector2f(0, -std::abs(radius) - 15.f);
        if (radius < 0)
        {
            textPos.y -= 10;
        }
        radiusText.setCharacterSize(14);
        sf::FloatRect textBounds = radiusText.getLocalBounds();
        radiusText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
        radiusText.setPosition(textPos);
    }
    std::string getParameterString() const override
    {
        return radiusText.getString();
    }
    sf::FloatRect getParameterBounds() const override
    {
        if (!fontPtr)
            return {};
        return radiusText.getGlobalBounds();
    }

    void setParameterFromString(const std::string &s) override
    {
        try
        {
            float newR = radius;
            if (!s.empty() && s != "-")
            {
                newR = std::stof(s);
            }
            else if (s == "-")
            {
                newR = -1.0f;
            }
            else if (s.empty())
            {
                updateParameterString();
                return;
            }
            if (std::abs(newR) < 1.0f)
            {
                radius = (newR >= 0) ? 1.0f : -1.0f;
            }
            else
            {
                radius = newR;
            }
        }
        catch (const std::invalid_argument &e)
        {
            std::cerr << "Invalid number format: '" << s << "'" << std::endl;
        }
        catch (const std::out_of_range &e)
        {
            std::cerr << "Number out of range: '" << s << "'" << std::endl;
        }
        updateParameterString();
    }
    void setFont(const sf::Font &font) const override
    {
        if (!fontPtr)
        {
            fontPtr = &font;
            radiusText.setFont(*fontPtr);
            updateParameterString();
        }
    }
    float getRadius() const
    {
        return radius;
    }
};

#endif // HEADER_GUARD_SPHERICAL_MIRROR
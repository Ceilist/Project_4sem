#ifndef HEADER_GUARD_IDEAL_LENS_HPP
#define HEADER_GUARD_IDEAL_LENS_HPP

#include "OpticalElement.hpp"

class IdealLens : public OpticalElement
{
public:
    sf::Vector2f center;                       // Центр линзы
    float height;                              // Длина (высота) линзы
    float angle;                               // Угол наклона линзы
    float focalLength;                         // Фокусное расстояние (+ для собирающей, - для рассеивающей)
    sf::Color color;                           // Цвет тела линзы
    mutable sf::Text focalLengthText;          // Текст "F = ..."
    mutable sf::Font const *fontPtr = nullptr; // Указатель на шрифт

    IdealLens(sf::Vector2f c, float h, float a, float f, sf::Color col = sf::Color::White) : center(c), height(h), angle(a), focalLength(f), color(col) { setupText(); }
    IdealLens(sf::Vector2f p1, sf::Vector2f p2, float f, sf::Color c = sf::Color::White) : focalLength(f), color(c)
    {
        center = (p1 + p2) / 2.f;
        height = VectorMath::distance(p1, p2);
        angle = std::atan2(p2.y - p1.y, p2.x - p1.x);
        setupText();
    }

    void setupText() const
    {
        focalLengthText.setCharacterSize(14);
        focalLengthText.setFillColor(sf::Color::White);
        updateParameterString();
    }

    Type getType() const override { return Type::LENS; }

    // Получение координат конечных точек линзы
    sf::Vector2f getP1() const
    {
        sf::Vector2f dir(std::cos(angle), std::sin(angle));
        return center - dir * (height / 2.f);
    }
    sf::Vector2f getP2() const
    {
        sf::Vector2f dir(std::cos(angle), std::sin(angle));
        return center + dir * (height / 2.f);
    }

    void setFont(const sf::Font &font) const override
    {
        if (!fontPtr)
        {
            fontPtr = &font;
            focalLengthText.setFont(*fontPtr);
            updateParameterString();
        }
    }

    void draw(sf::RenderTarget &target) const override
    {
        // Тело линзы
        float thickness = 3.0f;
        sf::RectangleShape rect(sf::Vector2f(height, thickness));
        rect.setOrigin(height / 2.f, thickness / 2.f);
        rect.setPosition(center);
        rect.setRotation(angle * 180.f / M_PI);
        rect.setFillColor(color);
        target.draw(rect);

        // Стрелки
        float arrowSize = 10.f;
        sf::ConvexShape arrowTop(3);
        sf::ConvexShape arrowBottom(3);
        const sf::Vector2f p1 = getP1();
        const sf::Vector2f p2 = getP2();
        sf::Vector2f dir = VectorMath::normalize(p2 - p1);
        sf::Vector2f normal(-dir.y, dir.x);
        sf::Vector2f topBase = p1;
        sf::Vector2f bottomBase = p2;
        // F<0 -> наружу, F>=0 -> внутрь
        if (focalLength < 0)
        {
            arrowTop.setPoint(0, topBase - normal * arrowSize);
            arrowTop.setPoint(1, topBase + dir * arrowSize);
            arrowTop.setPoint(2, topBase + normal * arrowSize);
            arrowBottom.setPoint(0, bottomBase - normal * arrowSize);
            arrowBottom.setPoint(1, bottomBase - dir * arrowSize);
            arrowBottom.setPoint(2, bottomBase + normal * arrowSize);
        }
        else
        {
            arrowTop.setPoint(0, topBase - normal * arrowSize);
            arrowTop.setPoint(1, topBase - dir * arrowSize);
            arrowTop.setPoint(2, topBase + normal * arrowSize);
            arrowBottom.setPoint(0, bottomBase - normal * arrowSize);
            arrowBottom.setPoint(1, bottomBase + dir * arrowSize);
            arrowBottom.setPoint(2, bottomBase + normal * arrowSize);
        }
        arrowTop.setFillColor(sf::Color::Red);
        arrowBottom.setFillColor(sf::Color::Red);
        target.draw(arrowTop);
        target.draw(arrowBottom);

        // Фокусы
        if (std::abs(focalLength) > EPSILON)
        {
            sf::Vector2f opticalAxisDir = normal;
            sf::Vector2f focus1 = center + opticalAxisDir * focalLength;
            sf::Vector2f focus2 = center - opticalAxisDir * focalLength;
            sf::CircleShape focusShape(3.f);
            focusShape.setOrigin(3.f, 3.f);
            focusShape.setFillColor(sf::Color::Blue);
            focusShape.setPosition(focus1);
            target.draw(focusShape);
            focusShape.setFillColor(sf::Color::Blue);
            focusShape.setPosition(focus2);
            target.draw(focusShape);
        }
        // Текст F=... рисуется в main.cpp
    }

    VectorMath::IntersectionResult findIntersection(const Ray &ray) const override { return VectorMath::raySegmentIntersection(ray.origin, ray.direction, getP1(), getP2()); }

    RayAction interact(const Ray &incomingRay, const sf::Vector2f &intersectionPoint) const override
    {
        // Локальная система координат линзы
        sf::Vector2f u_axis_geom = VectorMath::normalize(getP2() - getP1());
        sf::Vector2f v_axis = VectorMath::normalize(sf::Vector2f(-u_axis_geom.y, u_axis_geom.x));
        // Ориентируем локальную оптическую ось (v_axis) навстречу лучу
        if (VectorMath::dot(v_axis, incomingRay.direction) < 0.f)
            v_axis = -v_axis;
        // Ось u перпендикулярна v
        sf::Vector2f u_axis(-v_axis.y, v_axis.x);
        // Входные параметры луча в локальной системе
        float y_in = VectorMath::dot(intersectionPoint - center, u_axis); // Высота луча
        float v_axis_angle_global = std::atan2(v_axis.y, v_axis.x);
        float incoming_angle_global = std::atan2(incomingRay.direction.y, incomingRay.direction.x);
        float alpha_in = VectorMath::normalizeAngle(incoming_angle_global - v_axis_angle_global); // Угол входа отн. v_axis
        // Формула тонкой линзы для углов
        float alpha_out = alpha_in;
        if (std::abs(focalLength) > EPSILON)
        {
            float tan_alpha_in = std::tan(alpha_in);
            float tan_alpha_out_approx = tan_alpha_in - y_in / focalLength;
            alpha_out = std::atan(tan_alpha_out_approx);
        }
        // Выходной угол и направление в абсолютной системе
        float outgoing_angle_global = v_axis_angle_global + alpha_out;
        sf::Vector2f newDirection(std::cos(outgoing_angle_global), std::sin(outgoing_angle_global));
        // Формирование исходящего луча
        auto outgoingRay = Ray{intersectionPoint + newDirection * EPSILON * 10.f, VectorMath::normalize(newDirection), incomingRay.bounces_left - 1, incomingRay.color};
        return RayAction(intersectionPoint, outgoingRay);
    }

    bool isPointNear(const sf::Vector2f &point, float tolerance = 5.0f) const override { return VectorMath::distancePointSegment(point, getP1(), getP2()) <= tolerance; }
    void move(const sf::Vector2f &delta) override
    {
        center += delta;
        updateParameterString();
    }
    sf::Vector2f getCenter() const override { return center; }
    std::vector<sf::Vector2f> getHandles() const override { return {center, getP1(), getP2()}; }
    int getHandleAtPoint(const sf::Vector2f &point, float tolerance = 8.0f) const override
    {
        auto handles = getHandles();
        if (VectorMath::distance(point, handles[1]) <= tolerance)
            return 1;
        if (VectorMath::distance(point, handles[2]) <= tolerance)
            return 2;
        if (VectorMath::distance(point, handles[0]) <= tolerance)
            return 0;
        if (VectorMath::distancePointSegment(point, getP1(), getP2()) <= tolerance + 3.f)
            return 0;
        return -1;
    }
    void setHandlePosition(int handleIndex, sf::Vector2f newPos, const sf::Vector2f &lastPos) override
    {
        if (handleIndex == 0)
        {
            move(newPos - lastPos);
        }
        else if (handleIndex == 1 || handleIndex == 2)
        {
            sf::Vector2f otherP = (handleIndex == 1) ? getP2() : getP1();
            center = (newPos + otherP) / 2.f;
            sf::Vector2f delta = newPos - otherP;
            height = VectorMath::length(delta);
            angle = std::atan2(delta.y, delta.x);
            if (handleIndex == 1)
            {
                angle += M_PI;
                if (angle > M_PI)
                    angle -= 2.0 * M_PI;
            }
            updateParameterString();
        }
    }
    void drawHandles(sf::RenderTarget &target, sf::Color moveColor, sf::Color resizeColor) const override
    {
        auto handles = getHandles();
        sf::CircleShape handleShape(5.f);
        handleShape.setOrigin(5.f, 5.f);
        handleShape.setFillColor(resizeColor);
        handleShape.setPosition(handles[1]);
        target.draw(handleShape);
        handleShape.setPosition(handles[2]);
        target.draw(handleShape);
    }
    void rotate(float angleDelta) override
    {
        angle += angleDelta;
        updateParameterString();
    }
    void setAngle(float newAngle) override
    {
        angle = newAngle;
        updateParameterString();
    }
    float getAngle() const override { return angle; }

    void adjustParameter(float delta) override
    {
        focalLength += delta;
        if (std::abs(focalLength) < 1.0f && focalLength != 0.f)
        {
            focalLength = (focalLength > 0) ? 1.0f : -1.0f;
        }
        updateParameterString();
    }

    void updateParameterString() const
    {
        if (!fontPtr)
            return;
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << focalLength;
        std::string fStr = ss.str();
        if (fStr.length() > 2 && fStr.substr(fStr.length() - 2) == ".0")
        {
            fStr = fStr.substr(0, fStr.length() - 2);
        }
        focalLengthText.setString("F = " + fStr);
        sf::Vector2f p1 = getP1();
        sf::Vector2f p2 = getP2();
        sf::Vector2f dir = VectorMath::normalize(p2 - p1);
        sf::Vector2f normal(-dir.y, dir.x);
        sf::Vector2f textPos = p1 + normal * 28.f - dir * 20.f;
        focalLengthText.setCharacterSize(14);
        sf::FloatRect textBounds = focalLengthText.getLocalBounds();
        focalLengthText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
        focalLengthText.setPosition(textPos);
    }
    std::string getParameterString() const override { return focalLengthText.getString(); }
    sf::FloatRect getParameterBounds() const override
    {
        if (!fontPtr)
            return {};
        return focalLengthText.getGlobalBounds();
    }

    void setParameterFromString(const std::string &s) override
    {
        try
        {
            float newF = focalLength;
            if (!s.empty() && s != "-")
            {
                newF = std::stof(s);
            }
            else if (s.empty() || s == "-")
            {
                newF = 0.f;
            }
            if (std::abs(newF) < 1.0f && newF != 0.f)
            {
                focalLength = (newF > 0) ? 1.0f : -1.0f;
            }
            else
            {
                focalLength = newF;
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
    float getFocalLength() const { return focalLength; }
};

#endif // HEADER_GUARD_IDEAL_LENS_HPP
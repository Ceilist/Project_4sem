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
    mutable sf::Text numRaysText;              // Текст для отображения кол-ва лучей "N = ..."
    mutable sf::Font const *fontPtr = nullptr; // Указатель на шрифт

    PointSource(sf::Vector2f pos, int rays = 30, sf::Color c = sf::Color::Yellow, float start = 0.f, float span = 2.f * M_PI)
        : position(pos), numRays(std::max(1, rays)), color(c),
          startAngle(VectorMath::normalizeAngle(start)),
          spanAngle(std::clamp(span, 0.f, static_cast<float>(2.0 * M_PI)))
    {
        setupText();
    }

    void setupText() const
    {
        numRaysText.setCharacterSize(14);
        numRaysText.setFillColor(sf::Color::White);
        updateParameterString(); // Устанавливаем начальный текст
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
    void draw(sf::RenderTarget &target) const override
    {
        sf::CircleShape shape(6.f);
        shape.setOrigin(6.f, 6.f);
        shape.setPosition(position);
        shape.setFillColor(color);
        shape.setOutlineThickness(1.f);
        shape.setOutlineColor(sf::Color(255, 255, 255, 128));
        target.draw(shape);
    }

    bool isPointNear(const sf::Vector2f &point, float tolerance = 8.0f) const override
    {
        return VectorMath::distance(point, position) <= tolerance;
    }
    void move(const sf::Vector2f &delta) override
    {
        position += delta;
        updateParameterString();
    }
    sf::Vector2f getCenter() const override { return position; }

// Получение ручек: центр (0), начало угла (1), конец угла (2)
    std::vector<sf::Vector2f> getHandles() const override
    {
        float handleDist = 40.f;
        sf::Vector2f p1 = position + handleDist * sf::Vector2f(std::cos(startAngle), std::sin(startAngle));
        sf::Vector2f p2 = position + handleDist * sf::Vector2f(std::cos(startAngle + spanAngle), std::sin(startAngle + spanAngle));
        return {position, p1, p2};
    }

    int getHandleAtPoint(const sf::Vector2f &point, float tolerance = 8.0f) const override
    {
        auto handles = getHandles();
        if (VectorMath::distance(point, handles[1]) <= tolerance)
            return 1; // Ручка начала угла
        if (VectorMath::distance(point, handles[2]) <= tolerance)
            return 2; // Ручка конца угла
        if (VectorMath::distance(point, handles[0]) <= tolerance)
            return 0; // Ручка центра
        return -1;    // Не попали
    }

    void setHandlePosition(int handleIndex, sf::Vector2f newPos, const sf::Vector2f &lastPos) override
    {
        if (handleIndex == 0)
        { // Перемещение центра
            move(newPos - lastPos);
        }
        else if (handleIndex == 1)
        { // Перетаскивание ручки начала угла
            sf::Vector2f delta = newPos - position;
            float newAngle = std::atan2(delta.y, delta.x);
            float oldEndAngle = VectorMath::normalizeAngle(startAngle + spanAngle);
            float newSpan = VectorMath::normalizeAngle(oldEndAngle - newAngle);
            if (newSpan <= 0)
                newSpan += 2.f * M_PI;
            startAngle = VectorMath::normalizeAngle(newAngle);
            spanAngle = std::clamp(newSpan, 0.f, static_cast<float>(2.0 * M_PI));
        }
        else if (handleIndex == 2)
        { // Перетаскивание ручки конца угла
            sf::Vector2f delta = newPos - position;
            float newEndAngle = std::atan2(delta.y, delta.x);
            float newSpan = VectorMath::normalizeAngle(newEndAngle - startAngle);
            // Коррекция span при переходе через -PI/PI
            if (newSpan <= 0)
                newSpan += 2.f * M_PI;
            spanAngle = std::clamp(newSpan, 0.f, static_cast<float>(2.0 * M_PI));
        }
        updateParameterString();
    }

    void drawHandles(sf::RenderTarget &target, sf::Color moveColor, sf::Color resizeColor) const override
    {
        auto handles = getHandles();
        sf::CircleShape handleShape(5.f);
        handleShape.setOrigin(5.f, 5.f);

        // Ручка перемещения
        handleShape.setFillColor(moveColor);
        handleShape.setPosition(handles[0]);
        target.draw(handleShape);

        // Ручки углов
        handleShape.setFillColor(resizeColor);
        handleShape.setPosition(handles[1]);
        target.draw(handleShape);
        handleShape.setPosition(handles[2]);
        target.draw(handleShape);

        if (spanAngle < 2.f * M_PI - EPSILON)
        {
            sf::Vertex line[] = {
                sf::Vertex(handles[0], sf::Color(255, 255, 255, 70)), sf::Vertex(handles[1], sf::Color(255, 255, 255, 70)),
                sf::Vertex(handles[0], sf::Color(255, 255, 255, 70)), sf::Vertex(handles[2], sf::Color(255, 255, 255, 70))};
            target.draw(line, 4, sf::Lines);
        }
    }

    void rotate(float angleDelta) override
    {
        startAngle = VectorMath::normalizeAngle(startAngle + angleDelta);
        updateParameterString(); // Обновляем позицию текста N=...
    }

    void setAngle(float newAngle) override
    {
        startAngle = VectorMath::normalizeAngle(newAngle);
        updateParameterString();
    }

    // Изменение количества лучей
    void adjustParameter(float delta) override
    {
        numRays += static_cast<int>(delta);
        if (numRays < 1)
            numRays = 1;
        updateParameterString();
    }

    // Обновление текста "N = ..." и его позиции
    void updateParameterString() const
    {
        if (!fontPtr)
            return;
        numRaysText.setString("N = " + std::to_string(numRays));

        sf::Vector2f textPos = position + sf::Vector2f(0, -20.f);
        numRaysText.setCharacterSize(14);
        sf::FloatRect textBounds = numRaysText.getLocalBounds();
        numRaysText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
        numRaysText.setPosition(textPos);
    }

    std::string getParameterString() const override { return numRaysText.getString(); }
    sf::FloatRect getParameterBounds() const override
    {
        if (!fontPtr)
            return {};
        return numRaysText.getGlobalBounds();
    }

    void setParameterFromString(const std::string &s) override
    {
        try
        {
            int newN = numRays;
            if (!s.empty())
            {
                newN = std::stoi(s);
            }
            else
            {
                updateParameterString();
                return;
            }
            numRays = std::max(1, newN);
        }
        catch (const std::invalid_argument &e)
        {
            std::cerr << "Invalid number format for N: '" << s << "'" << std::endl;
        }
        catch (const std::out_of_range &e)
        {
            std::cerr << "Number out of range for N: '" << s << "'" << std::endl;
        }
        updateParameterString();
    }

    void setFont(const sf::Font &font) const override
    {
        if (!fontPtr)
        {
            fontPtr = &font;
            numRaysText.setFont(*fontPtr);
            updateParameterString();
        }
    }

    int getNumRays() const { return numRays; }
};

#endif // HEADER_GUARD_POINT_SOURCE_HPP
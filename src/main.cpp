#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <optional>

#include "OpticalElement.hpp"
#include "PointSource.hpp"
#include "Mirror.hpp"
#include "VectorMath.hpp"

// Псевдоним типа для пути луча
using RayPath = std::vector<sf::Vertex>;

// Глобальные переменные для оптических элементов, источников и путей лучей
std::vector<std::unique_ptr<OpticalElement>> elements;
std::vector<const PointSource *> sources; // Вектор указателей на источники
std::vector<RayPath> rayPaths;            // Вектор путей лучей

const float MAX_RAY_LENGTH = 2000.f;

void rebuildSourcesVector()
{
    sources.clear();
    for (const auto &el : elements)
    {
        if (!el)
            continue;

        if (auto *src = dynamic_cast<const PointSource *>(el.get()))
        {
            sources.push_back(src);
        }
    }
}

void traceRays()
{
    rayPaths.clear();
    if (sources.empty())
        return;

    for (const auto *source : sources)
    {
        if (!source)
            continue;

        std::vector<Ray> currentRaysFromSource = source->emitRays();

        for (Ray currentRay : currentRaysFromSource)
        {
            RayPath path; // Путь для текущего индивидуального луча
            path.push_back(sf::Vertex(currentRay.origin, currentRay.color));
            size_t sourceElementIndex = static_cast<size_t>(-1);
            for (size_t i = 0; i < elements.size(); ++i)
            {
                if (elements[i].get() == source)
                {
                    sourceElementIndex = i;
                    break;
                }
            }
            size_t lastHitIndex = sourceElementIndex;

            while (currentRay.bounces_left > 0)
            {
                VectorMath::IntersectionResult closestIntersection;
                closestIntersection.distance = MAX_RAY_LENGTH; // Инициализируем максимальным расстоянием
                const OpticalElement *hitElement = nullptr;
                size_t currentHitIndex = static_cast<size_t>(-1);

                // Ищем пересечения со всеми оптическими элементами
                for (size_t i = 0; i < elements.size(); ++i)
                {
                    if (!elements[i] || elements[i]->getType() == OpticalElement::Type::SOURCE)
                        continue;

                    VectorMath::IntersectionResult intersection = elements[i]->findIntersection(currentRay);

                    // Если найдено пересечение, оно ближе текущего ближайшего и находится перед лучом
                    if (intersection.intersects && intersection.distance > EPSILON && intersection.distance < closestIntersection.distance)
                    {
                        closestIntersection = intersection;
                        hitElement = elements[i].get();
                        currentHitIndex = i;
                    }
                }

                if (hitElement) // Если найдено пересечение с каким-либо элементом
                {
                    path.push_back(sf::Vertex(closestIntersection.point, currentRay.color));
                    RayAction interaction = hitElement->interact(currentRay, closestIntersection.point);

                    if (interaction.outgoingRay.has_value() && interaction.outgoingRay.value().bounces_left > 0)
                    {
                        currentRay = interaction.outgoingRay.value(); // Продолжаем трассировку с новым лучом
                        lastHitIndex = currentHitIndex;               // Запоминаем элемент, с которым было взаимодействие
                    }
                    else
                    {
                        break; // Луч поглощен, или закончились отскоки, или нет исходящего луча
                    }
                }
                else // Если пересечений не найдено, луч уходит в "бесконечность"
                {
                    path.push_back(sf::Vertex(currentRay.origin + currentRay.direction * MAX_RAY_LENGTH, currentRay.color));
                    break;
                }
            }
            if (path.size() > 1) // Добавляем путь, если он состоит более чем из одной точки
            {
                rayPaths.push_back(path);
            }
        }
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(1200, 800), "Interactive Ray Optics - Backend Logic");

    elements.push_back(std::make_unique<PointSource>(sf::Vector2f(100.f, 300.f)));
    rebuildSourcesVector(); // Обновляем список источников

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        traceRays(); // Вызываем трассировку на каждом кадре (для теста)

        window.clear(sf::Color::Black);
        
        for (const auto &p : rayPaths)
        {
            if (p.size() >= 2)
            {
                window.draw(p.data(), p.size(), sf::LinesStrip);
            }
        }

        window.display();
    }

    return 0;
}

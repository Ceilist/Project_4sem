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

// Константы окна
const unsigned int WINDOW_WIDTH = 1200;
const unsigned int WINDOW_HEIGHT = 800;

// Псевдонимы и структуры для данных 
using RayPath = std::vector<sf::Vertex>;
sf::Vector2f mousePos;

// Прототипы функций 
void handleEvents(sf::RenderWindow &window);
void traceRays(/* ... */); // Пока пустая или заглушка
void render(sf::RenderWindow &window);

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "SFML Ray Optics v1");

    while (window.isOpen()) {
        mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        handleEvents(window); 
        render(window); 
    }
    return 0;
}

// Реализация handleEvents 
void handleEvents(sf::RenderWindow &window) {
    sf::Event event;
    while (window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                window.close();
                break;
                
            default: break;
        }
    }
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

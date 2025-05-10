#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <optional>
#include <iostream> // Добавлено для возможной отладки

#include "OpticalElement.hpp"
#include "PointSource.hpp"
#include "Mirror.hpp"
#include "VectorMath.hpp"

enum class Mode {
    IDLE,
    PLACING_START,
    PLACING_END,
    DRAGGING_ELEMENT
    };

// Псевдоним типа для пути луча
using RayPath = std::vector<sf::Vertex>;

// Глобальные переменные для оптических элементов, источников и путей лучей
std::vector<std::unique_ptr<OpticalElement>> elements;
std::vector<const PointSource *> sources; // Вектор указателей на источники
std::vector<RayPath> rayPaths;            // Вектор путей лучей
Mode currentMode = Mode::IDLE;
std::optional<size_t> selectedElementIndex;
OpticalElement::Type placementType = OpticalElement::Type::NONE;
sf::Vector2f mousePos;
sf::Vector2f placementStartPos;
sf::VertexArray placementPreviewLine{sf::Lines, 2};
sf::Vector2f lastMousePos;

const float MAX_RAY_LENGTH = 2000.f; // максимальная длина луча

std::optional<size_t> findElementAt(const sf::Vector2f &pos) {
    for (int i = elements.size() - 1; i >= 0; --i) {
        if (elements[i] && elements[i]->isPointNear(pos)) {
            return static_cast<size_t>(i);
        }
    }
    return std::nullopt;
}

void rebuildSourcesVector();
void handleEvents(sf::RenderWindow &window) {
    sf::Event event;
    while (window.pollEvent(event)) {
        // ... case Closed ...
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                currentMode = Mode::IDLE;
                selectedElementIndex.reset();
                placementType = OpticalElement::Type::NONE;
            } else if (event.key.code == sf::Keyboard::S && currentMode == Mode::IDLE) {
                elements.push_back(std::make_unique<PointSource>(mousePos));
                rebuildSourcesVector();
            } else if (event.key.code == sf::Keyboard::M && currentMode == Mode::IDLE) {
                currentMode = Mode::PLACING_START;
                placementType = OpticalElement::Type::MIRROR;
                selectedElementIndex.reset();
            }
        } else if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                if (currentMode == Mode::PLACING_START) {
                    placementStartPos = mousePos;
                    placementPreviewLine[0].position = placementStartPos;
                    placementPreviewLine[1].position = mousePos; // Init end point
                    currentMode = Mode::PLACING_END;
                } else if (currentMode == Mode::PLACING_END) {
                    if (placementType == OpticalElement::Type::MIRROR) {
                        if (VectorMath::distance(placementStartPos, mousePos) > 1.0f) { // Min length
                             elements.push_back(std::make_unique<Mirror>(placementStartPos, mousePos));
                        }
                    }
                    currentMode = Mode::IDLE;
                    placementType = OpticalElement::Type::NONE;
                } else if (currentMode == Mode::IDLE) {
                    selectedElementIndex = findElementAt(mousePos);
                    if (selectedElementIndex.has_value()) {
                        currentMode = Mode::DRAGGING_ELEMENT;
                        lastMousePos = mousePos;
                    }
                }
            }
        } else if (event.type == sf::Event::MouseButtonReleased) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                if (currentMode == Mode::DRAGGING_ELEMENT) {
                    currentMode = Mode::IDLE;
                }
            }
        } else if (event.type == sf::Event::MouseMoved) {
            if (currentMode == Mode::PLACING_END) {
                placementPreviewLine[1].position = mousePos;
            } else if (currentMode == Mode::DRAGGING_ELEMENT && selectedElementIndex.has_value()) {
                sf::Vector2f delta = mousePos - lastMousePos;
                elements[selectedElementIndex.value()]->move(delta);
                lastMousePos = mousePos;
            }
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

            while (currentRay.bounces_left > 0)
            {
                VectorMath::IntersectionResult closestIntersection;
                closestIntersection.distance = MAX_RAY_LENGTH; // Инициализируем максимальным расстоянием
                const OpticalElement *hitElement = nullptr;
                // size_t currentHitIndex = static_cast<size_t>(-1); // Индекс элемента, с которым произошло пересечение

                // Ищем пересечения со всеми оптическими элементами
                for (size_t i = 0; i < elements.size(); ++i)
                {
                    if (!elements[i] || elements[i]->getType() == OpticalElement::Type::SOURCE) // Источники не должны прерывать лучи
                        continue;

                    // if (elements[i].get() == reinterpret_cast<const OpticalElement*>(source) && path.size() == 1) // Не пересекать самого себя на первом шаге
                    //    continue;

                    VectorMath::IntersectionResult intersection = elements[i]->findIntersection(currentRay);

                    // Если найдено пересечение, оно ближе текущего ближайшего и находится перед лучом
                    if (intersection.intersects && intersection.distance > EPSILON && intersection.distance < closestIntersection.distance)
                    {
                        closestIntersection = intersection;
                        hitElement = elements[i].get();
                        // currentHitIndex = i;
                    }
                }

                if (hitElement) // Если найдено пересечение с каким-либо элементом
                {
                    path.push_back(sf::Vertex(closestIntersection.point, currentRay.color));
                    RayAction interaction = hitElement->interact(currentRay, closestIntersection.point);

                    if (interaction.outgoingRay.has_value() && interaction.outgoingRay.value().bounces_left > 0)
                    {
                        currentRay = interaction.outgoingRay.value(); // Продолжаем трассировку с новым лучом
                        // lastHitIndex = currentHitIndex; // Запоминаем элемент, с которым было взаимодействие
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

void render(sf::RenderWindow &window) {
    traceRays(); // Вызываем трассировку на каждом кадре

    window.clear(sf::Color::Black);

    // Отрисовка оптических элементов
    for (const auto& el : elements) {
        if (el) {
            if (auto* mirror = dynamic_cast<Mirror*>(el.get())) {
                sf::Vertex line[] = {
                    sf::Vertex(mirror->getP1(), mirror->color),
                    sf::Vertex(mirror->getP2(), mirror->color)
                };
                window.draw(line, 2, sf::Lines);
            } else if (auto* source = dynamic_cast<PointSource*>(el.get())) {
                sf::CircleShape shape(5.f); // Небольшой круг для источника
                shape.setFillColor(source->color);
                shape.setOrigin(5.f, 5.f);
                shape.setPosition(source->position);
                window.draw(shape);
            }
        }
    }

    // Отрисовка путей лучей
    for (const auto &p : rayPaths)
    {
        if (p.size() >= 2)
        {
            window.draw(p.data(), p.size(), sf::LinesStrip);
        }
    }

    window.display();
    for (const auto &path : rayPaths) { /* ... */ }

    if (currentMode == Mode::PLACING_END && (placementType == OpticalElement::Type::MIRROR /* || LENS later */)) {
        window.draw(placementPreviewLine);
    }

    for (size_t i = 0; i < elements.size(); ++i) {
        if (elements[i]) {
            elements[i]->draw(window);
            if (selectedElementIndex.has_value() && selectedElementIndex.value() == i && currentMode != Mode::PLACING_START && currentMode != Mode::PLACING_END) {
                // Простое выделение: Нарисуем круг в центре выбранного элемента
                sf::CircleShape selectionCircle(5.f);
                selectionCircle.setOrigin(5.f, 5.f);
                selectionCircle.setPosition(elements[i]->getCenter());
                selectionCircle.setFillColor(sf::Color::Transparent);
                selectionCircle.setOutlineColor(sf::Color::Blue);
                selectionCircle.setOutlineThickness(2.f);
                window.draw(selectionCircle);
            }
        }
    }
    window.display();
}

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


const unsigned int WINDOW_WIDTH = 1200;
const unsigned int WINDOW_HEIGHT = 800;


int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "SFML Ray Optics v1");
    window.setFramerateLimit(60); 

    // Пример
    elements.push_back(std::make_unique<PointSource>(sf::Vector2f(100.f, WINDOW_HEIGHT / 2.f), 360));
    elements.push_back(std::make_unique<Mirror>(sf::Vector2f(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f), 200.f, (float)M_PI / 4.f));

    rebuildSourcesVector(); // Обновляем список источников после добавления элементов

    while (window.isOpen()) {
        mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        handleEvents(window);
        render(window);
    }
    return 0;
}




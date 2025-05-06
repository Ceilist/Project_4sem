#include <SFML/Graphics.hpp>
#include <vector>       
#include <memory>       
#include <optional>     
// #include "OpticalElement.hpp" // Пока не используется напрямую

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

// Реализация traceRays (заглушка)
void traceRays(/* ... */) {
}

// Реализация render 
void render(sf::RenderWindow &window) {
    window.clear(sf::Color(30, 30, 50));

    window.display();
}

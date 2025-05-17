#ifndef OPTICAL_APPLICATION_HPP
#define OPTICAL_APPLICATION_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "AppDefs.hpp"
#include "Constants.hpp"

// Заголовочные файлы для оптических элементов
#include "OpticalElement.hpp"
#include "PointSource.hpp"
#include "Mirror.hpp"
#include "IdealLens.hpp"
#include "SphericalMirror.hpp"
#include "VectorMath.hpp"
#include <iostream>


class OpticalApplication {
public:
    OpticalApplication();
    ~OpticalApplication();
    void run();

private:
    // SFML и окно
    sf::RenderWindow m_window;
    sf::Font m_font;
    bool m_fontLoaded;
    sf::Clock m_fpsClock;
    int m_frameCount;


    std::vector<OpticalElement*> m_elements;
    std::vector<const PointSource*> m_sources;
    std::vector<RayPath> m_rayPaths;

    // Состояние и UI-
    Mode m_currentMode;
    OpticalElement::Type m_placementType;
    std::optional<size_t> m_selectedElementIndex;
    int m_activeHandleIndex; // Используем HandleType для значений


    sf::Vector2f m_mousePos;         // Текущая позиция мыши
    sf::Vector2f m_lastMousePos;     // Предыдущая позиция мыши
    sf::Vector2f m_placementStartPos; // Начальная точка для размещения элемента
    sf::Text m_helpText;
    sf::Text m_editPromptText;       // "F = ", "R = ", "N = "
    sf::Text m_inputTextDisplay;     // Отображение вводимой строки m_currentInputString
    sf::RectangleShape m_inputBackground; // Фон для поля ввода

    sf::VertexArray m_placementPreviewLine;    // Для Mirror, IdealLens
    sf::CircleShape m_placementPreviewCircle;  // Для SphericalMirror


    std::string m_currentInputString;      // Строка, которую пользователь вводит для параметра
    std::string m_parameterBackupString;   // Сохранение значения параметра перед редактированием (для отмены)



    bool initialize();              // Главный метод инициализации
    bool loadFontInternal();        // Загрузка шрифта
    void setupUIElements();         // Первичная настройка sf::Text, sf::Shape и т.д.
    void createDefaultScene();


    void processEvents();           // Обработка всех событий SFML
    void update();                  // Обновление состояния приложения
    void render();                  // Отрисовка всего
    void handleSingleEvent(const sf::Event& event); // Обработчик одного события
    void handleTextEditingEvent(const sf::Event& event); // Если m_currentMode == EDITING_PARAMETER
    void handleGeneralEvent(const sf::Event& event);     // Если m_currentMode != EDITING_PARAMETER

    void handleKeyPressed(const sf::Event::KeyEvent& keyEvent);
    void handleMouseButtonPressed(const sf::Event::MouseButtonEvent& mouseButtonEvent);
    void handleMouseButtonReleased(const sf::Event::MouseButtonEvent& mouseButtonEvent);
    void handleMouseMoved(const sf::Event::MouseMoveEvent& mouseMoveEvent);
    void handleMouseWheelScrolled(const sf::Event::MouseWheelScrollEvent& mouseWheelEvent);
    void handleTextEntered(const sf::Event::TextEvent& textEvent);

    void updateMouseState();                   // Обновление m_mousePos, m_lastMousePos
    void updatePlacementPreviewVisuals();      // Обновление m_placementPreviewLine/Circle
    void updateDraggingLogic();                // Логика перетаскивания элемента/ручки
    void updateAndPositionParameterEditorUI(); // Расчет размеров/позиций для UI редактирования
    void updateFPSDisplay();                   // Обновление заголовка окна с FPS

    void drawAllRayPaths();
    void drawActivePlacementPreview();
    void drawElementsAndUI(); // Рисует элементы, их ручки и текст параметра
    void drawMainHelpText();
    void drawParameterEditingUI(); // Рисует UI для ввода текста параметра

    void traceRaysInternal();       // Трассировка лучей
    void rebuildSourcesVector();    // Обновление m_sources

    std::optional<size_t> findElementAt(const sf::Vector2f& pos); // Находит элемент под курсором
    void selectElementByIndex(std::optional<size_t> index, int handleIndexIfSelected = static_cast<int>(HandleType::MOVE));
    void deleteSelectedElementLogic();
    void beginParameterEditForSelected();
    void confirmParameterEdit();
    void cancelParameterEdit();
    void adjustSelectedParameterValue(float direction); // +1 или -1
    void rotateSelectedElementByDelta(float angleDelta);

    std::string getCurrentParameterValueAsString(const OpticalElement* element) const;
    void setFontForElement(OpticalElement* element); // Устанавливает шрифт для текста элемента
};

#endif // OPTICAL_APPLICATION_HPP
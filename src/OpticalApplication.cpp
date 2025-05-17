#include "OpticalApplication.hpp"
#include <iostream>

// Конструктор и деструктор
OpticalApplication::OpticalApplication()
    : m_window(sf::VideoMode(AppConstants::WINDOW_WIDTH, AppConstants::WINDOW_HEIGHT), AppConstants::WINDOW_TITLE_BASE),
      m_fontLoaded(false),
      m_frameCount(0),
      m_currentMode(Mode::IDLE),
      m_placementType(OpticalElement::Type::NONE),
      m_selectedElementIndex(std::nullopt),
      m_activeHandleIndex(static_cast<int>(HandleType::NONE)),
      m_placementPreviewLine(sf::Lines, 2)
{
    m_window.setFramerateLimit(60);
    if (!initialize()) {
        std::cerr << "Critical error: Application initialization failed." << std::endl;
        m_window.close();
    }
}

OpticalApplication::~OpticalApplication() {
    for (OpticalElement* el : m_elements) {
        delete el;
    }
    m_elements.clear();
}

// Запуск приложения
void OpticalApplication::run() {
    if (!m_fontLoaded) {
        return;
    }
    m_fpsClock.restart();

    while (m_window.isOpen()) {
        updateMouseState();
        processEvents();
        update();
        traceRaysInternal();
        render();
    }
}

// Инициализация
bool OpticalApplication::initialize() {
    m_fontLoaded = loadFontInternal();
    if (!m_fontLoaded) {
        return false;
    }
    setupUIElements();
   // createDefaultScene(); Для вызова дефолт системы
    rebuildSourcesVector();
    m_lastMousePos = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
    return true;
}

bool OpticalApplication::loadFontInternal() {
    for (const auto& path : AppConstants::FONT_PATHS) {
        if (m_font.loadFromFile(path)) {
            std::cout << "Font loaded successfully from: " << path << std::endl;
            return true;
        }
    }
    std::cerr << "Error loading font! Please ensure one of the font paths in Constants.hpp is accessible." << std::endl;
    return false;
}

void OpticalApplication::setupUIElements() {
    m_helpText.setFont(m_font);
    m_helpText.setCharacterSize(AppConstants::FONT_SIZE_UI);
    m_helpText.setFillColor(AppConstants::COLOR_HELP_TEXT);
    m_helpText.setString("Place: [M] Mirror | [L] Lens | [S] Source | [B] Sph. Mirror | [Del] Delete \nSelect & [=] Edit Param | [+/-] Adjust | [Wheel] Rotate ");
    m_helpText.setPosition(10.f, 10.f);

    m_placementPreviewCircle.setFillColor(sf::Color::Transparent);
    m_placementPreviewCircle.setOutlineThickness(2.f);

    m_editPromptText.setFont(m_font);
    m_editPromptText.setCharacterSize(AppConstants::FONT_SIZE_UI);
    m_editPromptText.setFillColor(AppConstants::COLOR_INPUT_TEXT_FG);

    m_inputTextDisplay.setFont(m_font);
    m_inputTextDisplay.setCharacterSize(AppConstants::FONT_SIZE_UI);
    m_inputTextDisplay.setFillColor(AppConstants::COLOR_INPUT_TEXT_FG);

    m_inputBackground.setFillColor(AppConstants::COLOR_INPUT_TEXT_BG);
    m_inputBackground.setOutlineColor(AppConstants::COLOR_INPUT_TEXT_OUTLINE);
    m_inputBackground.setOutlineThickness(1.f);
}

// Система по умолчанию
void OpticalApplication::createDefaultScene() {
    m_elements.push_back(new PointSource(sf::Vector2f(100.f, AppConstants::WINDOW_HEIGHT / 2.f), 360));
    setFontForElement(m_elements.back());
    m_elements.push_back(new Mirror(sf::Vector2f(AppConstants::WINDOW_WIDTH / 2.f, AppConstants::WINDOW_HEIGHT / 2.f), 200.f, (float)M_PI / 4.f));
    setFontForElement(m_elements.back());
}


void OpticalApplication::setFontForElement(OpticalElement* element) {
    if (element && m_fontLoaded) {
        element->setFont(m_font);
    }
}

// Цикличные функии
void OpticalApplication::updateMouseState() {
    m_lastMousePos = m_mousePos;
    m_mousePos = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
}

void OpticalApplication::processEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        handleSingleEvent(event);
    }
}

void OpticalApplication::update() {
    updatePlacementPreviewVisuals();
    updateDraggingLogic();
    if (m_currentMode == Mode::EDITING_PARAMETER) {
        updateAndPositionParameterEditorUI();
    }
    updateFPSDisplay();
}

void OpticalApplication::render() {
    m_window.clear(AppConstants::COLOR_BACKGROUND);
    drawAllRayPaths();
    drawElementsAndUI();
    drawActivePlacementPreview();
    drawMainHelpText();
    if (m_currentMode == Mode::EDITING_PARAMETER) {
        drawParameterEditingUI();
    }
    m_window.display();
}

// Обработка событий
void OpticalApplication::handleSingleEvent(const sf::Event& event) {
    if (event.type == sf::Event::Closed) {
        m_window.close();
        return;
    }
    if (m_currentMode == Mode::EDITING_PARAMETER) {
        handleTextEditingEvent(event);
    } else {
        handleGeneralEvent(event);
    }
}

void OpticalApplication::handleTextEditingEvent(const sf::Event& event) {
    if (event.type == sf::Event::TextEntered) {
        handleTextEntered(event.text);
    } else if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return) {
            confirmParameterEdit();
        } else if (event.key.code == sf::Keyboard::Escape) {
            cancelParameterEdit();
        } else if (event.key.code == sf::Keyboard::Backspace && !m_currentInputString.empty()) {
            m_currentInputString.pop_back();
        }
    } else if (event.type == sf::Event::MouseButtonPressed) {
        if (!m_inputBackground.getGlobalBounds().contains(m_mousePos)) {
            confirmParameterEdit();
        }
    }
}

void OpticalApplication::handleGeneralEvent(const sf::Event& event) {
    switch (event.type) {
    case sf::Event::KeyPressed:          handleKeyPressed(event.key);                 break;
    case sf::Event::MouseButtonPressed:  handleMouseButtonPressed(event.mouseButton); break;
    case sf::Event::MouseButtonReleased: handleMouseButtonReleased(event.mouseButton);break;
    case sf::Event::MouseMoved:          handleMouseMoved(event.mouseMove);           break;
    case sf::Event::MouseWheelScrolled:  handleMouseWheelScrolled(event.mouseWheelScroll);  break;
    default: break;
    }
}

void OpticalApplication::handleKeyPressed(const sf::Event::KeyEvent& keyEvent) {
    if (keyEvent.code == sf::Keyboard::Escape) {
        if (m_currentMode == Mode::PLACING_END || m_currentMode == Mode::PLACING_START) {
            m_currentMode = Mode::IDLE;
            m_placementType = OpticalElement::Type::NONE;
        } else {
            selectElementByIndex(std::nullopt);
        }
        return;
    }
    if (m_currentMode == Mode::IDLE) {
        OpticalElement::Type newType = OpticalElement::Type::NONE;
        if (keyEvent.scancode == sf::Keyboard::Scan::M) newType = OpticalElement::Type::MIRROR;
        else if (keyEvent.scancode == sf::Keyboard::Scan::L) newType = OpticalElement::Type::LENS;
        else if (keyEvent.scancode == sf::Keyboard::Scan::B) newType = OpticalElement::Type::SPHERICAL_MIRROR;

        if (newType != OpticalElement::Type::NONE) {
            m_currentMode = Mode::PLACING_START;
            m_placementType = newType;
            selectElementByIndex(std::nullopt);
            if (newType == OpticalElement::Type::MIRROR) {
                 m_placementPreviewLine[0].color = AppConstants::COLOR_PLACEMENT_PREVIEW_MIRROR;
                 m_placementPreviewLine[1].color = AppConstants::COLOR_PLACEMENT_PREVIEW_MIRROR;
            } else if (newType == OpticalElement::Type::LENS) {
                 m_placementPreviewLine[0].color = AppConstants::COLOR_PLACEMENT_PREVIEW_LENS;
                 m_placementPreviewLine[1].color = AppConstants::COLOR_PLACEMENT_PREVIEW_LENS;
            } else if (newType == OpticalElement::Type::SPHERICAL_MIRROR) {
                 m_placementPreviewCircle.setOutlineColor(AppConstants::COLOR_PLACEMENT_PREVIEW_SPHERICAL_MIRROR);
            }
            return;
        }
        if (keyEvent.scancode == sf::Keyboard::Scan::S) {
            m_elements.push_back(new PointSource(m_mousePos, 30, sf::Color::Yellow));
            setFontForElement(m_elements.back());
            rebuildSourcesVector();
            selectElementByIndex(m_elements.size() - 1);
            return;
        }
    }
    if (m_selectedElementIndex.has_value()) {
        if (keyEvent.code == sf::Keyboard::Delete || keyEvent.code == sf::Keyboard::Backspace) {
            deleteSelectedElementLogic();
        } else if (keyEvent.code == sf::Keyboard::Equal && !keyEvent.shift) {
            beginParameterEditForSelected();
        } else if (keyEvent.code == sf::Keyboard::Add || (keyEvent.code == sf::Keyboard::Equal && keyEvent.shift)) {
            adjustSelectedParameterValue(1.0f);
        } else if (keyEvent.code == sf::Keyboard::Subtract || keyEvent.code == sf::Keyboard::Hyphen) {
            adjustSelectedParameterValue(-1.0f);
        }
    }
}

void OpticalApplication::handleMouseButtonPressed(const sf::Event::MouseButtonEvent& mouseButtonEvent) {
    if (mouseButtonEvent.button != sf::Mouse::Left) return;

    if (m_currentMode == Mode::PLACING_START) {
        m_placementStartPos = m_mousePos;
        m_currentMode = Mode::PLACING_END;
    } else if (m_currentMode == Mode::PLACING_END) {
        bool placed = false;
        if (VectorMath::distance(m_placementStartPos, m_mousePos) > AppConstants::MIN_ELEMENT_PLACEMENT_DISTANCE) {
            if (m_placementType == OpticalElement::Type::MIRROR) {
                m_elements.push_back(new Mirror(m_placementStartPos, m_mousePos));
                placed = true;
            } else if (m_placementType == OpticalElement::Type::LENS) {
                m_elements.push_back(new IdealLens(m_placementStartPos, m_mousePos, 100.f));
                placed = true;
            } else if (m_placementType == OpticalElement::Type::SPHERICAL_MIRROR) {
                float radius = VectorMath::distance(m_mousePos, m_placementStartPos);
                float angleToMouse = std::atan2(m_mousePos.y - m_placementStartPos.y, m_mousePos.x - m_placementStartPos.x);
                float span = static_cast<float>(M_PI) / 1.5f;
                float start = VectorMath::normalizeAngle(angleToMouse - span / 2.f);
                m_elements.push_back(new SphericalMirror(m_placementStartPos, radius, start, span));
                placed = true;
            }
            if (placed && !m_elements.empty()) {
                setFontForElement(m_elements.back());
                rebuildSourcesVector();
                selectElementByIndex(m_elements.size() - 1);
            }
        }
        m_placementType = OpticalElement::Type::NONE;
        m_currentMode = Mode::IDLE;
    } else if (m_currentMode == Mode::IDLE) {
        std::optional<size_t> foundIndex = findElementAt(m_mousePos);
        if (foundIndex.has_value()) {
            selectElementByIndex(foundIndex);
            m_activeHandleIndex = m_elements[m_selectedElementIndex.value()]->getHandleAtPoint(m_mousePos, AppConstants::HANDLE_SELECT_TOLERANCE);
            if (m_activeHandleIndex == static_cast<int>(HandleType::NONE)) {
                m_activeHandleIndex = static_cast<int>(HandleType::MOVE);
            }
            m_currentMode = Mode::DRAGGING_ELEMENT;
        } else {
            selectElementByIndex(std::nullopt);
        }
    }
}

void OpticalApplication::handleMouseButtonReleased(const sf::Event::MouseButtonEvent& mouseButtonEvent) {
    if (mouseButtonEvent.button == sf::Mouse::Left) {
        if (m_currentMode == Mode::DRAGGING_ELEMENT) {
            m_currentMode = Mode::IDLE;
        }
    }
}

void OpticalApplication::handleMouseMoved(const sf::Event::MouseMoveEvent& ) {
}

void OpticalApplication::handleMouseWheelScrolled(const sf::Event::MouseWheelScrollEvent& mouseWheelEvent) {
    if (mouseWheelEvent.wheel == sf::Mouse::VerticalWheel && m_selectedElementIndex.has_value() && m_currentMode != Mode::EDITING_PARAMETER) {
        bool overInputField = false;
        if (m_currentMode == Mode::EDITING_PARAMETER && m_inputBackground.getGlobalBounds().contains(m_mousePos)) {
             overInputField = true;
        }
        if (!overInputField) {
            rotateSelectedElementByDelta(-mouseWheelEvent.delta * AppConstants::ROTATION_SPEED);
        }
    }
}

void OpticalApplication::handleTextEntered(const sf::Event::TextEvent& textEvent) {
     // Только видимые ASCII символы, backspace и delete
    if (textEvent.unicode >= 32 && textEvent.unicode < 127) {
        char enteredChar = static_cast<char>(textEvent.unicode);
        bool allowChar = false;
        if (m_selectedElementIndex.has_value()) {
            auto type = m_elements[m_selectedElementIndex.value()]->getType();
            if (type == OpticalElement::Type::LENS || type == OpticalElement::Type::SPHERICAL_MIRROR) {
                if (std::isdigit(enteredChar)) allowChar = true;
                else if (enteredChar == '.' && m_currentInputString.find('.') == std::string::npos) allowChar = true;
                else if (enteredChar == '-' && m_currentInputString.empty()) allowChar = true;
            } else if (type == OpticalElement::Type::SOURCE) {
                if (std::isdigit(enteredChar)) allowChar = true;
            }
            if (allowChar) {
                m_currentInputString += enteredChar;
            }
        }
    }
}

// Обновление логики
void OpticalApplication::updatePlacementPreviewVisuals() {
    if (m_currentMode == Mode::PLACING_END) {
        if (m_placementType == OpticalElement::Type::MIRROR || m_placementType == OpticalElement::Type::LENS) {
            m_placementPreviewLine[0].position = m_placementStartPos;
            m_placementPreviewLine[1].position = m_mousePos;
        } else if (m_placementType == OpticalElement::Type::SPHERICAL_MIRROR) {
            float radius = VectorMath::distance(m_mousePos, m_placementStartPos);
            m_placementPreviewCircle.setRadius(radius);
            m_placementPreviewCircle.setOrigin(radius, radius);
            m_placementPreviewCircle.setPosition(m_placementStartPos);
        }
    }
}

void OpticalApplication::updateDraggingLogic() {
    if (m_currentMode == Mode::DRAGGING_ELEMENT && m_selectedElementIndex.has_value() && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        if (m_activeHandleIndex != static_cast<int>(HandleType::NONE)) {
             m_elements[m_selectedElementIndex.value()]->setHandlePosition(m_activeHandleIndex, m_mousePos, m_lastMousePos);
        }
    }
}

void OpticalApplication::updateAndPositionParameterEditorUI() {
    if (m_currentMode != Mode::EDITING_PARAMETER || !m_selectedElementIndex.has_value() || !m_fontLoaded) return;
    const auto& el = m_elements[m_selectedElementIndex.value()];
    if (!el) return;

    auto type = el->getType();
    if (type == OpticalElement::Type::LENS) m_editPromptText.setString("F = ");
    else if (type == OpticalElement::Type::SPHERICAL_MIRROR) m_editPromptText.setString("R = ");
    else if (type == OpticalElement::Type::SOURCE) m_editPromptText.setString("N = ");
    else m_editPromptText.setString("Val = ");

    m_inputTextDisplay.setString(m_currentInputString);

    sf::FloatRect originalTextBounds = el->getParameterBounds();
    sf::Vector2f promptPosition;
    if (originalTextBounds.width > 0 || originalTextBounds.height > 0) {
        promptPosition = sf::Vector2f(originalTextBounds.left, originalTextBounds.top);
    } else {
        promptPosition = el->getCenter() + sf::Vector2f(0, -25.f);
    }
    m_editPromptText.setPosition(promptPosition);

    sf::FloatRect promptLocalBounds = m_editPromptText.getLocalBounds();
    sf::FloatRect inputTxtLocalBounds = m_inputTextDisplay.getLocalBounds();

    float currentInputTextWidth = inputTxtLocalBounds.width;
    if (m_currentInputString.empty()) {
        currentInputTextWidth = AppConstants::TEXT_INPUT_FIELD_MIN_WIDTH;
    } else if (inputTxtLocalBounds.width <= 0) { // Если текст не пустой, но ширина 0 (может быть для пробела)
        currentInputTextWidth = AppConstants::TEXT_INPUT_FIELD_MIN_WIDTH;
    }


    float totalTextContentWidth = promptLocalBounds.width + AppConstants::TEXT_INPUT_SPACING_PROMPT_TO_TEXT + currentInputTextWidth;
    float bgWidth = totalTextContentWidth + 2 * AppConstants::TEXT_INPUT_PADDING_HORIZONTAL_BG;
    float bgHeight = std::max(m_editPromptText.getCharacterSize(), m_inputTextDisplay.getCharacterSize()) + 2 * AppConstants::TEXT_INPUT_PADDING_VERTICAL_BG;

    m_inputBackground.setSize(sf::Vector2f(bgWidth, bgHeight));
    m_inputBackground.setPosition(
        m_editPromptText.getPosition().x - AppConstants::TEXT_INPUT_PADDING_HORIZONTAL_BG,
        m_editPromptText.getPosition().y - AppConstants::TEXT_INPUT_PADDING_VERTICAL_BG
    );

    m_inputTextDisplay.setPosition(
        m_editPromptText.getPosition().x + promptLocalBounds.width + AppConstants::TEXT_INPUT_SPACING_PROMPT_TO_TEXT,
        m_editPromptText.getPosition().y
    );
}


void OpticalApplication::updateFPSDisplay() {
    m_frameCount++;
    if (m_fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
        float fps = static_cast<float>(m_frameCount) / m_fpsClock.getElapsedTime().asSeconds();
        m_window.setTitle(AppConstants::WINDOW_TITLE_BASE + " - FPS: " + std::to_string(static_cast<int>(fps)));
        m_frameCount = 0;
        m_fpsClock.restart();
    }
}

// Рендер
void OpticalApplication::drawAllRayPaths() {
    for (const auto& path : m_rayPaths) {
        if (path.size() >= 2) {
            m_window.draw(path.data(), path.size(), sf::LinesStrip);
        }
    }
}

void OpticalApplication::drawActivePlacementPreview() {
    if (m_currentMode == Mode::PLACING_END) {
        if (m_placementType == OpticalElement::Type::MIRROR || m_placementType == OpticalElement::Type::LENS) {
            m_window.draw(m_placementPreviewLine);
        } else if (m_placementType == OpticalElement::Type::SPHERICAL_MIRROR) {
            m_window.draw(m_placementPreviewCircle);
        }
    }
}

void OpticalApplication::drawElementsAndUI() {
    for (size_t i = 0; i < m_elements.size(); ++i) {
        if (!m_elements[i]) continue;
        setFontForElement(m_elements[i]);
        m_elements[i]->draw(m_window);

        if (m_selectedElementIndex.has_value() && m_selectedElementIndex.value() == i &&
            m_currentMode != Mode::EDITING_PARAMETER &&
            m_currentMode != Mode::PLACING_START && m_currentMode != Mode::PLACING_END)
        {
            m_elements[i]->drawHandles(m_window, AppConstants::COLOR_HANDLE_MOVE, AppConstants::COLOR_HANDLE_RESIZE);
            auto type = m_elements[i]->getType();
            if (type == OpticalElement::Type::LENS) m_window.draw(dynamic_cast<IdealLens*>(m_elements[i])->focalLengthText);
            else if (type == OpticalElement::Type::SPHERICAL_MIRROR) m_window.draw(dynamic_cast<SphericalMirror*>(m_elements[i])->radiusText);
            else if (type == OpticalElement::Type::SOURCE) m_window.draw(dynamic_cast<PointSource*>(m_elements[i])->numRaysText);
        }
    }
}

void OpticalApplication::drawMainHelpText() {
    if (m_fontLoaded) {
        m_window.draw(m_helpText);
    }
}

void OpticalApplication::drawParameterEditingUI() {
    if (m_fontLoaded) {
        m_window.draw(m_inputBackground);
        m_window.draw(m_editPromptText);
        m_window.draw(m_inputTextDisplay);
    }
}

// Управление элементами
void OpticalApplication::traceRaysInternal() {
    m_rayPaths.clear();
    if (m_sources.empty()) return;

    for (const auto* sourcePtr : m_sources) {
        if (!sourcePtr) continue;
        std::vector<Ray> currentRaysBatch = sourcePtr->emitRays();
        for (Ray currentRay : currentRaysBatch) {
            RayPath singleRayPath;
            singleRayPath.push_back(sf::Vertex(currentRay.origin, currentRay.color));

            while (currentRay.bounces_left > 0) {
                VectorMath::IntersectionResult closestIntersection;
                closestIntersection.distance = AppConstants::MAX_RAY_LENGTH;
                const OpticalElement* hitElement = nullptr;

                for (OpticalElement* el_ptr : m_elements) {
                    if (!el_ptr) continue;
                    if (el_ptr == sourcePtr || el_ptr->getType() == OpticalElement::Type::SOURCE) {
                        continue;
                    }
                    VectorMath::IntersectionResult intersection = el_ptr->findIntersection(currentRay);
                    if (intersection.intersects && intersection.distance > EPSILON && intersection.distance < closestIntersection.distance) {
                        closestIntersection = intersection;
                        hitElement = el_ptr;
                    }
                }

                if (hitElement) {
                    singleRayPath.push_back(sf::Vertex(closestIntersection.point, currentRay.color));
                    RayAction interaction = hitElement->interact(currentRay, closestIntersection.point);
                    if (interaction.outgoingRay.has_value() && interaction.outgoingRay.value().bounces_left > 0) {
                        currentRay = interaction.outgoingRay.value();
                    } else {
                        break;
                    }
                } else {
                    singleRayPath.push_back(sf::Vertex(currentRay.origin + currentRay.direction * AppConstants::MAX_RAY_LENGTH, currentRay.color));
                    break;
                }
            }
            if (singleRayPath.size() > 1) {
                m_rayPaths.push_back(singleRayPath);
            }
        }
    }
}

void OpticalApplication::rebuildSourcesVector() {
    m_sources.clear();
    for (OpticalElement* el : m_elements) {
        if (!el) continue;
        if (auto* src = dynamic_cast<const PointSource*>(el)) {
            m_sources.push_back(src);
        }
    }
}

std::optional<size_t> OpticalApplication::findElementAt(const sf::Vector2f& pos) {
    for (int i = static_cast<int>(m_elements.size()) - 1; i >= 0; --i) {
        if (!m_elements[i]) continue;
        if (m_elements[i]->getHandleAtPoint(pos, AppConstants::HANDLE_SELECT_TOLERANCE) != static_cast<int>(HandleType::NONE) ) {
            return static_cast<size_t>(i);
        }
        if (m_elements[i]->isPointNear(pos, AppConstants::ELEMENT_SELECT_TOLERANCE)) {
            return static_cast<size_t>(i);
        }
    }
    return std::nullopt;
}

void OpticalApplication::selectElementByIndex(std::optional<size_t> index, int handleIndexIfSelected) {
    m_selectedElementIndex = index;
    if (m_selectedElementIndex.has_value()) {
        m_activeHandleIndex = handleIndexIfSelected;
    } else {
        m_activeHandleIndex = static_cast<int>(HandleType::NONE);
    }
    if (m_currentMode == Mode::EDITING_PARAMETER && !m_selectedElementIndex.has_value()) {
        cancelParameterEdit();
    }
}

void OpticalApplication::deleteSelectedElementLogic() {
    if (m_selectedElementIndex.has_value()) {
        OpticalElement* elementToDelete = m_elements[m_selectedElementIndex.value()];
        delete elementToDelete;
        m_elements.erase(m_elements.begin() + m_selectedElementIndex.value());
        rebuildSourcesVector();
        m_selectedElementIndex.reset();
        m_currentMode = Mode::IDLE;
        m_activeHandleIndex = static_cast<int>(HandleType::NONE);
    }
}

void OpticalApplication::beginParameterEditForSelected() {
    if (m_currentMode == Mode::IDLE && m_selectedElementIndex.has_value()) {
        OpticalElement* selectedElement = m_elements[m_selectedElementIndex.value()];
        auto type = selectedElement->getType();
        if (type == OpticalElement::Type::LENS || type == OpticalElement::Type::SPHERICAL_MIRROR || type == OpticalElement::Type::SOURCE) {
            m_currentMode = Mode::EDITING_PARAMETER;
            m_parameterBackupString = getCurrentParameterValueAsString(selectedElement);
            m_currentInputString = "";
        }
    }
}

void OpticalApplication::confirmParameterEdit() {
    if (m_selectedElementIndex.has_value()) {
        m_elements[m_selectedElementIndex.value()]->setParameterFromString(m_currentInputString);
    }
    m_currentMode = Mode::IDLE;
    m_currentInputString = "";
}

void OpticalApplication::cancelParameterEdit() {
    if (m_selectedElementIndex.has_value()) {
        m_elements[m_selectedElementIndex.value()]->setParameterFromString(m_parameterBackupString);
    }
    m_currentMode = Mode::IDLE;
    m_currentInputString = "";
}

void OpticalApplication::adjustSelectedParameterValue(float direction) {
    if (m_selectedElementIndex.has_value()) {
        OpticalElement* el = m_elements[m_selectedElementIndex.value()];
        float adjustAmount;
        if (el->getType() == OpticalElement::Type::SOURCE) {
            adjustAmount = static_cast<float>(AppConstants::SOURCE_PARAM_ADJUST_SPEED);
        } else {
            adjustAmount = AppConstants::PARAM_ADJUST_SPEED;
        }
        el->adjustParameter(direction * adjustAmount);
    }
}

void OpticalApplication::rotateSelectedElementByDelta(float angleDelta) {
    if (m_selectedElementIndex.has_value()) {
        m_elements[m_selectedElementIndex.value()]->rotate(angleDelta);
    }
}

std::string OpticalApplication::getCurrentParameterValueAsString(const OpticalElement* element) const {
    if (!element) return "";
    auto type = element->getType();
    std::stringstream ss;
    std::string valStr;

    if (type == OpticalElement::Type::LENS) {
        if (const auto* lens = dynamic_cast<const IdealLens*>(element)) {
            ss << std::fixed << std::setprecision(1) << lens->getFocalLength();
            valStr = ss.str();
        }
    } else if (type == OpticalElement::Type::SPHERICAL_MIRROR) {
        if (const auto* mirror = dynamic_cast<const SphericalMirror*>(element)) {
            ss << std::fixed << std::setprecision(1) << mirror->getRadius();
            valStr = ss.str();
        }
    } else if (type == OpticalElement::Type::SOURCE) {
        if (const auto* source = dynamic_cast<const PointSource*>(element)) {
            return std::to_string(source->getNumRays());
        }
    }
    if (valStr.length() > 2 && valStr.substr(valStr.length() - 2) == ".0") {
        valStr = valStr.substr(0, valStr.length() - 2);
    }
    return valStr;
}

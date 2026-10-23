#include "UI.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- UIButton ---
UIButton::UIButton(float x, float y, float w, float h, const std::string& text, float textScale)
    : x(x), y(y), w(w), h(h), text(text), textScale(textScale),
      isHovered(false), isPressed(false), animScale(1.0f),
      baseColor(Color::GoldButton()), hoverColor(Color::GoldHover()),
      pressColor(Color::GoldActive()), textColor(Color::Black()) {}

void UIButton::update(float mouseX, float mouseY, bool mouseDown, float dt) {
    bool inside = (mouseX >= x && mouseX <= x + w && mouseY >= y && mouseY <= y + h);
    isHovered = inside;

    if (inside && mouseDown) {
        isPressed = true;
    } else if (!mouseDown) {
        isPressed = false;
    }

    float targetScale = isPressed ? 0.97f : (isHovered ? 1.05f : 1.0f);
    animScale += (targetScale - animScale) * std::min(1.0f, dt * 14.0f);
}

void UIButton::render() const {
    float cx = x + w * 0.5f;
    float cy = y + h * 0.5f;
    float rw = w * animScale;
    float rh = h * animScale;
    float rx = cx - rw * 0.5f;
    float ry = cy - rh * 0.5f;

    // Drop shadow
    Renderer2D::drawRoundedRect(rx + 2.0f, ry + 4.0f, rw, rh, 14.0f * animScale, Color(0.0f, 0.0f, 0.0f, 0.22f));

    // Button body
    Color col = isPressed ? pressColor : (isHovered ? hoverColor : baseColor);
    Renderer2D::drawRoundedRect(rx, ry, rw, rh, 14.0f * animScale, col);

    // Border
    Color borderCol = isHovered ? Color(0.40f, 0.28f, 0.05f) : Color(0.25f, 0.18f, 0.04f);
    Renderer2D::drawRoundedRectOutline(rx, ry, rw, rh, 14.0f * animScale, borderCol, 2.5f);

    // Centered Text
    float textY = cy - Renderer2D::getTextHeight(textScale * animScale) * 0.5f;
    if (isPressed) textY += 1.5f;
    Renderer2D::drawText(text, cx, textY, textScale * animScale, textColor, true, true);
}

bool UIButton::isClicked(float mouseX, float mouseY, bool mouseReleased) {
    return mouseReleased && (mouseX >= x && mouseX <= x + w && mouseY >= y && mouseY <= y + h);
}


// --- UIHealthBar ---
UIHealthBar::UIHealthBar(float x, float y, float w, float h)
    : x(x), y(y), w(w), h(h), currentHp(100.0f), maxHp(100.0f), displayHp(100.0f) {}

void UIHealthBar::setHp(float hp, float max) {
    maxHp = max;
    currentHp = std::max(0.0f, std::min(hp, max));
}

void UIHealthBar::update(float dt) {
    displayHp += (currentHp - displayHp) * std::min(1.0f, dt * 8.0f);
}

void UIHealthBar::render() const {
    // Drop shadow
    Renderer2D::drawRect(x + 2.0f, y + 2.0f, w, h, Color(0.0f, 0.0f, 0.0f, 0.2f));

    // Outer dark frame
    Renderer2D::drawRect(x, y, w, h, Color(0.2f, 0.2f, 0.2f, 0.8f));
    Renderer2D::drawRectOutline(x, y, w, h, Color::Black(), 2.0f);

    // Health Fill Bar (Salmon/Pink #ff737a)
    float fillRatio = displayHp / maxHp;
    if (fillRatio > 0.001f) {
        float fillW = (w - 4.0f) * fillRatio;
        Renderer2D::drawRect(x + 2.0f, y + 2.0f, fillW, h - 4.0f, Color(1.0f, 0.44f, 0.46f));
    }

    // Heart badge on the right
    float circleR = h * 0.42f;
    float circleX = x + w - circleR - 6.0f;
    float circleY = y + h * 0.5f;

    Renderer2D::drawCircle(circleX, circleY, circleR + 2.0f, Color::Black());
    Renderer2D::drawCircle(circleX, circleY, circleR, Color::White());
    Renderer2D::drawHeart(circleX, circleY, circleR * 1.5f, Color(1.0f, 0.40f, 0.44f));
}


// --- UIFuelBar ---
UIFuelBar::UIFuelBar(float x, float y, float w, float h)
    : x(x), y(y), w(w), h(h), currentFuel(100.0f), maxFuel(100.0f), displayFuel(100.0f), flashTimer(0.0f) {}

void UIFuelBar::setFuel(float fuel, float max) {
    maxFuel = max;
    currentFuel = std::max(0.0f, std::min(fuel, max));
}

void UIFuelBar::triggerErrorFlash() {
    flashTimer = 0.6f;
}

void UIFuelBar::update(float dt) {
    displayFuel += (currentFuel - displayFuel) * std::min(1.0f, dt * 10.0f);
    if (flashTimer > 0.0f) flashTimer -= dt;
}

void UIFuelBar::render() const {
    // Label "Minyak:" above bar
    Renderer2D::drawText("Minyak:", x, y - 24.0f, 2.2f, Color::Black(), false, true);

    // Dark empty bar container (dark olive brown #27280d)
    Color bgCol(0.18f, 0.19f, 0.08f);
    Renderer2D::drawRect(x, y, w, h, bgCol);

    // Fill bar (Yellow/Gold #e0b400)
    float ratio = displayFuel / maxFuel;
    if (ratio > 0.001f) {
        float fillW = (w - 4.0f) * ratio;
        Renderer2D::drawRect(x + 2.0f, y + 2.0f, fillW, h - 4.0f, Color(0.90f, 0.74f, 0.04f));
    }

    // Frame (flashes red when error triggered)
    Color frameCol = (flashTimer > 0.0f && fmodf(flashTimer, 0.2f) > 0.1f) ? Color::Red() : Color::Black();
    Renderer2D::drawRectOutline(x, y, w, h, frameCol, 3.0f);
}


// --- UIEyeMeter ---
UIEyeMeter::UIEyeMeter(float cx, float cy, float radius)
    : cx(cx), cy(cy), radius(radius), alertLevel(0.0f), displayAlert(0.0f), pulseTime(0.0f), isWarning(false) {}

void UIEyeMeter::setAlert(float level) {
    alertLevel = std::max(0.0f, std::min(level, 1.0f));
}

void UIEyeMeter::update(float dt) {
    displayAlert += (alertLevel - displayAlert) * std::min(1.0f, dt * 14.0f);
    pulseTime += dt * (1.0f + displayAlert * 5.0f);
    isWarning = (displayAlert > 0.45f);
}

void UIEyeMeter::render() const {
    // Background disc
    Renderer2D::drawCircle(cx, cy, radius, Color(0.08f, 0.14f, 0.09f, 0.85f));

    // Outer alert ring gauge (0 to 360 deg)
    float startAngle = -M_PI * 0.5f;
    float endAngle = startAngle + (float)(2.0 * M_PI * displayAlert);
    Color ringCol = isWarning ? Color(0.95f, 0.25f, 0.20f) : Color(0.92f, 0.85f, 0.20f);

    if (displayAlert > 0.01f) {
        Renderer2D::drawArcRing(cx, cy, radius - 4.0f, 6.0f, startAngle, endAngle, ringCol);
    }

    // Outer circle border
    float borderPulse = (isWarning) ? (sinf(pulseTime * 12.0f) * 1.5f) : 0.0f;
    Color borderCol = isWarning ? Color::Red() : Color::White();
    Renderer2D::drawCircleOutline(cx, cy, radius + borderPulse, borderCol, 3.0f);

    // Eye icon inside
    Color eyeCol = isWarning ? Color(1.0f, 0.3f, 0.3f) : Color::White();
    Renderer2D::drawEyeIcon(cx, cy, radius * 0.95f, eyeCol);
}


// --- UIInventory ---
UIInventory::UIInventory(float centerX, float bottomY, float slotSize)
    : slotSize(slotSize), spacing(4.0f), selectedSlot(0), hoveredSlot(-1), shakeTimer(0.0f) {
    float totalW = 5 * slotSize + 4 * spacing;
    x = centerX - totalW * 0.5f;
    y = bottomY - slotSize;

    slots = { ItemType::Torch, ItemType::Bow, ItemType::None, ItemType::None, ItemType::None };
}

void UIInventory::update(float mouseX, float mouseY, bool mouseClicked, float dt) {
    hoveredSlot = -1;
    for (int i = 0; i < 5; ++i) {
        float sx = x + i * (slotSize + spacing);
        float sy = y;
        if (mouseX >= sx && mouseX <= sx + slotSize && mouseY >= sy && mouseY <= sy + slotSize) {
            hoveredSlot = i;
            if (mouseClicked) {
                selectSlot(i);
            }
            break;
        }
    }
    if (shakeTimer > 0.0f) shakeTimer -= dt;
}

bool UIInventory::isHovered(float mouseX, float mouseY) const {
    float totalW = 5 * slotSize + 4 * spacing;
    return (mouseX >= x && mouseX <= x + totalW && mouseY >= y && mouseY <= y + slotSize);
}

bool UIInventory::handleClick(float mouseX, float mouseY) {
    for (int i = 0; i < 5; ++i) {
        float sx = x + i * (slotSize + spacing);
        float sy = y;
        if (mouseX >= sx && mouseX <= sx + slotSize && mouseY >= sy && mouseY <= sy + slotSize) {
            selectSlot(i);
            return true;
        }
    }
    return false;
}

void UIInventory::selectSlot(int slotIndex) {
    if (slotIndex >= 0 && slotIndex < 5) {
        selectedSlot = slotIndex;
        if (slots[slotIndex] == ItemType::None) {
            UIToast::show("Tangan kosong [Tanpa Senjata]", 1.5f, Color(0.2f, 0.7f, 0.3f, 0.9f));
        }
    }
}

void UIInventory::clearSelection() {
    // Select an empty slot or deselect
    selectedSlot = 2; // Slot 3 is empty
    UIToast::show("Tangan kosong [Tanpa Senjata]", 1.5f, Color(0.2f, 0.7f, 0.3f, 0.9f));
}

ItemType UIInventory::getSelectedItem() const {
    if (selectedSlot >= 0 && selectedSlot < 5) {
        return slots[selectedSlot];
    }
    return ItemType::None;
}

void UIInventory::triggerErrorShake() {
    shakeTimer = 0.4f;
}

void UIInventory::render() const {
    float shakeX = 0.0f;
    if (shakeTimer > 0.0f) {
        shakeX = sinf(shakeTimer * 35.0f) * 6.0f;
    }

    for (int i = 0; i < 5; ++i) {
        float sx = x + shakeX + i * (slotSize + spacing);
        float sy = y;

        // Slot background
        bool isSel = (selectedSlot == i);
        bool isHov = (hoveredSlot == i);
        Color bgCol = isSel ? Color(0.9f, 0.8f, 0.3f, 0.35f) : (isHov ? Color(1, 1, 1, 0.25f) : Color(0.0f, 0.0f, 0.0f, 0.30f));
        Renderer2D::drawRect(sx, sy, slotSize, slotSize, bgCol);

        // Slot border
        Color borderCol = isSel ? Color(1.0f, 0.85f, 0.1f) : Color::Black();
        float bWidth = isSel ? 3.0f : 1.5f;
        Renderer2D::drawRectOutline(sx, sy, slotSize, slotSize, borderCol, bWidth);

        // Number shortcut label (1..5)
        std::string numStr = std::to_string(i + 1);
        Renderer2D::drawText(numStr, sx + 4.0f, sy + 4.0f, 1.2f, Color(0.9f, 0.9f, 0.9f, 0.75f));

        // Draw Item Icons inside slots
        float cx = sx + slotSize * 0.5f;
        float cy = sy + slotSize * 0.55f;

        if (slots[i] == ItemType::Torch) {
            // Mini Torch Icon
            // Handle
            Renderer2D::drawLine(cx - 10.0f, cy + 14.0f, cx + 5.0f, cy - 6.0f, Color::DarkWood(), 4.0f);
            // Flame
            Renderer2D::drawFlame(cx + 6.0f, cy - 9.0f, 12.0f, 0.0f);
        } else if (slots[i] == ItemType::Bow) {
            // Mini Bow Icon
            // Bow arc
            Renderer2D::drawArcRing(cx - 4.0f, cy, 16.0f, 3.0f, -M_PI * 0.45f, M_PI * 0.45f, Color(0.6f, 0.6f, 0.6f));
            // Bow string
            Renderer2D::drawLine(cx - 4.0f + cosf(-M_PI * 0.45f) * 16.0f, cy + sinf(-M_PI * 0.45f) * 16.0f,
                                 cx - 4.0f + cosf( M_PI * 0.45f) * 16.0f, cy + sinf( M_PI * 0.45f) * 16.0f,
                                 Color::White(), 1.2f);
        }
    }
}


// --- UIToast ---
std::string UIToast::s_message = "";
float UIToast::s_timer = 0.0f;
float UIToast::s_maxDuration = 2.5f;
Color UIToast::s_color = Color(0.95f, 0.25f, 0.25f, 0.95f);

void UIToast::show(const std::string& message, float duration, Color color) {
    s_message = message;
    s_timer = duration;
    s_maxDuration = duration;
    s_color = color;
}

void UIToast::update(float dt) {
    if (s_timer > 0.0f) {
        s_timer -= dt;
    }
}

void UIToast::render() {
    if (s_timer <= 0.0f || s_message.empty()) return;

    float alpha = std::min(1.0f, s_timer * 3.0f); // smooth fade out
    float textW = Renderer2D::getTextWidth(s_message, 1.6f);
    float padX = 20.0f;
    float padY = 7.0f;
    float boxW = textW + padX * 2.0f;
    float boxH = 30.0f;
    float boxX = 640.0f - boxW * 0.5f;
    float boxY = 92.0f; // Positioned cleanly in the open sky below the top HUD elements

    Color bg = s_color;
    bg.a *= alpha;
    Renderer2D::drawRoundedRect(boxX, boxY, boxW, boxH, 8.0f, bg);
    Renderer2D::drawRoundedRectOutline(boxX, boxY, boxW, boxH, 8.0f, Color(1, 1, 1, 0.7f * alpha), 1.5f);

    Color txt = Color::White();
    txt.a = alpha;
    Renderer2D::drawText(s_message, 640.0f, boxY + padY, 1.6f, txt, true, true);
}

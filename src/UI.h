#ifndef UI_H
#define UI_H

#include "Renderer2D.h"
#include <string>
#include <vector>

// Interactive Button with smooth hover scaling and click states
class UIButton {
public:
    float x, y, w, h;
    std::string text;
    float textScale;
    bool isHovered;
    bool isPressed;
    float animScale; // 1.0f to 1.06f
    Color baseColor;
    Color hoverColor;
    Color pressColor;
    Color textColor;

    UIButton(float x, float y, float w, float h, const std::string& text, float textScale = 2.5f);
    void update(float mouseX, float mouseY, bool mouseDown, float dt);
    void render() const;
    bool isClicked(float mouseX, float mouseY, bool mouseReleased);
};

// Health Bar with heart icon on right (matches mockup)
class UIHealthBar {
public:
    float x, y, w, h;
    float currentHp;
    float maxHp;
    float displayHp; // for smooth health drain animation

    UIHealthBar(float x = 990.0f, float y = 50.0f, float w = 240.0f, float h = 48.0f);
    void setHp(float hp, float max = 100.0f);
    void update(float dt);
    void render() const;
};

// Fuel / Oil Bar ("Minyak:")
class UIFuelBar {
public:
    float x, y, w, h;
    float currentFuel;
    float maxFuel;
    float displayFuel;
    float flashTimer; // for error flash when trying to burn with 0 fuel

    UIFuelBar(float x = 30.0f, float y = 50.0f, float w = 200.0f, float h = 32.0f);
    void setFuel(float fuel, float max = 100.0f);
    void triggerErrorFlash();
    void update(float dt);
    void render() const;
};

// Stealth Eye Alert Indicator (Phase 2)
class UIEyeMeter {
public:
    float cx, cy, radius;
    float alertLevel; // 0.0 to 1.0
    float displayAlert;
    float pulseTime;
    bool isWarning;

    UIEyeMeter(float cx = 85.0f, float cy = 645.0f, float radius = 55.0f);
    void setAlert(float level);
    void update(float dt);
    void render() const;
};

// 5-Slot Inventory Grid (matches mockup)
enum class ItemType {
    None,
    Torch,
    Bow
};

class UIInventory {
public:
    float x, y, slotSize, spacing;
    int selectedSlot; // 0 to 4 (-1 = unequipped / empty hands)
    int hoveredSlot;
    float shakeTimer; // Error handling shake animation
    std::vector<ItemType> slots;

    UIInventory(float centerX = 640.0f, float bottomY = 700.0f, float slotSize = 64.0f);
    void update(float mouseX, float mouseY, bool mouseClicked, float dt);
    bool handleClick(float mouseX, float mouseY);
    bool isHovered(float mouseX, float mouseY) const;
    void render() const;
    void selectSlot(int slotIndex);
    void clearSelection(); // Hand empty (tangan kosong)
    ItemType getSelectedItem() const;
    void triggerErrorShake();
};

// Error Handling Toast / Notification Banner
class UIToast {
public:
    static void show(const std::string& message, float duration = 2.5f, Color color = Color(0.95f, 0.25f, 0.25f, 0.95f));
    static void update(float dt);
    static void render();

private:
    static std::string s_message;
    static float s_timer;
    static float s_maxDuration;
    static Color s_color;
};

#endif // UI_H

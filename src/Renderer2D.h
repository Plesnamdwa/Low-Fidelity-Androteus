#ifndef RENDERER2D_H
#define RENDERER2D_H

#include <string>
#include <vector>

struct Color {
    float r, g, b, a;
    Color() : r(1), g(1), b(1), a(1) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

    static Color White()       { return Color(1.0f, 1.0f, 1.0f, 1.0f); }
    static Color Black()       { return Color(0.0f, 0.0f, 0.0f, 1.0f); }
    static Color Red()         { return Color(0.9f, 0.2f, 0.2f, 1.0f); }
    static Color Green()       { return Color(0.2f, 0.8f, 0.2f, 1.0f); }
    static Color Blue()        { return Color(0.2f, 0.5f, 0.9f, 1.0f); }
    static Color Yellow()      { return Color(1.0f, 0.85f, 0.0f, 1.0f); }
    static Color SkyBlue()     { return Color(0.68f, 0.90f, 0.96f, 1.0f); } // #aee5f5
    static Color ForestGreen() { return Color(0.09f, 0.26f, 0.14f, 1.0f); } // #174224
    static Color HillGreen()   { return Color(0.07f, 0.22f, 0.12f, 1.0f); }
    static Color DeepForest()  { return Color(0.06f, 0.35f, 0.15f, 1.0f); }
    static Color GoldButton()  { return Color(0.85f, 0.65f, 0.05f, 1.0f); } // #d9a60d
    static Color GoldHover()   { return Color(0.96f, 0.75f, 0.10f, 1.0f); }
    static Color GoldActive()  { return Color(0.72f, 0.52f, 0.02f, 1.0f); }
    static Color DarkWood()    { return Color(0.42f, 0.28f, 0.12f, 1.0f); }
    static Color WarmBrown()   { return Color(0.50f, 0.27f, 0.08f, 1.0f); }
    static Color DeerGold()    { return Color(0.88f, 0.58f, 0.15f, 1.0f); }
    static Color DeerFur()     { return Color(0.94f, 0.86f, 0.54f, 1.0f); }
    static Color AntlerBrown() { return Color(0.42f, 0.36f, 0.18f, 1.0f); }
    static Color HealthRed()   { return Color(1.00f, 0.45f, 0.48f, 1.0f); } // #ff737a
    static Color Gray(float v, float a = 1.0f) { return Color(v, v, v, a); }
};

class Renderer2D {
public:
    static void init(int windowWidth, int windowHeight);
    static void setViewport(int windowWidth, int windowHeight);
    static void beginFrame();
    static void endFrame();

    // Geometric Primitives
    static void drawRect(float x, float y, float w, float h, Color color);
    static void drawRectOutline(float x, float y, float w, float h, Color color, float lineWidth = 1.5f);
    static void drawRoundedRect(float x, float y, float w, float h, float radius, Color color);
    static void drawRoundedRectOutline(float x, float y, float w, float h, float radius, Color color, float lineWidth = 2.0f);
    static void drawCircle(float cx, float cy, float radius, Color color, int segments = 40);
    static void drawCircleOutline(float cx, float cy, float radius, Color color, float lineWidth = 2.0f, int segments = 40);
    static void drawArcRing(float cx, float cy, float radius, float thickness, float startAngleRad, float endAngleRad, Color color, int segments = 40);
    static void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3, Color color);
    static void drawLine(float x1, float y1, float x2, float y2, Color color, float lineWidth = 1.5f);
    static void drawCurvedRibbon(const std::vector<std::pair<float, float>>& points, const std::vector<float>& widths, Color color);
    static void drawCurvedRibbonOutline(const std::vector<std::pair<float, float>>& points, const std::vector<float>& widths, Color color, float lineWidth = 1.5f);

    // Complex / Thematic Vector Graphics
    static void drawPineTree(float x, float y, float scale, Color foliageColor, Color trunkColor);
    static void drawCloud(float x, float y, float scale, Color color = Color::White());
    static void drawDeerLogo(float cx, float cy, float radius);
    static void drawHeart(float cx, float cy, float size, Color color);
    static void drawEyeIcon(float cx, float cy, float size, Color color);
    static void drawFlame(float cx, float cy, float size, float timeOffset = 0.0f);

    // Texture Loading & Rendering
    static unsigned int loadTexture(const std::string& filepath);
    static void drawTexture(unsigned int textureId, float x, float y, float w, float h, Color tint = Color::White());

    // Text Rendering
    static void drawText(const std::string& text, float x, float y, float scale = 2.0f, Color color = Color::Black(), bool center = false, bool bold = false);
    static float getTextWidth(const std::string& text, float scale = 2.0f);
    static float getTextHeight(float scale = 2.0f);

private:
    static int s_width;
    static int s_height;
};

#endif // RENDERER2D_H

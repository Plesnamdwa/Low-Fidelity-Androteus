#include "Renderer2D.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int Renderer2D::s_width = 1280;
int Renderer2D::s_height = 720;

// Standard 8x8 font dataset (ASCII 32 ' ' through 126 '~')
// Each character is 8 bytes, 1 byte per row, 8 bits per byte.
static const unsigned char FONT_8X8[95][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // 32 ' '
    {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00}, // 33 '!'
    {0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00}, // 34 '"'
    {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00}, // 35 '#'
    {0x18,0x3E,0x60,0x3C,0x06,0x7C,0x18,0x00}, // 36 '$'
    {0x00,0x63,0x66,0x0C,0x18,0x33,0x63,0x00}, // 37 '%'
    {0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00}, // 38 '&'
    {0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00}, // 39 '''
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00}, // 40 '('
    {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00}, // 41 ')'
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, // 42 '*'
    {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00}, // 43 '+'
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30}, // 44 ','
    {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00}, // 45 '-'
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00}, // 46 '.'
    {0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00}, // 47 '/'
    {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00}, // 48 '0'
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00}, // 49 '1'
    {0x3C,0x66,0x06,0x0C,0x18,0x30,0x7E,0x00}, // 50 '2'
    {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00}, // 51 '3'
    {0x0C,0x1C,0x3C,0x6C,0xFE,0x0C,0x0C,0x00}, // 52 '4'
    {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00}, // 53 '5'
    {0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00}, // 54 '6'
    {0x7E,0x66,0x06,0x0C,0x18,0x18,0x18,0x00}, // 55 '7'
    {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00}, // 56 '8'
    {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00}, // 57 '9'
    {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00}, // 58 ':'
    {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x30}, // 59 ';'
    {0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00}, // 60 '<'
    {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00}, // 61 '='
    {0x60,0x30,0x18,0x0C,0x18,0x30,0x60,0x00}, // 62 '>'
    {0x3C,0x66,0x06,0x0C,0x18,0x00,0x18,0x00}, // 63 '?'
    {0x3C,0x66,0x6E,0x6A,0x6E,0x60,0x3C,0x00}, // 64 '@'
    {0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00}, // 65 'A'
    {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00}, // 66 'B'
    {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00}, // 67 'C'
    {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00}, // 68 'D'
    {0x7E,0x60,0x60,0x7C,0x60,0x60,0x7E,0x00}, // 69 'E'
    {0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x00}, // 70 'F'
    {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3C,0x00}, // 71 'G'
    {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00}, // 72 'H'
    {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, // 73 'I'
    {0x0E,0x06,0x06,0x06,0x06,0x66,0x3C,0x00}, // 74 'J'
    {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00}, // 75 'K'
    {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00}, // 76 'L'
    {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00}, // 77 'M'
    {0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00}, // 78 'N'
    {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, // 79 'O'
    {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00}, // 80 'P'
    {0x3C,0x66,0x66,0x66,0x6A,0x6C,0x36,0x00}, // 81 'Q'
    {0x7C,0x66,0x66,0x7C,0x6C,0x66,0x66,0x00}, // 82 'R'
    {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00}, // 83 'S'
    {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, // 84 'T'
    {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, // 85 'U'
    {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00}, // 86 'V'
    {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00}, // 87 'W'
    {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00}, // 88 'X'
    {0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00}, // 89 'Y'
    {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00}, // 90 'Z'
    {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00}, // 91 '['
    {0xC0,0x60,0x30,0x18,0x0C,0x06,0x02,0x00}, // 92 '\'
    {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00}, // 93 ']'
    {0x10,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00}, // 94 '^'
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF}, // 95 '_'
    {0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00}, // 96 '`'
    {0x00,0x00,0x3C,0x06,0x3E,0x66,0x3E,0x00}, // 97 'a'
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x7C,0x00}, // 98 'b'
    {0x00,0x00,0x3C,0x66,0x60,0x66,0x3C,0x00}, // 99 'c'
    {0x06,0x06,0x3E,0x66,0x66,0x66,0x3E,0x00}, // 100 'd'
    {0x00,0x00,0x3C,0x66,0x7E,0x60,0x3C,0x00}, // 101 'e'
    {0x1C,0x30,0x7C,0x30,0x30,0x30,0x30,0x00}, // 102 'f'
    {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x3C}, // 103 'g'
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0x00}, // 104 'h'
    {0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00}, // 105 'i'
    {0x0C,0x00,0x1C,0x0C,0x0C,0x0C,0x4C,0x38}, // 106 'j'
    {0x60,0x60,0x66,0x6C,0x78,0x6C,0x66,0x00}, // 107 'k'
    {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, // 108 'l'
    {0x00,0x00,0x66,0x7F,0x7F,0x6B,0x63,0x00}, // 109 'm'
    {0x00,0x00,0x7C,0x66,0x66,0x66,0x66,0x00}, // 110 'n'
    {0x00,0x00,0x3C,0x66,0x66,0x66,0x3C,0x00}, // 111 'o'
    {0x00,0x00,0x7C,0x66,0x66,0x7C,0x60,0x60}, // 112 'p'
    {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x06}, // 113 'q'
    {0x00,0x00,0x7C,0x66,0x60,0x60,0x60,0x00}, // 114 'r'
    {0x00,0x00,0x3E,0x60,0x3C,0x06,0x7C,0x00}, // 115 's'
    {0x18,0x18,0x7E,0x18,0x18,0x18,0x0C,0x00}, // 116 't'
    {0x00,0x00,0x66,0x66,0x66,0x66,0x3E,0x00}, // 117 'u'
    {0x00,0x00,0x66,0x66,0x66,0x3C,0x18,0x00}, // 118 'v'
    {0x00,0x00,0x63,0x6B,0x7F,0x3E,0x36,0x00}, // 119 'w'
    {0x00,0x00,0x66,0x3C,0x18,0x3C,0x66,0x00}, // 120 'x'
    {0x00,0x00,0x66,0x66,0x66,0x3E,0x06,0x3C}, // 121 'y'
    {0x00,0x00,0x7E,0x0C,0x18,0x30,0x7E,0x00}, // 122 'z'
    {0x0E,0x18,0x18,0x70,0x18,0x18,0x0E,0x00}, // 123 '{'
    {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, // 124 '|'
    {0x70,0x18,0x18,0x0E,0x18,0x18,0x70,0x00}, // 125 '}'
    {0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00}  // 126 '~'
};

void Renderer2D::init(int windowWidth, int windowHeight) {
    s_width = windowWidth;
    s_height = windowHeight;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
}

void Renderer2D::setViewport(int windowWidth, int windowHeight) {
    s_width = windowWidth;
    s_height = windowHeight;
    glViewport(0, 0, windowWidth, windowHeight);
}

void Renderer2D::beginFrame() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // 2D Orthographic coordinate: Top-left is (0, 0), Bottom-right is (s_width, s_height)
    glOrtho(0.0, (double)s_width, (double)s_height, 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer2D::endFrame() {
    // Flush OpenGL calls
    glFlush();
}

void Renderer2D::drawRect(float x, float y, float w, float h, Color color) {
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void Renderer2D::drawRectOutline(float x, float y, float w, float h, Color color, float lineWidth) {
    glColor4f(color.r, color.g, color.b, color.a);
    glLineWidth(lineWidth);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
    glLineWidth(1.0f);
}

void Renderer2D::drawRoundedRect(float x, float y, float w, float h, float radius, Color color) {
    if (radius <= 0.0f) {
        drawRect(x, y, w, h, color);
        return;
    }
    radius = std::min(radius, std::min(w * 0.5f, h * 0.5f));

    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_POLYGON);

    const int cornerSegs = 10;
    // Top-Right corner
    float cx = x + w - radius;
    float cy = y + radius;
    for (int i = 0; i <= cornerSegs; ++i) {
        float angle = -M_PI * 0.5f + (M_PI * 0.5f * i / cornerSegs);
        glVertex2f(cx + cosf(angle) * radius, cy + sinf(angle) * radius);
    }
    // Bottom-Right corner
    cy = y + h - radius;
    for (int i = 0; i <= cornerSegs; ++i) {
        float angle = 0.0f + (M_PI * 0.5f * i / cornerSegs);
        glVertex2f(cx + cosf(angle) * radius, cy + sinf(angle) * radius);
    }
    // Bottom-Left corner
    cx = x + radius;
    for (int i = 0; i <= cornerSegs; ++i) {
        float angle = M_PI * 0.5f + (M_PI * 0.5f * i / cornerSegs);
        glVertex2f(cx + cosf(angle) * radius, cy + sinf(angle) * radius);
    }
    // Top-Left corner
    cy = y + radius;
    for (int i = 0; i <= cornerSegs; ++i) {
        float angle = M_PI + (M_PI * 0.5f * i / cornerSegs);
        glVertex2f(cx + cosf(angle) * radius, cy + sinf(angle) * radius);
    }

    glEnd();
}

void Renderer2D::drawRoundedRectOutline(float x, float y, float w, float h, float radius, Color color, float lineWidth) {
    if (radius <= 0.0f) {
        drawRectOutline(x, y, w, h, color, lineWidth);
        return;
    }
    radius = std::min(radius, std::min(w * 0.5f, h * 0.5f));

    glColor4f(color.r, color.g, color.b, color.a);
    glLineWidth(lineWidth);
    glBegin(GL_LINE_LOOP);

    const int cornerSegs = 10;
    // Top-Right
    float cx = x + w - radius;
    float cy = y + radius;
    for (int i = 0; i <= cornerSegs; ++i) {
        float angle = -M_PI * 0.5f + (M_PI * 0.5f * i / cornerSegs);
        glVertex2f(cx + cosf(angle) * radius, cy + sinf(angle) * radius);
    }
    // Bottom-Right
    cy = y + h - radius;
    for (int i = 0; i <= cornerSegs; ++i) {
        float angle = 0.0f + (M_PI * 0.5f * i / cornerSegs);
        glVertex2f(cx + cosf(angle) * radius, cy + sinf(angle) * radius);
    }
    // Bottom-Left
    cx = x + radius;
    for (int i = 0; i <= cornerSegs; ++i) {
        float angle = M_PI * 0.5f + (M_PI * 0.5f * i / cornerSegs);
        glVertex2f(cx + cosf(angle) * radius, cy + sinf(angle) * radius);
    }
    // Top-Left
    cy = y + radius;
    for (int i = 0; i <= cornerSegs; ++i) {
        float angle = M_PI + (M_PI * 0.5f * i / cornerSegs);
        glVertex2f(cx + cosf(angle) * radius, cy + sinf(angle) * radius);
    }

    glEnd();
    glLineWidth(1.0f);
}

void Renderer2D::drawCircle(float cx, float cy, float radius, Color color, int segments) {
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * (float)M_PI * i / segments;
        glVertex2f(cx + cosf(angle) * radius, cy + sinf(angle) * radius);
    }
    glEnd();
}

void Renderer2D::drawCircleOutline(float cx, float cy, float radius, Color color, float lineWidth, int segments) {
    glColor4f(color.r, color.g, color.b, color.a);
    glLineWidth(lineWidth);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * (float)M_PI * i / segments;
        glVertex2f(cx + cosf(angle) * radius, cy + sinf(angle) * radius);
    }
    glEnd();
    glLineWidth(1.0f);
}

void Renderer2D::drawArcRing(float cx, float cy, float radius, float thickness, float startAngleRad, float endAngleRad, Color color, int segments) {
    glColor4f(color.r, color.g, color.b, color.a);
    float innerR = radius - thickness * 0.5f;
    float outerR = radius + thickness * 0.5f;
    if (innerR < 0.0f) innerR = 0.0f;

    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= segments; ++i) {
        float t = (float)i / segments;
        float angle = startAngleRad + t * (endAngleRad - startAngleRad);
        float cosA = cosf(angle);
        float sinA = sinf(angle);
        glVertex2f(cx + cosA * outerR, cy + sinA * outerR);
        glVertex2f(cx + cosA * innerR, cy + sinA * innerR);
    }
    glEnd();
}

void Renderer2D::drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3, Color color) {
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glEnd();
}

void Renderer2D::drawLine(float x1, float y1, float x2, float y2, Color color, float lineWidth) {
    glColor4f(color.r, color.g, color.b, color.a);
    glLineWidth(lineWidth);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
    glLineWidth(1.0f);
}

void Renderer2D::drawCurvedRibbon(const std::vector<std::pair<float, float>>& points, const std::vector<float>& widths, Color color) {
    size_t n = points.size();
    if (n < 2) return;

    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_TRIANGLE_STRIP);

    for (size_t i = 0; i < n; ++i) {
        float dx, dy;
        if (i == 0) {
            dx = points[1].first - points[0].first;
            dy = points[1].second - points[0].second;
        } else if (i == n - 1) {
            dx = points[n - 1].first - points[n - 2].first;
            dy = points[n - 1].second - points[n - 2].second;
        } else {
            dx = points[i + 1].first - points[i - 1].first;
            dy = points[i + 1].second - points[i - 1].second;
        }

        float len = sqrtf(dx * dx + dy * dy);
        if (len < 0.0001f) len = 1.0f;
        float nx = -dy / len;
        float ny =  dx / len;

        float hw = (i < widths.size() ? widths[i] : 10.0f) * 0.5f;

        glVertex2f(points[i].first + nx * hw, points[i].second + ny * hw);
        glVertex2f(points[i].first - nx * hw, points[i].second - ny * hw);
    }

    glEnd();
}

void Renderer2D::drawCurvedRibbonOutline(const std::vector<std::pair<float, float>>& points, const std::vector<float>& widths, Color color, float lineWidth) {
    size_t n = points.size();
    if (n < 2) return;

    glColor4f(color.r, color.g, color.b, color.a);
    glLineWidth(lineWidth);

    // Left edge
    glBegin(GL_LINE_STRIP);
    for (size_t i = 0; i < n; ++i) {
        float dx = (i == 0) ? (points[1].first - points[0].first) : ((i == n - 1) ? (points[n - 1].first - points[n - 2].first) : (points[i + 1].first - points[i - 1].first));
        float dy = (i == 0) ? (points[1].second - points[0].second) : ((i == n - 1) ? (points[n - 1].second - points[n - 2].second) : (points[i + 1].second - points[i - 1].second));
        float len = sqrtf(dx * dx + dy * dy);
        if (len < 0.0001f) len = 1.0f;
        float nx = -dy / len;
        float ny =  dx / len;
        float hw = (i < widths.size() ? widths[i] : 10.0f) * 0.5f;
        glVertex2f(points[i].first + nx * hw, points[i].second + ny * hw);
    }
    glEnd();

    // Right edge
    glBegin(GL_LINE_STRIP);
    for (size_t i = 0; i < n; ++i) {
        float dx = (i == 0) ? (points[1].first - points[0].first) : ((i == n - 1) ? (points[n - 1].first - points[n - 2].first) : (points[i + 1].first - points[i - 1].first));
        float dy = (i == 0) ? (points[1].second - points[0].second) : ((i == n - 1) ? (points[n - 1].second - points[n - 2].second) : (points[i + 1].second - points[i - 1].second));
        float len = sqrtf(dx * dx + dy * dy);
        if (len < 0.0001f) len = 1.0f;
        float nx = -dy / len;
        float ny =  dx / len;
        float hw = (i < widths.size() ? widths[i] : 10.0f) * 0.5f;
        glVertex2f(points[i].first - nx * hw, points[i].second - ny * hw);
    }
    glEnd();

    glLineWidth(1.0f);
}

// Draw a stylized pine tree with 3 green triangle layers & trunk
void Renderer2D::drawPineTree(float x, float y, float scale, Color foliageColor, Color trunkColor) {
    float trunkW = 8.0f * scale;
    float trunkH = 20.0f * scale;
    drawRect(x - trunkW * 0.5f, y - trunkH, trunkW, trunkH, trunkColor);

    // Layer 3 (bottom)
    float baseW = 40.0f * scale;
    float baseH = 26.0f * scale;
    float yTier3 = y - trunkH + 4.0f * scale;
    drawTriangle(x, yTier3 - baseH, x - baseW * 0.5f, yTier3, x + baseW * 0.5f, yTier3, foliageColor);

    // Layer 2 (middle)
    float midW = 32.0f * scale;
    float midH = 24.0f * scale;
    float yTier2 = yTier3 - baseH * 0.55f;
    drawTriangle(x, yTier2 - midH, x - midW * 0.5f, yTier2, x + midW * 0.5f, yTier2, foliageColor);

    // Layer 1 (top)
    float topW = 24.0f * scale;
    float topH = 22.0f * scale;
    float yTier1 = yTier2 - midH * 0.55f;
    drawTriangle(x, yTier1 - topH, x - topW * 0.5f, yTier1, x + topW * 0.5f, yTier1, foliageColor);
}

// Draw fluffy rounded cloud
void Renderer2D::drawCloud(float x, float y, float scale, Color color) {
    float r = 24.0f * scale;
    drawCircle(x, y, r * 0.85f, color);
    drawCircle(x + r * 0.9f, y - r * 0.25f, r * 1.15f, color);
    drawCircle(x + r * 1.9f, y + r * 0.05f, r * 0.95f, color);
    drawCircle(x + r * 2.7f, y + r * 0.2f, r * 0.7f, color);
    drawRoundedRect(x - r * 0.4f, y + r * 0.1f, r * 3.4f, r * 0.9f, r * 0.45f, color);
}

// Draw the majestic deer emblem logo from Mockup 1
void Renderer2D::drawDeerLogo(float cx, float cy, float radius) {
    // Outer dark brown thick circle border
    drawCircle(cx, cy, radius, Color(0.32f, 0.18f, 0.07f)); // Dark brown #522e12
    drawCircle(cx, cy, radius * 0.92f, Color(0.86f, 0.54f, 0.13f)); // Gold amber disc #db8a21

    // Deer head & antlers
    float s = radius / 120.0f; // Scale factor based on radius 120px

    // 1. Antlers (Dark bronze / olive brown #614a22)
    Color antlerCol(0.38f, 0.29f, 0.14f);
    // Left Main Antler Beam
    for (int i = 0; i < 18; ++i) {
        float t = (float)i / 18.0f;
        float ax = cx - (16.0f + sinf(t * 2.2f) * 44.0f) * s;
        float ay = cy - (20.0f + t * 65.0f) * s;
        float sz = (8.0f - t * 4.5f) * s;
        drawCircle(ax, ay, sz, antlerCol);
    }
    // Left Tines
    for (int i = 0; i < 10; ++i) {
        float t = (float)i / 10.0f;
        drawCircle(cx - (35.0f + t * 20.0f) * s, cy - (42.0f + t * 14.0f) * s, (4.5f - t * 2.5f) * s, antlerCol);
        drawCircle(cx - (46.0f - t * 16.0f) * s, cy - (60.0f + t * 24.0f) * s, (4.0f - t * 2.0f) * s, antlerCol);
    }

    // Right Main Antler Beam
    for (int i = 0; i < 18; ++i) {
        float t = (float)i / 18.0f;
        float ax = cx + (16.0f + sinf(t * 2.2f) * 44.0f) * s;
        float ay = cy - (20.0f + t * 65.0f) * s;
        float sz = (8.0f - t * 4.5f) * s;
        drawCircle(ax, ay, sz, antlerCol);
    }
    // Right Tines
    for (int i = 0; i < 10; ++i) {
        float t = (float)i / 10.0f;
        drawCircle(cx + (35.0f + t * 20.0f) * s, cy - (42.0f + t * 14.0f) * s, (4.5f - t * 2.5f) * s, antlerCol);
        drawCircle(cx + (46.0f - t * 16.0f) * s, cy - (60.0f + t * 24.0f) * s, (4.0f - t * 2.0f) * s, antlerCol);
    }

    // 2. Ears
    Color earCol(0.92f, 0.82f, 0.52f);
    // Left ear
    drawTriangle(cx - 18.0f * s, cy - 20.0f * s, cx - 44.0f * s, cy - 32.0f * s, cx - 24.0f * s, cy - 8.0f * s, earCol);
    // Right ear
    drawTriangle(cx + 18.0f * s, cy - 20.0f * s, cx + 44.0f * s, cy - 32.0f * s, cx + 24.0f * s, cy - 8.0f * s, earCol);

    // 3. Neck & Head Base
    Color headCol(0.94f, 0.86f, 0.54f);
    drawTriangle(cx, cy + 50.0f * s, cx - 28.0f * s, cy + 85.0f * s, cx + 28.0f * s, cy + 85.0f * s, headCol);
    drawRect(cx - 22.0f * s, cy + 18.0f * s, 44.0f * s, 45.0f * s, headCol);

    // Head Upper Forehead
    drawCircle(cx, cy - 4.0f * s, 26.0f * s, headCol);
    // Muzzle / Snout
    drawRoundedRect(cx - 16.0f * s, cy + 6.0f * s, 32.0f * s, 34.0f * s, 8.0f * s, headCol);

    // Nose (Dark Brown)
    Color noseCol(0.32f, 0.22f, 0.12f);
    drawRoundedRect(cx - 8.0f * s, cy + 26.0f * s, 16.0f * s, 10.0f * s, 3.5f * s, noseCol);

    // Eyes
    drawCircle(cx - 14.0f * s, cy + 4.0f * s, 3.8f * s, noseCol);
    drawCircle(cx + 14.0f * s, cy + 4.0f * s, 3.8f * s, noseCol);
}

// Draw heart icon (for player health)
void Renderer2D::drawHeart(float cx, float cy, float size, Color color) {
    float r = size * 0.28f;
    // Left circle
    drawCircle(cx - r, cy - r * 0.4f, r, color);
    // Right circle
    drawCircle(cx + r, cy - r * 0.4f, r, color);
    // Bottom triangle
    drawTriangle(cx - r * 1.95f, cy - r * 0.1f, cx + r * 1.95f, cy - r * 0.1f, cx, cy + size * 0.55f, color);
}

// Draw eye icon (for Phase 2 stealth indicator)
void Renderer2D::drawEyeIcon(float cx, float cy, float size, Color color) {
    float w = size * 0.9f;
    float h = size * 0.45f;

    // Draw eye contour using segments
    glColor4f(color.r, color.g, color.b, color.a);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    // Upper lid
    for (int i = 0; i <= 20; ++i) {
        float t = (float)i / 20.0f; // 0 to 1
        float x = (cx - w * 0.5f) + t * w;
        float y = cy - sinf(t * (float)M_PI) * h;
        glVertex2f(x, y);
    }
    // Lower lid
    for (int i = 20; i >= 0; --i) {
        float t = (float)i / 20.0f;
        float x = (cx - w * 0.5f) + t * w;
        float y = cy + sinf(t * (float)M_PI) * h;
        glVertex2f(x, y);
    }
    glEnd();
    glLineWidth(1.0f);

    // Iris & Pupil
    drawCircle(cx, cy, h * 0.65f, color);
    drawCircle(cx, cy, h * 0.28f, Color::Black());
}

// Draw animated flame
void Renderer2D::drawFlame(float cx, float cy, float size, float timeOffset) {
    float flicker = sinf(timeOffset * 10.0f) * 0.08f;
    float fH = size * (1.0f + flicker);
    float fW = size * 0.55f;

    // Outer Red/Orange Flame
    Color outerCol(0.95f, 0.32f, 0.08f);
    glBegin(GL_TRIANGLES);
    glColor4f(outerCol.r, outerCol.g, outerCol.b, outerCol.a);
    glVertex2f(cx, cy - fH);
    glVertex2f(cx - fW, cy);
    glVertex2f(cx + fW, cy);
    glEnd();
    drawCircle(cx, cy - fW * 0.2f, fW * 0.85f, outerCol);

    // Mid Gold Flame
    Color midCol(0.98f, 0.72f, 0.12f);
    glBegin(GL_TRIANGLES);
    glColor4f(midCol.r, midCol.g, midCol.b, midCol.a);
    glVertex2f(cx + flicker * 12.0f, cy - fH * 0.75f);
    glVertex2f(cx - fW * 0.65f, cy);
    glVertex2f(cx + fW * 0.65f, cy);
    glEnd();
    drawCircle(cx, cy - fW * 0.15f, fW * 0.55f, midCol);

    // Inner White Core
    Color innerCol(1.0f, 1.0f, 0.85f);
    drawCircle(cx, cy + fW * 0.1f, fW * 0.3f, innerCol);
}

// Text Rendering implementation using FONT_8X8
void Renderer2D::drawText(const std::string& text, float x, float y, float scale, Color color, bool center, bool bold) {
    float startX = x;
    if (center) {
        float totalW = getTextWidth(text, scale);
        startX -= totalW * 0.5f;
    }

    glColor4f(color.r, color.g, color.b, color.a);

    float curX = startX;
    float curY = y;

    for (char c : text) {
        if (c == '\n') {
            curY += 10.0f * scale;
            curX = startX;
            continue;
        }

        unsigned char ch = (unsigned char)c;
        if (ch < 32 || ch > 126) ch = 32;
        const unsigned char* glyph = FONT_8X8[ch - 32];

        glBegin(GL_QUADS);
        for (int row = 0; row < 8; ++row) {
            unsigned char rowBits = glyph[row];
            for (int col = 0; col < 8; ++col) {
                if (rowBits & (0x80 >> col)) {
                    float px = curX + col * scale;
                    float py = curY + row * scale;
                    float sz = scale;

                    glVertex2f(px, py);
                    glVertex2f(px + sz, py);
                    glVertex2f(px + sz, py + sz);
                    glVertex2f(px, py + sz);

                    if (bold) {
                        glVertex2f(px + 0.6f * scale, py);
                        glVertex2f(px + 1.6f * scale, py);
                        glVertex2f(px + 1.6f * scale, py + sz);
                        glVertex2f(px + 0.6f * scale, py + sz);
                    }
                }
            }
        }
        glEnd();

        curX += 8.0f * scale;
    }
}

float Renderer2D::getTextWidth(const std::string& text, float scale) {
    size_t maxLen = 0;
    size_t curLen = 0;
    for (char c : text) {
        if (c == '\n') {
            maxLen = std::max(maxLen, curLen);
            curLen = 0;
        } else {
            curLen++;
        }
    }
    maxLen = std::max(maxLen, curLen);
    return maxLen * 8.0f * scale;
}

float Renderer2D::getTextHeight(float scale) {
    return 8.0f * scale;
}

unsigned int Renderer2D::loadTexture(const std::string& filepath) {
    int width = 0, height = 0, channels = 0;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 4);
    if (!data) {
        std::cout << "[Renderer2D] Gagal memuat tekstur dari: " << filepath << std::endl;
        return 0;
    }

    unsigned int textureId = 0;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    std::cout << "[Renderer2D] Berhasil memuat tekstur: " << filepath << " (" << width << "x" << height << ")" << std::endl;
    return textureId;
}

void Renderer2D::drawTexture(unsigned int textureId, float x, float y, float w, float h, Color tint) {
    if (textureId == 0) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glColor4f(tint.r, tint.g, tint.b, tint.a);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(x + w, y);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(x + w, y + h);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y + h);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

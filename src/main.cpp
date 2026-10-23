#include <GLFW/glfw3.h>
#include <iostream>
#include "Renderer2D.h"
#include "Game.h"

static Game* s_gameInstance = nullptr;

static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (s_gameInstance) {
        s_gameInstance->onMouseMove((float)xpos, (float)ypos);
    }
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (s_gameInstance) {
        s_gameInstance->onMouseButton(button, action, mods);
    }
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (s_gameInstance) {
        s_gameInstance->onKey(key, scancode, action, mods);
    }
}

int main() {
    // 1. Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Gagal menginisialisasi GLFW!" << std::endl;
        return -1;
    }

    // Fixed resolution 1280x720, non-resizable as requested
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Prototype Game UI & Animasi - Pengalaman Pengguna", NULL, NULL);
    if (!window) {
        std::cerr << "Gagal membuat window GLFW 1280x720!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable VSync for smooth UI animations

    // Setup input callbacks
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);

    // 2. Initialize 2D Renderer
    Renderer2D::init(1280, 720);

    // 3. Initialize Game
    s_gameInstance = new Game(window);

    double lastTime = glfwGetTime();

    // 4. Main Game Loop
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float dt = (float)(currentTime - lastTime);
        lastTime = currentTime;

        // Cap dt to prevent massive jumps during debug pauses
        if (dt > 0.1f) dt = 0.1f;

        // Poll events
        glfwPollEvents();

        // Update Game State
        s_gameInstance->update(dt);

        // Render
        Renderer2D::beginFrame();
        s_gameInstance->render();
        Renderer2D::endFrame();

        glfwSwapBuffers(window);
    }

    delete s_gameInstance;
    s_gameInstance = nullptr;

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

#ifndef GAME_H
#define GAME_H

#include <GLFW/glfw3.h>
#include "UI.h"

enum class GameState {
    MainMenu,
    Settings,
    Phase1_Burn,
    Phase2_Stealth,
    Phase3_Boss,
    Victory,
    Defeat
};

enum class ConfirmDialogType {
    None,
    ReturnToMenu,
    ExitGame
};

class Game {
public:
    Game(GLFWwindow* window);
    ~Game();

    void update(float dt);
    void render();

    // Input handlers
    void onMouseMove(float x, float y);
    void onMouseButton(int button, int action, int mods);
    void onKey(int key, int scancode, int action, int mods);

    void switchState(GameState newState);

private:
    GLFWwindow* m_window;
    GameState m_state;

    // Mouse tracking
    float m_mouseX;
    float m_mouseY;
    bool m_mouseDown;
    bool m_mouseClicked;
    bool m_mouseReleased;

    // Main Menu components
    UIButton m_btnPlay;
    UIButton m_btnSetting;
    UIButton m_btnExit;
    UIButton m_btnBackSetting;
    UIButton m_btnRestart;
    UIButton m_btnMenuVictory;
    UIButton m_btnMenuDefeat;

    // Clouds in Main Menu
    struct Cloud {
        float x, y, speed, scale;
    };
    std::vector<Cloud> m_clouds;

    // Common HUD components
    UIHealthBar m_playerHpBar;
    UIFuelBar m_fuelBar;
    UIInventory m_inventory;

    // Segmen 1 helper methods
    void updateMainMenu(float dt);
    void renderMainMenu();

    void updateSettings(float dt);
    void renderSettings();

    void updateVictory(float dt);
    void renderVictory();

    void updateDefeat(float dt);
    void renderDefeat();

    void updatePhase1(float dt);
    void renderPhase1();

    void updatePhase2(float dt);
    void renderPhase2();

    void updatePhase3(float dt);
    void renderPhase3();

    // Return Navigation Confirmation Dialog helpers
    void updateConfirmDialog(float dt);
    void renderConfirmDialog();

    // First-person equipment render helpers
    void renderFirstPersonTorch(float swingAngle, float time);
    void renderFirstPersonBow(float drawTime, bool hasArrow);
    void renderFirstPersonHands(float bobTime, bool isSneaking);

    // Game runtime variables
    UIEyeMeter m_eyeMeter;
    float m_gameTime;
    float m_playerHp;

    // Phase 1 variables
    float m_oilFuel;
    float m_burnedPercent;
    float m_torchSwingTime;
    bool m_isSwingingTorch;

    // Phase 2 variables
    float m_alertLevel;
    bool m_isSneaking; // Left Ctrl held
    float m_playerWalkX;
    float m_handBobTime;
    float m_stealthGraceTimer;

    // Phase 3 variables
    float m_bossHp;
    int m_arrowsCount;
    bool m_isAimingBow;
    float m_bowDrawTime;
    float m_deerSkillTimer;
    std::string m_bossCurrentSkill;
    float m_bossSkillCooldown;

    // Textures & Visual Assets
    unsigned int m_logoTexture;

    // Return Navigation Confirmation Dialog components
    bool m_showConfirmDialog;
    ConfirmDialogType m_confirmType;
    UIButton m_btnConfirmYes;
    UIButton m_btnConfirmCancel;
};

#endif // GAME_H

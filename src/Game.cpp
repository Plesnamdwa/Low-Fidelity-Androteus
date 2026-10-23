#include "Game.h"
#include <cmath>
#include <iostream>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper structure for boss projectiles
struct BossProjectile {
    float x, y, vx, vy, radius;
    bool active;
};
static std::vector<BossProjectile> s_bossProjectiles;

Game::Game(GLFWwindow* window)
    : m_window(window),
      m_state(GameState::MainMenu),
      m_mouseX(640.0f), m_mouseY(360.0f),
      m_mouseDown(false), m_mouseClicked(false), m_mouseReleased(false),
      m_btnPlay(520.0f, 350.0f, 240.0f, 58.0f, "PLAY", 2.6f),
      m_btnSetting(520.0f, 430.0f, 240.0f, 58.0f, "SETTING", 2.6f),
      m_btnExit(520.0f, 510.0f, 240.0f, 58.0f, "EXIT", 2.6f),
      m_btnBackSetting(520.0f, 550.0f, 240.0f, 50.0f, "KEMBALI", 2.2f),
      m_btnRestart(520.0f, 440.0f, 240.0f, 54.0f, "MAIN LAGI", 2.4f),
      m_btnMenuVictory(520.0f, 510.0f, 240.0f, 54.0f, "MENU UTAMA", 2.2f),
      m_btnMenuDefeat(520.0f, 510.0f, 240.0f, 54.0f, "MENU UTAMA", 2.2f),
      m_playerHpBar(990.0f, 40.0f, 240.0f, 44.0f),
      m_fuelBar(30.0f, 50.0f, 210.0f, 34.0f),
      m_inventory(640.0f, 700.0f, 62.0f),
      m_eyeMeter(85.0f, 635.0f, 48.0f),
      m_gameTime(0.0f),
      m_playerHp(100.0f),
      m_oilFuel(100.0f),
      m_burnedPercent(0.0f),
      m_torchSwingTime(0.0f),
      m_isSwingingTorch(false),
      m_alertLevel(0.0f),
      m_isSneaking(false),
      m_playerWalkX(0.0f),
      m_handBobTime(0.0f),
      m_stealthGraceTimer(0.0f),
      m_bossHp(100.0f),
      m_arrowsCount(34),
      m_isAimingBow(false),
      m_bowDrawTime(0.0f),
      m_deerSkillTimer(2.0f),
      m_bossCurrentSkill("Siap Menyerang"),
      m_bossSkillCooldown(3.5f),
      m_logoTexture(0),
      m_showConfirmDialog(false),
      m_confirmType(ConfirmDialogType::None),
      m_btnConfirmYes(425.0f, 405.0f, 180.0f, 50.0f, "YES", 2.2f),
      m_btnConfirmCancel(675.0f, 405.0f, 180.0f, 50.0f, "CANCEL", 2.0f)
{
    // Configure Cancel button colors (neutral slate)
    m_btnConfirmCancel.baseColor = Color(0.32f, 0.38f, 0.44f);
    m_btnConfirmCancel.hoverColor = Color(0.42f, 0.48f, 0.55f);
    m_btnConfirmCancel.pressColor = Color(0.24f, 0.28f, 0.34f);
    m_btnConfirmCancel.textColor = Color::White();

    // Initialize procedural clouds
    m_clouds.push_back({ 140.0f, 240.0f, 15.0f, 1.0f });
    m_clouds.push_back({ 850.0f, 260.0f, 10.0f, 1.25f });
    m_clouds.push_back({ 450.0f, 160.0f, 18.0f, 0.85f });

    // Load logo texture
    m_logoTexture = Renderer2D::loadTexture("assets/logo.png");
    if (!m_logoTexture) {
        m_logoTexture = Renderer2D::loadTexture("logo.png");
    }
}

Game::~Game() {
    if (m_logoTexture) {
        glDeleteTextures(1, &m_logoTexture);
        m_logoTexture = 0;
    }
}

void Game::switchState(GameState newState) {
    m_state = newState;
    m_showConfirmDialog = false;
    m_confirmType = ConfirmDialogType::None;
    if (newState == GameState::Phase1_Burn) {
        m_playerHp = 100.0f;
        m_oilFuel = 100.0f;
        m_burnedPercent = 0.0f;
        m_fuelBar.setFuel(100.0f);
        m_inventory.selectSlot(0); // Select torch
        UIToast::show("Fase 1: Bakar pepohonan dengan obor [Klik Kiri]!", 3.0f, Color(0.2f, 0.7f, 0.3f, 0.95f));
    } else if (newState == GameState::Phase2_Stealth) {
        m_alertLevel = 0.0f;
        m_playerWalkX = 0.0f;
        m_isSneaking = false;
        m_stealthGraceTimer = 0.0f;
        m_inventory.clearSelection(); // Empty hands recommended
        UIToast::show("Fase 2: Menyusup! Kosongkan tangan [X] dan mengendap [Left Ctrl]!", 3.5f, Color(0.9f, 0.6f, 0.1f, 0.95f));
    } else if (newState == GameState::Phase3_Boss) {
        m_bossHp = 100.0f;
        m_arrowsCount = 34;
        m_inventory.selectSlot(1); // Select Bow
        s_bossProjectiles.clear();
        m_bossSkillCooldown = 5.0f;
        UIToast::show("Fase 3: Kalahkan Dewa Hutan! Tahan [Klik Kiri] untuk membidik!", 3.5f, Color(0.8f, 0.2f, 0.2f, 0.95f));
    }
}

void Game::onMouseMove(float x, float y) {
    m_mouseX = x;
    m_mouseY = y;
}

void Game::onMouseButton(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            m_mouseDown = true;
            m_mouseClicked = true;

            // If confirmation dialog is active, block game world interactions
            if (m_showConfirmDialog) {
                return;
            }

            // 1. Check if click is on inventory HUD slots first!
            if (m_state == GameState::Phase1_Burn || m_state == GameState::Phase2_Stealth || m_state == GameState::Phase3_Boss) {
                if (m_inventory.handleClick(m_mouseX, m_mouseY)) {
                    // Click was absorbed by inventory selection! Do NOT trigger any world action.
                    return;
                }
            }

            // Phase 1: Swing Torch / Burn
            if (m_state == GameState::Phase1_Burn) {
                ItemType currentItem = m_inventory.getSelectedItem();
                if (currentItem == ItemType::Torch) {
                    if (m_oilFuel > 0.0f) {
                        m_isSwingingTorch = true;
                        m_torchSwingTime = 0.35f;
                        m_burnedPercent = std::min(100.0f, m_burnedPercent + 12.5f);
                        // Fuel decreases proportionally to leave exactly 25% (1/4 remaining) when burned reaches 100%!
                        m_oilFuel = 100.0f - (m_burnedPercent / 100.0f) * 75.0f;

                        if (m_burnedPercent >= 100.0f) {
                            UIToast::show("Pohon habis terbakar! Gerbang terbuka!", 2.5f, Color(0.2f, 0.8f, 0.3f, 0.95f));
                        }
                    } else {
                        m_inventory.triggerErrorShake();
                        UIToast::show("Minyak habis! Tidak dapat membakar lagi!", 2.0f, Color::Red());
                    }
                } else if (currentItem == ItemType::Bow) {
                    m_inventory.triggerErrorShake();
                    UIToast::show("Gunakan Obor [1] untuk membakar pepohonan!", 2.0f, Color(0.95f, 0.6f, 0.1f));
                } else {
                    m_inventory.triggerErrorShake();
                    UIToast::show("Pegang Obor [1] untuk membakar pepohonan!", 2.0f, Color(0.95f, 0.6f, 0.1f));
                }
            }

            // Phase 2: Doing strange action increases alert!
            if (m_state == GameState::Phase2_Stealth) {
                if (m_inventory.getSelectedItem() != ItemType::None) {
                    m_alertLevel = std::min(1.0f, m_alertLevel + 0.25f);
                    UIToast::show("Aksi mencurigakan! Jangan gunakan senjata!", 2.0f, Color::Red());
                }
            }

            // Phase 3: Bow draw initiation
            if (m_state == GameState::Phase3_Boss) {
                if (m_inventory.getSelectedItem() == ItemType::Bow) {
                    if (m_arrowsCount > 0) {
                        m_isAimingBow = true;
                    } else {
                        m_inventory.triggerErrorShake();
                        UIToast::show("Panah habis! Tidak dapat menembak!", 2.0f, Color::Red());
                    }
                } else {
                    m_inventory.triggerErrorShake();
                    UIToast::show("Gunakan Busur Panah [2] untuk menyerang Dewa Hutan!", 2.0f, Color(0.95f, 0.6f, 0.1f));
                }
            }

        } else if (action == GLFW_RELEASE) {
            m_mouseDown = false;
            m_mouseReleased = true;

            // If confirmation dialog is active, block game world interactions
            if (m_showConfirmDialog) {
                return;
            }

            // Phase 3: Shoot Arrow on release
            if (m_state == GameState::Phase3_Boss && m_isAimingBow) {
                m_isAimingBow = false;
                if (m_arrowsCount > 0) {
                    m_arrowsCount--;

                    // Hit test fixed center crosshair (640, 360) on boss (Boss center ~ 640, 430, radius 135)
                    float cx = 640.0f;
                    float cy = 360.0f;
                    float dx = cx - 640.0f;
                    float dy = cy - 430.0f;
                    float dist = sqrtf(dx * dx + dy * dy);

                    if (dist < 135.0f) {
                        float damage = 15.0f;
                        m_bossHp = std::max(0.0f, m_bossHp - damage);
                        UIToast::show("Serangan Panah Telak! Boss HP -15", 1.2f, Color(0.2f, 0.8f, 0.3f, 0.9f));
                        if (m_bossHp <= 0.0f) {
                            switchState(GameState::Victory);
                        }
                    } else {
                        UIToast::show("Tembakan meleset!", 1.0f, Color(0.8f, 0.5f, 0.2f, 0.9f));
                    }
                }
            }
        }
    }
}

void Game::onKey(int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        // Inventory shortcut numbers 1..5
        if (key >= GLFW_KEY_1 && key <= GLFW_KEY_5) {
            m_inventory.selectSlot(key - GLFW_KEY_1);
        }
        // X key to empty hands
        if (key == GLFW_KEY_X) {
            m_inventory.clearSelection();
        }
        // Left Control to sneak
        if (key == GLFW_KEY_LEFT_CONTROL) {
            m_isSneaking = true;
            if (m_state == GameState::Phase2_Stealth) {
                UIToast::show("Mode Mengendap [Aktif]", 1.0f, Color(0.2f, 0.7f, 0.3f, 0.9f));
            }
        }
        // Space to stand up
        if (key == GLFW_KEY_SPACE) {
            m_isSneaking = false;
            if (m_state == GameState::Phase2_Stealth) {
                UIToast::show("Berdiri / Bangkit", 1.0f, Color(0.8f, 0.6f, 0.1f, 0.9f));
            }
        }
        // ESC to return to menu or exit (consistent Return Navigation)
        if (key == GLFW_KEY_ESCAPE) {
            if (m_showConfirmDialog) {
                // Pressing ESC again cancels the dialog!
                m_showConfirmDialog = false;
                m_confirmType = ConfirmDialogType::None;
            } else {
                if (m_state == GameState::MainMenu) {
                    m_showConfirmDialog = true;
                    m_confirmType = ConfirmDialogType::ExitGame;
                } else {
                    m_showConfirmDialog = true;
                    m_confirmType = ConfirmDialogType::ReturnToMenu;
                    m_isAimingBow = false; // Cancel aiming if in middle of pulling bow
                }
            }
        }
    } else if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT_CONTROL) {
            // Optional: can require holding or toggle. We allow holding Ctrl or pressing Space to stand
        }
    }
}

void Game::update(float dt) {
    m_gameTime += dt;
    UIToast::update(dt);

    if (m_showConfirmDialog) {
        updateConfirmDialog(dt);
    } else {
        switch (m_state) {
            case GameState::MainMenu: updateMainMenu(dt); break;
            case GameState::Settings: updateSettings(dt); break;
            case GameState::Phase1_Burn: updatePhase1(dt); break;
            case GameState::Phase2_Stealth: updatePhase2(dt); break;
            case GameState::Phase3_Boss: updatePhase3(dt); break;
            case GameState::Victory: updateVictory(dt); break;
            case GameState::Defeat: updateDefeat(dt); break;
        }
    }

    // Reset single-frame mouse flags
    m_mouseClicked = false;
    m_mouseReleased = false;
}

void Game::render() {
    glClear(GL_COLOR_BUFFER_BIT);

    switch (m_state) {
        case GameState::MainMenu: renderMainMenu(); break;
        case GameState::Settings: renderSettings(); break;
        case GameState::Phase1_Burn: renderPhase1(); break;
        case GameState::Phase2_Stealth: renderPhase2(); break;
        case GameState::Phase3_Boss: renderPhase3(); break;
        case GameState::Victory: renderVictory(); break;
        case GameState::Defeat: renderDefeat(); break;
    }

    // Render Return Navigation Confirmation Dialog on top if active
    if (m_showConfirmDialog) {
        renderConfirmDialog();
    }

    // Always render Toast notifications on top
    UIToast::render();
}


// ==========================================
// SEGMEN 1: MAIN MENU & SETTINGS
// ==========================================

void Game::updateMainMenu(float dt) {
    // Animate clouds
    for (auto& cloud : m_clouds) {
        cloud.x += cloud.speed * dt;
        if (cloud.x > 1380.0f) {
            cloud.x = -150.0f;
        }
    }

    // Update buttons
    m_btnPlay.update(m_mouseX, m_mouseY, m_mouseDown, dt);
    m_btnSetting.update(m_mouseX, m_mouseY, m_mouseDown, dt);
    m_btnExit.update(m_mouseX, m_mouseY, m_mouseDown, dt);

    if (m_btnPlay.isClicked(m_mouseX, m_mouseY, m_mouseReleased)) {
        switchState(GameState::Phase1_Burn);
    }
    if (m_btnSetting.isClicked(m_mouseX, m_mouseY, m_mouseReleased)) {
        switchState(GameState::Settings);
    }
    if (m_btnExit.isClicked(m_mouseX, m_mouseY, m_mouseReleased)) {
        m_showConfirmDialog = true;
        m_confirmType = ConfirmDialogType::ExitGame;
    }
}

void Game::renderMainMenu() {
    // 1. Sky blue background #aee5f5
    Renderer2D::drawRect(0, 0, 1280, 720, Color::SkyBlue());

    // 2. Clouds
    for (const auto& cloud : m_clouds) {
        Renderer2D::drawCloud(cloud.x, cloud.y, cloud.scale);
    }

    // 3. Dark green rolling hills at bottom (#0f341b and #092312)
    // Left Hill
    Renderer2D::drawCircle(290.0f, 960.0f, 440.0f, Color::ForestGreen(), 60);
    // Right Hill
    Renderer2D::drawCircle(990.0f, 960.0f, 440.0f, Color::ForestGreen(), 60);

    // Pine trees scattered across the hills
    Renderer2D::drawPineTree(150.0f, 655.0f, 0.9f, Color(0.05f, 0.18f, 0.08f), Color::DarkWood());
    Renderer2D::drawPineTree(310.0f, 630.0f, 1.1f, Color(0.05f, 0.18f, 0.08f), Color::DarkWood());
    Renderer2D::drawPineTree(460.0f, 655.0f, 0.95f, Color(0.05f, 0.18f, 0.08f), Color::DarkWood());

    Renderer2D::drawPineTree(800.0f, 660.0f, 0.9f, Color(0.05f, 0.18f, 0.08f), Color::DarkWood());
    Renderer2D::drawPineTree(950.0f, 630.0f, 1.1f, Color(0.05f, 0.18f, 0.08f), Color::DarkWood());
    Renderer2D::drawPineTree(1070.0f, 640.0f, 1.0f, Color(0.05f, 0.18f, 0.08f), Color::DarkWood());
    Renderer2D::drawPineTree(1190.0f, 665.0f, 0.85f, Color(0.05f, 0.18f, 0.08f), Color::DarkWood());

    // 4. Center Deer Emblem Logo (matches Mockup 1)
    if (m_logoTexture != 0) {
        float logoRadius = 110.0f;
        float logoSize = logoRadius * 2.0f;
        Renderer2D::drawTexture(m_logoTexture, 640.0f - logoRadius, 195.0f - logoRadius, logoSize, logoSize);
    } else {
        Renderer2D::drawDeerLogo(640.0f, 195.0f, 110.0f);
    }

    // 5. Buttons: PLAY, SETTING, EXIT
    m_btnPlay.render();
    m_btnSetting.render();
    m_btnExit.render();

    // Subtle footer info
    Renderer2D::drawText("Prototype Pengalaman Pengguna - C++ & OpenGL (1280x720)", 640.0f, 695.0f, 1.3f, Color(0.2f, 0.4f, 0.3f, 0.85f), true);
}


// --- SETTINGS (Polos / Wireframe Low Fidelity) ---
void Game::updateSettings(float dt) {
    m_btnBackSetting.update(m_mouseX, m_mouseY, m_mouseDown, dt);
    if (m_btnBackSetting.isClicked(m_mouseX, m_mouseY, m_mouseReleased)) {
        switchState(GameState::MainMenu);
    }
}

void Game::renderSettings() {
    // Dark clean background overlay
    Renderer2D::drawRect(0, 0, 1280, 720, Color(0.12f, 0.16f, 0.18f));

    // Wireframe panel box
    Renderer2D::drawRoundedRect(340.0f, 80.0f, 600.0f, 560.0f, 16.0f, Color(0.18f, 0.23f, 0.26f));
    Renderer2D::drawRoundedRectOutline(340.0f, 80.0f, 600.0f, 560.0f, 16.0f, Color::White(), 2.0f);

    // Title
    Renderer2D::drawText("PENGATURAN / SETTINGS", 640.0f, 125.0f, 2.5f, Color::White(), true, true);
    Renderer2D::drawLine(400.0f, 165.0f, 880.0f, 165.0f, Color(0.5f, 0.5f, 0.5f), 1.5f);

    // Low-fidelity option wireframes
    Renderer2D::drawText("Volume Musik (BGM):", 420.0f, 210.0f, 1.8f, Color::White());
    Renderer2D::drawRect(420.0f, 240.0f, 440.0f, 20.0f, Color(0.3f, 0.35f, 0.4f));
    Renderer2D::drawRect(420.0f, 240.0f, 320.0f, 20.0f, Color::GoldButton());
    Renderer2D::drawText("75%", 875.0f, 242.0f, 1.6f, Color::White());

    Renderer2D::drawText("Volume Efek Suara (SFX):", 420.0f, 290.0f, 1.8f, Color::White());
    Renderer2D::drawRect(420.0f, 320.0f, 440.0f, 20.0f, Color(0.3f, 0.35f, 0.4f));
    Renderer2D::drawRect(420.0f, 320.0f, 380.0f, 20.0f, Color::GoldButton());
    Renderer2D::drawText("85%", 875.0f, 322.0f, 1.6f, Color::White());

    Renderer2D::drawText("Resolusi Layar:", 420.0f, 370.0f, 1.8f, Color::White());
    Renderer2D::drawRoundedRect(420.0f, 400.0f, 440.0f, 36.0f, 6.0f, Color(0.24f, 0.28f, 0.32f));
    Renderer2D::drawRoundedRectOutline(420.0f, 400.0f, 440.0f, 36.0f, 6.0f, Color::White(), 1.2f);
    Renderer2D::drawText("1280 x 720 (Fixed Aspect Ratio)", 640.0f, 410.0f, 1.6f, Color::White(), true);

    Renderer2D::drawText("[Rancangan Polos - Wireframe Konseptual]", 640.0f, 475.0f, 1.4f, Color(0.7f, 0.7f, 0.7f), true);

    // Back Button
    m_btnBackSetting.render();
}


// ==========================================
// FIRST-PERSON EQUIPMENT RENDERING HELPERS
// ==========================================

void Game::renderFirstPersonTorch(float swingTime, float time) {
    float swingAngle = (swingTime > 0.0f) ? sinf(swingTime * 20.0f) * 0.35f : 0.0f;
    float handleBaseX = 1060.0f - swingAngle * 60.0f;
    float handleBaseY = 760.0f;
    float handleTopX = 1000.0f - swingAngle * 100.0f;
    float handleTopY = 560.0f - (swingTime > 0.0f ? 40.0f : 0.0f);

    // Player arm holding torch
    Color armCol(0.98f, 0.92f, 0.65f);
    Renderer2D::drawTriangle(handleBaseX - 30.0f, 760.0f, handleBaseX + 60.0f, 760.0f, handleBaseX, 680.0f, armCol);

    // Torch Wooden Handle
    Renderer2D::drawLine(handleBaseX, handleBaseY, handleTopX, handleTopY, Color(0.55f, 0.40f, 0.18f), 24.0f);
    // Torch Head Binding
    Renderer2D::drawLine(handleTopX + 5.0f, handleTopY + 10.0f, handleTopX - 5.0f, handleTopY - 10.0f, Color::Black(), 12.0f);
    // Blazing Torch Flame
    Renderer2D::drawFlame(handleTopX, handleTopY - 10.0f, 65.0f, time * 1.5f);
}

void Game::renderFirstPersonBow(float drawTime, bool hasArrow) {
    float pullOffset = drawTime * 65.0f;

    // 1. Bow tips and rest string position
    float topTipX = 1180.0f, topTipY = 150.0f;
    float botTipX = 1180.0f, botTipY = 770.0f;

    // Center of the string at rest: straight vertical line between topTip and botTip!
    float restNockX = (topTipX + botTipX) * 0.5f; // 1180.0f
    float restNockY = (topTipY + botTipY) * 0.5f; // 460.0f

    // Aim vector from resting nock towards screen center crosshair (640, 360)
    float targetX = 640.0f;
    float targetY = 360.0f;
    float dx = targetX - restNockX;
    float dy = targetY - restNockY;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.001f) len = 1.0f;
    float ux = dx / len;
    float uy = dy / len;

    // Grip on the bow riser lies forward along the aim vector (towards target)
    float braceHeight = 165.0f; // Distance from string to riser
    float gripX = restNockX + ux * braceHeight; // ~1018.0f
    float gripY = restNockY + uy * braceHeight; // ~430.0f

    // 2. Generate smooth points along the curved bow limb (curves forward to grip)
    std::vector<std::pair<float, float>> bowPoints;
    std::vector<float> bowWidths;
    const int NUM_SEGS = 40;

    for (int i = 0; i <= NUM_SEGS; ++i) {
        float t = (float)i / NUM_SEGS;
        float x, y, w;
        if (t <= 0.5f) {
            // Upper limb (topTip to grip)
            float u = t / 0.5f;
            float p0x = topTipX, p0y = topTipY;
            float p1x = 1040.0f, p1y = 260.0f;
            float p2x = gripX,   p2y = gripY;
            x = (1.0f - u) * (1.0f - u) * p0x + 2.0f * (1.0f - u) * u * p1x + u * u * p2x;
            y = (1.0f - u) * (1.0f - u) * p0y + 2.0f * (1.0f - u) * u * p1y + u * u * p2y;
            w = 10.0f + u * 10.0f; // 10px at tip, 20px at grip
        } else {
            // Lower limb (grip to botTip)
            float u = (t - 0.5f) / 0.5f;
            float p0x = gripX,   p0y = gripY;
            float p1x = 1040.0f, p1y = 630.0f;
            float p2x = botTipX, p2y = botTipY;
            x = (1.0f - u) * (1.0f - u) * p0x + 2.0f * (1.0f - u) * u * p1x + u * u * p2x;
            y = (1.0f - u) * (1.0f - u) * p0y + 2.0f * (1.0f - u) * u * p1y + u * u * p2y;
            w = 20.0f - u * 10.0f; // 20px at grip, 10px at bottom tip
        }
        bowPoints.push_back({ x, y });
        bowWidths.push_back(w);
    }

    // Drop shadow
    std::vector<std::pair<float, float>> shadowPoints = bowPoints;
    for (auto& pt : shadowPoints) {
        pt.first += 3.0f;
        pt.second += 5.0f;
    }
    Renderer2D::drawCurvedRibbon(shadowPoints, bowWidths, Color(0.0f, 0.0f, 0.0f, 0.22f));

    // Outer dark outline
    Renderer2D::drawCurvedRibbonOutline(bowPoints, bowWidths, Color(0.18f, 0.20f, 0.22f), 2.0f);

    // Solid bow body (Slate gray #6e767d)
    Renderer2D::drawCurvedRibbon(bowPoints, bowWidths, Color(0.52f, 0.56f, 0.60f));

    // 3. Bow String
    // When drawTime == 0, nock is at (restNockX, restNockY).
    // The string is a 100% STRAIGHT vertical line between topTip and botTip!
    // When drawTime > 0, nock is pulled BACKWARDS (away from target, to the right)
    float nockX = restNockX - ux * pullOffset;
    float nockY = restNockY - uy * pullOffset;

    // Draw Bowstring: top to nock, and nock to bottom
    Renderer2D::drawLine(topTipX, topTipY, nockX, nockY, Color::White(), 1.8f);
    Renderer2D::drawLine(nockX, nockY, botTipX, botTipY, Color::White(), 1.8f);

    // 4. Arrow (Anak Panah)
    if (hasArrow) {
        float arrowLen = 350.0f; // Long enough to rest on string at 1180, pass through grip at ~1018, and extend to ~836 forward
        float tipX = nockX + ux * arrowLen;
        float tipY = nockY + uy * arrowLen;

        // Wooden Arrow Shaft
        Renderer2D::drawLine(nockX, nockY, tipX, tipY, Color(0.48f, 0.32f, 0.14f), 5.5f);

        // Arrowhead at tip pointing along (ux, uy)
        float headLen = 22.0f;
        float headHalfW = 9.0f;
        float nx = -uy;
        float ny =  ux;

        float pLeftX  = tipX - ux * headLen + nx * headHalfW;
        float pLeftY  = tipY - uy * headLen + ny * headHalfW;
        float pRightX = tipX - ux * headLen - nx * headHalfW;
        float pRightY = tipY - uy * headLen - ny * headHalfW;

        // Arrowhead polygon & outline
        Renderer2D::drawTriangle(tipX, tipY, pLeftX, pLeftY, pRightX, pRightY, Color(0.85f, 0.88f, 0.92f));
        Renderer2D::drawLine(tipX, tipY, pLeftX, pLeftY, Color::Black(), 1.5f);
        Renderer2D::drawLine(pLeftX, pLeftY, pRightX, pRightY, Color::Black(), 1.5f);
        Renderer2D::drawLine(pRightX, pRightY, tipX, tipY, Color::Black(), 1.5f);

        // Arrow fletching feathers near the nock:
        // Aerodynamic orientation: Root starts on shaft forward, and vanes flare BACKWARD toward the nock!
        float fFrontX = nockX + ux * 45.0f;
        float fFrontY = nockY + uy * 45.0f;
        float fRearX  = nockX + ux * 12.0f;
        float fRearY  = nockY + uy * 12.0f;

        float vaneTopX = fRearX + nx * 11.0f - ux * 2.0f;
        float vaneTopY = fRearY + ny * 11.0f - uy * 2.0f;
        float vaneBotX = fRearX - nx * 11.0f - ux * 2.0f;
        float vaneBotY = fRearY - ny * 11.0f - uy * 2.0f;

        Color fletchCol(0.92f, 0.22f, 0.22f);
        Color fletchOutline(0.60f, 0.10f, 0.10f);

        // Top vane (slants backward toward nock)
        Renderer2D::drawTriangle(fFrontX, fFrontY, vaneTopX, vaneTopY, fRearX, fRearY, fletchCol);
        Renderer2D::drawLine(fFrontX, fFrontY, vaneTopX, vaneTopY, fletchOutline, 1.5f);
        Renderer2D::drawLine(vaneTopX, vaneTopY, fRearX, fRearY, fletchOutline, 1.5f);

        // Bottom vane (slants backward toward nock)
        Renderer2D::drawTriangle(fFrontX, fFrontY, vaneBotX, vaneBotY, fRearX, fRearY, fletchCol);
        Renderer2D::drawLine(fFrontX, fFrontY, vaneBotX, vaneBotY, fletchOutline, 1.5f);
        Renderer2D::drawLine(vaneBotX, vaneBotY, fRearX, fRearY, fletchOutline, 1.5f);
    }

    // 5. Player Hand & Arm holding the grip
    Color skinCol(0.98f, 0.92f, 0.65f);
    Renderer2D::drawTriangle(1090.0f, 760.0f, 1220.0f, 760.0f, gripX, gripY, skinCol);
    // Leather grip wrap around the riser
    Renderer2D::drawRoundedRect(gripX - 12.0f, gripY - 20.0f, 24.0f, 40.0f, 6.0f, Color(0.32f, 0.20f, 0.10f));
    Renderer2D::drawRoundedRectOutline(gripX - 12.0f, gripY - 20.0f, 24.0f, 40.0f, 6.0f, Color::Black(), 1.5f);

    // Hand/Fingers gripping over the riser
    Renderer2D::drawCircle(gripX, gripY, 22.0f, skinCol);
    Renderer2D::drawCircleOutline(gripX, gripY, 22.0f, Color(0.25f, 0.20f, 0.10f), 1.5f);
    Renderer2D::drawLine(gripX - 8.0f, gripY - 8.0f, gripX - 8.0f, gripY + 8.0f, Color(0.85f, 0.75f, 0.50f), 2.5f);
}

void Game::renderFirstPersonHands(float bobTime, bool isSneaking) {
    float bobY = sinf(bobTime) * 12.0f;
    float crouchOffset = isSneaking ? 40.0f : 0.0f;

    // Left Hand
    float lhX = 320.0f;
    float lhY = 620.0f + bobY + crouchOffset;
    Color skinCol(0.98f, 0.92f, 0.65f); // Soft peach/yellow skin tone
    // Arm
    Renderer2D::drawTriangle(lhX - 80.0f, 760.0f, lhX + 20.0f, 760.0f, lhX - 20.0f, lhY, skinCol);
    // Palm
    Renderer2D::drawCircle(lhX - 20.0f, lhY, 34.0f, skinCol);
    // Fingers
    Renderer2D::drawLine(lhX - 40.0f, lhY, lhX - 52.0f, lhY - 45.0f, skinCol, 12.0f);
    Renderer2D::drawLine(lhX - 26.0f, lhY, lhX - 30.0f, lhY - 60.0f, skinCol, 12.0f);
    Renderer2D::drawLine(lhX - 12.0f, lhY, lhX - 10.0f, lhY - 65.0f, skinCol, 12.0f);
    Renderer2D::drawLine(lhX + 2.0f,  lhY, lhX + 10.0f, lhY - 55.0f, skinCol, 12.0f);

    // Right Hand
    float rhX = 960.0f;
    float rhY = 620.0f - bobY + crouchOffset;
    Renderer2D::drawTriangle(rhX - 20.0f, 760.0f, rhX + 80.0f, 760.0f, rhX + 20.0f, rhY, skinCol);
    Renderer2D::drawCircle(rhX + 20.0f, rhY, 34.0f, skinCol);
    Renderer2D::drawLine(rhX + 40.0f, rhY, rhX + 52.0f, rhY - 45.0f, skinCol, 12.0f);
    Renderer2D::drawLine(rhX + 26.0f, rhY, rhX + 30.0f, rhY - 60.0f, skinCol, 12.0f);
    Renderer2D::drawLine(rhX + 12.0f, rhY, rhX + 10.0f, rhY - 65.0f, skinCol, 12.0f);
    Renderer2D::drawLine(rhX - 2.0f,  rhY, rhX - 10.0f, rhY - 55.0f, skinCol, 12.0f);
}

// ==========================================
// FASE 1: MEMBAKAR HUTAN (THE BURNING PHASE)
// ==========================================

void Game::updatePhase1(float dt) {
    m_playerHpBar.setHp(m_playerHp);
    m_playerHpBar.update(dt);
    m_fuelBar.setFuel(m_oilFuel);
    m_fuelBar.update(dt);
    m_inventory.update(m_mouseX, m_mouseY, false, dt);

    if (m_torchSwingTime > 0.0f) {
        m_torchSwingTime -= dt;
        if (m_torchSwingTime <= 0.0f) {
            m_isSwingingTorch = false;
        }
    }

    // Auto-advance to Phase 2 once forest is burned
    if (m_burnedPercent >= 100.0f) {
        static float advanceTimer = 0.0f;
        advanceTimer += dt;
        if (advanceTimer >= 1.8f) {
            advanceTimer = 0.0f;
            switchState(GameState::Phase2_Stealth);
        }
    }
}

void Game::renderPhase1() {
    // 1. Sky Blue background #aee5f5
    Renderer2D::drawRect(0, 0, 1280, 720, Color::SkyBlue());

    // 2. Background Cloud/Hill Circular Domes behind the trees (2 circles, per Figma mockup)
    // Left circular dome
    Renderer2D::drawCircle(280.0f, 820.0f, 460.0f, Color(0.82f, 0.93f, 0.96f, 0.85f), 60);
    // Right circular dome (bundaran ke-2)
    Renderer2D::drawCircle(980.0f, 860.0f, 440.0f, Color(0.82f, 0.93f, 0.96f, 0.85f), 60);

    // 3. Dense Pine Trees (2 rows: back darker row, front brighter row)
    // Trees turn charred black progressively as m_burnedPercent increases!
    // Back row (15 trees)
    for (int i = 0; i < 15; ++i) {
        float tx = 25.0f + i * 90.0f;
        float treeThreshold = ((float)i / 15.0f) * 85.0f;
        float burnFactor = 0.0f;
        if (m_burnedPercent > treeThreshold) {
            burnFactor = (m_burnedPercent - treeThreshold) / 20.0f;
            if (burnFactor > 1.0f) burnFactor = 1.0f;
        }

        // Color transition: from deep pine green to charred charcoal black
        Color folCol(
            0.06f * (1.0f - burnFactor) + 0.10f * burnFactor,
            0.22f * (1.0f - burnFactor) + 0.09f * burnFactor,
            0.11f * (1.0f - burnFactor) + 0.07f * burnFactor
        );
        Color trkCol = (burnFactor > 0.4f) ? Color(0.20f, 0.15f, 0.10f) : Color::DarkWood();
        Renderer2D::drawPineTree(tx, 720.0f, 4.4f, folCol, trkCol);
    }
    // Front row (14 trees)
    for (int i = 0; i < 14; ++i) {
        float tx = 65.0f + i * 90.0f;
        float treeThreshold = ((float)i / 14.0f) * 85.0f;
        float burnFactor = 0.0f;
        if (m_burnedPercent > treeThreshold) {
            burnFactor = (m_burnedPercent - treeThreshold) / 20.0f;
            if (burnFactor > 1.0f) burnFactor = 1.0f;
        }

        // Color transition: from lush forest green #174224 to charred black with slight ember ash
        Color folCol(
            0.09f * (1.0f - burnFactor) + 0.11f * burnFactor,
            0.26f * (1.0f - burnFactor) + 0.09f * burnFactor,
            0.14f * (1.0f - burnFactor) + 0.07f * burnFactor
        );
        Color trkCol = (burnFactor > 0.4f) ? Color(0.18f, 0.14f, 0.09f) : Color::DarkWood();
        Renderer2D::drawPineTree(tx, 720.0f, 4.8f, folCol, trkCol);
    }

    // 4. Render Flames on Trees (strictly positioned on pine branches, never floating!)
    static const struct { float x, y, size; } s_fixedTreeFlames[] = {
        {  60.0f, 570.0f, 28.0f },
        { 160.0f, 505.0f, 32.0f },
        { 240.0f, 495.0f, 34.0f },
        { 295.0f, 585.0f, 28.0f },
        { 405.0f, 525.0f, 32.0f },
        { 455.0f, 590.0f, 28.0f },
        { 510.0f, 505.0f, 32.0f },
        { 575.0f, 530.0f, 28.0f },
        { 650.0f, 560.0f, 30.0f },
        { 720.0f, 505.0f, 32.0f },
        { 790.0f, 575.0f, 28.0f },
        { 860.0f, 515.0f, 30.0f },
        { 930.0f, 565.0f, 32.0f },
        { 1005.0f, 510.0f, 30.0f },
        { 1080.0f, 570.0f, 28.0f },
        { 1160.0f, 520.0f, 32.0f }
    };

    // Calculate number of flames visible according to burned progress (up to 16 flames at 100%)
    int maxFlames = 16;
    int activeFlames = (int)ceilf((m_burnedPercent / 100.0f) * maxFlames);
    if (activeFlames > maxFlames) activeFlames = maxFlames;

    for (int i = 0; i < activeFlames; ++i) {
        float timeOffset = i * 0.35f;
        Renderer2D::drawFlame(s_fixedTreeFlames[i].x, s_fixedTreeFlames[i].y, s_fixedTreeFlames[i].size, m_gameTime + timeOffset);
    }

    // 5. First-Person Equipment View: Accurately reflects what is in the inventory slot!
    ItemType equipped = m_inventory.getSelectedItem();
    if (equipped == ItemType::Torch) {
        renderFirstPersonTorch(m_torchSwingTime, m_gameTime);
    } else if (equipped == ItemType::Bow) {
        renderFirstPersonBow(0.0f, true);
    } else {
        renderFirstPersonHands(0.0f, false);
    }

    // 6. HUD
    // Top-left: Minyak bar
    m_fuelBar.render();

    // Top-center: "Terbakar : X%" text
    std::string burnStr = "Terbakar : " + std::to_string((int)m_burnedPercent) + "%";
    Renderer2D::drawText(burnStr, 640.0f, 35.0f, 2.5f, Color::Black(), true, true);

    // Top-right: Player HP Bar
    m_playerHpBar.render();

    // Bottom-center: 5-slot Inventory
    m_inventory.render();
}


// ==========================================
// FASE 2: MENGENDAP-ENDAP (THE STEALTH PHASE)
// ==========================================

void Game::updatePhase2(float dt) {
    m_playerHpBar.update(dt);
    m_inventory.update(m_mouseX, m_mouseY, false, dt);

    // Update stealth grace timer (cooldown after getting caught)
    if (m_stealthGraceTimer > 0.0f) {
        m_stealthGraceTimer -= dt;
        m_alertLevel = 0.0f;
    }

    // Player sneaking detection mechanics
    bool isMoving = (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS ||
                     glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS);

    bool holdingWeapon = (m_inventory.getSelectedItem() != ItemType::None);

    if (m_stealthGraceTimer <= 0.0f) {
        if (isMoving) {
            m_handBobTime += dt * (m_isSneaking ? 5.0f : 10.0f);
            m_playerWalkX += (m_isSneaking ? 40.0f : 100.0f) * dt;

            if (!m_isSneaking && holdingWeapon) {
                // Running while holding weapon: Extreme alert!
                m_alertLevel = std::min(1.0f, m_alertLevel + 1.20f * dt);
                UIToast::show("BAHAYA! Jangan berlari membawa senjata!", 1.0f, Color::Red());
            } else if (holdingWeapon) {
                // Sneaking but holding weapon (guards glimpse weapon): Moderate alert!
                m_alertLevel = std::min(1.0f, m_alertLevel + 0.45f * dt);
                UIToast::show("Senjata terlihat! Kosongkan tangan [X]!", 1.0f, Color(0.95f, 0.6f, 0.2f));
            } else if (!m_isSneaking) {
                // Walking upright without sneaking: High alert!
                m_alertLevel = std::min(1.0f, m_alertLevel + 0.65f * dt);
                UIToast::show("Hati-hati! Tahan [Left Ctrl] untuk mengendap!", 1.0f, Color::Red());
            } else {
                // Properly sneaking with empty hands: Alert safely decreases!
                m_alertLevel = std::max(0.0f, m_alertLevel - 0.35f * dt);
            }
        } else {
            // Idle (Standing still / hiding) -> Alert ALWAYS decays down to 0!
            if (m_isSneaking) {
                // Sneaking while hiding: Fastest alert decay!
                m_alertLevel = std::max(0.0f, m_alertLevel - 0.45f * dt);
            } else if (!holdingWeapon) {
                // Standing still with empty hands: Fast decay!
                m_alertLevel = std::max(0.0f, m_alertLevel - 0.35f * dt);
            } else {
                // Standing still holding weapon: Steady decay down to 0!
                m_alertLevel = std::max(0.0f, m_alertLevel - 0.25f * dt);
            }
        }
    }

    m_eyeMeter.setAlert(m_alertLevel);
    m_eyeMeter.update(dt);

    // If alert reaches 100%, caught error handling!
    if (m_alertLevel >= 1.0f) {
        // Break any loop immediately: auto-drop weapon, 2.5s immunity grace period, reset alert to 0!
        m_inventory.clearSelection();
        m_inventory.triggerErrorShake();
        m_alertLevel = 0.0f;
        m_stealthGraceTimer = 2.5f;

        m_playerHp = std::max(0.0f, m_playerHp - 20.0f);
        m_playerHpBar.setHp(m_playerHp);
        m_playerWalkX = std::max(0.0f, m_playerWalkX - 40.0f); // pushed back slightly
        UIToast::show("Tertangkap Penjaga! Senjata disita & HP -20!", 2.5f, Color::Red());

        if (m_playerHp <= 0.0f) {
            switchState(GameState::Defeat);
        }
    }

    // Reaching gate successfully
    if (m_playerWalkX > 300.0f) {
        UIToast::show("Berhasil menyusup masuk ke dalam gerbang!", 2.5f, Color(0.2f, 0.8f, 0.3f, 0.95f));
        switchState(GameState::Phase3_Boss);
    }
}

void Game::renderPhase2() {
    bool isMoving = (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS ||
                     glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS);

    float walkProgress = std::min(1.0f, m_playerWalkX / 300.0f);
    // Smooth zoom in towards the golden gate as player advances
    float zoomScale = 1.0f + walkProgress * 0.85f;

    // Gentle camera bobbing (not too strong, natural walking feel)
    float camBobY = isMoving ? sinf(m_handBobTime * 1.5f) * 4.0f : 0.0f;
    float camSwayX = isMoving ? cosf(m_handBobTime * 0.75f) * 2.2f : 0.0f;

    float gateCenterX = 640.0f;
    float gateCenterY = 435.0f;

    glPushMatrix();
    glTranslatef(gateCenterX + camSwayX, gateCenterY + camBobY, 0.0f);
    glScalef(zoomScale, zoomScale, 1.0f);
    glTranslatef(-gateCenterX, -gateCenterY, 0.0f);

    // 1. Lush Green Forest floor background #117822
    Renderer2D::drawRect(-300.0f, -200.0f, 1880.0f, 1120.0f, Color(0.06f, 0.47f, 0.13f));

    // Pine trees background behind the wall
    for (int i = -1; i < 18; ++i) {
        float tx = 20.0f + i * 80.0f;
        Renderer2D::drawPineTree(tx, 260.0f, 2.0f, Color::ForestGreen(), Color::DarkWood());
    }

    // 2. Wooden Wall / Palisade (matches Mockup 3)
    float wallTop = 320.0f;
    float wallH = 200.0f;
    Renderer2D::drawRect(-300.0f, wallTop, 1880.0f, wallH, Color(0.42f, 0.30f, 0.10f));

    // Wooden vertical logs
    for (int i = -3; i < 20; ++i) {
        float lx = 10.0f + i * 80.0f;
        if (i >= 7 && i <= 8) continue; // Doorway opening
        Renderer2D::drawRect(lx, wallTop - 22.0f, 46.0f, wallH + 22.0f, Color(0.48f, 0.34f, 0.12f));
        Renderer2D::drawRectOutline(lx, wallTop - 22.0f, 46.0f, wallH + 22.0f, Color(0.32f, 0.22f, 0.08f), 2.0f);
    }

    // Golden glowing doorway gate in center
    float gateX = 565.0f;
    float gateY = 350.0f;
    float gateW = 150.0f;
    float gateH = 170.0f;
    Renderer2D::drawRect(gateX, gateY, gateW, gateH, Color(0.88f, 0.72f, 0.05f));
    // Doorway interior illumination
    Renderer2D::drawRect(gateX + 15.0f, gateY + 15.0f, gateW - 30.0f, gateH - 15.0f, Color(1.0f, 0.88f, 0.25f));
    Renderer2D::drawRectOutline(gateX, gateY, gateW, gateH, Color(0.45f, 0.35f, 0.05f), 4.0f);

    glPopMatrix();

    // 3. First-Person Equipment View (visible according to equipped inventory slot)
    ItemType equipped = m_inventory.getSelectedItem();
    if (equipped == ItemType::None) {
        renderFirstPersonHands(m_handBobTime, m_isSneaking);
    } else if (equipped == ItemType::Torch) {
        renderFirstPersonTorch(0.0f, m_gameTime);
    } else if (equipped == ItemType::Bow) {
        renderFirstPersonBow(0.0f, true);
    }

    // 4. HUD
    // Bottom-left: Eye Icon with alert indicator (matches Mockup 3)
    m_eyeMeter.render();

    // Top-right: Player HP Bar
    m_playerHpBar.render();

    // Bottom-center: Inventory
    m_inventory.render();

    // Top-left compact control badge (neatly separated from HP bar, zero collision!)
    Renderer2D::drawRoundedRect(30.0f, 30.0f, 480.0f, 58.0f, 8.0f, Color(0.0f, 0.0f, 0.0f, 0.40f));
    Renderer2D::drawRoundedRectOutline(30.0f, 30.0f, 480.0f, 58.0f, 8.0f, Color(1.0f, 1.0f, 1.0f, 0.35f), 1.2f);
    Renderer2D::drawText("Maju: [W] / [Panah Atas]", 46.0f, 40.0f, 1.25f, Color::White(), false, true);
    Renderer2D::drawText("Mengendap: [Left Ctrl]  |  Tangan Kosong: [X]", 46.0f, 60.0f, 1.15f, Color(0.92f, 0.92f, 0.85f), false);
}


// ==========================================
// FASE 3: PENYERANGAN (BOSS FIGHT DEWA HUTAN)
// ==========================================

void Game::updatePhase3(float dt) {
    m_playerHpBar.update(dt);
    m_inventory.update(m_mouseX, m_mouseY, false, dt);

    if (m_isAimingBow) {
        m_bowDrawTime = std::min(1.0f, m_bowDrawTime + dt * 3.5f);
    } else {
        m_bowDrawTime = std::max(0.0f, m_bowDrawTime - dt * 5.0f);
    }

    // Periodic player HP drain every 5 seconds (demonstrating HP depletion UI animation without boss attack skills)
    m_bossSkillCooldown -= dt;
    if (m_bossSkillCooldown <= 0.0f) {
        m_bossSkillCooldown = 5.0f;
        m_playerHp = std::max(0.0f, m_playerHp - 15.0f);
        m_playerHpBar.setHp(m_playerHp);
        UIToast::show("Darah berkurang! (-15 HP)", 1.5f, Color(0.95f, 0.35f, 0.35f, 0.95f));

        if (m_playerHp <= 0.0f) {
            switchState(GameState::Defeat);
        }
    }
}

void Game::renderPhase3() {
    // 1. Forest Arena Floor & Pine backdrop
    Renderer2D::drawRect(0, 0, 1280, 720, Color(0.08f, 0.45f, 0.15f));

    // Pine trees background
    for (int i = 0; i < 18; ++i) {
        float tx = 30.0f + i * 74.0f;
        Renderer2D::drawPineTree(tx, 320.0f, 1.9f, Color::ForestGreen(), Color::DarkWood());
    }

    // Burning trees remnants in background
    Renderer2D::drawFlame(110.0f, 110.0f, 26.0f, m_gameTime);
    Renderer2D::drawFlame(310.0f, 90.0f, 28.0f, m_gameTime + 0.4f);
    Renderer2D::drawFlame(480.0f, 110.0f, 24.0f, m_gameTime + 0.8f);
    Renderer2D::drawFlame(950.0f, 240.0f, 30.0f, m_gameTime + 1.2f);

    // 2. Dewa Hutan (White Deer Boss) in center (matches Mockup 4)
    float bx = 640.0f;
    float by = 430.0f;

    // Boss Health Bar above head (Green bar with black border, matches Mockup 4)
    float bHpW = 120.0f;
    float bHpH = 14.0f;
    float bHpRatio = m_bossHp / 100.0f;
    Renderer2D::drawRect(bx - bHpW * 0.5f, by - 125.0f, bHpW, bHpH, Color(0.2f, 0.2f, 0.2f));
    Renderer2D::drawRect(bx - bHpW * 0.5f, by - 125.0f, bHpW * bHpRatio, bHpH, Color(0.18f, 0.82f, 0.22f));
    Renderer2D::drawRectOutline(bx - bHpW * 0.5f, by - 125.0f, bHpW, bHpH, Color::Black(), 2.0f);

    // Boss Antlers (Golden brown #7a5024)
    Color bossAntlerCol(0.50f, 0.32f, 0.16f);
    // Left antler curve
    for (int i = 0; i < 14; ++i) {
        float t = (float)i / 14.0f;
        float ax = bx - 14.0f - sinf(t * 2.4f) * 45.0f;
        float ay = by - 55.0f - t * 45.0f;
        Renderer2D::drawCircle(ax, ay, 6.0f - t * 3.0f, bossAntlerCol);
    }
    // Right antler curve
    for (int i = 0; i < 14; ++i) {
        float t = (float)i / 14.0f;
        float ax = bx + 14.0f + sinf(t * 2.4f) * 45.0f;
        float ay = by - 55.0f - t * 45.0f;
        Renderer2D::drawCircle(ax, ay, 6.0f - t * 3.0f, bossAntlerCol);
    }

    // Boss White Deer Body & Head (Pure White / Soft Ivory)
    Color deerWhite(0.97f, 0.98f, 1.0f);
    // Torso
    Renderer2D::drawRoundedRect(bx - 35.0f, by - 20.0f, 130.0f, 65.0f, 24.0f, deerWhite);
    // Neck
    Renderer2D::drawTriangle(bx - 30.0f, by - 10.0f, bx + 10.0f, by - 10.0f, bx - 10.0f, by - 55.0f, deerWhite);
    // Head
    Renderer2D::drawCircle(bx - 12.0f, by - 58.0f, 18.0f, deerWhite);
    // Snout
    Renderer2D::drawRoundedRect(bx - 20.0f, by - 55.0f, 16.0f, 18.0f, 6.0f, deerWhite);
    Renderer2D::drawCircle(bx - 12.0f, by - 40.0f, 3.5f, Color(0.3f, 0.2f, 0.1f)); // Nose
    // Legs
    Renderer2D::drawRect(bx - 25.0f, by + 35.0f, 8.0f, 70.0f, deerWhite); // Front Left
    Renderer2D::drawRect(bx - 10.0f, by + 35.0f, 8.0f, 68.0f, deerWhite); // Front Right
    Renderer2D::drawRect(bx + 65.0f, by + 35.0f, 8.0f, 65.0f, deerWhite); // Back Left
    Renderer2D::drawRect(bx + 80.0f, by + 35.0f, 8.0f, 62.0f, deerWhite); // Back Right

    // 3. First-Person Equipment View
    ItemType equipped = m_inventory.getSelectedItem();
    if (equipped == ItemType::Bow) {
        renderFirstPersonBow(m_bowDrawTime, m_arrowsCount > 0);
    } else if (equipped == ItemType::Torch) {
        renderFirstPersonTorch(0.0f, m_gameTime);
    } else {
        renderFirstPersonHands(0.0f, false);
    }

    // 4. Crosshair (Appears when holding Left Mouse Button to draw bow, permanently centered)
    if (m_isAimingBow) {
        float cx = 640.0f;
        float cy = 360.0f;
        float chSize = 24.0f + (1.0f - m_bowDrawTime) * 16.0f; // Crosshair tightens as pulled!

        // Outer reticle circle
        Renderer2D::drawCircleOutline(cx, cy, chSize, Color::Red(), 2.0f);
        // Cross lines
        Renderer2D::drawLine(cx - chSize * 1.4f, cy, cx - chSize * 0.4f, cy, Color::Red(), 2.0f);
        Renderer2D::drawLine(cx + chSize * 0.4f, cy, cx + chSize * 1.4f, cy, Color::Red(), 2.0f);
        Renderer2D::drawLine(cx, cy - chSize * 1.4f, cx, cy - chSize * 0.4f, Color::Red(), 2.0f);
        Renderer2D::drawLine(cx, cy + chSize * 0.4f, cx, cy + chSize * 1.4f, Color::Red(), 2.0f);
        // Center pip
        Renderer2D::drawCircle(cx, cy, 2.5f, Color::Red());
    }

    // 5. HUD
    // Bottom-left: "Arrow : 34" ammo counter (matches Mockup 4)
    std::string arrowStr = "Arrow : " + std::to_string(m_arrowsCount);
    Renderer2D::drawText(arrowStr, 40.0f, 650.0f, 2.8f, Color::White(), false, true);

    // Top-right: Player HP Bar
    m_playerHpBar.render();

    // Bottom-center: Inventory
    m_inventory.render();
}


// ==========================================
// VICTORY & DEFEAT (POLOS / WIREFRAME)
// ==========================================

void Game::updateVictory(float dt) {
    m_btnRestart.update(m_mouseX, m_mouseY, m_mouseDown, dt);
    m_btnMenuVictory.update(m_mouseX, m_mouseY, m_mouseDown, dt);

    if (m_btnRestart.isClicked(m_mouseX, m_mouseY, m_mouseReleased)) {
        switchState(GameState::Phase1_Burn);
    }
    if (m_btnMenuVictory.isClicked(m_mouseX, m_mouseY, m_mouseReleased)) {
        switchState(GameState::MainMenu);
    }
}

void Game::renderVictory() {
    // Dark victory screen
    Renderer2D::drawRect(0, 0, 1280, 720, Color(0.08f, 0.16f, 0.12f, 0.95f));

    // Wireframe panel box
    Renderer2D::drawRoundedRect(340.0f, 150.0f, 600.0f, 440.0f, 16.0f, Color(0.12f, 0.24f, 0.18f));
    Renderer2D::drawRoundedRectOutline(340.0f, 150.0f, 600.0f, 440.0f, 16.0f, Color::Green(), 2.5f);

    Renderer2D::drawText("VICTORY!", 640.0f, 200.0f, 3.5f, Color::Green(), true, true);
    Renderer2D::drawText("Dewa Hutan Berhasil Dikalahkan!", 640.0f, 260.0f, 2.0f, Color::White(), true);
    Renderer2D::drawText("[Tampilan Polos / Wireframe Low Fidelity]", 640.0f, 340.0f, 1.4f, Color(0.7f, 0.8f, 0.7f), true);

    m_btnRestart.render();
    m_btnMenuVictory.render();
}

void Game::updateDefeat(float dt) {
    m_btnRestart.update(m_mouseX, m_mouseY, m_mouseDown, dt);
    m_btnMenuDefeat.update(m_mouseX, m_mouseY, m_mouseDown, dt);

    if (m_btnRestart.isClicked(m_mouseX, m_mouseY, m_mouseReleased)) {
        switchState(GameState::Phase1_Burn);
    }
    if (m_btnMenuDefeat.isClicked(m_mouseX, m_mouseY, m_mouseReleased)) {
        switchState(GameState::MainMenu);
    }
}

void Game::renderDefeat() {
    // Dark defeat screen
    Renderer2D::drawRect(0, 0, 1280, 720, Color(0.20f, 0.08f, 0.08f, 0.95f));

    // Wireframe panel box
    Renderer2D::drawRoundedRect(340.0f, 150.0f, 600.0f, 440.0f, 16.0f, Color(0.28f, 0.12f, 0.12f));
    Renderer2D::drawRoundedRectOutline(340.0f, 150.0f, 600.0f, 440.0f, 16.0f, Color::Red(), 2.5f);

    Renderer2D::drawText("DEFEAT!", 640.0f, 200.0f, 3.5f, Color::Red(), true, true);
    Renderer2D::drawText("Anda Telah Gugur di Dalam Hutan...", 640.0f, 260.0f, 2.0f, Color::White(), true);
    Renderer2D::drawText("[Tampilan Polos / Wireframe Low Fidelity]", 640.0f, 340.0f, 1.4f, Color(0.8f, 0.7f, 0.7f), true);

    m_btnRestart.render();
    m_btnMenuDefeat.render();
}

// ==========================================
// RETURN NAVIGATION CONFIRMATION MODAL
// ==========================================

void Game::updateConfirmDialog(float dt) {
    // Style Yes button according to confirmation context
    if (m_confirmType == ConfirmDialogType::ExitGame) {
        m_btnConfirmYes.baseColor = Color(0.80f, 0.22f, 0.22f);
        m_btnConfirmYes.hoverColor = Color(0.92f, 0.32f, 0.32f);
        m_btnConfirmYes.pressColor = Color(0.65f, 0.16f, 0.16f);
        m_btnConfirmYes.textColor = Color::White();
    } else {
        m_btnConfirmYes.baseColor = Color::GoldButton();
        m_btnConfirmYes.hoverColor = Color::GoldHover();
        m_btnConfirmYes.pressColor = Color::GoldActive();
        m_btnConfirmYes.textColor = Color::Black();
    }

    m_btnConfirmYes.update(m_mouseX, m_mouseY, m_mouseDown, dt);
    m_btnConfirmCancel.update(m_mouseX, m_mouseY, m_mouseDown, dt);

    if (m_btnConfirmYes.isClicked(m_mouseX, m_mouseY, m_mouseReleased)) {
        if (m_confirmType == ConfirmDialogType::ExitGame) {
            glfwSetWindowShouldClose(m_window, GLFW_TRUE);
        } else {
            m_showConfirmDialog = false;
            m_confirmType = ConfirmDialogType::None;
            switchState(GameState::MainMenu);
        }
    } else if (m_btnConfirmCancel.isClicked(m_mouseX, m_mouseY, m_mouseReleased)) {
        m_showConfirmDialog = false;
        m_confirmType = ConfirmDialogType::None;
    }
}

void Game::renderConfirmDialog() {
    // 1. Semi-transparent black backdrop overlay covering whole screen
    Renderer2D::drawRect(0.0f, 0.0f, 1280.0f, 720.0f, Color(0.0f, 0.0f, 0.0f, 0.65f));

    // 2. Dialog Panel Box (Centered at 640, 360)
    float pw = 540.0f;
    float ph = 270.0f;
    float px = 640.0f - pw * 0.5f; // 370.0f
    float py = 360.0f - ph * 0.5f; // 225.0f

    // Drop shadow
    Renderer2D::drawRoundedRect(px + 4.0f, py + 6.0f, pw, ph, 16.0f, Color(0.0f, 0.0f, 0.0f, 0.45f));

    // Solid Dialog Body
    Color panelBg = (m_confirmType == ConfirmDialogType::ExitGame) 
                    ? Color(0.18f, 0.12f, 0.12f, 0.98f) 
                    : Color(0.13f, 0.17f, 0.22f, 0.98f);
    Renderer2D::drawRoundedRect(px, py, pw, ph, 16.0f, panelBg);

    // Border outline
    Color borderCol = (m_confirmType == ConfirmDialogType::ExitGame) 
                      ? Color(0.85f, 0.35f, 0.35f, 0.90f) 
                      : Color(0.85f, 0.70f, 0.20f, 0.90f);
    Renderer2D::drawRoundedRectOutline(px, py, pw, ph, 16.0f, borderCol, 2.5f);

    // 3. Header Title, Divider, and Message
    if (m_confirmType == ConfirmDialogType::ExitGame) {
        Renderer2D::drawText("KONFIRMASI KELUAR", 640.0f, py + 26.0f, 2.3f, Color(1.0f, 0.45f, 0.45f), true, true);
        Renderer2D::drawLine(px + 40.0f, py + 62.0f, px + pw - 40.0f, py + 62.0f, Color(0.55f, 0.30f, 0.30f), 1.5f);
        Renderer2D::drawText("Keluar dari permainan?", 640.0f, py + 92.0f, 1.9f, Color::White(), true);
        Renderer2D::drawText("Tekan [ESC] lagi untuk membatalkan", 640.0f, py + 128.0f, 1.3f, Color(0.75f, 0.75f, 0.75f), true);
    } else {
        Renderer2D::drawText("KEMBALI KE MENU UTAMA", 640.0f, py + 26.0f, 2.3f, Color::GoldHover(), true, true);
        Renderer2D::drawLine(px + 40.0f, py + 62.0f, px + pw - 40.0f, py + 62.0f, Color(0.55f, 0.45f, 0.20f), 1.5f);
        Renderer2D::drawText("Yakin ingin kembali ke menu utama?", 640.0f, py + 92.0f, 1.9f, Color::White(), true);
        Renderer2D::drawText("Tekan [ESC] lagi untuk membatalkan", 640.0f, py + 128.0f, 1.3f, Color(0.75f, 0.75f, 0.75f), true);
    }

    // 4. Buttons (Yes and Cancel)
    m_btnConfirmYes.render();
    m_btnConfirmCancel.render();
}

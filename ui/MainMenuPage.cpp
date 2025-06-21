//
// Created by Matti Kjellstadli on 21/06/2025.
//

#include "MainMenuPage.h"
#include "../FileUtils.h"

MainMenuPage::MainMenuPage(std::function<void()> onStart)
    : onStartCallback(std::move(onStart)) {
    float sidebarWidth = GetScreenWidth() * 0.24f;
    float sidebarHeight = GetScreenHeight();
    float btnX = 30;
    float btnWidth = sidebarWidth - 60;
    float textFontSize = 40;
    const char* logoText = "Sonique";
    Vector2 textSize = MeasureTextEx(font, logoText, textFontSize, 0);
    float startBtnHeight = 0.075f * sidebarHeight;
    float groupY = 0.06f * sidebarHeight;
    float logoHeight = logoTexture.height * (80.0f / logoTexture.width);
    float textY = groupY + (std::max(textSize.y, logoHeight) - textSize.y) / 2;
    float startBtnY = textY + textSize.y + 0.05f * sidebarHeight;
    startBtn = {btnX, startBtnY, btnWidth, startBtnHeight};
    font = LoadFontEx(GetResourcePath("Lexend.ttf").c_str(), 128, nullptr, 0);
    Image logoImg = LoadImage(GetResourcePath("assets/logo.png").c_str());
    logoTexture = LoadTextureFromImage(logoImg);
    UnloadImage(logoImg);
}

MainMenuPage::~MainMenuPage() {
    UnloadFont(font);
    Texture2D logoTexture;
}

// The "Start" button is now above the dummy buttons, "Settings" at the bottom
void MainMenuPage::Draw() {
    BeginDrawing();
    ClearBackground((Color){221, 255, 250, 255});

    float sidebarWidth = GetScreenWidth() * 0.24f;
    float sidebarHeight = GetScreenHeight();
    DrawRectangle(0, 0, sidebarWidth, sidebarHeight, (Color){240, 250, 247, 255});

    // Logo and logotext side by side at top of sidebar
    float logoWidth = 80.0f;
    float scale = logoWidth / logoTexture.width;
    float logoHeight = logoTexture.height * scale;
    const char* logoText = "Sonique";
    float textFontSize = 40;
    Vector2 textSize = MeasureTextEx(font, logoText, textFontSize, 0);
    float spacing = 16.0f;
    float totalWidth = logoWidth + spacing + textSize.x;
    float groupX = sidebarWidth / 2 - totalWidth / 2;
    float groupY = 0.06f * sidebarHeight;
    float logoY = groupY + (std::max(textSize.y, logoHeight) - logoHeight) / 2;
    float textY = groupY + (std::max(textSize.y, logoHeight) - textSize.y) / 2;
    DrawTextureEx(logoTexture, {groupX, logoY}, 0, scale, WHITE);
    DrawTextEx(font, logoText, {groupX + logoWidth + spacing, textY}, textFontSize, 0, (Color){30, 30, 60, 255});

    // "Start" button just below logo/text
    float btnX = 30;
    float btnWidth = sidebarWidth - 60;
    float startBtnHeight = 0.075f * sidebarHeight;
    float startBtnY = textY + textSize.y + 0.05f * sidebarHeight;
    Rectangle startBtnSidebar = {btnX, startBtnY, btnWidth, startBtnHeight};
    DrawRectangleRounded(startBtnSidebar, 0.3f, 8, DARKGRAY);
    DrawTextEx(font, "Start", {startBtnSidebar.x + 50, startBtnSidebar.y + 15}, 32, 0, WHITE);

    // Dummy menu buttons below "Start"
    float btnY = startBtnY + startBtnHeight + 0.03f * sidebarHeight;
    float btnHeight = 0.06f * sidebarHeight;
    float btnSpacing = 0.025f * sidebarHeight;
    for (int i = 0; i < 3; ++i) {
        Rectangle btn = {btnX, btnY + i * (btnHeight + btnSpacing), btnWidth, btnHeight};
        DrawRectangleRounded(btn, 0.3f, 8, (Color){200, 220, 230, 255});
        DrawTextEx(font, ("Button " + std::to_string(i + 1)).c_str(),
                   {btn.x + 24, btn.y + 10}, 28, 0, (Color){40, 60, 90, 255});
    }

    // "Settings" button at the bottom
    float settingsBtnHeight = 0.075f * sidebarHeight;
    Rectangle settingsBtn = {btnX, sidebarHeight - settingsBtnHeight - 30, btnWidth, settingsBtnHeight};
    DrawRectangleRounded(settingsBtn, 0.3f, 8, (Color){180, 200, 210, 255});
    DrawTextEx(font, "Settings", {settingsBtn.x + 32, settingsBtn.y + 15}, 32, 0, (Color){40, 60, 90, 255});

    // Version text above the settings button
    std::string versionText = "Version 0.1.0 Preview";
    Vector2 versionSize = MeasureTextEx(font, versionText.c_str(), 20, 0);
    float versionX = sidebarWidth / 2 - versionSize.x / 2;
    float versionY = settingsBtn.y - versionSize.y - 10;
    DrawTextEx(font, versionText.c_str(), {versionX, versionY}, 20, 0, (Color){100, 100, 100, 255});

    EndDrawing();
}

void MainMenuPage::HandleInput() {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, startBtn)) {
            if (onStartCallback) onStartCallback();
        }
    }
}

void MainMenuPage::Update() {}
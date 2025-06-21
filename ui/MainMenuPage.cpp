//
// Created by Matti Kjellstadli on 21/06/2025.
//

#include "MainMenuPage.h"
#include "../utils/FileUtils.h"

MainMenuPage::MainMenuPage(std::function<void()> onStart)
    : onStartCallback(std::move(onStart)) {
    float sidebarWidth = GetScreenWidth() * 0.24f;
    float sidebarHeight = GetScreenHeight();
    float btnX = 30;
    float btnWidth = sidebarWidth - 60;
    float textFontSize = 40;
    const char *logoText = "Sonique";
    Vector2 textSize = MeasureTextEx(font, logoText, textFontSize, 0);
    float startBtnHeight = 0.075f * sidebarHeight;
    float groupY = 0.06f * sidebarHeight;
    float logoHeight = logoTexture.height * (80.0f / logoTexture.width);
    float textY = groupY + (std::max(textSize.y, logoHeight) - textSize.y) / 2;
    float startBtnY = textY + textSize.y + 0.05f * sidebarHeight;
    startBtn = {btnX, startBtnY, btnWidth, startBtnHeight};
    font = LoadFontEx(GetResourcePath("Lexend.ttf").c_str(), 128, nullptr, 0);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
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
    ClearBackground((Color){243, 243, 243, 255});
    float margin = 16.0f;
    float borderThickness = 1.0f;
    Color borderColor = (Color){220, 225, 225, 255};
    int borderSegments = 12;

    // Sidebar (left panel)
    float sidebarOuterWidth = GetScreenWidth() * 0.24f;
    float sidebarWidth = sidebarOuterWidth - 2 * margin;
    float sidebarHeight = GetScreenHeight() - 2 * margin;
    float sidebarX = margin;
    float sidebarY = margin;
    float sidebarCornerRadius = 50.0f;
    float sidebarRoundness = sidebarCornerRadius / fminf(sidebarWidth, sidebarHeight);

    // Draw sidebar border
    DrawRectangleRounded(
        (Rectangle){sidebarX, sidebarY, sidebarWidth, sidebarHeight},
        sidebarRoundness, borderSegments, borderColor
    );
    // Draw sidebar inner white box
    DrawRectangleRounded(
        (Rectangle){
            sidebarX + borderThickness,
            sidebarY + borderThickness,
            sidebarWidth - 2 * borderThickness,
            sidebarHeight - 2 * borderThickness
        },
        sidebarRoundness, borderSegments, WHITE
    );

    // Right panel (less rounded, no extra margin)
    float rightPanelX = sidebarOuterWidth;
    float rightPanelY = margin;
    float rightPanelWidth = GetScreenWidth() - sidebarOuterWidth - margin;
    float rightPanelHeight = GetScreenHeight() - 2 * margin;
    float rightPanelCornerRadius = 50.0f;
    float rightPanelRoundness = rightPanelCornerRadius / fminf(rightPanelWidth, rightPanelHeight);

    // Draw right panel border
    DrawRectangleRounded(
        (Rectangle){rightPanelX, rightPanelY, rightPanelWidth, rightPanelHeight},
        rightPanelRoundness, borderSegments, borderColor
    );
    // Draw right panel inner white box
    DrawRectangleRounded(
        (Rectangle){
            rightPanelX + borderThickness,
            rightPanelY + borderThickness,
            rightPanelWidth - 2 * borderThickness,
            rightPanelHeight - 2 * borderThickness
        },
        rightPanelRoundness, borderSegments, WHITE
    );

    // Logo and logotext side by side at top of sidebar
    float logoWidth = 50.0f;
    float scale = logoWidth / logoTexture.width;
    float logoHeight = logoTexture.height * scale;
    const char *logoText = "Sonique";
    float textFontSize = 30;
    Vector2 textSize = MeasureTextEx(font, logoText, textFontSize, 0);
    float spacing = 16.0f;
    float totalWidth = logoWidth + spacing + textSize.x;
    float groupX = sidebarX + sidebarWidth / 2 - totalWidth / 2;
    float groupY = 0.06f * sidebarHeight;
    float logoY = groupY + (std::max(textSize.y, logoHeight) - logoHeight) / 2;
    float textY = groupY + (std::max(textSize.y, logoHeight) - textSize.y) / 2;
    DrawTextureEx(logoTexture, {groupX, logoY}, 0, scale, WHITE);
    DrawTextEx(font, logoText, {groupX + logoWidth + spacing, textY}, textFontSize, 0, (Color){70, 85, 83, 255});

    // "Start" button just below logo/text
    float btnX = sidebarX + 20;
    float btnWidth = sidebarWidth - 40;
    float startBtnHeight = 0.055f * sidebarHeight;
    float startBtnY = textY + textSize.y + 0.05f * sidebarHeight;
    float buttonRoundness = 0.5f;
    Rectangle startBtnSidebar = {btnX, startBtnY, btnWidth, startBtnHeight + 15.0f};
    DrawRectangleRounded(startBtnSidebar, buttonRoundness, 8, (Color){52, 120, 246, 255});
    DrawTextEx(font, "Select A Song", {startBtnSidebar.x + 55, startBtnSidebar.y + 18}, 20, 0, WHITE);
    // Menu buttons below "Start"
    float btnY = startBtnY + startBtnHeight + 0.03f * sidebarHeight;
    float btnHeight = 0.06f * sidebarHeight;
    float btnSpacing = 0.025f * sidebarHeight;

    // Free Play button
    Rectangle freePlayBtn = {btnX, btnY, btnWidth, btnHeight};
    DrawRectangleRounded(freePlayBtn, buttonRoundness, 8, (Color){200, 220, 230, 255});
    DrawTextEx(font, "Free Play", {freePlayBtn.x + 70, freePlayBtn.y + 12}, 20, 0, (Color){40, 60, 90, 255});

    // Credits button
    Rectangle creditsBtn = {btnX, btnY + btnHeight + btnSpacing, btnWidth, btnHeight};
    DrawRectangleRounded(creditsBtn, buttonRoundness, 8, (Color){200, 220, 230, 255});
    DrawTextEx(font, "Credits", {creditsBtn.x + 80, creditsBtn.y + 12}, 20, 0, (Color){40, 60, 90, 255});

    // GitHub button
    Rectangle githubBtn = {btnX, btnY + 2 * (btnHeight + btnSpacing), btnWidth, btnHeight};
    DrawRectangleRounded(githubBtn, buttonRoundness, 8, (Color){200, 220, 230, 255});
    DrawTextEx(font, "See project on GitHub", {githubBtn.x + 30, githubBtn.y + 12}, 20, 0, (Color){40, 60, 90, 255});

    // "Settings" button at the bottom
    float settingsBtnHeight = 0.055f * sidebarHeight;
    Rectangle settingsBtn = {btnX, sidebarY + sidebarHeight - settingsBtnHeight - 30, btnWidth, settingsBtnHeight};
    DrawRectangleRounded(settingsBtn, buttonRoundness, 8, (Color){180, 200, 210, 255});
    DrawTextEx(font, "Settings", {settingsBtn.x + 80, settingsBtn.y + 12}, 20, 0, (Color){40, 60, 90, 255});

    // Version text above the settings button
    std::string versionText = "Version 0.1.0 Preview";
    Vector2 versionSize = MeasureTextEx(font, versionText.c_str(), 20, 0);
    float versionX = sidebarX + sidebarWidth / 2 - versionSize.x / 2;
    float versionY = settingsBtn.y - versionSize.y - 10;
    DrawTextEx(font, versionText.c_str(), {versionX, versionY}, 20, 0, (Color){100, 100, 100, 255});

    EndDrawing();
}

void MainMenuPage::HandleInput() {
    // Add this near the top of MainMenuPage.cpp
#define GITHUB_URL "https://github.com/mattkje/sonique/tree/master"

    // In HandleInput()
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, startBtn)) {
            if (onStartCallback) onStartCallback();
        }
        // GitHub button logic
        float sidebarWidth = GetScreenWidth() * 0.24f;
        float sidebarHeight = GetScreenHeight();
        float btnX = 30;
        float btnWidth = sidebarWidth - 60;
        float startBtnHeight = 0.075f * sidebarHeight;
        float textFontSize = 50;
        Vector2 textSize = MeasureTextEx(font, "Sonique", textFontSize, 0);
        float groupY = 0.06f * sidebarHeight;
        float textY = groupY + (std::max(textSize.y, logoTexture.height * (60.0f / logoTexture.width)) - textSize.y) /
                      2;
        float startBtnY = textY + textSize.y + 0.05f * sidebarHeight;
        float btnY = startBtnY + startBtnHeight + 0.03f * sidebarHeight;
        float btnHeight = 0.06f * sidebarHeight;
        float btnSpacing = 0.025f * sidebarHeight;
        Rectangle githubBtn = {btnX, btnY + 2 * (btnHeight + btnSpacing), btnWidth, btnHeight};
        if (CheckCollisionPointRec(mouse, githubBtn)) {
            OpenURL(GITHUB_URL);
        }
    }
}

void MainMenuPage::Update() {
}

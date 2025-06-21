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

// Add this helper at the top of the file (or in an anonymous namespace)
void DrawSidebarButton(Rectangle btn, const char *text, Font font, float fontSize,
                       Color normal, Color hover, Color pressed, Color textColor, bool *outHovered = nullptr) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, btn);
    bool pressedState = hovered && IsMouseButtonDown(MOUSE_LEFT_BUTTON);

    if (outHovered) *outHovered = hovered;

    Color btnColor = normal;
    float scale = 1.0f;
    if (pressedState) {
        btnColor = pressed;
        scale = 0.97f;
    } else if (hovered) {
        btnColor = hover;
        scale = 1.04f;
    }

    Rectangle scaledBtn = {
        btn.x + (btn.width - btn.width * scale) / 2,
        btn.y + (btn.height - btn.height * scale) / 2,
        btn.width * scale,
        btn.height * scale
    };

    DrawRectangleRounded(scaledBtn, 0.5f, 8, btnColor);

    Vector2 textSize = MeasureTextEx(font, text, fontSize, 0);
    DrawTextEx(
        font,
        text,
        {
            scaledBtn.x + (scaledBtn.width - textSize.x) / 2,
            scaledBtn.y + (scaledBtn.height - textSize.y) / 2
        },
        fontSize, 0, textColor
    );
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

    // Menu buttons below "Start"
    float btnY = startBtnY + startBtnHeight + 0.03f * sidebarHeight;
    float btnHeight = 0.05f * sidebarHeight;
    float btnSpacing = 0.015f * sidebarHeight;

    // Free Play button
    Rectangle freePlayBtn = {btnX, btnY, btnWidth, btnHeight};

    // Credits button
    Rectangle gitHubBtn = {btnX, btnY + btnHeight + btnSpacing, btnWidth, btnHeight};

    // GitHub button
    Rectangle exitBtn = {btnX, btnY + 2 * (btnHeight + btnSpacing), btnWidth, btnHeight};

    // "Settings" button at the bottom
    float settingsBtnHeight = 0.055f * sidebarHeight;
    Rectangle settingsBtn = {btnX, sidebarY + sidebarHeight - settingsBtnHeight - 30, btnWidth, settingsBtnHeight};

    // Draw all buttons with hover/click effects
    DrawSidebarButton(startBtnSidebar, "Select A Song", font, 23,
                      (Color){124, 58, 237, 255}, // normal
                      (Color){140, 80, 255, 255}, // hover
                      (Color){100, 40, 200, 255}, // pressed
                      WHITE);

    DrawSidebarButton(freePlayBtn, "Free Play", font, 20,
                      (Color){220, 230, 245, 255}, // normal
                      (Color){200, 215, 240, 255}, // hover
                      (Color){180, 200, 225, 255}, // pressed
                      (Color){40, 60, 90, 255});

    DrawSidebarButton(gitHubBtn, "See project on GitHub", font, 20,
                      (Color){220, 230, 245, 255}, // normal
                      (Color){200, 215, 240, 255}, // hover
                      (Color){180, 200, 225, 255}, // pressed
                      (Color){40, 60, 90, 255});

    DrawSidebarButton(exitBtn, "Exit", font, 20,
                      (Color){255, 99, 99, 255},   // normal (#FF6363)
                      (Color){255, 120, 120, 255}, // hover (lighter red)
                      (Color){220, 60, 60, 255},   // pressed (darker red)
                      (Color){255, 255, 255, 255}  // white text
    );

    DrawSidebarButton(settingsBtn, "Settings", font, 20,
                      (Color){220, 230, 245, 255}, // normal
                      (Color){200, 215, 240, 255}, // hover
                      (Color){180, 200, 225, 255}, // pressed
                      (Color){40, 60, 90, 255});
    // Version text above the settings button
    std::string versionText = "Version 0.1.0 Preview";
    Vector2 versionSize = MeasureTextEx(font, versionText.c_str(), 20, 0);
    float versionX = sidebarX + sidebarWidth / 2 - versionSize.x / 2;
    float versionY = settingsBtn.y - versionSize.y - 10;
    DrawTextEx(font, versionText.c_str(), {versionX, versionY}, 20, 0, (Color){100, 100, 100, 255});
    EndDrawing();
}

void MainMenuPage::HandleInput() {
#define GITHUB_URL "https://github.com/mattkje/sonique/tree/master"

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = GetMousePosition();

        // Recompute sidebar and button rects exactly as in Draw()
        float sidebarWidth = GetScreenWidth() * 0.24f;
        float sidebarHeight = GetScreenHeight();
        float margin = 16.0f;
        float sidebarX = margin;
        float sidebarY = margin;
        float sidebarInnerWidth = sidebarWidth - 2 * margin;
        float sidebarInnerHeight = sidebarHeight - 2 * margin;

        float btnX = sidebarX + 20;
        float btnWidth = sidebarWidth - 40;
        float textFontSize = 30;
        const char *logoText = "Sonique";
        Vector2 textSize = MeasureTextEx(font, logoText, textFontSize, 0);
        float logoWidth = 50.0f;
        float logoHeight = logoTexture.height * (logoWidth / logoTexture.width);
        float groupY = 0.06f * (sidebarHeight - 2 * margin);
        float textY = groupY + (std::max(textSize.y, logoHeight) - textSize.y) / 2;
        float startBtnHeight = 0.055f * (sidebarHeight - 2 * margin);
        float startBtnY = textY + textSize.y + 0.05f * (sidebarHeight - 2 * margin);
        Rectangle startBtnSidebar = {btnX, startBtnY, btnWidth, startBtnHeight + 15.0f};

        float btnY = startBtnY + startBtnHeight + 0.03f * (sidebarHeight - 2 * margin);
        float btnHeight = 0.06f * (sidebarHeight - 2 * margin);
        float btnSpacing = 0.015f * (sidebarHeight - 2 * margin);

        Rectangle freePlayBtn = {btnX, btnY, btnWidth, btnHeight};
        Rectangle githubBtn = {btnX, btnY + btnHeight + btnSpacing, btnWidth, btnHeight};
        Rectangle exitBtn = {btnX, btnY + 2 * (btnHeight + btnSpacing), btnWidth, btnHeight};
        float settingsBtnHeight = 0.055f * (sidebarHeight - 2 * margin);
        Rectangle settingsBtn = {btnX, sidebarY + sidebarHeight - settingsBtnHeight - 30, btnWidth, settingsBtnHeight};

        // Handle clicks
        if (CheckCollisionPointRec(mouse, startBtnSidebar)) {
            if (onStartCallback) onStartCallback();
        } else if (CheckCollisionPointRec(mouse, freePlayBtn)) {
            // TODO: Handle Free Play button click
        } else if (CheckCollisionPointRec(mouse, githubBtn)) {
            OpenURL(GITHUB_URL);
        } else if (CheckCollisionPointRec(mouse, exitBtn)) {
            // Close app
            CloseWindow();
        } else if (CheckCollisionPointRec(mouse, settingsBtn)) {
            // TODO: Handle Settings button click
        }
    }
}

void MainMenuPage::Update() {
}

#pragma once
#include "raylib.h"
#include <functional>


class MainMenuPage {
public:
    explicit MainMenuPage(std::function<void()> onStart);
    ~MainMenuPage();
    void Draw();
    void HandleInput();
    void Update();
private:
    std::function<void()> onStartCallback;
    Rectangle startBtn;
    Font font;
    Texture2D logoTexture;
};
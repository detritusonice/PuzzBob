#ifndef TITLE_H
#define TITLE_H

class TextureManager;
class BaseButton;
class BaseTexture;

class Title: public GameState
{
    enum TITLE_BUTTONS
    {
        ID_BTN_PLAY=0,
        ID_BTN_MENU,
        ID_BTN_ABOUT,
        ID_BTN_EXIT,
        ID_BTN_PROGRESS,
        ID_BTN_PROFILE,
        NUM_TITLE_BUTTONS
    };
    enum TITLE_HANDLES
    {
        HANDLE_LOGO=0,
        HANDLE_BUTTONS,
        HANDLE_PROFILE_BACKGROUND,
        NUM_HANDLES
    };

    int handles[NUM_HANDLES];

    BaseTexture* texVersion;

    BaseTexture* texWelcome;
    BaseTexture* texProfileHelp;

    BaseButton* titleButtons[NUM_TITLE_BUTTONS];

    SDL_Rect rcWelcome;
    SDL_Rect rcProfileHelp;

    void initButtons();
    void unloadImages();
    void loadImages();

  public:

    Title();
    ~Title();

    void Logic(float dt, float phase=0.);
    void Render(float phase=0.);
    void HandleEvents();
};

#endif // TITLE_H

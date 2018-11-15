#ifndef GAME_MENU_H
#define GAME_MENU_H

class GlassButton;
class SimpleButton;

enum INITGAME_ACTIONS
{
    ACTION_LOAD_SAVED=0,
    ACTION_PLAYLEVEL,
    ACTION_SELECTLEVEL
};


class GameMenu
{
    enum GAME_MENU_BUTTONS
    {
        ID_NEWGAME=0,
        ID_CONTINUE,
        ID_MAXLEVEL,
        ID_SELECT,
        NUM_GAMEMENU_BUTTONS
    };
/*
    enum LOWBUTTONS
    {
        ID_CANCEL=0,
        ID_OK
    };
*/
    TextureButton* buttons[NUM_GAMEMENU_BUTTONS];
 //   SimpleButton* lowButtons[2];
    int action, level;
    bool done;

    BaseTexture* texMessage;

    void initButtons();

public:
    GameMenu( );
    ~GameMenu();

    void handleEvents(SDL_Event& event);
    void render(float phase=0.);
    void logic(float dt, float phase=0.);

    bool isDone(int *action, int *level);
};


#endif // GAME_MENU_H

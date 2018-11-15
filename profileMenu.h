#ifndef PROFILE_MENU_H
#define PROFILE_MENU_H

#include <string>
#include <vector>

class SimpleButton;
class BaseTexture;
class ProfileEditor;
class BaseButton;

#include "scrollPage.h"

class ProfileMenu: public GameState
{
    enum PROFILE_MENU_BUTTONS
    {
        ID_BTN_CANCEL,
        ID_BTN_DELETE,
        ID_BTN_NEW,
        ID_BTN_SELECT,
        ID_BTN_CANCEL_DELETE,
        ID_BTN_CONFIRM_DELETE,
        ID_BTN_DENIAL_RETURN,
        NUM_PROFILE_MENU_BUTTONS
    };
    enum PROFILE_MENU_STATES
    {
        PROFILE_BROWSER=0,
        PROFILE_EDITOR,
        PROFILE_WARNING,
        PROFILE_DENIAL,
        PROFILE_DUPLICATE_DENIAL
    };

    BaseTexture* texMsgBackground;
    BaseTexture* texButtons[2];
    BaseTexture* texMessages[3];

    PageHandler* pageHandler;
    ProfileEditor* profileEditor;

    int mode;

    bool skinChanged;

    TextureButton* profileButtons[NUM_PROFILE_MENU_BUTTONS];

    std::vector<std::pair< std::string,int> > profileVector;

    void initButtons();
    void loadProfiles();
    void writeProfiles();
    void populatePages();
    void addProfile();
    void deleteProfile();

    void goBack();

    void browserHandleEvents(SDL_Event& event);
    void editorHandleEvents(SDL_Event& event);
    void warningHandleEvents(SDL_Event& event);
    void denialHandleEvents(SDL_Event& event);

    void browserLogic(float dt,float phase);
    void editorLogic(float dt,float phase);
    void warningLogic(float dt,float phase);
    void denialLogic(float dt,float phase);

    void browserRender(float phase);
    void editorRender(float phase);
    void warningRender(float phase);
    void denialRender(float phase);

    void createWarningMessages();
    void createDenialMessages();

  public:

    ProfileMenu();
    ~ProfileMenu();

    void Logic(float dt, float phase=0.);
    void Render(float phase=0.);
    void HandleEvents();

    void setProfile();

};
//=====================================================

class ProfileInfoPage: public ScrollPage
{
    LevelStats* profileStats;

   // BaseTexture* texButton;
    BaseTexture* texMsgBackground;
    std::vector< BaseTexture* > texInfoVector;
    std::vector< SDL_Rect > recsVector;


    void initControls();
    void loadImages();
    void setMessageRects();

public:
    ProfileInfoPage(std::string titleStr, const FPoint& pos, const SDL_Rect& rect);
    ~ProfileInfoPage();

    void render(float alphaModifier =1.);
    bool handleEvents();

};
//=======================================================

class ProfileEditor
{
    enum EDITOR_KEYS
    {
        EDKEY_1=0,EDKEY_2,EDKEY_3,EDKEY_4,EDKEY_5,EDKEY_6,EDKEY_7,EDKEY_8,EDKEY_9,EDKEY_0,
        EDKEY_Q,EDKEY_W,EDKEY_E,EDKEY_R,EDKEY_T,EDKEY_Y,EDKEY_U,EDKEY_I,EDKEY_O,EDKEY_P,
        EDKEY_A,EDKEY_S,EDKEY_D,EDKEY_F,EDKEY_G,EDKEY_H,EDKEY_J,EDKEY_K,EDKEY_L,
        EDKEY_Z,EDKEY_X,EDKEY_C,EDKEY_V,EDKEY_B,EDKEY_N,EDKEY_M,
        EDKEY_BACKSPACE, EDKEY_CANCEL, EDKEY_SPACE, EDKEY_ENTER,
        NUM_EDITOR_KEYS
    };
    const char* keyValues[36]={"1","2","3","4","5","6","7","8","9","0",
                        "Q","W","E","R","T","Y","U","I","O","P",
                        "A","S","D","F","G","H","J","K","L",
                        "Z","X","C","V","B","N","M"};

    SDL_Rect rc;
    std::string name;

    std::vector<BaseButton*> keyVector;

    BaseTexture* texButtons[4];
    BaseTexture* texMsgBackground;
    BaseTexture* texName;

    bool done ;
    bool cancelled;
    void initControls();
    void loadImages();
    void setNameImage();


public:
    bool handleEvents(SDL_Event& event);
    void logic(float dt, float phase);
    void render(float phase,float alphaModifier);

    bool isDone();
    bool isCancelled();
    void notDoneYet();

    std::string getName();

    ProfileEditor(const SDL_Rect& rc);
    ~ProfileEditor();
};

#endif // PROFILE_MENU_H

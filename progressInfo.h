
#ifndef PROGRESS_INFO_H
#define PROGRESS_INFO_H

class SimpleButton;
class BaseTexture;
class BaseButton;

#include "scrollPage.h"

class ProgressInfo: public GameState
{
    BaseTexture* texButton;

    PageHandler* pageHandler;

    BaseButton* returnButton;

    void initButtons();

    void populatePages();

  public:

    ProgressInfo();
    ~ProgressInfo();

    void Logic(float dt, float phase=0.);
    void Render(float phase=0.);
    void HandleEvents();

};
//=====================================================

#include <vector>
#include <string>

class ProgressInfoPage: public ScrollPage
{
    int firstItem;
    int numItems;
    int maxItems;

    bool isSummonsPage;

    BaseTexture*                texMsgBackground;

    std::vector<std::string>    strVector;
    std::vector<bool>           statusVector;
    std::vector<BaseTexture*>   texBadgeVector;
    std::vector<BaseTexture*>   texMsgVector;
    std::vector<SDL_Rect>       msgRectVector;
    std::vector<SDL_Rect>       badgeRectVector;

    void initControls();
    void getStatusStrings();
    void loadImages();
    void setMessageRects();
    void setIconRects();

public:
    ProgressInfoPage(std::string titleStr, const FPoint& pos, const SDL_Rect& rect, bool isSummons, int first, int maxItems);
    ~ProgressInfoPage();

    void render(float alphaModifier =1.);
    bool handleEvents();

};
//=======================================================
#endif // PROGRESS_INFO_H

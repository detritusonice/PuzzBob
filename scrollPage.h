#ifndef SCROLLPAGE_H
#define SCROLLPAGE_H

#include <vector>
#include <string>
#include "FPoint.h"

class TextureButton;
class BaseTexture;

class ScrollPage
{
protected:
    std::string title;

    int pageID;
    SDL_Rect rc;

    int imageHandle;
    int alpha;

    BaseTexture* texPageTitle;

    std::vector<TextureButton*> buttonVector;

    FPoint referencePosition;
    FPoint position;//UL corner
    FPoint velocity;
    FPoint origin;
    FPoint target;//where UL corner will be when movement is finished

    void initPage(std::string titleStr, const FPoint& pos, const SDL_Rect& rect,int alphaVal);

    virtual void loadImages()=0;
    virtual void initControls()=0;
    virtual void setPageContent(int newID);
public:
    virtual ~ScrollPage(){}

    void resetOrigin();
    void setRect( const SDL_Rect& newRect);
    void setOrigin(const FPoint& newPos);
    void setPosition(const FPoint& newPos);
    void setVelocity(const FPoint& newVel);
    void setTarget(const FPoint& newTarget);
    void setAlpha(int newAlpha);

    bool pollControlStateChange();

    SDL_Rect getRect();
    FPoint getOrigin();
    FPoint getPosition();
    FPoint getVelocity();
    FPoint getTarget();
    int getAlpha();

    bool isVisible();

    void updatePos(float dt);
    FPoint targetDiff();
    FPoint positionDiff();
    void setPageID(int newID);
    int getPageID();

    virtual void updateContentPos() {};

    virtual void update(float dt, float phase);
    virtual void render(float alphaModifier =1.)=0;
    virtual bool handleEvents()=0;
    virtual void renderTitle(float alphaModifier=1.);

};

//======================================================================================

class PageRing
{
protected:
    std::vector<ScrollPage*> pv;

    int current;
    int isLinear;

    int lastID;

    int ringSize;

    void deleteAllPages();
    void initRing(ScrollPage* newPage=NULL);


public:
    PageRing( bool linear=true,ScrollPage* firstPage=NULL);
    virtual ~PageRing();

    void addPage( ScrollPage* newPage, bool setAsCurrent=false);
    void deleteCurrentPage();

    virtual ScrollPage* getCurrentPage();

    virtual ScrollPage* getNextPage(int direction);
    virtual ScrollPage* getPreviousPage(int direction);

    virtual void gotoNext(int direction);
    virtual void gotoPrevious(int direction);

    virtual void setNavMetrics();

    int getSize();

    bool isCircle();

    int getCurrentID();
};

//--------------------------------------------------------------

class ManhattanRing : public PageRing
{
protected:
    int manhattanWidth;
    int manhattanMode;

    virtual ScrollPage* getNextVerticalPage();
    virtual ScrollPage* getPreviousVerticalPage();
    virtual ScrollPage* getNextHorizontalPage();
    virtual ScrollPage* getPreviousHorizontalPage();

    virtual void gotoHorizontalNext();
    virtual void gotoHorizontalPrevious();
    virtual void gotoVerticalNext();
    virtual void gotoVerticalPrevious();

public:
    enum MANHATTAN_MODES
    {
        ARRAY_MANHATTAN=0,
        ROLLING_MANHATTAN
    };

    ManhattanRing( int width, bool linear=true, int mode=ROLLING_MANHATTAN, ScrollPage* firstPage=NULL);

    ScrollPage* getNextPage(int direction);
    ScrollPage* getPreviousPage(int direction);

    void gotoNext(int direction);
    void gotoPrevious(int direction);


    int getWidth();
    int getMode();
};

//====================================================================

class PseudoManhattanRing : public ManhattanRing
{
    int domainSize;
    int vDiff;
    int center;
    int initialPage;

    ScrollPage* getNextVerticalPage();
    ScrollPage* getPreviousVerticalPage();
    ScrollPage* getNextHorizontalPage();
    ScrollPage* getPreviousHorizontalPage();

    void gotoHorizontalNext();
    void gotoHorizontalPrevious();
    void gotoVerticalNext();
    void gotoVerticalPrevious();

    int getPreviousVerticalIndex(int index);
    int getNextVerticalIndex(int index);
    int getPreviousHorizontalIndex(int index);
    int getNextHorizontalIndex(int index);

    void swapToPreviousHorizontal();
    void swapToNextHorizontal();
    void swapToPreviousVertical();
    void swapToNextVertical();


public:
    ScrollPage* getCurrentPage();
    void setNavMetrics();

    PseudoManhattanRing( int realSize, int startingPage, int width, bool linear=true, int mode=ROLLING_MANHATTAN, ScrollPage* firstPage=NULL);
};

//====================================================================

class PageHandler
{
protected:

    FPoint position;

    SDL_Rect rc;
    SDL_Rect invRc[2];
    SDL_Rect swipeRC;

    int imageHandle;// texture manager int handles of textures
    int invHandle;


    int alpha;
    int orientation;
    int direction;

    bool isTouchActive;
    bool inTransit;

    bool showInvalidBlock[NUM_AXIAL_NEIGHBORS];// blocks signify end of scrolling range when applicable
    int invalidAlpha[NUM_AXIAL_NEIGHBORS];      // fading out after a while

    BaseTexture* pageBuffers[2];                // pointers to pre-rendered images of the pages to be scrolled

    BaseTexture* texSwipeHelp;//new             // indicating that scrolling is available for first time users

    int bufferIDs[2];                           // page ids in the page ring
    bool isBufferValid[2];                      // does it need to be re-rendered? for example a button was pressed or released

    PageRing* pageRing;                         // storage for the pages of the scrollable collection

    Uint8 prevMouseState;                       // to control motion and state transition
    FPoint prevTouch;
    FPoint velocity;

    void initHandler();

    void resetInvalidBlocks();                // 'drop' end of scrolling range blocks
    void renderInvalidBlocks(float alphaModifier=1.);
    void updateInvalidBlocks(float dt);

    void invalidateBuffer(int pageID);                  // signal a page to be re-drawn
    void updateBuffer(int bufferID, ScrollPage* page);  // redraw, to buffer if possible

    void renderPage( ScrollPage* page, int currenPageID, float alphaModifier);// transparently handle the drawing to buffer or not of a page


    void setTargets();  // when page scrolling is signalled by the user, determine what is the landing point of the current page

    void launchPages();                 // initiate the page scrolling, release any controls that might be in an incomplete touch phase
    void movePages(const FPoint& dp);   // handle motion while scrolling
    void stopPages();                   // terminate scrolling and activate interaction with the new current page


    FPoint regulateVector(const FPoint& v);             // determine the direction the users wants to scroll to in the initial stages of finger motion
    FPoint regulateHorizontalVector(const FPoint& v);   // according to pageHandler type, use one of these two
    FPoint regulateVerticalVector(const FPoint& v);

    void placeCircularNeighboringPage(int order);


    virtual int getMovementDirection(const FPoint& v)=0;

    virtual void setPreviousAsTarget();
    virtual void setNextAsTarget();

    virtual void updateVelocity(float) {}
    virtual void resetDirection()=0;
    virtual void setDirection(int newDirection)=0;

public:


    virtual ~PageHandler();


    void update(float dt, float phase);
    void handleEvents();

    virtual void render(float phase=0., float alphaModifier=1.);

    void addPage( ScrollPage* newPage, bool setAsCurrent=false);
    virtual void removeCurrentPage();

    virtual void doneInserting()=0;
    virtual int getCurrentPageID();
};

//--------------------------------------------------------------

class AxialPageHandler : public PageHandler // in case of single-axis scrollable page menu
{
    int getMovementDirection(const FPoint& v);
    void resetDirection();
    void setDirection(int newDirection);

public:
    AxialPageHandler(int orientationType,bool isCircular,const FPoint& pos, const SDL_Rect& rect,int alphaValue);
    void doneInserting();
};

//--------------------------------------------------------------

class ManhattanPageHandler : public PageHandler // for grid based, two axis scrollable
{
    bool isPseudo;  // pseudo manhattan creates a fixed grid and reloads border pages according to direction
    int getMovementDirection(const FPoint& v);

    void resetDirection();
    void setDirection(int newDirection);

public:
    ManhattanPageHandler(int manhattanMode, bool isCircular, int width, const FPoint& pos, const SDL_Rect& rect,int alphaValue,int pseudoMaxPage=-1,int startingPage=0);
    void removeCurrentPage();
    void doneInserting();
    int getCurrentPageID();
};
#endif // SCROLLPAGE_H

#include "base.h"
#include "globals.h"
#include "basefuncs.h"
#include "FPoint.h"

#include "texture.h"

#include "scrollPage.h"
#include "gameSettings.h"
#include "button.h"

#include "myMath.h"

#include <cstdio>
#include <cmath>

extern float framesPerSec;
//-------------------------------------------------------
void ScrollPage::renderTitle(float alphaModifier)
{
    if (texPageTitle==NULL) return;

    SDL_Rect borderRc={0,0,
                    (int)(texPageTitle->getWidth()+guiUnit/4),
                    (int)(texPageTitle->getHeight()+guiUnit/8)};

    FPoint titlePosition(position.x+(rc.w-texPageTitle->getWidth())/2,position.y+guiUnit/8);

    glbTexManager->getTexturePtr(imageHandle)->render(titlePosition.x-guiUnit/8,
                                                      titlePosition.y,&borderRc,-1,alphaModifier*50);

    texPageTitle->render(titlePosition.x, titlePosition.y,NULL,-1,alpha*alphaModifier);
}
//-------------------------------------------------------
void ScrollPage::setRect( const SDL_Rect& newRect)
{
    rc=newRect;
}
//-------------------------------------------------------
void ScrollPage::setOrigin(const FPoint& newPos)
{
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"ScrollPage::setOrigin\n");
    mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
    origin = newPos;
    setPosition(newPos);
    setTarget(newPos);
    updateContentPos();
}
//-------------------------------------------------------
void ScrollPage::setPosition(const FPoint& newPos)
{
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"ScrollPage::setPosition, (%f,%f)\n",newPos.x,newPos.y);
    mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
    position = newPos;
    //updateContentPos();
}
//-------------------------------------------------------
void ScrollPage::setVelocity(const FPoint& newVel)
{
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"ScrollPage::setVelocity, (%f,%f)\n",newVel.x,newVel.y);
    mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
    velocity = newVel;
}
//-------------------------------------------------------
void ScrollPage::setTarget(const FPoint& newTarget)
{
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"ScrollPage::setTarget [ %f , %f ]\n",newTarget.x, newTarget.y);
    mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
    target= newTarget;
}
//-------------------------------------------------------
void ScrollPage::setAlpha(int newAlpha)
{
    alpha = std::min(255, std::max(newAlpha,0));
}
//-------------------------------------------------------

SDL_Rect ScrollPage::getRect()
{
    return rc;
}
//-------------------------------------------------------
FPoint ScrollPage::getOrigin()
{
    return origin;
}
//-------------------------------------------------------
FPoint ScrollPage::getPosition()
{
    return position;
}
//-------------------------------------------------------
FPoint ScrollPage::getVelocity()
{
    return velocity;
}
//-------------------------------------------------------
FPoint ScrollPage::getTarget()
{
    return target;
}
//-------------------------------------------------------
int ScrollPage::getAlpha()
{
    return alpha;
}
//----------------------------------------------------------
FPoint ScrollPage::targetDiff()
{
    return target-origin;
}
//----------------------------------------------------------
FPoint ScrollPage::positionDiff()
{
    return position-origin;
}
//-------------------------------------------------------
bool ScrollPage::isVisible()
{

    if ( ( (position.x < screenRect->w ) && ( (position.x>=0) || (position.x+rc.w>=0)) )
        && ( (position.y < screenRect->h ) && ( (position.y>=0) || (position.y+rc.h>=0)) ) )
        return true;

    return false;
}
//-------------------------------------------------------

void ScrollPage::update(float dt, float phase)
{
    updatePos(dt);
}

void ScrollPage::updatePos(float dt)
{
    position+=velocity*dt;
}

void ScrollPage::setPageID(int newID)
{
    if (pageID!=newID)
    {
        setPageContent(newID);
        pageID=newID;
    }
}

int ScrollPage::getPageID()
{
    return pageID;
}

void ScrollPage::setPageContent(int newID)
{
}

void ScrollPage::initPage(std::string titleStr, const FPoint& pos, const SDL_Rect& rect,int alphaVal)
{
    texPageTitle=NULL;

    rc         = rect;
    referencePosition=pos;
    origin     = pos;
    position   = pos;
    target     = pos;
    title = titleStr;
    alpha = alphaVal;
}

void ScrollPage::resetOrigin()
{
    setOrigin(referencePosition);
}

bool ScrollPage::pollControlStateChange()
{
    bool stateChange=false;
    for (int i=0;i<buttonVector.size();i++)
        stateChange|=buttonVector[i]->changedState();

    return stateChange;
}
//==================== PAGERING ====================================


//----------------------------------------------------------------------
void PageRing::deleteAllPages()
{
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"PageRing::DeleteAllPages\n");
    mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
    for (int i=0;i<ringSize;i++)
    {
        delete pv[i];

        pv[i]=(ScrollPage*)NULL;
    }
    pv.clear();
}
//----------------------------------------------------------------------

PageRing::PageRing( bool linear, ScrollPage* firstPage)
{
    isLinear=linear;
    initRing(firstPage);
}
//----------------------------------------------------------------------
PageRing::~PageRing()
{
    deleteAllPages();
}
//-----------------------------------------------------------------------
void PageRing::initRing(ScrollPage* firstPage)
{
    pv.clear();
    lastID=0;

    pv.push_back(firstPage);

    if (firstPage==NULL)
        ringSize=0;
    else
    {
        ringSize=1;
        firstPage->setPageID(lastID++);//used for page identification. increases only. in pseudoManhattan has other function
    }

    current=0;


}
//----------------------------------------------------------------------
void PageRing::addPage( ScrollPage* newPage, bool setAsCurrent)
{
    newPage->setPageID(lastID++);//used for page identification. increases only. in pseudoManhattan has other function

    if (ringSize==0)
        pv[0] = newPage;
    else
        pv.push_back(newPage);

    if (setAsCurrent)//addition
        current=ringSize;

    ringSize++;

}
//----------------------------------------------------------------------
void PageRing::deleteCurrentPage()
{
    if (ringSize<2) return;// application specific. only player profile browser allows deleting entries and will not allow deleting the last one.

    delete pv[current];

    if (current==ringSize-1)
        current--;
    else
        for (int i=current;i<ringSize-1;i++)
            pv[i]=pv[i+1];

    pv[ringSize-1]=(ScrollPage*)NULL;

    pv.pop_back();
    ringSize--;//

    pv[current]->resetOrigin();

}
//----------------------------------------------------------------------
ScrollPage* PageRing::getCurrentPage()
{
    return pv[current];
}
//----------------------------------------------------------------------
ScrollPage* PageRing::getNextPage(int direction)
{
    if ( isLinear && ( ( current + 1 ) == ringSize) )
       return NULL;

    return pv[ (current+1)%ringSize ];
}
//----------------------------------------------------------------------
ScrollPage* PageRing::getPreviousPage(int direction)
{
    if ( isLinear && ( current == 0) )
            return NULL;

    return pv[ (current+ringSize-1)%ringSize ];
}
//------------------------------------------------------------------------
void PageRing::gotoNext(int direction)
{
    if ( isLinear && ( current == ringSize) )
        return;

    current=(current+1)%ringSize;
}
//----------------------------------------------------------------------
void PageRing::gotoPrevious(int direction)
{
    if ( isLinear && ( current == 0) )
        return;

    current=(current+ringSize-1)%ringSize;
}
//----------------------------------------------------------------------
int PageRing::getSize()
{
    return ringSize;
}
//----------------------------------------------------------------------
bool PageRing::isCircle()
{
    return !isLinear;
}
//-----------------------------------------------------------------------
void PageRing::setNavMetrics()
{
    if (ringSize<=2)
        isLinear=true;
}
//----------------------------------------------------------------------
int PageRing::getCurrentID()
{
    return current;
}

//=========================================================================
ManhattanRing::ManhattanRing( int width, bool linear, int mode,ScrollPage* firstPage)
{
    isLinear=linear;

    manhattanWidth=width;
    manhattanMode=mode;

    if (mode==ARRAY_MANHATTAN)
        isLinear=true;

    initRing(firstPage);
}
//----------------------------------------------------------------------
ScrollPage* ManhattanRing::getNextPage(int direction)
{
    if (direction==VERTICAL) return getNextVerticalPage();
    else return getNextHorizontalPage();
}
//----------------------------------------------------------------------
ScrollPage* ManhattanRing::getPreviousPage(int direction)
{
    if (direction==VERTICAL) return getPreviousVerticalPage();
    else return getPreviousHorizontalPage();
}
//----------------------------------------------------------------------

ScrollPage* ManhattanRing::getNextVerticalPage()
{
    int index=current+manhattanWidth;

    if (!isLinear)//a ring
        index%=ringSize;
    else
        if ( index >= ringSize)
            return NULL;

    return pv[index];
}
//------------------------------------------------------------------
ScrollPage* ManhattanRing::getPreviousVerticalPage()
{
    int index;
    if (isLinear)//finite list
    {
        index = current - manhattanWidth;
        if ( index < 0)
            return NULL;
    }
    else //page ring
        index= (current+ringSize - manhattanWidth)%ringSize;

    return pv[index];
}
//------------------------------------------------------------------
ScrollPage* ManhattanRing::getNextHorizontalPage()
{
    if (manhattanMode==ROLLING_MANHATTAN)
        return PageRing::getNextPage(HORIZONTAL);
    else
    if ( ( ((current+1)%manhattanWidth)==0) || ( (current+1)==ringSize ) )
        return NULL;
    else
        return pv[current+1];
}
//------------------------------------------------------------------
ScrollPage* ManhattanRing::getPreviousHorizontalPage()
{
    if (manhattanMode==ROLLING_MANHATTAN)
        return PageRing::getPreviousPage(HORIZONTAL);
    else
    if ( (current%manhattanWidth)==0)
        return NULL;
    else
        return pv[current-1];
}

//------------------------------------------------------------------
void ManhattanRing::gotoNext(int direction)
{
    if (direction==VERTICAL) gotoVerticalNext();
    else gotoHorizontalNext();
}
//------------------------------------------------------------------
void ManhattanRing::gotoPrevious(int direction)
{
    if (direction==VERTICAL) gotoVerticalPrevious();
    else gotoHorizontalPrevious();
}

//--------------------------------------------------------------------
void ManhattanRing::gotoHorizontalNext()
{
    if (manhattanMode==ARRAY_MANHATTAN)
        if ( ( ((current+1)%manhattanWidth)==0) || ( (current+1)==ringSize ) )
            return;
    PageRing::gotoNext(HORIZONTAL);
}
//------------------------------------------------------------------
void ManhattanRing::gotoHorizontalPrevious()
{
    if (manhattanMode==ARRAY_MANHATTAN)
       if ( (current%manhattanWidth) == 0 )
        return;
    PageRing::gotoPrevious(HORIZONTAL);
}
//------------------------------------------------------------------
void ManhattanRing::gotoVerticalNext()
{
    int index=current+manhattanWidth;

    if (!isLinear)//a ring
        index%=ringSize;
    else
        if ( index >= ringSize)
            return;
    current=index;
}
//------------------------------------------------------------------
void ManhattanRing::gotoVerticalPrevious()
{
    int index;

    if (isLinear)//finite list
    {
        index = current - manhattanWidth;
        if ( index < 0)
            return;
    }
    else //page ring
        index= (current+ringSize - manhattanWidth)%ringSize;

    current=index;
}
//----------------------------------------------------------------------
int ManhattanRing::getWidth()
{
    return manhattanWidth;
}
//----------------------------------------------------------------------
int ManhattanRing::getMode()
{
    return manhattanMode;
}
//=========================================================================


void PseudoManhattanRing::setNavMetrics()
{
    if (vDiff==0)//5 pages, +-2 for below/above, 0-based 2 is the center
    {
        if (ringSize!=5)
        {
#ifdef CONSOLE_LOG
            sprintf(glbLogMsg,"PSEUDOMANHATTAN RING: ==ERROR! MUST HAVE 5 PAGES===\n");
            mySDL_Log(glbLogMsg);
#endif
            return;
        }

        vDiff=2;
        center=2;
        current=initialPage;

#ifdef CONSOLE_LOG
            sprintf(glbLogMsg,"PSEUDOMANHATTAN RING: CURRENT PAGE=%d\n",current);
            mySDL_Log(glbLogMsg);
#endif


        pv[center]->setPageID( current );
        pv[center-1]->setPageID( getPreviousHorizontalIndex(current) );
        pv[center+1]->setPageID( getNextHorizontalIndex(current) );
        pv[center-vDiff]->setPageID( getPreviousVerticalIndex(current) );
        pv[center+vDiff]->setPageID( getNextVerticalIndex(current) );
    }
}
//--------------------------------------------------------------------------
PseudoManhattanRing::PseudoManhattanRing( int realSize, int startingPage,int width, bool linear, int mode, ScrollPage* firstPage)
:ManhattanRing(width,linear,mode,firstPage)
{
    domainSize=realSize;
    initialPage=startingPage;//used to carry th first "current" value;
    vDiff=0;
    center=0;
}
//---------------------------------------------------------------------
ScrollPage* PseudoManhattanRing::getCurrentPage()
{
    return pv[center];
}
//----------------------------------------------------------------------
ScrollPage* PseudoManhattanRing::getNextVerticalPage()
{
    if (getNextVerticalIndex(current)==-1) return NULL;

    return pv[center+vDiff];
}
//------------------------------------------------------------------
ScrollPage* PseudoManhattanRing::getPreviousVerticalPage()
{
    if (getPreviousVerticalIndex(current)==-1) return NULL;

    return pv[center-vDiff];
}
//------------------------------------------------------------------
ScrollPage* PseudoManhattanRing::getNextHorizontalPage()
{
    if (getNextHorizontalIndex(current)==-1) return NULL;

    return pv[center+1];
}
//------------------------------------------------------------------
ScrollPage* PseudoManhattanRing::getPreviousHorizontalPage()
{
    if (getPreviousHorizontalIndex(current)==-1) return NULL;
    return pv[center-1];
}

//------------------------------------------------------------
int PseudoManhattanRing::getPreviousHorizontalIndex(int index)
{
    if (manhattanMode==ROLLING_MANHATTAN)
    {
        if ( isLinear && ( current == 0) )
                return -1;
        else
            return (index+domainSize-1)%domainSize;
    }
    else
    {
        if ( (index%manhattanWidth)==0)
            return -1;
        else
            return index-1;
    }
}
//-------------------------------------------------------------------
int PseudoManhattanRing::getNextHorizontalIndex(int index)
{
    if (manhattanMode==ROLLING_MANHATTAN)
    {
        if ( isLinear && ( ( index + 1 ) == domainSize) )
               return -1;
        else
                return (index+1)%domainSize;
    }
    else
    {
        if ( ( (index+1)%manhattanWidth == 0) || ( (index+1)==domainSize ) )
            return -1;
        else
            return index+1;
    }
}

//----------------------------------------------------------
int PseudoManhattanRing::getPreviousVerticalIndex(int index)
{
    int newIndex=index;

    if (isLinear)//finite list
    {
        newIndex = index - manhattanWidth;
        if ( newIndex < 0)
            return -1;
    }
    else //page ring
        newIndex= (index+domainSize - manhattanWidth)%domainSize;

    return newIndex;

}
//------------------------------------------------------------------
int PseudoManhattanRing::getNextVerticalIndex(int index)
{
    int newIndex = index+manhattanWidth;

    if (!isLinear)//a ring
        newIndex%=domainSize;
    else
        if ( newIndex >= domainSize)
            return -1;

    return newIndex;
}
//---------------------------------------------------------------
void PseudoManhattanRing::swapToPreviousHorizontal()
{
    ScrollPage* tmp=pv[center+1];

    pv [center+1]=pv[center];
    pv[center]=pv[center-1];
    pv[center-1]=tmp;

    current=getPreviousHorizontalIndex(current);

    pv[center-1]->setPageID( getPreviousHorizontalIndex(current) );

    pv[center-vDiff]->setPageID( getPreviousVerticalIndex(current) );
    pv[center+vDiff]->setPageID( getNextVerticalIndex(current) );
}
//--------------------------------------------------------------------------

void PseudoManhattanRing::swapToNextHorizontal()
{
    ScrollPage* tmp=pv[center-1];

    pv[center-1]=pv[center];
    pv[center]=pv[center+1];
    pv[center+1]=tmp;

    current=getNextHorizontalIndex(current);

    pv[center+1]->setPageID( getNextHorizontalIndex(current) );

    pv[center-vDiff]->setPageID( getPreviousVerticalIndex(current) );
    pv[center+vDiff]->setPageID( getNextVerticalIndex(current) );
}
//--------------------------------------------------------------------------
void PseudoManhattanRing::swapToPreviousVertical()
{
    ScrollPage* tmp=pv[center+vDiff];

    pv [center+vDiff]=pv[center];
    pv[center]=pv[center-vDiff];
    pv[center-vDiff]=tmp;

    current=getPreviousVerticalIndex(current);

    pv[center-vDiff]->setPageID( getPreviousVerticalIndex(current) );

    pv[center-1]->setPageID( getPreviousHorizontalIndex(current) );
    pv[center+1]->setPageID( getNextHorizontalIndex(current) );
}
//--------------------------------------------------------------------------
void PseudoManhattanRing::swapToNextVertical()
{
    ScrollPage* tmp=pv[center-vDiff];

    pv [center-vDiff]=pv[center];
    pv[center]=pv[center+vDiff];
    pv[center+vDiff]=tmp;

    current=getNextVerticalIndex(current);

    pv[center+vDiff]->setPageID( getNextVerticalIndex(current) );

    pv[center-1]->setPageID( getPreviousHorizontalIndex(current) );
    pv[center+1]->setPageID( getNextHorizontalIndex(current) );
}
//--------------------------------------------------------------------------
void PseudoManhattanRing::gotoHorizontalNext()
{
    if (getNextHorizontalIndex(current)<0)
            return;
    swapToNextHorizontal();
}
//------------------------------------------------------------------
void PseudoManhattanRing::gotoHorizontalPrevious()
{
    if (getPreviousHorizontalIndex(current)<0)
        return;
    swapToPreviousHorizontal();
}
//------------------------------------------------------------------
void PseudoManhattanRing::gotoVerticalNext()
{
    if (getNextVerticalIndex(current)<0)
            return;
    swapToNextVertical();
}
//------------------------------------------------------------------
void PseudoManhattanRing::gotoVerticalPrevious()
{
    if (getPreviousVerticalIndex(current)<0)
            return;
    swapToPreviousVertical();
}

//=========================================================================
PageHandler::~PageHandler()
{
    if (pageRing!=NULL)
        delete pageRing;

    for (int i=0;i<2;i++)
        if (pageBuffers[i]!=NULL)
            delete pageBuffers[i];

    if (texSwipeHelp!=NULL)
        delete texSwipeHelp;
}
//----------------------------------------------------------------------
void PageHandler::handleEvents()
{
    if (inTransit) return;

    if (pageRing->getSize()==0) return;

    ScrollPage* currentPage=pageRing->getCurrentPage();

    FPoint diff = currentPage->positionDiff();

    if (diff==FPoint(0.,0.))
            isTouchActive=false;
        else isTouchActive=true;

    if (!isTouchActive)
    {
        bool contentChanged=currentPage->handleEvents();

        if (contentChanged || currentPage->pollControlStateChange())
        {
            invalidateBuffer(currentPage->getPageID());
            return;
        }
    }

    if (pageRing->getSize()==1) return;//no movement to handle


    FPoint mousePos;
    Uint8 mouseBtn;

    {
        int mx,my;
        mouseBtn=SDL_GetMouseState(&mx,&my);
        mousePos=FPoint(mx,my);
    }

    if ( mouseBtn&SDL_BUTTON(1))
    {

        if (!(prevMouseState & SDL_BUTTON(1)) )//first touch
        {
            prevTouch=mousePos;
            resetDirection();
            invalidateBuffer(currentPage->getPageID());
        }
        FPoint mousemove=mousePos-prevTouch;

        FPoint regDp = regulateVector(mousemove);
        if (std::max(myfabs(regDp.x),myfabs(regDp.y)) > guiUnit/8)//small variation between touchdown and up led to insensitivity to taps
        {
            movePages(regDp-diff);
            velocity = regDp;
        }
    }
    else
    {

        if (prevMouseState & SDL_BUTTON(1) )
        {
            invalidateBuffer(currentPage->getPageID());
            setTargets();
            launchPages();
        }

        //select Target - switch page
    }

    prevMouseState=mouseBtn;

}
//----------------------------------------------------------------------
void PageHandler::resetInvalidBlocks()
{
    for (int i=0;i<NUM_AXIAL_NEIGHBORS;i++)
    {
        showInvalidBlock[i]=false;
        invalidAlpha[i]=0;
    }
}
//----------------------------------------------------------------------
void PageHandler::updateInvalidBlocks(float dt)
{
    for (int i=0;i<NUM_AXIAL_NEIGHBORS;i++)
    {
        if (invalidAlpha[i]>0)
            invalidAlpha[i] = std::max(0,(int)(invalidAlpha[i] - dt*512));

        if (invalidAlpha[i]==0)
            showInvalidBlock[i]=false;
    }
}
//----------------------------------------------------------------------
void PageHandler::placeCircularNeighboringPage(int order)
{
    ScrollPage *currentPage = pageRing->getCurrentPage();
    ScrollPage* neighborPage;

    if (order==NEIGHBOR_PREVIOUS)
        neighborPage = pageRing->getPreviousPage(direction);
    else
        neighborPage = pageRing->getNextPage(direction);

    if ((neighborPage==NULL) || (neighborPage==currentPage))
        return;

    FPoint origin=currentPage->getOrigin();

    float sign = (order==NEIGHBOR_NEXT)?1.:-1;

    if (direction==HORIZONTAL)
        origin.x +=screenRect->w*sign;
    else
        origin.y+=screenRect->h*sign;

    if (neighborPage->getOrigin()!=origin)
        neighborPage->setOrigin(origin);
}
//----------------------------------------------------------------------
void PageHandler::update(float dt, float phase)
{
    updateInvalidBlocks(dt);

    if (prevMouseState&SDL_BUTTON(1)) return;//still touching

    if (pageRing->getSize()==0) return;//no pages

    if (inTransit)//moving
    {
        ScrollPage *currentPage = pageRing->getCurrentPage();

        FPoint pos,target,nextPos;

        pos=currentPage->getPosition();
        target=currentPage->getTarget();

        nextPos = pos +velocity*dt;

        if ( (target-pos)*(target-nextPos)<=0)//passes the target
        {
            stopPages();
            return;
        }
        updateVelocity(dt);

        currentPage->update(dt,phase);
        currentPage->setVelocity(velocity); //if i decide to use variable velocity

        ScrollPage* prevPage = pageRing->getPreviousPage(direction);
        ScrollPage* nextPage = pageRing->getNextPage(direction);

        if (prevPage!=NULL)
        {
            prevPage->update(dt,phase);
            prevPage->setVelocity(velocity); //if i decide to use variable velocity
        }

        if (nextPage!=NULL)
        {
            nextPage->update(dt,phase);
            nextPage->setVelocity(velocity); //if i decide to use variable velocity
        }
    }
}

//-----------------------------------------------------------------

void PageHandler::stopPages()
{
    if (pageRing->getSize()<=1) return;
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"STOP PAGES\n");
    mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG

    FPoint stopped(0.,0.);

    ScrollPage* currentPage=pageRing->getCurrentPage();

    ScrollPage* nextPage = pageRing->getNextPage(direction);
    ScrollPage* prevPage = pageRing->getPreviousPage(direction);

    if (nextPage!=NULL)
    {
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"STOP PAGES NEXT PAGE\n");
    mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
        nextPage->setVelocity(stopped);
        nextPage->setOrigin(nextPage->getTarget());
        if (nextPage->getOrigin()==currentPage->getOrigin())
            pageRing->gotoNext(direction);
    }

    if (prevPage!=NULL)
    {
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"STOP PAGES PREV PAGE\n");
    mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
        prevPage->setVelocity(stopped);
        prevPage->setOrigin(prevPage->getTarget());
        if (prevPage->getOrigin()==currentPage->getOrigin())
            pageRing->gotoPrevious(direction);
    }

    currentPage->setVelocity(stopped);
    currentPage->setOrigin(currentPage->getTarget());

    inTransit=false;

    velocity=stopped;
    resetDirection();
}
//----------------------------------------------------------------------
void PageHandler::launchPages()
{
    if (pageRing->getSize()<=1) return;
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"LAUNCH PAGES\n");
    mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG

    ScrollPage* currentPage=pageRing->getCurrentPage();

    velocity = currentPage->targetDiff()-currentPage->positionDiff();
    velocity*=4;

    currentPage->setVelocity(velocity);

    if ( velocity == FPoint(0.,0.) )
        return;

//need both for the small and big gesture movements
    ScrollPage* nextPage = pageRing->getNextPage(direction);
    ScrollPage* prevPage = pageRing->getPreviousPage(direction);

    if (nextPage!=NULL)
            nextPage->setVelocity(velocity);
    if (prevPage!=NULL)
            prevPage->setVelocity(velocity);

    inTransit=true;

}
//----------------------------------------------------------------------
void PageHandler::movePages(const FPoint& dp)
{
    if (pageRing->getSize()<=1) return;
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"MOVE PAGES\n");
    mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
    ScrollPage* currentPage=pageRing->getCurrentPage();

    ScrollPage* prevPage = pageRing->getPreviousPage(direction);
    ScrollPage* nextPage = pageRing->getNextPage(direction);

    currentPage->setPosition(dp+currentPage->getPosition());

    if (nextPage!=NULL)
            placeCircularNeighboringPage(NEIGHBOR_NEXT);
    if ((prevPage!=NULL) )
            placeCircularNeighboringPage(NEIGHBOR_PREVIOUS);//problem with 2 pages...

    bool done=false;

    if (nextPage!=NULL)
    {
        if (currentPage->positionDiff()*(nextPage->getOrigin()-currentPage->getOrigin()) <0.)
        {
                nextPage->setPosition(dp+nextPage->getPosition());
                done=true;
        }
        else
            nextPage->setPosition(nextPage->getOrigin());

    }
    if ((prevPage!=NULL))
    {
        if (!done)
            prevPage->setPosition(dp +prevPage->getPosition());
        else
            prevPage->setPosition(prevPage->getOrigin());
    }
}
//----------------------------------------------------------------------
void PageHandler::renderInvalidBlocks(float alphaModifier)
{
    if (showInvalidBlock[NEIGHBOR_LEFT])
                glbTexManager->getTexturePtr(invHandle)->render(position.x,
                                                                position.y+guiUnit/4.,
                                                                &invRc[VERTICAL],-1,
                                                                alphaModifier*invalidAlpha[NEIGHBOR_LEFT]);
    if (showInvalidBlock[NEIGHBOR_RIGHT])
                glbTexManager->getTexturePtr(invHandle)->render(position.x+rc.w-invRc[VERTICAL].w,
                                                                position.y+guiUnit/4.,
                                                                &invRc[VERTICAL],-1,
                                                                alphaModifier*invalidAlpha[NEIGHBOR_RIGHT]);
    if (showInvalidBlock[NEIGHBOR_TOP])
                glbTexManager->getTexturePtr(invHandle)->render(position.x+guiUnit/4.,
                                                                position.y,&invRc[HORIZONTAL],-1,
                                                                alphaModifier*invalidAlpha[NEIGHBOR_TOP]);
    if (showInvalidBlock[NEIGHBOR_BOTTOM])
                glbTexManager->getTexturePtr(invHandle)->render(position.x+guiUnit/4.,
                                                                position.y+rc.h-invRc[HORIZONTAL].h,
                                                                &invRc[HORIZONTAL],-1,
                                                                alphaModifier*invalidAlpha[NEIGHBOR_BOTTOM]);
}
//---------------------------------------------------------------------------
void PageHandler::initHandler()
{
    isTouchActive =false;
    prevMouseState=0;
    inTransit=false;

    velocity=FPoint(0,0);

    resetInvalidBlocks();

    imageHandle = glbTexManager->insertTexture("message_background.png",SKIN_UNIVERSAL,true);
    invHandle = glbTexManager->insertTexture("warning_block.png",SKIN_UNIVERSAL,true);

    if (gameSettings->showSwipeHint)
    {
        texSwipeHelp = new BaseTexture();
        texSwipeHelp->fromText("SWIPE TO SCROLL",gameSettings->colors[COLOR_PAGE_TITLE]);
    }
    else texSwipeHelp=NULL;

    for (int i=0;i<2;i++)
    {
        pageBuffers[i]=NULL;
        bufferIDs[i]=-1;
        isBufferValid[i]=false;
    }

    invRc[VERTICAL]={0,0,int(guiUnit/4.),(int)(rc.h-guiUnit/4.)};
    invRc[HORIZONTAL]={0,0,rc.w-(int)(guiUnit/4.),(int)(guiUnit/4.)};

    if (gameSettings->showSwipeHint)
    {
        swipeRC.w   =   texSwipeHelp->getWidth();
        swipeRC.h   =   swipeRC.w*texSwipeHelp->getHeight()/texSwipeHelp->getWidth();

        swipeRC.x   =   guiBox->x + (guiBox->w-swipeRC.w)/2,
        swipeRC.y   =   guiBox->y + (guiBox->h - swipeRC.h);
    }

    resetDirection();
}
//----------------------------------------------------------------------
void PageHandler::invalidateBuffer(int pageID)
{
    for (int i=0;i<2;i++)
        if (bufferIDs[i]==pageID)
            isBufferValid[i]=false;
}
//----------------------------------------------------------------------

void PageHandler::updateBuffer(int bufferID, ScrollPage* page)
{
    if (pageBuffers[bufferID]==NULL)
    {
        pageBuffers[bufferID]= new BaseTexture;
        if (!pageBuffers[bufferID]->createBlank(screenRect->w, screenRect->h,
                                        SDL_TEXTUREACCESS_TARGET))
        {

#ifdef CONSOLE_LOG
            sprintf(glbLogMsg, "InGame:LauncherImage creation Failed\n");
            mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
            delete pageBuffers[bufferID];
            pageBuffers[bufferID]=NULL;
        }
        else
            pageBuffers[bufferID]->useAlphaBlending(true);
    }

    if (pageBuffers[bufferID]!=NULL)
    {
        pageBuffers[bufferID]->setAsRenderTarget(gameSettings->colors[COLOR_ORIGINAL_BACKGROUND]);
        FPoint pos=page->getPosition();

        page->setPosition(pageRing->getCurrentPage()->getOrigin());

        page->render(1.);

        page->setPosition(pos);//resetting position

        pageBuffers[bufferID]->resetRenderTarget();
    }

    isBufferValid[bufferID]=true;
}

//----------------------------------------------------------------------
void PageHandler::renderPage( ScrollPage* page, int currenPageID, float alphaModifier)
{
    int pageID=page->getPageID();
    int buffer=-1;

    for (int i=0;i<2;i++)
    {
        if (pageID==bufferIDs[i])
        {
            buffer=i;
            break;
        }
    }

    if (buffer==-1)
    {

        if (pageID==currenPageID)//non-existent current page
            buffer=0;
        else
            for (int i=0;i<2;i++)
            {
                if (bufferIDs[i]==currenPageID)//the other page has changed
                {
                    buffer=i^1;
                    break;
                }
            }

        //SDL_Log("Buffer %d was assigned to unused page %d",buffer,bufferIDs[buffer]);
        //SDL_Log("Buffer %d is now assigned to %d and invalidated",buffer,pageID);

        bufferIDs[buffer]=pageID;
        isBufferValid[buffer]=false;
    }

    if (!isBufferValid[buffer])
    {
         updateBuffer(buffer,page);
         //SDL_Log("Buffer %d has been updated",buffer);
    }

    FPoint pos=page->getPosition();

    if (pageBuffers[buffer]!=NULL)
        pageBuffers[buffer]->render(pos.x-position.x,pos.y-position.y,NULL,-1,255*alphaModifier);
    else
        page->render(alphaModifier);

}
//----------------------------------------------------------------------
void PageHandler::render(float phase, float alphaModifier)
{
//   glbTexManager->getTexturePtr(imageHandle)->render(position.x, position.y,&rc,-1,alpha);
    if (pageRing->getSize()==0) return;

    ScrollPage* currentPage=pageRing->getCurrentPage();

    renderPage(currentPage,currentPage->getPageID(),alphaModifier);

    FPoint dp=currentPage->positionDiff();

    renderInvalidBlocks(alphaModifier);

    float diff=0.;

    if (direction==HORIZONTAL)
        diff=dp.x;
    else if (direction==VERTICAL)
        diff=dp.y;

    if ( diff!= 0)
    {
        ScrollPage* nextPage= pageRing->getNextPage(direction);

        if ((nextPage!=NULL)&&(diff<0))
        {
            //nextPage->render(alphaModifier);
            renderPage(nextPage,currentPage->getPageID(),alphaModifier);
            return;
        }

        ScrollPage* prevPage = pageRing->getPreviousPage(direction);
        if ((prevPage!=NULL) &&(diff>0))
        {
            //prevPage->render(alphaModifier);
            renderPage(prevPage,currentPage->getPageID(),alphaModifier);
        }
    }
    if (gameSettings->showSwipeHint && pageRing->getSize()>1)
    {
        float a=pow(mySine(3.*phase),4);

        texSwipeHelp->render(swipeRC.x, swipeRC.y,&swipeRC,-1,a*250);
    }

}
//----------------------------------------------------------------------------
void PageHandler::setTargets()
{
    ScrollPage* currentPage=pageRing->getCurrentPage();

#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"STOP PAGES\n");
    mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG

    FPoint dp = currentPage->positionDiff();


    //if (dp.GetLength()>guiUnit)//might be better for non axis aligned page handlers
     if ( (std::abs(dp.x)> rc.w/6) || (std::abs(dp.y)> rc.h/6) )//better chance to rollback
    {

        if (dp*FPoint(1.,1.) >0.)//positive axis movement
            setPreviousAsTarget();
        else
            setNextAsTarget();

        playSound(SFX_PAGESCROLL,0);
    }
}
//---------------------------------------------------------------------
FPoint PageHandler::regulateVector(const FPoint& v)
{
    int dir=getMovementDirection(v);

    setDirection(dir);

    if (direction==HORIZONTAL)
        return regulateHorizontalVector(v);
    else
        return regulateVerticalVector(v);
}
//---------------------------------------------------------------------

FPoint PageHandler::regulateHorizontalVector(const FPoint& v)
{
    FPoint reg(v.x,0.);

    if (reg.x>0)
    {
        if (pageRing->getPreviousPage(HORIZONTAL)==NULL)
        {
            reg.x=0.;
            showInvalidBlock[NEIGHBOR_LEFT]=true;
            invalidAlpha[NEIGHBOR_LEFT]=255;
        }
    }
    else
    if (reg.x<0)
    {
        if (pageRing->getNextPage(HORIZONTAL)==NULL)
        {
            reg.x=0.;
            showInvalidBlock[NEIGHBOR_RIGHT]=true;
            invalidAlpha[NEIGHBOR_RIGHT]=255;
        }
    }
    return reg;
}
//----------------------------------------------------------------------
FPoint PageHandler::regulateVerticalVector(const FPoint& v)
{
    FPoint reg(0.,v.y);

    if (reg.y>0)
    {
        if (pageRing->getPreviousPage(VERTICAL)==NULL)
        {
            reg.y=0.;
            showInvalidBlock[NEIGHBOR_TOP]=true;
            invalidAlpha[NEIGHBOR_TOP]=255;
        }
    }
    else
    if (reg.y<0)
    {
        if (pageRing->getNextPage(VERTICAL)==NULL)
        {
            reg.y=0.;
            showInvalidBlock[NEIGHBOR_BOTTOM]=true;
            invalidAlpha[NEIGHBOR_BOTTOM]=255;
        }
    }
    return reg;
}
//----------------------------------------------------------------------

void PageHandler::setPreviousAsTarget()
{
    FPoint diff(0.,0.);

    if (direction == HORIZONTAL)
        diff.x=screenRect->w;
    else
        diff.y=screenRect->h;

    ScrollPage* currentPage = pageRing->getCurrentPage();
    ScrollPage* prevPage    = pageRing->getPreviousPage(direction);
    ScrollPage* nextPage    = pageRing->getNextPage(direction);

    prevPage->setTarget(currentPage->getOrigin());
    currentPage->setTarget(currentPage->getOrigin() + diff);

    if (nextPage!=NULL)
        nextPage->setTarget(nextPage->getOrigin());
}
//----------------------------------------------------------------
void PageHandler::setNextAsTarget()
{
    FPoint diff(0.,0.);

    if (direction == HORIZONTAL)
        diff.x=screenRect->w;
    else
        diff.y=screenRect->h;

    ScrollPage* currentPage = pageRing->getCurrentPage();
    ScrollPage* prevPage    = pageRing->getPreviousPage(direction);
    ScrollPage* nextPage    = pageRing->getNextPage(direction);

    nextPage->setTarget(currentPage->getOrigin());
    currentPage->setTarget(currentPage->getOrigin() - diff);
    if (prevPage!=NULL)
        prevPage->setTarget(prevPage->getOrigin());
}
//---------------------------------------------------------------
void PageHandler::addPage( ScrollPage* newPage, bool setAsCurrent)
{
    pageRing->addPage(newPage,setAsCurrent);
}
//---------------------------------------------------------------
int PageHandler::getCurrentPageID()
{
    return pageRing->getCurrentID();
}
//---------------------------------------------------------------
void PageHandler::removeCurrentPage()
{
    if (pageRing->getSize()>1)
    {
        pageRing->deleteCurrentPage();
    }
    pageRing->setNavMetrics();
}
//===========================================================================

AxialPageHandler::AxialPageHandler(int orientationType, bool isCircular, const FPoint& pos, const SDL_Rect& rect,int alphaValue)
{

    position=pos;
    rc=rect;
    alpha=alphaValue;
    orientation = orientationType;

    initHandler();

    pageRing = new PageRing(!isCircular);
}

//---------------------------------------------------------
int AxialPageHandler::getMovementDirection(const FPoint& v)
{
    return orientation;
}
//----------------------------------------------------------------------
void AxialPageHandler::doneInserting()
{
    pageRing->setNavMetrics();
}

//--------------------------------------------------
void AxialPageHandler::resetDirection()
{
    direction=orientation;
}
//--------------------------------------------------
void AxialPageHandler::setDirection(int newDirection)
{

}

//===========================================================================

ManhattanPageHandler::ManhattanPageHandler( int manhattanMode, bool isCircular, int width,const FPoint& pos, const SDL_Rect& rect,int alphaValue,int pseudoMaxPage,int startingPage)
{

    position=pos;
    rc=rect;
    alpha=alphaValue;
    orientation = MANHATTAN;

    initHandler();

    if (pseudoMaxPage>=0)
    {
        pageRing = new PseudoManhattanRing(pseudoMaxPage,startingPage, width,!isCircular,manhattanMode,NULL);
        isPseudo=true;
    }
    else
    {
        pageRing = new ManhattanRing(width,!isCircular,manhattanMode,NULL);
        isPseudo = false;
    }
}
//---------------------------------------------------------
int ManhattanPageHandler::getMovementDirection(const FPoint& v)
{
    if (direction!=FREE_MOVEMENT)
        return direction;

    if (v==FPoint(0.,0.))
        return FREE_MOVEMENT;

    if ( myfabs(v.x)>=myfabs(v.y) )
        return HORIZONTAL;

    return VERTICAL;
}

//----------------------------------------------------------------------

void ManhattanPageHandler::resetDirection()
{
    direction=FREE_MOVEMENT;
}

void ManhattanPageHandler::setDirection(int newDirection)
{
    if (direction==FREE_MOVEMENT)
        direction = newDirection;
}

void ManhattanPageHandler::doneInserting()
{
    pageRing->setNavMetrics();
}

int ManhattanPageHandler::getCurrentPageID()
{
    if (isPseudo) return 0;
    else
        return PageHandler::getCurrentPageID();
}

void ManhattanPageHandler::removeCurrentPage()
{
    if (isPseudo) return;
    else
        PageHandler::removeCurrentPage();
}


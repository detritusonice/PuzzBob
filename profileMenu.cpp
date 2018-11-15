
#include "base.h"

#include "basefuncs.h"

#include "stateMachine.h"
#include "states.h"
#include "ProfileMenu.h"

#include "texture.h"
#include "button.h"

#include "screenEffects.h"
#include "levelStats.h"
#include "gameSettings.h"

#include <cstdio> //for sprintf()
#include <cstring>

#include "myMath.h"

//=====================================================

ProfileMenu::ProfileMenu()
{
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"OptionsMenu\n");
    mySDL_Log(glbLogMsg);
#endif
    profileVector.resize(0);
    profileVector.clear();

    isHibernating=false;
    skinChanged=false;


    texMsgBackground = glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("message_background.png",SKIN_UNIVERSAL,true));


    texButtons[0] = glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("buttons/buttons_cyan.png",SKIN_UNIVERSAL,true));

    texButtons[1] = glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("buttons/buttons_yellow.png",SKIN_UNIVERSAL,true));

    for (int i=0;i<3;i++)
    {
        texMessages[i]=new BaseTexture;
        texMessages[i]->fromText("");
    }



    initButtons();

    SDL_Rect handlerRect={0,0,(int)(guiBox->w-0.1*guiUnit),(int)(guiBox->h - 1.7*guiUnit)};

    pageHandler = new AxialPageHandler(
                                       HORIZONTAL,true,
                                       FPoint( guiBox->x+ (guiBox->w-handlerRect.w)/2,
                                              guiBox->y+guiUnit/20),
                                       handlerRect,70);

    loadProfiles();

    populatePages();

    mode =PROFILE_BROWSER;

    profileEditor=NULL;

  //  sndClick        = Mix_LoadWAV( "media/snd_menu.wav" );

    SDL_ShowCursor(SDL_ENABLE);

}
//------------------------------------------------------
ProfileMenu::~ProfileMenu()
{
  SDL_ShowCursor(SDL_DISABLE);

 //   Mix_HaltChannel(-1);

 //   Mix_FreeChunk(sndClick);

    delete pageHandler;

    for (int i=0;i<NUM_PROFILE_MENU_BUTTONS;i++)
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"deleting button %d\n",i);
        mySDL_Log(glbLogMsg);
#endif
        delete profileButtons[i];
        profileButtons[i]=NULL;
    }

    for (int i=0;i<2;i++)
       texButtons[i]=NULL;//texture manager deletes them

    for (int i=0;i<3;i++)
       delete texMessages[i];

    texMsgBackground=NULL;

    if (skinChanged)
        gameSettings->setSkin(gameSettings->skin);//has already been set, emptying texmanager, loading colors
}
//------------------------------------------------------
void ProfileMenu::initButtons()
{
    const char* captions[NUM_PROFILE_MENU_BUTTONS]=
    { "Cancel","Delete","New","Select","Cancel","Confirm","OK"};

    int buttonImages[NUM_PROFILE_MENU_BUTTONS]=
    { 0,1,1,0,0,1,0};

    float buttonWidth=0.2*guiBox->w;
    float columnOffset=buttonWidth*1.2;
    int i;

    for (i=0;i<=ID_BTN_SELECT;i++)
        profileButtons[i]= new SimpleButton(texButtons[buttonImages[i]],
                                          guiBox->x + i*columnOffset +0.2*buttonWidth,
                                          guiBox->y + guiBox->h - 1.5*guiUnit,
                                          buttonWidth,
                                          guiUnit,
                                          captions[i],
                                          BT_PRESSONLY,
                                          SIMPLE_BUTTON_ALPHA);


    columnOffset=buttonWidth*1.75;

    for (i=ID_BTN_CANCEL_DELETE;i<=ID_BTN_CONFIRM_DELETE;i++)
        profileButtons[i]= new SimpleButton(texButtons[buttonImages[i]],
                                          guiBox->x + (guiBox->w - 3.25*buttonWidth)/2+(i-ID_BTN_CANCEL_DELETE)*columnOffset,
                                          guiBox->y + guiBox->h/2 +guiUnit*1.2,
                                          buttonWidth*1.5,
                                          guiUnit,
                                          captions[i],
                                          BT_PRESSONLY,
                                          SIMPLE_BUTTON_ALPHA);

    i=ID_BTN_DENIAL_RETURN;

    profileButtons[i]= new SimpleButton(texButtons[buttonImages[i]],
                                      guiBox->x + guiBox->w/2 - buttonWidth ,
                                      guiBox->y + guiBox->h/2 +guiUnit*1.2,
                                      buttonWidth*2,
                                      guiUnit,
                                      captions[i],
                                      BT_PRESSONLY,
                                      SIMPLE_BUTTON_ALPHA);
}

//------------------------------------------------------
void ProfileMenu::Logic(float dt, float phase)
{
    pageHandler->update(dt,phase);
    switch (mode)
    {
        case PROFILE_BROWSER: browserLogic(dt,phase);break;

        case PROFILE_EDITOR:  editorLogic(dt,phase);break;

        case PROFILE_WARNING: warningLogic(dt,phase);break;

        case PROFILE_DENIAL:
        case PROFILE_DUPLICATE_DENIAL:
                              denialLogic(dt,phase);break;
    }
}
//------------------------------------------------------
void ProfileMenu::browserLogic(float dt,float phase)
{
}
//------------------------------------------------------
void ProfileMenu::editorLogic(float dt,float phase)
{
    profileEditor->logic(dt,phase);
    //handle done and create new profile
}
//------------------------------------------------------
void ProfileMenu::warningLogic(float dt,float phase)
{
}
//------------------------------------------------------
void ProfileMenu::denialLogic(float dt,float phase)
{
}
//------------------------------------------------------
void ProfileMenu::Render(float phase)
{


    glbEffect->render();

    switch (mode)
    {
        case PROFILE_BROWSER:   browserRender(phase);break;
        case PROFILE_EDITOR:    editorRender(phase);break;
        case PROFILE_WARNING:   warningRender(phase);break;
        case PROFILE_DENIAL:
        case PROFILE_DUPLICATE_DENIAL:
                                denialRender(phase);break;
    }

    //SDL_RenderPresent(glbRenderer);//this is called by the state machine to facilitate profiling
}
//------------------------------------------------------
void ProfileMenu::browserRender(float phase)
{
    pageHandler->render(phase);
    for (int i=0;i<=ID_BTN_SELECT;i++)
        profileButtons[i]->render();
}
//------------------------------------------------------
void ProfileMenu::editorRender(float phase)
{
    profileEditor->render(phase,1.);
}
//------------------------------------------------------
void ProfileMenu::warningRender(float phase)
{
    pageHandler->render(phase,0.2);

    SDL_Rect rc={ 0,0,(int)(guiBox->w*0.8),(int)(guiBox->h*0.4)};
    rc.x = guiBox->x+ (guiBox->w-rc.w)/2;
    rc.y = guiBox->y+ (guiBox->h-rc.h)/2;

    texMsgBackground->render(rc.x,rc.y,&rc,-1,80);

    for (int i=0;i<3;i++)
    {
        texMessages[i]->render(  (screenRect->w - texMessages[i]->getWidth())/2,
                                 rc.y+(1+i)*guiUnit);
    }
    profileButtons[ID_BTN_CANCEL_DELETE]->render();
    profileButtons[ID_BTN_CONFIRM_DELETE]->render();
};
//------------------------------------------------------
void ProfileMenu::denialRender(float phase)
{
    if (mode==PROFILE_DUPLICATE_DENIAL)
        profileEditor->render(phase,0.2);
    else
        pageHandler->render(phase,0.2);

    SDL_Rect rc={ 0,0,(int)(guiBox->w*0.8),(int)(guiBox->h*0.4)};
    rc.x = guiBox->x+ (guiBox->w-rc.w)/2;
    rc.y = guiBox->y+ (guiBox->h-rc.h)/2;

    texMsgBackground->render(rc.x,rc.y,&rc,-1,80);

    for (int i=0;i<3;i++)
    {
        texMessages[i]->render(  (screenRect->w - texMessages[i]->getWidth())/2,
                                 rc.y+(1+i)*guiUnit);
    }
    profileButtons[ID_BTN_DENIAL_RETURN]->render();
}
//------------------------------------------------------
void ProfileMenu::HandleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if ( (event.type==SDL_QUIT) || (event.type==SDL_APP_TERMINATING ))
            StateMachine::SetNextState(STATE_EXIT);
        else
            if ( (event.type==SDL_APP_WILLENTERBACKGROUND) ||
                 (event.type==SDL_APP_DIDENTERFOREGROUND ))
                 {
                     isHibernating=!isHibernating;

                     if (gameSettings->musicVolume>0)
                        if (isHibernating) Mix_PauseMusic();
                            else Mix_ResumeMusic();
                 }
        else if( event.type == SDL_WINDOWEVENT )
            { //Window resize/orientation change
                if( event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED )
                {
                    setScreenRect(event.window.data1,event.window.data2);

                    glbEffect->resize();

                    SDL_RenderPresent( glbRenderer );
                }
            }
        else
            {
                bool handled =false;

                if (event.type==SDL_KEYDOWN)
                {
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_AC_BACK:
                        case SDLK_ESCAPE: goBack();handled=true;break;
                    }
                }

                else if (event.type==SDL_MOUSEMOTION)
                {
                    glbEffect->moveEffect(FPoint(event.motion.x, event.motion.y));
                }

                if (!handled)
                    switch (mode)
                    {
                        case PROFILE_BROWSER:
                            browserHandleEvents(event);break;

                        case PROFILE_EDITOR:
                            editorHandleEvents(event);break;

                        case PROFILE_WARNING:
                            warningHandleEvents(event);break;

                        case PROFILE_DENIAL:
                        case PROFILE_DUPLICATE_DENIAL:
                            denialHandleEvents(event);break;

                    };
            }
    }
}
//------------------------------------------------------
void ProfileMenu::goBack()
{
    switch (mode)
    {
        case PROFILE_BROWSER:
            StateMachine::SetNextState(STATE_TITLE);break;

        case PROFILE_EDITOR:
                delete profileEditor;
                profileEditor=NULL;
                mode = PROFILE_BROWSER;
                break;

        case PROFILE_DUPLICATE_DENIAL:
                mode = PROFILE_EDITOR;
                break;

        case PROFILE_WARNING:
                mode = PROFILE_BROWSER;
                break;
        case PROFILE_DENIAL:
                mode = PROFILE_BROWSER;
                break;
    }
}

//------------------------------------------------------
void ProfileMenu::browserHandleEvents(SDL_Event& event)
{
    if (event.type==SDL_KEYDOWN)
        switch (event.key.keysym.sym)
        {
            case SDLK_n:        addProfile();break;

            case SDLK_d:        deleteProfile();break;

            case SDLK_RETURN:   setProfile();
                                writeProfiles();
                                StateMachine::SetNextState(STATE_TITLE);
                                break;
            default:break;
        };

    bool handled=false;

    for (int i=0; i<= ID_BTN_SELECT; i++)
        handled |= profileButtons[i]->handleEvents();

    if (!handled)
        pageHandler->handleEvents();

    for (int i=0; i<= ID_BTN_SELECT; i++)
        if (profileButtons[i]->gotPressed() )
        {
            //Mix_PlayChannel( -1, sndClick, 0 );
            switch(i)
            {
                case ID_BTN_CANCEL:
                        StateMachine::SetNextState(STATE_TITLE);break;
                case ID_BTN_NEW:
                        addProfile();break;

                case ID_BTN_DELETE:
                        deleteProfile();break;

                case ID_BTN_SELECT:
                        setProfile();
                        writeProfiles();
                        StateMachine::SetNextState(STATE_TITLE);
                        break;

                default:break;
            }

           break;
        }
}
//------------------------------------------------------

void ProfileMenu::editorHandleEvents(SDL_Event& event)
{
    profileEditor->handleEvents(event);
    if (profileEditor->isDone())
    {
        std::string newName=profileEditor->getName();
        bool found=false;

        for (int i=0;i<profileVector.size();i++)
            if (profileVector[i].first==newName)
            {
                found=true;
                break;
            }
        if ( found )
        {
            mode=PROFILE_DUPLICATE_DENIAL;
            createDenialMessages();
            profileEditor->notDoneYet();
        }
        else
        {
            SDL_Rect rc = { 0, 0, (int)( guiBox->w - 0.25*guiUnit),
                         (int)(guiBox->h - 1.85*guiUnit)};

            FPoint pos( (screenRect->w-rc.w)/2. ,guiBox->y + guiUnit/8);

            profileVector.push_back( std::pair<std::string,int>(newName,0));
            pageHandler->addPage( new ProfileInfoPage(newName,pos,rc),true);
            pageHandler->doneInserting();

            mode=PROFILE_BROWSER;
            delete profileEditor;
            profileEditor=NULL;
        }
    }
    else
    if (profileEditor->isCancelled())
    {
            mode=PROFILE_BROWSER;
            delete profileEditor;
            profileEditor=NULL;
    }
}
//------------------------------------------------------

void ProfileMenu::warningHandleEvents(SDL_Event& event)
{
    bool confirmed=false;

    if (event.type==SDL_KEYDOWN)
        if (event.key.keysym.sym == SDLK_RETURN)
            {
                //DELETE the profile entry
                confirmed=true;
            }

    for (int i= ID_BTN_CANCEL_DELETE;i<=ID_BTN_CONFIRM_DELETE; i++)
        profileButtons[i]->handleEvents();

    for (int i= ID_BTN_CANCEL_DELETE;i<=ID_BTN_CONFIRM_DELETE; i++)
    {
        if (profileButtons[i]->gotPressed())
        {
            switch (i)
            {
                    case ID_BTN_CANCEL_DELETE: goBack();break;
                    case ID_BTN_CONFIRM_DELETE: confirmed=true; break;
            }
            break;
        }
    }
    if (confirmed)
    {
        int cp=pageHandler->getCurrentPageID();
        for (int i=cp;i<profileVector.size()-1;i++)
            profileVector[i]=profileVector[i+1];
        profileVector.pop_back();
        pageHandler->removeCurrentPage();

        mode=PROFILE_BROWSER;
        //delete the profile
    }


}
//------------------------------------------------------

void ProfileMenu::denialHandleEvents(SDL_Event& event)
{

    if (event.type==SDL_KEYDOWN)
        if (event.key.keysym.sym == SDLK_RETURN)
            {
                goBack();
                return;
            }
    profileButtons[ID_BTN_DENIAL_RETURN]->handleEvents();
    if (profileButtons[ID_BTN_DENIAL_RETURN]->gotPressed())
        goBack();
}
//------------------------------------------------------
void ProfileMenu::setProfile()
{
    std::string newProfile= profileVector[pageHandler->getCurrentPageID()].first;
    if (newProfile!=playerStats->getPlayerName())
    {
        playerStats->changePlayer(newProfile);

        int oldSkin=gameSettings->skin;

        gameSettings->loadSettings();
        setMusicVolume(gameSettings->musicVolume);

        if (oldSkin!=gameSettings->skin)
            skinChanged=true;
    }
}
//------------------------------------------------------
void ProfileMenu::loadProfiles()
{
    std::string filename;

    filename=glbAppPath;

    filename += "/profiles.bin";//.txt

    char namebuffer[MAX_PROFILENAME_LENGTH+1];
    int lastUsed=0;

    SDL_RWops *in=NULL;

    in = SDL_RWFromFile(filename.c_str(),"r");

    if (in==NULL)
    {
        profileVector.push_back(std::pair <std::string,int>(playerStats->getPlayerName(),0) );
        writeProfiles();
        gameSettings->writeSettings();
        return;
    }
    int numProfiles=0;

    SDL_RWread(in,&numProfiles,sizeof(int),1);

    while (numProfiles--)
    {
        SDL_RWread(in,namebuffer,sizeof(char),MAX_PROFILENAME_LENGTH);
        SDL_RWread(in,&lastUsed,sizeof(int),1);
        profileVector.push_back(std::pair<std::string,int>(namebuffer,lastUsed) );
    }
    SDL_RWclose(in);
}

//------------------------------------------------------
void ProfileMenu::writeProfiles()
{
    std::string filename;

    filename=glbAppPath;


    filename += "/profiles.bin";
    SDL_RWops *out=NULL;

    out=SDL_RWFromFile(filename.c_str(),"w");

    if (out==NULL)
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"Cannot write %s\r\n",filename.c_str());
        mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
        return;
    }
    int numProfiles=profileVector.size();
    int lastUsed;
    char namebuffer[MAX_PROFILENAME_LENGTH+1];

    SDL_RWwrite(out,&numProfiles,sizeof(int),1);

    for (int i=0;i<numProfiles;i++)
    {
        strcpy(namebuffer,profileVector[i].first.c_str());

        bool wipe=false;
        for (int j=0;j<MAX_PROFILENAME_LENGTH+1;j++)
        {
            if (namebuffer[j]==0)
                wipe=true;
            else
                if (wipe)
                    namebuffer[j]=0;
        }


        SDL_RWwrite(out,namebuffer,sizeof(char),MAX_PROFILENAME_LENGTH);

        if (profileVector[i].first !=playerStats->getPlayerName())
            lastUsed = profileVector[i].second+1;
        else lastUsed=0;

        SDL_RWwrite(out,&lastUsed,sizeof(int),1);
    }
    SDL_RWclose(out);
}

//------------------------------------------------------
void ProfileMenu::populatePages()
{
    SDL_Rect rc = { 0, 0, (int)( guiBox->w - 0.25*guiUnit),
                         (int)(guiBox->h - 1.85*guiUnit)};

    FPoint pos( (screenRect->w-rc.w)/2. ,guiBox->y + guiUnit/8);
    for (int i=0;i<profileVector.size();i++)
        pageHandler->addPage( new ProfileInfoPage( profileVector[i].first,pos,rc),
                              profileVector[i].first == playerStats->getPlayerName()
                             );

    pageHandler->doneInserting();
}
//------------------------------------------------------
void ProfileMenu::addProfile()
{
    mode = PROFILE_EDITOR;

    SDL_Rect rc={0,0,(int)(guiBox->w-0.1*guiUnit),(int)(guiBox->h - 1.7*guiUnit)};

    rc.x=guiBox->x+ (guiBox->w-rc.w)/2;
    rc.y=guiBox->y+guiUnit/20;

    profileEditor=new ProfileEditor(rc);

}
//------------------------------------------------------
void ProfileMenu::deleteProfile()
{//must verify and reload profiles, if all deleted, dummy page, new profile by default
    if (profileVector.size()==1)
    {
        mode= PROFILE_DENIAL;
        createDenialMessages();
    }
    else
    {
        mode = PROFILE_WARNING;
        createWarningMessages();
    }
}
//------------------------------------------------------
void ProfileMenu::createWarningMessages()
{
    texMessages[0]->fromText("Really DELETE this profile?");
    texMessages[1]->fromText("NOTE: you may later recover it by",gameSettings->colors[COLOR_WARNING]);
    texMessages[2]->fromText("creating a profile of the same name",gameSettings->colors[COLOR_WARNING]);
}
//------------------------------------------------------
void ProfileMenu::createDenialMessages()
{
    if (mode==PROFILE_DENIAL)
    {
        texMessages[0]->fromText("You try to delete the last profile.");
        texMessages[1]->fromText("Please create another profile",gameSettings->colors[COLOR_WARNING]);
        texMessages[2]->fromText("before deleting this one.",gameSettings->colors[COLOR_WARNING]);
    }
    else
    {
        texMessages[0]->fromText("This profile name already exists.");
        texMessages[1]->fromText("Please enter another name",gameSettings->colors[COLOR_WARNING]);
        texMessages[2]->fromText("for the new profile.",gameSettings->colors[COLOR_WARNING]);
    }
}


//==============================================================

ProfileInfoPage::ProfileInfoPage(std::string titleStr, const FPoint& pos, const SDL_Rect& rect)
{
    profileStats = new LevelStats(titleStr,true);

    initPage(titleStr,pos,rect,255);

    loadImages();

    initControls();

    setMessageRects();
}
//------------------------------------------------------
ProfileInfoPage::~ProfileInfoPage()
{
    delete profileStats;
/*
    for (int i=0;i<buttonVector.size();i++)
    {
        delete buttonVector[i];
        buttonVector[i]=NULL;
    }
    */
    buttonVector.clear();

    delete texPageTitle;

    //texButton=NULL;

    texMsgBackground = NULL;

    for (int i=0;i<texInfoVector.size();i++)
    {
        delete texInfoVector[i];
        texInfoVector[i]=NULL;
    }
    texInfoVector.clear();
    recsVector.clear();
}
//------------------------------------------------------
void ProfileInfoPage::loadImages()
{
    imageHandle = glbTexManager->insertTexture("page_background.png",SKIN_UNIVERSAL,true);

    texMsgBackground = glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("message_background.png",SKIN_UNIVERSAL,true));

   // texButton = glbTexManager->getTexturePtr(glbTexManager->insertTexture("buttons/buttons_green.png",SKIN_UNIVERSAL,true));

    texPageTitle = new BaseTexture();
    texPageTitle ->fromText( title,gameSettings->colors[COLOR_PAGE_TITLE],glbFont);

    char msg[60];
    int li=0;//last item


    sprintf(msg,"Total levels passed: %d",profileStats->getTotalLevels());
    texInfoVector.push_back( new BaseTexture() );
    texInfoVector[li++]->fromText(msg);

    for (int w=0;w<NUM_DIMENSIONS;w++)
    {
        int numLevels=profileStats->getMaxLevel(w);
        if (numLevels==0)
            continue;

        sprintf(msg,"       Area width %d : %d",gameSettings->getWidthValueByIndex(w) ,numLevels);
        texInfoVector.push_back( new BaseTexture() );
        texInfoVector[li++]->fromText(msg);
    }
    int totalsec,sec,mins,hr;
    totalsec=profileStats->getTotalPlayTime();

    sec=totalsec;
    hr =sec /3600;
    sec-=hr*3600;
    mins=sec/60;
    sec-=mins*60;

    sprintf(msg,"Total playTime: %dh %02dm %02ds",hr,mins,sec);
    texInfoVector.push_back( new BaseTexture() );
    texInfoVector[li++]->fromText(msg);

    float totalShots=profileStats->getTotalShots();

    sprintf(msg,"Total Shots: %.0f",totalShots);
    texInfoVector.push_back( new BaseTexture() );
    texInfoVector[li++]->fromText(msg);

    if (totalShots>0)
    {
        sprintf(msg,"    Average time per shot: %.2fs",totalsec/totalShots);
        texInfoVector.push_back( new BaseTexture() );
        texInfoVector[li++]->fromText(msg,gameSettings->colors[COLOR_TEXT1]);
    }

    texInfoVector.push_back( new BaseTexture() );
    texInfoVector[li++]->fromText("Unlocked:");

    std::string strUnlocks="";
    int numUnl=0;
    int sep=0;
    char* sepStr[2]={ ",   ", ",\n" };

    for (int i=0;i<NUM_UNLOCKABLES;i++)
    {
        if (profileStats->getUnlockableStatus(i))
        {
            if (numUnl>0)
                strUnlocks += sepStr[sep];
            strUnlocks += profileStats->getUnlockableName(i);
            if ( (i==UNL_DIM16) || (i==UNL_CHAOS) || (i==UNL_SKIN_GEOMETRIC2)|| (i==UNL_MATCH6)
#ifdef EXTRA_GAME_MODES
                    || (i==UNL_MODE_SCORE)
#endif // EXTRA_GAME_MODES
                )
                sep=1;
            else
                sep=0;
            ++numUnl;
        }
    }
    texInfoVector.push_back( new BaseTexture() );
    texInfoVector[li++]->fromText(strUnlocks, gameSettings->colors[COLOR_NOBG3] ,glbMessageFont, (int)(0.72*rc.w) );


}
//------------------------------------------------------
void ProfileInfoPage::initControls()
{
    buttonVector.clear();
    /*
    int buttonWidth=0.4*guiBox->w;

    int offset=rc.w/8;

    buttonVector.push_back( new SimpleButton(texButton,
                                          offset,
                                          guiUnit*10,
                                          buttonWidth,
                                          guiUnit*0.8,
                                          "Show more...",
                                          BT_PUSHONLY,
                                          SIMPLE_BUTTON_ALPHA)
                           );
    for (int i=0;i<buttonVector.size();i++)
        buttonVector[i]->setOnScrollPage();
        */
}
//--------------------------------------------------------
void ProfileInfoPage::setMessageRects()
{
    int offsetX=rc.w/8;
    int offsetY=(int)guiUnit;
    int i;

    for (i=0;i<texInfoVector.size()-1;i++)
        recsVector.push_back({offsetX,(int)((1.2+i)*offsetY),(int)(3*rc.w/4),(int)(guiUnit*0.8)});

    i=texInfoVector.size()-1;

    recsVector.push_back({offsetX,(int)((1.+i)*offsetY),(int)(3*rc.w/4),(int) std::max( guiUnit*0.8 , texInfoVector[i]->getHeight()*1.02 ) });
}
//------------------------------------------------------
void ProfileInfoPage::render(float alphaModifier )
{
//    renderTitle();
    if (!isVisible()) return;

    renderTitle(alphaModifier);
    int i;

    glbTexManager->getTexturePtr(imageHandle)->render(position.x,position.y,
                                                      &rc,-1,alphaModifier*50);

/*
    for (i=0;i<buttonVector.size();i++)
        if (buttonVector[i]->getState()!=BUTTONSTATE_DISABLED)
            buttonVector[i]->render( position,alphaModifier);
*/
    for (i=0;i<texInfoVector.size();i++)
    {
        texMsgBackground->render(position.x+recsVector[i].x,
                           position.y+recsVector[i].y-guiUnit*0.1,
                           &recsVector[i],-1,50*alphaModifier);

        texInfoVector[i]->render(position.x+recsVector[i].x + recsVector[i].w*0.02,
                           position.y+recsVector[i].y,
                           NULL,-1,255*alphaModifier);
    }
}
//------------------------------------------------------
bool ProfileInfoPage::handleEvents()
{

    bool handled =false;

    //for (int i=0;i<buttonVector.size();i++)
    //    handled |= buttonVector[i]->handleEvents(position);

    return handled;
}
//===================================================================================

ProfileEditor::ProfileEditor(const SDL_Rect& rect)
{
    rc=rect;
    rc.h*=0.6;

    name="";
    done=false;
    cancelled=false;

    loadImages();
    initControls();
}
//-------------------------------------------
ProfileEditor::~ProfileEditor()
{
    int i;
    for (i=0;i<keyVector.size();i++)
    {
        delete keyVector[i];
        keyVector[i]=NULL;
    }
    keyVector.clear();

    texMsgBackground=NULL;

    for (i=0;i<4;i++)
        texButtons[i]=NULL;

}
//---------------------------------------------------------
bool ProfileEditor::isDone()
{
    return done;
}
//---------------------------------------------------------
bool ProfileEditor::isCancelled()
{
    return cancelled;
}

//---------------------------------------------------------
void ProfileEditor::notDoneYet()
{
   done=false;
}
//---------------------------------------------------------
std::string ProfileEditor::getName()
{
    return name;
}
//---------------------------------------------------------
void ProfileEditor::loadImages()
{
    texMsgBackground = glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("message_background.png",SKIN_UNIVERSAL,true));

    texButtons[0]=glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("buttons/buttons_orange.png",SKIN_UNIVERSAL,true));
    texButtons[1]=glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("buttons/buttons_yellow.png",SKIN_UNIVERSAL,true));
    texButtons[2]=glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("buttons/buttons_green.png",SKIN_UNIVERSAL,true));
    texButtons[3]=glbTexManager->getTexturePtr(
                        glbTexManager->insertTexture("buttons/buttons_cyan.png",SKIN_UNIVERSAL,true));

    texName = new BaseTexture;

    setNameImage();
}
//---------------------------------------------------------
void ProfileEditor::setNameImage()
{
    texName->fromText(name,glbTextColor,glbFont);
}
//---------------------------------------------------------
void ProfileEditor::initControls()
{
    int buttonSize=(int)((rc.w-guiUnit/8)/10);
    int offset = (rc.w -10*buttonSize+1)/2;

    int buttonY = rc.y + 2.5*guiUnit;
    int buttonX = rc.x+offset;
    int line=0;

    int termKey[3]={EDKEY_0,EDKEY_P,EDKEY_L};
    int i;

    for (i=0;i<=EDKEY_M;i++)
    {
        int texIndex= ((i<=EDKEY_0)?0:1);

        keyVector.push_back( new SimpleButton (texButtons[texIndex],buttonX,
                                                  buttonY,buttonSize-1,buttonSize-1,keyValues[i],
                                                  BT_PRESSONLY,SIMPLE_BUTTON_ALPHA) );
        buttonX+=buttonSize;

        if ( ( line<3 ) && ( i==termKey[line] ) )
        {
            buttonY+=buttonSize*1.2;
            buttonX=rc.x+offset;
            line++;
            if (line>1) buttonX+=(line-1)*buttonSize/2;

        }
    }
    keyVector.push_back( new SimpleButton (texButtons[2],buttonX,
                                                  buttonY,buttonSize-1,buttonSize-1,"<-",
                                                  BT_PRESSONLY,SIMPLE_BUTTON_ALPHA));
    buttonY+=buttonSize*1.2;
    buttonX=rc.x+offset;

    keyVector.push_back( new SimpleButton (texButtons[3],buttonX,
                                                  buttonY,2.5*buttonSize-1,buttonSize-1,"Cancel",
                                                  BT_PRESSONLY,SIMPLE_BUTTON_ALPHA));

    keyVector.push_back( new SimpleButton (texButtons[2],buttonX+buttonSize*2.5,
                                                  buttonY,buttonSize*5 -1,buttonSize-1,"---",
                                                  BT_PRESSONLY,SIMPLE_BUTTON_ALPHA));

    keyVector.push_back( new SimpleButton (texButtons[3],buttonX+buttonSize*7.5,
                                                  buttonY,2.5*buttonSize -1,buttonSize -1,"Done",
                                                  BT_PRESSONLY,SIMPLE_BUTTON_ALPHA));
    keyVector[EDKEY_ENTER]->disable();
    keyVector[EDKEY_BACKSPACE]->disable();
}
//---------------------------------------------------------
bool ProfileEditor::handleEvents(SDL_Event& event)
{
    int i;
    bool update=true;
    int oldlength=name.length();

    for (i=0;i<keyVector.size();i++)
        keyVector[i]->handleEvents();

    for (i=0;i<keyVector.size();i++)
    {
        if (keyVector[i]->gotPressed())
        {
            switch (i)
            {
                case EDKEY_ENTER:       done=true;return true;
                case EDKEY_CANCEL:      name.clear();cancelled=true;return true;

                case EDKEY_BACKSPACE:   if (name.length()>0)
                                            name.erase(--(name.end()));
                                        else
                                            update=false;
                                        break;

                case EDKEY_SPACE:       if (name.length()<MAX_PROFILENAME_LENGTH)
                                            name.push_back('-');
                                        else update=false;
                                            break;
                default: if (name.length()<MAX_PROFILENAME_LENGTH)
                                name+=keyValues[i];
                         else   update=false;
                            break;
            }

            if (update)
                setNameImage();

            if (name.length()>0)
            {
                if (oldlength==0)
                {
                    keyVector[EDKEY_ENTER]->enable();
                    keyVector[EDKEY_BACKSPACE]->enable();
                }
            }
            else
            {
                if (oldlength>0)
                {
                    keyVector[EDKEY_ENTER]->disable();
                    keyVector[EDKEY_BACKSPACE]->disable();
                }
            }
            return true;
        }
    }

    return false;
}
//---------------------------------------------------------
void ProfileEditor::logic(float dt, float phase)
{

}
//---------------------------------------------------------
void ProfileEditor::render(float phase,float alphaModifier)
{
    texMsgBackground->render(rc.x,rc.y,&rc,-1,80*alphaModifier);

    SDL_Rect rcName={rc.x+(int) (guiUnit/2),rc.y+(int) (guiUnit/2),rc.w-(int)guiUnit,(int) (guiUnit*1.2)};
    texMsgBackground->render(rcName.x,rcName.y,&rcName,-1,80);//*alphaModifier);

    if (name.length() > 0)
    {
        if (texName->getWidth()<rcName.w)
            texName->render(rcName.x+(rcName.w-texName->getWidth())/2,
                        rcName.y+(rcName.h-texName->getHeight())/2,
                        NULL,-1,255);//*alphaModifier);
        else
        {
            int rcHeight= rcName.h;

            rcName.h=texName->getHeight()*rcName.w/texName->getWidth();
            texName->render(rcName.x,
                        rcName.y+(rcHeight-rcName.h)/2,
                        &rcName,-1,255);//*alphaModifier);
        }
    }

    for (int i=0;i<keyVector.size();i++)
        keyVector[i]->render(FPoint(0,0),alphaModifier);

}


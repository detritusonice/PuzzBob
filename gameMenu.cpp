#include "base.h"
#include "basefuncs.h"
#include "texture.h"
#include "button.h"
#include "levelStats.h"
#include "gameMenu.h"
#include "gameSettings.h"
#include <cstdio>

GameMenu::GameMenu( )
{
    done=false;
    level=0;

    action=ACTION_PLAYLEVEL;


    texMessage=new BaseTexture;

    initButtons();

}

GameMenu::~GameMenu()
{
    delete texMessage;//->destroyInstance();

    for (int i =0;i<NUM_GAMEMENU_BUTTONS;i++)
        delete buttons[i];
}


void GameMenu::initButtons()
{
    const char* strTitles[NUM_GAMEMENU_BUTTONS]={"New Game","Continue","Max Level","Select"};

    int offset = guiUnit/2;
    int buttonWidth = (guiBox->w -offset)/2.;
    int buttonHeight = (guiBox->h -3*offset)/2.;

    BaseTexture* texButton = glbTexManager->getTexturePtr(
                                    glbTexManager->insertTexture("levelSelector.png",gameSettings->skin,true));



    for (int i=0;i<NUM_GAMEMENU_BUTTONS-1;i++)
        buttons[i]= new LevelButton( NULL,
                                        guiBox->x  +(buttonWidth+offset)*(i&1),//%2
                                        guiBox->y  + offset +(buttonHeight+offset)*(i>>1),//i/2
                                        buttonWidth,buttonHeight,
                                        strTitles[i],BT_PUSHONLY,
                                        GLASS_BUTTON_ALPHA);

    ((LevelButton*)buttons[ID_NEWGAME])->setLevel(0);



    char fname[500];
    sprintf(fname,"%s/%s%02d.sav",glbAppPath,playerStats->getPlayerName().c_str(),  gameSettings->getWellWidthValue()  );



    ((LevelButton*)buttons[ID_CONTINUE])->setFile( fname );

    ((LevelButton*)buttons[ID_MAXLEVEL])->setLevel(playerStats->getMaxLevel());

    buttons[ID_SELECT]=new GlassButton( texButton,
                                        guiBox->x +(buttonWidth+offset),
                                        guiBox->y +offset +(buttonHeight+offset),
                                        buttonWidth,buttonHeight,
                                        strTitles[ID_SELECT],BT_PUSHONLY,
                                        GLASS_BUTTON_ALPHA);
}

void GameMenu::handleEvents(SDL_Event& event)
{
    int i;
    for (i=0;i<NUM_GAMEMENU_BUTTONS;i++)
        buttons[i]->handleEvents();


    for (i=0;i<NUM_GAMEMENU_BUTTONS;i++)
        if (buttons[i]->getState() == BUTTONSTATE_PUSHEDHOVER)
        {
            switch (i)
            {
                case ID_NEWGAME: {action=ACTION_PLAYLEVEL;
                                    level=0;
                                    break;}
                case ID_MAXLEVEL: {action=ACTION_PLAYLEVEL;
                                    level = ((LevelButton*)(buttons[i]))->getLevel();
                                    break;}
                case ID_CONTINUE: {action=ACTION_LOAD_SAVED;
                                    level=0;
                                    break;}
                case ID_SELECT: {action=ACTION_SELECTLEVEL;
                                level= ((LevelButton*)(buttons[ID_CONTINUE]))->getLevel();
                                break;}
            }
            done=true;
        }
}

void GameMenu::render(float phase)
{
    for (int i=0;i<NUM_GAMEMENU_BUTTONS;i++)
        buttons[i]->render();
}

void GameMenu::logic(float dt, float phase)
{

}

bool GameMenu::isDone(int *retAction, int *retLevel)
{
    *retAction=action;
    *retLevel=level;

    return done;
}


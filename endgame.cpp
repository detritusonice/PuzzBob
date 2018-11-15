#include "base.h"
#include "endgame.h"
#include "myMath.h"

#include "texture.h"
#include "button.h"
#include "levelStats.h"
#include "basefuncs.h"
#include "gameSettings.h"
#include "localEffects.h"

#include <cstdio>

//===================================================================================================

EndGame::EndGame(const SDL_Rect& rcWell, int level, bool victorious, bool scoreRecord, bool timeRecord, bool shotRecord, int unlockedFeature)
{
    rc=rcWell;
    playLevel=false;
    done = false;
    pressedContinue=false;
    timePassed=0;

    lastLevel=level;
    hasWon=victorious;

    for (int i=0;i<3;i++)
        showStar[i]=false;

    unlocked=unlockedFeature;


    if (hasWon)
    {
        mode = ENDGAME_VICTORY_INFO;
        newScore = scoreRecord;
        newTime = timeRecord;
        newShots = shotRecord;

        if (newTime)
            showStar[GM_TIMELIMIT-1]=playerStats->hasBeatenModeStat(level,GM_TIMELIMIT);//GM_NORMAL at place 0;

        if (newShots)
            showStar[GM_SHOTLIMIT-1]=playerStats->hasBeatenModeStat(level,GM_SHOTLIMIT);

        if (newScore)
            showStar[GM_SCORELIMIT-1]=playerStats->hasBeatenModeStat(level,GM_SCORELIMIT);

    }
    else
    {
        mode = ENDGAME_LOSS_INFO;
        newScore = false;
        newTime = false;
        newShots = false;
    }

    loadImages();

    initButtons();

    createMessages();
    createStaticMessageImage();

    createUnlockedEffect();

    if (unlockedEffect!=NULL)
        unlockedEffect->start();

}

//------------------------------------------------------------------------------------------------------

EndGame::~EndGame()
{
    int i;
    for (i=0;i<NUM_ENDGAME_BUTTONS;i++)
        delete buttons[i];

    for (i=0;i<NUM_MESSAGES;i++)
        if (msg[i]!=NULL)
            delete msg[i];

    texButtons=NULL;
    texBackground=NULL;
    texUnlocked=NULL;

    for (i=0;i<3;i++)
        texStars[i]=NULL;

    if (unlockedEffect!=NULL)
        delete unlockedEffect;

    if (texTotalMessages!=NULL)
        delete texTotalMessages;
}

//------------------------------------------------------------------------------------------------------

void EndGame::initButtons()
{
    const char* captions[]={
                            (mode==ENDGAME_VICTORY_INFO)?
#ifdef DEMO_GAME
                            ( (lastLevel==MAX_DEMO_LEVEL)?"Replay Level":"NextLevel")
#else
                            ( ((!isLegitimateCopy)&&(lastLevel==MAX_DEMO_LEVEL))?"Replay Level":"NextLevel")
#endif // DEMO_GAME

                            :"Replay level",
                            "Select another"};

    for (int i=0;i<NUM_ENDGAME_BUTTONS;i++)
    {
        buttons[i]= new SimpleButton(texButtons,rc.x + rc.w/4,
                                     rc.y + rc.h*0.7 +1.2*i*guiUnit ,
                                     rc.w/2,guiUnit,
                                     captions[i],
                                     BT_PUSHONLY,SIMPLE_BUTTON_ALPHA  );
    }

}

//------------------------------------------------------------------------------------------------------

void EndGame::loadImages()
{

    texBackground = glbTexManager->getTexturePtr(glbTexManager->insertTexture("message_background.png",SKIN_UNIVERSAL,true));
    texButtons    = glbTexManager->getTexturePtr(glbTexManager->insertTexture("buttons/buttons_green.png",SKIN_UNIVERSAL,true));

    for (int i=0;i<NUM_MESSAGES;i++)
        msg[i]= new BaseTexture;

    texTotalMessages=NULL;
    texUnlocked=NULL;

    unlockedPos=FPoint(guiBox->x+guiBox->w/2,guiBox->y+guiBox->h/2);

    int summonUnlocked=unlocked&7;

    if (mode==ENDGAME_VICTORY_INFO)
    {
        std::string unlockedName="";
        switch (summonUnlocked)//number of summon unlocked is in these 3 bits
        {
            case UNL_BOMB:          unlockedName="balls/ball_bomb.png";break;

            case UNL_COLORSHIFTER:  unlockedName="balls/ball_shifter.png";break;

            case UNL_TIMESTOP:       unlockedName="balls/timestop.png"; break;

            case UNL_PAINTER:       unlockedName="balls/ball_painter.png";break;

            case UNL_LEVELER:       unlockedName="balls/ball_leveler.png";break;

            case UNL_CHROME:        unlockedName="balls/ball_chrome.png";break;
        }

        if ( ((summonUnlocked) >=UNL_BOMB) && ((summonUnlocked) <=UNL_CHROME))
                texUnlocked =glbTexManager->getTexturePtr(glbTexManager->insertTexture(unlockedName,SKIN_UNIVERSAL,true));

        if (showStar[GM_TIMELIMIT-1])
            texStars[GM_TIMELIMIT-1]= glbTexManager->getTexturePtr(glbTexManager->insertTexture("star_cyan.png",SKIN_UNIVERSAL,true));

        if (showStar[GM_SHOTLIMIT-1])
            texStars[GM_SHOTLIMIT-1]= glbTexManager->getTexturePtr(glbTexManager->insertTexture("star_green.png",SKIN_UNIVERSAL,true));

        if (showStar[GM_SCORELIMIT-1])
            texStars[GM_SCORELIMIT-1]= glbTexManager->getTexturePtr(glbTexManager->insertTexture("star_yellow.png",SKIN_UNIVERSAL,true));
    }

}

//------------------------------------------------------------------------------------------------------

void EndGame::createMessages()
{
    for (int i=0;i<NUM_MESSAGES;i++)
        msg[i]->fromText(" ");

    if (mode==ENDGAME_VICTORY_INFO)
    {

        if ((lastLevel>=MAX_DEMO_LEVEL)
#ifndef DEMO_GAME
                &&(!isLegitimateCopy)
#endif
           )
        {
                    //SDL_Color specialColor={225,130,0};
              msg[0]->fromText("That was the last Free Version Level. - Well Done.\n",glbTextColor,glbFont,rc.w-guiUnit);
              msg[2]->fromText("You can re-play all past levels by using the game menu Select button.\n\n\
You can also try changing the GamePlay Settings and start beating the levels again.\n\n\
The full version contains hundreds of handcrafted, plus millions of generated levels.",
                                gameSettings->colors[COLOR_TEXT4],glbMessageFont,rc.w-guiUnit);
               msg[7]->fromText("If you liked PuzzBob, please consider\nbuying the full version, and kindly remember to Rate this game.\n\nThanks for playing.",
                                 gameSettings->colors[COLOR_TEXT2],glbMessageFont,rc.w-guiUnit);
         }
            else
            {
                const char* text1[4]={"Well Done!","Great!","Fantastic!","Excellent!"};

                const int msg1Lines=7;
                const char* msg1[msg1Lines]={
                                        "You finished level %d\n",
                                        "Level %d didn't stand a chance\n",
                                        "That's about level %d.\n",
                                        "Level %d is a thing of the past.\n",
                                        "I knew you could beat level %d\n",
                                        "Level %d is finished.\n",
                                        "To beat level %d is no small feat.\n"
                                };

                int index = 0 + (newScore?1:0)+(newTime?1:0)+(newShots?1:0);

                msg[0]->fromText(text1[index],glbTextColor,glbFont);

                char str[80];

                index=genRand10000()%msg1Lines;

                sprintf(str,msg1[index],lastLevel+1);
                msg[1]->fromText(str,gameSettings->colors[COLOR_TEXT4],glbMessageFont, rc.w-guiUnit);


                int nextFree=2;


                if (newTime)
                {
                    starMessageIndex[GM_TIMELIMIT-1]=nextFree;
                    int mintime=playerStats->getMinTime(lastLevel);
                    sprintf(str,"New Shortest Time Record:    %d.%03d s",mintime/1000,mintime%1000);
                    msg[nextFree++]->fromText(str,gameSettings->colors[COLOR_TEXT3]);
                }

                if (newShots)
                {
                    starMessageIndex[GM_SHOTLIMIT-1]=nextFree;
                    sprintf(str,"New Minimum Shots Record:   %d",playerStats->getMinShots(lastLevel));
                    msg[nextFree++]->fromText(str,gameSettings->colors[COLOR_TEXT3]);
                }

                if (newScore)
                {
                    starMessageIndex[GM_SCORELIMIT-1]=nextFree;
                    sprintf(str,"New Round Score Record:      %d",playerStats->getRoundScore(lastLevel));
                    msg[nextFree++]->fromText(str,gameSettings->colors[COLOR_TEXT3]);
                }

           const int tipLines=21;
           const char* tip[tipLines]={//first tip will appear at level tiplines because bomb is unlocked after level 1
                                "Tip:\n\nChrome balls drop everything they touch before they reach a wall, where they finally explode.\n\nThey belong to the class called Power Balls, and are rare to appear.",
                                "Tip:\n\nThe Geometric and Patterned Themes are better if for any reason the ball colors do not come out right.\n\nJust go to Options, scroll to Themes, and select the one that serves you better.",
                                "Tip:\n\nDid you know you can switch colors with the next ball?\n\nJust tap on it, or tap and aim in one move.\n\nYou can switch colors as many times as you want.",
                                "Tip:\n\nColorShifters find the longest same-colored chain they touch and adopt that color.\n\nThey can bridge multiple sub-match chains.\n\nResulting matches are dropped.",
                                "Tip:\n\nSometimes tapping to shoot serves you better, but for precision aiming, try moving your finger towards the desired spot.\n\nThe Aiming aid also helps a lot.",
                                "Tip:\n\nYou can cancel a shot while aiming by getting your finger lower than the launch point.\n\nYou can also move it outside the playing area, then lift it.\n\nKeep away from the touch-screen edges.",
                                "Tip:\n\nPower Balls can be unlocked as you progress through the game. You can summon an available  power ball and it will appear as the next ball.\n\nUse them wisely, because they won't be available in the next levels.",
                                "Tip:\n\nTap on the user name in the main menu to access the Profiles page.\n\nCreate as many profiles as you want, each with its own progress, settings, stats and saved game.",
                                "Tip:\n\nYou can browse and play any of the levels you have unlocked by using the Select button in the Play menu.",
                                "Tip:\n\nThe Level Select page is four way scrollable.\n\nRight and Left will get the next and previous page of levels, Up and Down will skip a number of pages.\n\nBoth directions are cyclic.",
                                "Tip:\n\nDuring play, you can exit the level at any time- your game will be saved and be available to continue when you return.\n\nIf you exit when you're not actively playing, nothing is touched.",
                                "Tip:\n\nWhen precision aiming, please keep away from the physical screen edges.\n\nScreen-edge exits can accidentally trigger a ball launching and ruin your shot.",
                                "Tip:\n\nMany menu screens have multiple pages and are finger-scrollable, just try them out.\n\nExamples are the Options, Profiles and Level Select screens.",
                                "Tip:\n\nYou could make the game more fun by manipulating the GamePlay Settings.\n\nAdjust the Randomness, Pressure, Match number, use of aids and special balls to suit your favorite playing style.",
                                "Tip:\n\nWhen you choose Continue from the Play menu, you load the saved game AND the GamePlay Settings of that game.\n\nYour current settings will change to those older values if they differ, and remain as such until you change them back.",
                                "Tip:\n\nAfter all the handmade levels are passed, there are millions of unique generated levels.\n\nWhat's more, you can browse and select them too, as if they were locally stored.",
                                "Tip:\n\nCreate a profile for each playing style you want, and give it an appropriate name.\n\nThat way, you'll have all the settings ready for a relaxed, fast, challenging, puzzle, you-name-it session, and access it in seconds.",
                                "Tip:\n\nIf you like PuzzBob, please rate the game with five stars and leave a nice comment.\n\nIn case you already have, thank you very, very much for your support.",
                                "Tip:\n\nIf you come across any bugs, glitches, crashes or weird behaviors in general, please use the e-mail address in the about screen and the game page to report the incident.\n\nThanks in advance.",
                                "Tip:\n\nPainters shade up to a maximum number of neighboring balls to the color of the ball they collide with.\n\nThey can be quite powerful, especially when you take into account the color of the next ball.",
                                "Tip:\n\nLevelers explode all the balls on the row they land on. Consequently, all balls below them are dropped.\n\nLike all Power Balls, they can be used at most once per level.",
                                };

           const char* item[NUM_SUMMONS+NUM_UNLOCKABLES]={
                                //============================SUMMONS============================
                                "",//this should never be displayed
                                "The Bomb\n\n",
                                "The ColorShifter\n\n",
                                "Time Stop\n\n",
                                "The Painter\n\n",
                                "The Leveler\n\n",
                                "The Chrome Ball\n\n",
                                //==============================UNLOCKABLES=======================
                                //"The Aiming Aid\n\n",
                                "[ True Random ] Setting\n\n",
                                "[ Chaos ] Pressure Setting\n\n",
                                "[ Dark II ] Theme\n\n",
                                "[ Light II ] Theme\n\n",
                                "[ Patterned II ] Theme\n\n",
                                "[ Geometric II ] Theme\n\n",
                                "[ Match 4 ] GamePlay Setting\n\n",
                                "[ Match 5 ] GamePlay Setting\n\n",
                                "[ Match 6 ] GamePlay Setting\n\n",
                                "[ 10x17 ] Playing Area Size\n\n",
                                "[ 12x21 ] Playing Area Size\n\n",
                                "[ 14x24 ] Playing Area Size\n\n",
                                "[ 16x28 ] Playing Area Size\n\n",
#ifdef EXTRA_GAME_MODES
                                "[ Play vs Time limit ] Mode\n\n",
                                "[ Play vs Shots limit ] Mode\n\n",
                                "[ Play vs Score limit ] Mode\n\n",
                                "[ Level Editor ]\n\n",//\nSelect your best and send them over\nWe shall choose the best user-created levels and enter your name and number of levels admitted in the about screen",
#endif
                                };

           const char* descr[NUM_SUMMONS+NUM_UNLOCKABLES]={
                                "",
                                "This Special ball explodes upon landing, obliterating all its immediate neighbors.",
                                "This Special ball, upon landing, adopts the color that produces the longest same-colored tuple.",
                                "In the Called Shot window, tap on the hourglass icon and time will halt until you launch a ball.",
                                "This Power Ball will paint a number of neighboring balls with the color of the ball it collided with.",
                                "This Power Ball will explode all others in the horizontal line it lands.\n\nTip: Try to place it as high as you can.",
                                "This Power Ball will obliterate every ball it touches until it hits a wall, whereupon it itself will explode.",
                                //"Increases you precision by showing you the trajectory and final placement of the ball shot.",
                                "In this mode generated balls are created independently of the existing ball colors in the playing area.",
                                "An ultrafast mode to challenge the players demanding fast-paced thought and action.",
                                "A highly transparent remake of the Dark theme, now available in the second Themes page.",
                                "A highly transparent remake of the Light theme, now available in the second Themes page.",
                                "A highly transparent remake of the Patterned theme, now available in the second Themes page.",
                                "A highly transparent remake of the Geometric theme, now available in the second Themes page.",
                                "Increase game challenge by setting balls to drop once four or more of them match colors.",
                                "Increase game challenge even more by setting balls to drop only when five or more of them match colors.",
                                "Even greater strategic challenge available: You must match six or more balls of the same color to drop them.",
                                "As balls shrink in diameter, challenge increases. More time is needed to complete a level, combos can be greater and more complex.",
                                "Will you conquer this board size? Things get even better with an appropriate match setting.",
                                "Board gets bigger. Previously prohibitive pressure modes might very well be within reach now.",
                                "Maximum board size. Intricate combinations, greater satisfaction at level completion.",
#ifdef EXTRA_GAME_MODES
                                "You are a speedy player, keep it up with a shown time limit and live countdown, by selecting this game mode.",
                                "You are a strategic player, keep it up with shown shots limit and countdown, by selecting this game mode.",
                                "You are a competitive player, keep it up with shown minimum score limit and points remaining, by selecting this game mode.",
                                "You may now create, modify and save your own levels.",//\nSelect your best and send them over\nWe shall choose the best user-created levels and enter your name and number of levels admitted in the about screen",
#endif
                                };

            std::string strUnlocked="";
            if (unlocked<NUM_SUMMONS)
            {
                if (unlocked==UNL_NONE)
                    strUnlocked=tip[lastLevel%tipLines];
                else
                {
                    strUnlocked="Congratulations: You have unlocked:\n\n";
                    strUnlocked+=item[unlocked];
                    strUnlocked+=descr[unlocked];
                }
            }
            else
            {
                int numUnlocked=0;
                int k=0;

                strUnlocked="Congratulations: You have unlocked:\n\n";
                if (unlocked&7)//three lowest bits are summon number
                {
                    strUnlocked+=item[unlocked&7];
                    numUnlocked++;
                    k++;
                }
                int unlockable= unlocked>>3;

                for (int i=0;i<NUM_UNLOCKABLES;i++)
                {
                    if (unlockable & (1<<i))
                        numUnlocked++;
                }

                for (int i=0;i<NUM_UNLOCKABLES;i++)
                {
                    if (unlockable & (1<<i))
                    {

                        if (k>0) strUnlocked +=" + ";
                        strUnlocked +="the ";
                        strUnlocked += item[ i +NUM_SUMMONS];

                        k++;
                        if (numUnlocked==1)
                            strUnlocked += descr[ i +NUM_SUMMONS];
                    }
                }
            }

            msg[nextFree]->fromText(strUnlocked.c_str(),gameSettings->colors[COLOR_TEXT2],glbMessageFont, rc.w-guiUnit);

            unlockedPos.x= rc.x + rc.w*0.6;
            unlockedPos.y= rc.y +guiUnit*(nextFree+1.6);

            rcUnlocked={unlockedPos.x-guiUnit/2,unlockedPos.y-guiUnit/2,guiUnit,guiUnit};

        }
    }
    else if (mode==ENDGAME_LOSS_INFO)
    {
            const char* text1[4]={"Bummer!","Drat!","Bah!","What?"};
            int index = genRand10000()%4;

            //SDL_Color colorTitle={0,0,100};
            //SDL_Color colorText1={0,0,0};

            const int msg1Lines=7;
            const char* msg1[msg1Lines]={
                                        "OK, level %d is harder than I thought.\n",
                                        "Really, did the infamous level %d beat you?\n",
                                        "Level %d got the better of you. . .\n",
                                        "Oh, its that level %d again. . .\n",
                                        "Level %d has done it again.\n",
                                        "Is level %d too difficult?\n",
                                        "Level %d turned out somewhat hard.\n"
                                };


            const int msg2Lines=15;
           const char* msg2[msg2Lines]={
                                "Perhaps I should fire the level editor.\n\n...Come to think of it-I really shouldn't.",
                                "Seriously now, maybe you're just tired.\n",
                                "I know...\n\nThe Random Number Generator is to blame.",
                                "Its Mersenne Twister. That's the culprit!\n",
                                "You will overcome it eventually, just keep trying.\n",
                                "Come on. Give it another shot.\nYou can do it!\n",
                                "Perhaps play some other level,\nthen return to this one.",
                                "Some Level - Settings combinations\nare really impossible to pass.\n\nPerhaps some fiddling with the GamePlay Settings will get the job done.",
                                "GamePlay Settings are your friend.\n\nMaybe you should visit some time.",
                                "Keep your screen spotless.\nYour aim is greatly improved.",
                                "It might be that the graphics Skin does not help.\n\n Go to Options, scroll to Skins and select the one that suits you better.\n",
                                "If your framerate suffers, go to Options, scroll to Effects and disable all.\n\nThen try enabling and testing them one by one.\n",
                                "If you don't like seeing stats while you play, go to Options and scroll to GameInfo, then disable away. . .\n",
                                "Did you know you can switch colors with the next ball?\n\nJust tap on it, or tap and aim in one move.\n\nYou can switch colors as many times as you want.",
                                "You may aim better if you touch and move your finger across the screen.\n\nCancel the shot by getting your finger lower than the launch point.\n",
                                };

            msg[0]->fromText(text1[index],glbTextColor,glbFont);

            char str[100];

            index=genRand10000()%msg1Lines;

            sprintf(str,msg1[index],lastLevel+1);
            msg[1]->fromText(str,gameSettings->colors[COLOR_TEXT4],glbMessageFont, rc.w-guiUnit);

            index=genRand10000()%msg2Lines;

            msg[3]->fromText(msg2[index],gameSettings->colors[COLOR_TEXT2],glbMessageFont, rc.w-guiUnit);

    }

}
//------------------------------------------------------------------------------------------------------
bool EndGame::hitContinue()
{
    if (pressedContinue)
    {
        pressedContinue=false;
        return true;
    }
    else
        return false;
}
//------------------------------------------------------------------------------------------------------

bool EndGame::isDone ( bool *playOn )
{
    if (done)
    {
        *playOn=playLevel;
    }

    return done;
}
//------------------------------------------------------------------------------------------------------

void EndGame::handleEvents(SDL_Event& event)
{
    int i;

    for (i=0;i<NUM_ENDGAME_BUTTONS;i++)
        if (buttons[i]->isEnabled())
            buttons[i]->handleEvents();

    for (i=0;i<NUM_ENDGAME_BUTTONS;i++)
        if (buttons[i]->isEnabled())
            if (buttons[i]->getState()==BUTTONSTATE_PUSHEDHOVER)
            {
                switch (i)
                {
                    case ID_BTN_CONTINUE: playLevel=true; pressedContinue=true; break;
                    case ID_BTN_SELECT:   playLevel=false;break;
                }

                 timePassed=0;
                 mode = ENDGAME_TIMED_SCREEN;
                 createMessages();
                 createStaticMessageImage();

                 for (int k=0;k<NUM_ENDGAME_BUTTONS;k++)
                 {
                     buttons[k]->disable();
                 }
                 return;
            }

}

//------------------------------------------------------------------------------------------------------

void EndGame::logic(float dt)
{
    if ( mode == ENDGAME_TIMED_SCREEN )
        timePassed+=dt;

    if (timePassed>=0.2)
        done=true;

    if (unlockedEffect!=NULL)
        unlockedEffect->update(dt);
}

//------------------------------------------------------------------------------------------------------
void EndGame::createStaticMessageImage()
{
    if (texTotalMessages!=NULL)
    {
        delete texTotalMessages;
        texTotalMessages=NULL;
    }

    texTotalMessages = new BaseTexture;

    if (!texTotalMessages->createBlank(rc.w,rc.h,SDL_TEXTUREACCESS_TARGET) )
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg, "InGame:texTotalMessages creation Failed\n");
        mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG

        delete texTotalMessages;
        texTotalMessages=NULL;
    }
    else
        texTotalMessages->useAlphaBlending(true);

    if (texTotalMessages!=NULL)
    {
        texTotalMessages->setAsRenderTarget();

        FPoint offset( -rc.x, -rc.y );

        renderMessages(offset);

        texTotalMessages->resetRenderTarget();
    }
}

//------------------------------------------------------------------------------------------------------
void EndGame::createUnlockedEffect()
{
    unlockedEffect=NULL;
    if (mode==ENDGAME_VICTORY_INFO)
    {
        switch ( unlocked&7 )
        {
            case UNL_BOMB:          unlockedEffect = new FuseEffect( unlockedPos, FPoint(27.,-32).GetNormal()*guiUnit*0.47,guiUnit/8,FPoint(0,2*guiBox->h) ); break;

            case UNL_COLORSHIFTER:  unlockedEffect = new PixiesEffect( unlockedPos, COLOR_SHIFTER, guiUnit/8); break;

            case UNL_PAINTER:       unlockedEffect = new PixiesEffect( unlockedPos, COLOR_PAINTER, guiUnit/8); break;

            case UNL_LEVELER:       unlockedEffect = new PixiesEffect( unlockedPos, COLOR_LEVELER, guiUnit/8); break;

            case UNL_CHROME:        unlockedEffect = new PixiesEffect( unlockedPos, COLOR_CHROME, guiUnit/8); break;
        }
    }

}
//------------------------------------------------------------------------------------------------------
void EndGame::renderMessages(const FPoint& offset)
{
    int px,py,i;

    px = offset.x + rc.x+guiUnit/2;
    py = offset.y + rc.y+guiUnit/2;

    for (i=0;i<NUM_MESSAGES;i++)
    {
        msg[i]->render(px,py+i*guiUnit);
    }

    SDL_Rect rcStar={0,0,(int)guiUnit*0.8,(int)guiUnit*0.8};
    for (int i=0;i<3;i++)
    {
        if (showStar[i])
            texStars[i]->render(rc.w-guiUnit, py + (starMessageIndex[i]-0.2)*guiUnit,&rcStar,-1,255,21*i);
    }
}
//------------------------------------------------------------------------------------------------------

void EndGame::render(float phase)
{
    if (timePassed>0.2) return;

    float alphaMod=(0.2-timePassed)/0.2;

    float unlockAlpha= 200.f+ 40.f*mySine(phase*3);

    int bgAlpha=(mode==ENDGAME_VICTORY_INFO)?180:200;

    texBackground->render( rc.x , rc.y,&rc,-1,bgAlpha*alphaMod);//,0,NULL,SDL_FLIP_VERTICAL);

    if (texTotalMessages!=NULL)
        texTotalMessages->render(rc.x,rc.y,NULL,-1,255*alphaMod);
    else
        renderMessages();

    if (texUnlocked!=NULL)
        texUnlocked->render(rcUnlocked.x,rcUnlocked.y,&rcUnlocked,-1,unlockAlpha*alphaMod);

    if (unlockedEffect!=NULL)
        unlockedEffect->render(alphaMod);//*unlockAlpha/255.);

    for (int i=0;i<NUM_ENDGAME_BUTTONS;i++)
        buttons[i]->render(FPoint(0.,0.),alphaMod);
}




#include "base.h"

#include "levelStats.h"
#include "basefuncs.h"
#include "gameSettings.h"

extern bool integrityCheckComplete;
extern bool licenceCheckComplete;
extern bool isOriginal;

const int STATS_VERSION=4;
const int BUFFER_SIZE=6;

enum PASSED_OFFSETS {
    POFFSET_SKIN1=16,
    POFFSET_SKIN2,
    POFFSET_SKIN3,
    POFFSET_SKIN4,
    POFFSET_MATCH3,
    POFFSET_MATCH4,
    POFFSET_MATCH5,
    POFFSET_MATCH6,
    POFFSET_DIM10,
    POFFSET_DIM12,
    POFFSET_DIM14,
    POFFSET_DIM16,
    POFFSET_TIME_BEATEN,
    POFFSET_SHOTS_BEATEN,
    POFFSET_SCORE_BEATEN,//bit 30
};

enum REQUIREMENTS
{
    REQ_TRUERANDOM=0,
    REQ_CHAOS,
    REQ_SKIN1,
    REQ_SKIN2,
    REQ_SKIN3,
    REQ_SKIN4,
    REQ_MATCH4,
    REQ_MATCH5,
    REQ_MATCH6,
    REQ_DIM10,
    REQ_DIM12,
    REQ_DIM14,
    REQ_DIM16,
    REQ_MODE_TIME,
    REQ_MODE_SHOT,
    REQ_MODE_SCORE,
    NUM_REQUIREMENTS
};

const int minRequirement[NUM_UNLOCKABLES]=
{
    30,//UNL_TRUERANDOM,
    30,//UNL_CHAOS,
    50,//UNL_SKIN_DARK2,
    50,//UNL_SKIN_LIGHT2,
    50,//UNL_SKIN_PATTERN2,
    50,//UNL_SKIN_GEOMETRIC,
    50,//UNL_MATCH4,
    100,//UNL_MATCH5,
    150,//UNL_MATCH6,
    100,//UNL_DIM10,
    100,//UNL_DIM12,
    100,//UNL_DIM14,
    100,//UNL_DIM16,
#ifdef EXTRA_GAME_MODES
    100,//UNL_MODE_TIME,
    100,//UNL_MODE_SHOT,
    100,//UNL_MODE_SCORE,
    200//UNL_EDITOR,
#endif
};

const int minLevel[NUM_UNLOCKABLES]=
{
    50,//UNL_TRUERANDOM,
    75,//UNL_CHAOS,
    110,//UNL_SKIN_DARK2,
    110,//UNL_SKIN_LIGHT2,
    110,//UNL_SKIN_PATTERN2,
    110,//UNL_SKIN_GEOMETRIC,
    100,//UNL_MATCH4,
    200,//UNL_MATCH5,
    300,//UNL_MATCH6,
    125,//UNL_DIM10,
    250,//UNL_DIM12,
    375,//UNL_DIM14,
    450,//UNL_DIM16,
#ifdef EXTRA_GAME_MODES
    520,//UNL_MODE_TIME,
    680,//UNL_MODE_SHOT,
    820,//UNL_MODE_SCORE,
    1000//UNL_EDITOR,
#endif
};

const int activationLevel[NUM_SUMMONS]={
    0,//UNL_NONE=0
    1,//UNL_BOMB,
    4,//UNL_COLORSHIFTER,//9
    9,//UNL_TIMESTOP,//19
    19,//UNL_PAINTER,//59
    29,//UNL_LEVELER,//99
    59,//UNL_CHROME//159
    };

const int disablePeriod[NUM_SUMMONS]={
    0,//UNL_NONE=0
    1,//UNL_BOMB,
    1,//UNL_COLORSHIFTER,
    0,//UNL_TIMESTOP,
    2,//UNL_PAINTER,//4
    4,//UNL_LEVELER,//9
    5//UNL_CHROME,//9
};

#include <cstdio>
#include <cstring>

#ifdef PLATFORM_ANDROID
float meanShotTime;
#endif

LevelStats::LevelStats(std::string name,bool dropChanges)
{
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"LevelStats: Constructor [%s]\n",playerName.c_str());
        mySDL_Log(glbLogMsg);
#endif
    playerName=name;

    maxTotalScore=0;
    totalPlayTime=0;
    totalShots=0;

    dontSave=dropChanges;
    std::vector<stats*> v(0);

    for (int w=0;w<NUM_DIMENSIONS;w++)
        vs.push_back(v);//one vector of level stats per well width id

    readStats();
}

//--------------------------------------------------------------------

LevelStats::~LevelStats()
{
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"LevelStats: Destructor for [%s]\n",playerName.c_str());
        mySDL_Log(glbLogMsg);
#endif

      storeAndClear();
}
//--------------------------------------------------------------------
void LevelStats::storeAndClear()
{

    if (!dontSave)
    {
        writeStats();
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"LevelStats: Store Stats for [%s]\n",playerName.c_str());
        mySDL_Log(glbLogMsg);
#endif

    }

#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"LevelStats: Clear Stats for [%s]\n",playerName.c_str());
        mySDL_Log(glbLogMsg);
#endif

    for (int w=0;w<NUM_DIMENSIONS;w++)
    {
        for (int i=0;i<vs[w].size();i++)
        {
            delete vs[w][i];
            vs[w][i]=NULL;
        }
        vs[w].clear();
    }
    playerName="";

    totalShots=0;
    totalPlayTime=0;
    maxTotalScore=0;

    initUnlockables();

#ifdef PLATFORM_ANDROID
    meanShotTime=1.;
#endif
}
//--------------------------------------------------------------------
bool LevelStats::changePlayer(std::string newName)
{
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,
                "LevelStats: ChangeName: [%s] -> [%s]\n",
                    playerName.c_str(),newName.c_str());
        mySDL_Log(glbLogMsg);
#endif

    if (playerName!=newName)
    {
        storeAndClear();

        playerName=newName;
        readStats();
        return true;
    }

    return false;
}
//--------------------------------------------------------------------
std::string LevelStats::getPlayerName()
{
    return playerName;
}
//--------------------------------------------------------------------
int LevelStats::getIndex(int randomness,int pressure)
{
    return randomness*NUM_RANDOMNESS_LEVELS + pressure;
}
//--------------------------------------------------------------------
void  LevelStats::zeroStats(stats* s)
{
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg, "LevelStats: zeroStats\n");
        mySDL_Log(glbLogMsg);
#endif
        s->passed=0;//((unsigned int)1<<31)-1;     //for full passed and max unlocks
        s->maxRoundScore=0;//999;
        s->minTime=1000000000;//10000;
        s->minShots=1000000000;//10000;
        s->summons=0;
        s->matchStats=0;//(100<<20)|(100<<10)|100;
}
//--------------------------------------------------------------------
void LevelStats::addLevel(int level,int widthID)
{
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg, "LevelStats: addLevel: %d\n",level);
    mySDL_Log(glbLogMsg);
#endif

    while ( level+1 > vs[widthID].size() )//might be new, possibly a skipped level might exist.
    {
#ifdef CONSOLE_LOG
            sprintf(glbLogMsg, "LevelStats: addLevel: adding new level %d\n",vs[widthID].size());
            mySDL_Log(glbLogMsg);
#endif

        stats* s= new stats;
        zeroStats( s );
        vs[widthID].push_back(s);
        s=NULL;
    }
}
//--------------------------------------------------------------------
void  LevelStats::addAttempt(int level)
{
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg, "LevelStats: addAttempt:");
    mySDL_Log(glbLogMsg);
#endif
    addLevel(level,gameSettings->wellWidthID);//precaution
    levelShots=0;
}
//--------------------------------------------------------------------
void LevelStats::addPlayTime(int playtime)
{
    totalPlayTime += playtime/1000;//reading ticks
}

//--------------------------------------------------------------------
int LevelStats::addVictory(int level,int roundScore, int playTime,int summonUsed, bool usedTimestop, int match4,int match5,int match6,int ballNumber, int startSkin)
{
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg, "LevelStats: addVictory: level=%d rand=%d pres=%d, time=%d\n",
                        level,gameSettings->randomness,gameSettings->pressure, (int)(playTime));
    mySDL_Log(glbLogMsg);
#endif
    int unlockFeature=0;//UNL_NONE;


    if (vs[gameSettings->wellWidthID][level]->passed==0)//new beaten level
    {
        for (int i=0;i<NUM_SUMMONS;i++)
        {
            if ((level+1)==(activationLevel[i]) ) //explain: player fnished level id 18 (0 based) and level unlock is at 20 (1 based)
            {
                unlockFeature=i;
                break;
            }
        }
    }
    else removeLevelFromRequirements(level,gameSettings->wellWidthID);//will be updated with the new values shortly


    addPlayTime(playTime);

    storeRandPressSettings(level);
    storeMatchComboData(level, match4,match5,match6);
    storeMatchSetting(level);
    storeSkin(level, startSkin);
    storeBeatenStatsData(level,roundScore,playTime,ballNumber);


    if ( roundScore > vs[gameSettings->wellWidthID][level]->maxRoundScore)
    {
        maxTotalScore+=roundScore - vs[gameSettings->wellWidthID][level]->maxRoundScore;
        vs[gameSettings->wellWidthID][level]->maxRoundScore = roundScore;
    }

    if ( levelShots < vs[gameSettings->wellWidthID][level]->minShots )
        vs[gameSettings->wellWidthID][level]->minShots=levelShots;

    if (playTime < vs[gameSettings->wellWidthID][level]->minTime )
        vs[gameSettings->wellWidthID][level]->minTime = playTime;

    storeSummons(level,summonUsed, usedTimestop);

    addLevelToRequirements(level,gameSettings->wellWidthID);// unlockable requirements, that is

    int unlockablesUnlocked= updateUnlockableStatus();

    unlockFeature |= (unlockablesUnlocked <<3);//NUM_SUMMONS is 7.// summons are unlocked only one at a time. so returning
                                              //a number of summon unlocked + a shifted bitmask of unlockables unlocked
    return unlockFeature;

}
//--------------------------------------------------------------------
void LevelStats::addShot()
{
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg, "LevelStats: addShot: %d\n",levelShots);
    mySDL_Log(glbLogMsg);
#endif
        ++totalShots;
        ++levelShots;
}

//================ACCESSORS=====================================
int LevelStats::getMaxLevel()
{
       return vs[gameSettings->wellWidthID].size()-1;
}
//-------------------------------------------------------------------
int LevelStats::getMaxLevel(int widthID)
{
       return vs[widthID].size()-1;
}
//-------------------------------------------------------------------
int LevelStats::getTotalLevels()
{
    int total=0;
    for (int i=0;i<NUM_DIMENSIONS;i++)
        total+=(vs[i].size()-1);
    return total;
}

//-------------------------------------------------------------------
bool LevelStats::hasBeaten(int level)
{
    if ( (level<vs[gameSettings->wellWidthID].size()) && (vs[gameSettings->wellWidthID][level]->passed ))
        return true;
    return false;
}

//--------------------------------------------------------------------
bool LevelStats::hasBeaten(int level,int randomness, int pressure)
{
    if (level<vs[gameSettings->wellWidthID].size())
        return ( vs[gameSettings->wellWidthID][level]->passed >>getIndex(randomness,pressure) )&1;

    return false;
}
//--------------------------------------------------------------------
int LevelStats::getTotalPlayTime()
{
    return totalPlayTime;
}
//--------------------------------------------------------------------
int LevelStats::getTotalShots()
{
    return totalShots;
}
//--------------------------------------------------------------------
int LevelStats::getMinShots(int level)
{
    return vs[gameSettings->wellWidthID][level]->minShots;
}
//--------------------------------------------------
int LevelStats::getMinTime(int level)
{
    return vs[gameSettings->wellWidthID][level]->minTime;
}
//--------------------------------------------------
int LevelStats::getRoundScore(int level)
{
    return vs[gameSettings->wellWidthID][level]->maxRoundScore;
}
//--------------------------------------------------
int LevelStats::getMaxTotalScore()
{
    return maxTotalScore;
}
//--------------------------------------------------
int LevelStats::getMaxScore(int level)
{
    int score=0;

    if (level==-1)
        level = getMaxLevel();

    for (int i=0;i<level;i++)
        score += getRoundScore(i);

    return score;
}
//=================================================================================
std::string LevelStats::createFilename()
{
    std::string filename;


    filename= glbAppPath;


    filename += "/" + playerName +".bin";//.txt

    return filename;
}
//--------------------------------------------------------------

void LevelStats::readStats()
{
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg, "LevelStats: readStats [%s]\n",playerName.c_str() );
    mySDL_Log(glbLogMsg);
#endif

    std::string filename=createFilename();

    initUnlockables();//zero unlockable status and data

#ifdef CONSOLE_LOG
    sprintf(glbLogMsg, "LevelStats: readStats: Filename is [%s]\n",filename.c_str() );
    mySDL_Log(glbLogMsg);
#endif

    SDL_RWops* in=NULL;

    in=SDL_RWFromFile( filename.c_str(),"r");

    if (in == NULL)
    {
    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg, "ERROR READING %s\n",filename.c_str());
        mySDL_Log(glbLogMsg);
    #endif
        for (int w=0; w< NUM_DIMENSIONS;w++)
            addLevel(0,w);
        return ;
    }

    int numRecords=0;

    SDL_RWread(in,&totalPlayTime,sizeof(int),1);
    SDL_RWread(in,&totalShots,sizeof(int),1);

    SDL_RWread(in,&numRecords,sizeof(int),1);

#ifndef DEMO_GAME
    #ifdef PLATFORM_ANDROID
        if (
            (!integrityCheckComplete)
            || ( (integrityCheckComplete)&&(isOriginal)&&(!licenceCheckComplete))
            ) isLegitimateCopy = (numRecords>>30);//last time this player played,
        numRecords &= ((1<<30) -1);
    #endif
#endif

    int fileVersion=numRecords>>24;
    int numLevels=0;

    if (fileVersion<4)
        numLevels =numRecords& ((1<<24)-1);

    //SDL_Log("[%s] fileVersion: %d",playerName.c_str(),fileVersion);


    int buffer[BUFFER_SIZE];

    int numFields;

    switch (fileVersion)
    {
        case 0:numFields=4;break;
        case 1:
        case 2:numFields=5;break;
        case 3:numFields=6;break;
        default : numFields=BUFFER_SIZE;
    }

    for ( int width=0; width<NUM_DIMENSIONS; width++)
    {
        if (fileVersion>=4)//added records per dimension per level
            SDL_RWread(in,&numLevels,sizeof(int),1);//see bottom of loop

        for (int level=0;level<numLevels;level++)
        {
            addLevel(level,width);

            for (int i=0;i<BUFFER_SIZE;i++)
                buffer[i]=0;

            SDL_RWread(in,buffer,sizeof(int),numFields);

            vs[width][level]->passed               = buffer[0];
            vs[width][level]->maxRoundScore        = buffer[1];
            vs[width][level]->minTime              = buffer[2];
            vs[width][level]->minShots             = buffer[3];
            vs[width][level]->summons              = buffer[4];
            vs[width][level]->matchStats           = buffer[5];

            if (fileVersion<=1)
            {
                vs[width][level]->passed=  ((vs[width][level]->passed>>6)<<8) | ((vs[width][level]->passed>>3)<<4) | (vs[width][level]->passed & ( (1<<3)-1));//turning length -9 bit field to length 12 with topmost bits =0;
            }

            maxTotalScore+=vs[width][level]->maxRoundScore;

            addLevelToRequirements(level,width);
        }
        numLevels=0; //thus for versions older than 4 only the first vector will be read
    }

    SDL_RWclose(in);
    in=NULL;

    if (fileVersion<4)
    {
        for (int w=DIM_10;w<NUM_DIMENSIONS;w++)
            addLevel(0,w);
    }

    updateUnlockableStatus();
#ifdef PLATFORM_ANDROID
    if (totalShots>50)
        meanShotTime = totalPlayTime*1. / totalShots;
    else meanShotTime=1.;
#endif
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg, "done reading %s\n",filename.c_str());
    mySDL_Log(glbLogMsg);
#endif
}
//--------------------------------------------------------------------
void LevelStats::writeStats()
{
#ifdef CONSOLE_LOG
    sprintf(glbLogMsg, "LevelStats: writeStats [%s]\n",playerName.c_str() );
    mySDL_Log(glbLogMsg);
#endif
    std::string filename=createFilename();

    SDL_RWops* out=NULL;

    out=SDL_RWFromFile( filename.c_str(),"w");

    if (out== NULL)
    {
    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg, "ERROR READING %s\n",filename.c_str());
        mySDL_Log(glbLogMsg);
    #endif
        return ;
    }
    SDL_RWwrite( out, &totalPlayTime, sizeof(int), 1 );
    SDL_RWwrite( out, &totalShots, sizeof(int), 1 );

    int numRecords=0;
    for (int w=0;w<NUM_DIMENSIONS;w++)
        numRecords+=vs[w].size()+1;

    numRecords |= (STATS_VERSION<<24);//inserting version number
#ifndef DEMO_GAME
    #ifdef PLATFORM_ANDROID
        if ( isLegitimateCopy &&
            ((integrityCheckComplete) && (isOriginal))//legitimate copy might be inherited by a previous saved stats file. integrity check is mandatory though.
            )
             numRecords |= (1<<30);
    #endif
#endif
    SDL_RWwrite( out, &numRecords, sizeof(int), 1 );

    unsigned int buffer[BUFFER_SIZE];
    int numLevels;

    for (int w=0;w<NUM_DIMENSIONS;w++)
    {
        numLevels=vs[w].size();
        SDL_RWwrite( out, &numLevels, sizeof(int), 1 );


        for (int level=0;level < vs[w].size();level++)
        {
            buffer[0] = vs[w][level]->passed;
            buffer[1] = vs[w][level]->maxRoundScore;
            buffer[2] = vs[w][level]->minTime;
            buffer[3] = vs[w][level]->minShots;
            buffer[4] = vs[w][level]->summons;
            buffer[5] = vs[w][level]->matchStats;

            SDL_RWwrite(out,buffer,sizeof(int),BUFFER_SIZE);
        }
    }

    SDL_RWclose(out);
    out=NULL;
    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg, "done writing %s\n",filename.c_str());
        mySDL_Log(glbLogMsg);
    #endif
}
//--------------------------------------
int  LevelStats::getLevelShots()
{
    return levelShots;
}
//-----------------------------------------------
void LevelStats::setLevelShots(int shots)
{
    levelShots=shots;
}


//--------------------------------------------------------------------
void LevelStats::storeRandPressSettings(int level)
{
    vs[gameSettings->wellWidthID][level]->passed |= (1<<getIndex(gameSettings->randomness,gameSettings->pressure));//storing randomness and pressure setting
}
//--------------------------------------------------------------------
void LevelStats::storeMatchComboData(int level, int match4,int match5,int match6)
{
    int m4,m5,m6;
    m6=vs[gameSettings->wellWidthID][level]->matchStats;

    m4=m6&( (1<<10) -1);//10 lowest bits
    m6>>=10;
    m5=m6&( (1<<10) -1);//10 next bits
    m6>>=10;//10 next bits

    m4= std::max(m4,match4) & ( (1<<10) -1);
    m5= std::max(m5,match5) & ( (1<<10) -1);
    m6= std::max(m6,match6) & ( (1<<10) -1);

    vs[gameSettings->wellWidthID][level]->matchStats = (m6<<20) | (m5<<10) | m4;//stored in one int

}
//--------------------------------------------------------------------
void LevelStats::storeMatchSetting(int level)
{
    if (gameSettings->matchNumber>=3)
        vs[gameSettings->wellWidthID][level]->passed |= (1 << ( POFFSET_MATCH3+ (gameSettings->matchNumber-3) ) );
}

//--------------------------------------------------------------------
void LevelStats::storeSkin(int level, int startSkin)
{
    if (startSkin < 4)//only the basic skins
        vs[gameSettings->wellWidthID][level]->passed |= (1 << ( POFFSET_SKIN1+ (startSkin) ) );
}

//--------------------------------------------------------------------
void LevelStats::storeBeatenStatsData(int level,int roundScore,int playTime,int levelBalls)
{
    if (roundScore >= levelBalls*4 )
        vs[gameSettings->wellWidthID][level]->passed |= 1 << (POFFSET_SCORE_BEATEN);

    if (playTime < levelBalls*1000)//milliseconds,
        vs[gameSettings->wellWidthID][level]->passed |= 1 << (POFFSET_TIME_BEATEN);

    if (levelShots < (levelBalls+1)/2 )
        vs[gameSettings->wellWidthID][level]->passed |= 1 << (POFFSET_SHOTS_BEATEN);
}
//--------------------------------------------------------------------

void LevelStats::storeSummons(int level,int summonUsed, bool usedTimestop)
{
    vs[gameSettings->wellWidthID][level]->summons=0;
    if (summonUsed>=0)
        vs[gameSettings->wellWidthID][level]->summons |= (1<<summonUsed);
    if (usedTimestop)
        vs[gameSettings->wellWidthID][level]->summons |= (1<<UNL_TIMESTOP);
}

//------------------------------------------------
bool LevelStats::isSummonUnlocked( int summon , int level)
{
    return ( level > activationLevel[summon]);//correct
}

//-----------------------------------------------------

bool LevelStats::wasSummonUsed( int unl, int level)
{
    if ( level >= vs[gameSettings->wellWidthID].size() )
        return false;

    return ((vs[gameSettings->wellWidthID][level]->summons >> unl) &1);
}

//---------------------------------------------------------

bool LevelStats::isSummonAvailable( int unl,int level, int* lockLevel )
{
    if ( level <= activationLevel[unl]-1)
    {
        if (lockLevel!=NULL)
            *lockLevel = activationLevel[unl];
        return false;
    }

    int minLevel= std::max(0,level-disablePeriod[unl]);
    int maxLevel= std::min(int(vs[gameSettings->wellWidthID].size()-1),level+disablePeriod[unl]);

    for (int i=minLevel;i<=maxLevel;i++)
    {
        if (i==level) continue;//re-playing this level,  so he may use it

        if (wasSummonUsed(unl,i))
        {
            if (lockLevel!=NULL)
                *lockLevel = i;
            return false;
        }
    }

    if (lockLevel!=NULL)
       *lockLevel = -1;

    return true;
}

//--------------------------------------------------------------

void LevelStats::markSummonAsUsed(int unl, int level )
{
    if ( level < vs[gameSettings->wellWidthID].size() )
         vs[gameSettings->wellWidthID][level]->summons |=  (1<<unl);
}
//---------------------------------------------------------------
int LevelStats::getAvailableSummonsVector(int level)
{
    int v=0;

    for (int i=UNL_BOMB; i<=UNL_CHROME;i++)
        if (isSummonAvailable(i,level))
            v|=(1<<i);
    return v;
}
//--------------------------------------------------------------------

void LevelStats::initUnlockables()
{
    for (int i=0;i<NUM_REQUIREMENTS;i++)
        unlockableRequirements[i]=0;

    for (int i=0;i<NUM_UNLOCKABLES;i++)
        unlockableStatus[i]=false;

}
//--------------------------------------------------------------------

void LevelStats::recomputeUnlockablesData()//not used
{
    for (int i=0;i<NUM_REQUIREMENTS;i++)
        unlockableRequirements[i]=0;

    for (int w=0;w<NUM_DIMENSIONS;w++)
        for (int level=0;level<vs[w].size();level++)
            addLevelToRequirements(level,w);
}

//--------------------------------------------------------------------
void LevelStats::removeLevelFromRequirements(int level, int widthID)//used for performing incremental update on level completion
{
    unlockableRequirements[REQ_TRUERANDOM] -=  (((vs[widthID][level]->passed>>8)&15)?1:0);//3d level of randomness, passed in any of 4 pressures - checking bits 9-12
    unlockableRequirements[REQ_CHAOS]      -=  (((vs[widthID][level]->passed)&( (1<<2) | (1<<6) | (1<<10) | (1<<14)))?1:0);//any randomness, checking 3d bits of every 4bit set
                                                //alternatively, use 1<< getIndex( XXX, PRESSURE_HIGH)

    unlockableRequirements[REQ_SKIN1] -=  (vs[widthID][level]->passed& (1 << POFFSET_SKIN1))?1:0;
    unlockableRequirements[REQ_SKIN2] -=  (vs[widthID][level]->passed& (1 << POFFSET_SKIN2))?1:0;
    unlockableRequirements[REQ_SKIN3] -=  (vs[widthID][level]->passed& (1 << POFFSET_SKIN3))?1:0;
    unlockableRequirements[REQ_SKIN4] -=  (vs[widthID][level]->passed& (1 << POFFSET_SKIN4))?1:0;

    unlockableRequirements[REQ_MATCH4]  -=  (vs[widthID][level]->passed& (1 << POFFSET_MATCH3))?1:0;
    unlockableRequirements[REQ_MATCH5]  -=  (vs[widthID][level]->passed& (1 << POFFSET_MATCH4))?1:0;
    unlockableRequirements[REQ_MATCH6]  -=  (vs[widthID][level]->passed& (1 << POFFSET_MATCH5))?1:0;

    unlockableRequirements[REQ_MODE_TIME]   -= (vs[widthID][level]->passed& (1 << POFFSET_TIME_BEATEN))?1:0;
    unlockableRequirements[REQ_MODE_SHOT]   -= (vs[widthID][level]->passed& (1 << POFFSET_SHOTS_BEATEN))?1:0;
    unlockableRequirements[REQ_MODE_SCORE]  -= (vs[widthID][level]->passed& (1 << POFFSET_SCORE_BEATEN))?1:0;
}

//--------------------------------------------------------------------

void LevelStats::addLevelToRequirements(int level,int widthID)
{
    unlockableRequirements[REQ_TRUERANDOM] +=  (((vs[widthID][level]->passed>>8)&15)?1:0);//3d level of randomness, passed in any of 4 pressures - checking bits 9-12
    unlockableRequirements[REQ_CHAOS]      +=  (((vs[widthID][level]->passed)&( (1<<2) | (1<<6) | (1<<10) | (1<<14)))?1:0);//any randomness, checking 3d bits of every 4bit set
                                                //alternatively, use 1<< getIndex( XXX, PRESSURE_HIGH)

    unlockableRequirements[REQ_SKIN1] +=  (vs[widthID][level]->passed& (1 << POFFSET_SKIN1))?1:0;
    unlockableRequirements[REQ_SKIN2] +=  (vs[widthID][level]->passed& (1 << POFFSET_SKIN2))?1:0;
    unlockableRequirements[REQ_SKIN3] +=  (vs[widthID][level]->passed& (1 << POFFSET_SKIN3))?1:0;
    unlockableRequirements[REQ_SKIN4] +=  (vs[widthID][level]->passed& (1 << POFFSET_SKIN4))?1:0;

    unlockableRequirements[REQ_MATCH4]  +=  (vs[widthID][level]->passed& (1 << POFFSET_MATCH3))?1:0;
    unlockableRequirements[REQ_MATCH5]  +=  (vs[widthID][level]->passed& (1 << POFFSET_MATCH4))?1:0;
    unlockableRequirements[REQ_MATCH6]  +=  (vs[widthID][level]->passed& (1 << POFFSET_MATCH5))?1:0;

    unlockableRequirements[REQ_DIM10]   = vs[DIM_8].size();
    unlockableRequirements[REQ_DIM12]   = vs[DIM_10].size();
    unlockableRequirements[REQ_DIM14]   = vs[DIM_12].size();
    unlockableRequirements[REQ_DIM16]   = vs[DIM_14].size();

    unlockableRequirements[REQ_MODE_TIME]   += (vs[widthID][level]->passed& (1 << POFFSET_TIME_BEATEN))?1:0;
    unlockableRequirements[REQ_MODE_SHOT]   += (vs[widthID][level]->passed& (1 << POFFSET_SHOTS_BEATEN))?1:0;
    unlockableRequirements[REQ_MODE_SCORE]  += (vs[widthID][level]->passed& (1 << POFFSET_SCORE_BEATEN))?1:0;
}
//--------------------------------------------------------------------
int LevelStats::updateUnlockableStatus()
{
    #ifdef DEMO_GAME
        return 0;
    #else // DEMO_GAME

    int totalPassed=getTotalLevels();

    int statusAltered=0;
    int i;

    for ( i=0;i<
#ifdef EXTRA_GAME_MODES
    UNL_EDITOR
#else
    NUM_UNLOCKABLES
#endif
    ;i++)
    {
        if (unlockableStatus[i])
            continue;

        if ( (minLevel[i]<=totalPassed) && (unlockableRequirements[i]>=minRequirement[i]) )
        {
            unlockableStatus[i]=true;//isLegitimateCopy;
            statusAltered|= (1<<i);
        }
    }
#ifdef EXTRA_GAME_MODES
    if ( (!unlockableStatus[UNL_EDITOR])&&
        (minLevel[UNL_EDITOR]<=totalPassed) &&
        (unlockableRequirements[REQ_MODE_SCORE]>=minRequirement[UNL_EDITOR]) &&
        (unlockableRequirements[REQ_MODE_TIME]>=minRequirement[UNL_EDITOR])&&
        (unlockableRequirements[REQ_MODE_SHOT]>=minRequirement[UNL_EDITOR])
        )
    {
        unlockableStatus[UNL_EDITOR]=true;//isLegitimateCopy;
        statusAltered|= (1<<UNL_EDITOR);
    }
#endif
    return statusAltered;
    #endif
}

//--------------------------------------------------------------------

bool LevelStats::getUnlockableStatus(int unlockableID)
{
    #ifdef DEMO_GAME
        return false;
    #else // DEMO_GAME
        return  ( ( isLegitimateCopy ) && ( unlockableStatus[unlockableID] ) );
    #endif
}

//--------------------------------------------------------------------
const char* LevelStats::getSummonName(int summonID)
{
    static   const char* summonNames[NUM_SUMMONS]=
    {
        "",
        "Bomb",
        "ColorShifter",
        "Time Stop",
        "Painter",
        "Leveler",
        "Chrome Ball"
    };

    return summonNames[summonID];
}

//------------------------------------------------------------------

bool LevelStats::getSummonString(int summonID, std::string *str)
{

    char buf[100];
    sprintf(buf,"%s\n\n Reach level %d in current area size",getSummonName(summonID),activationLevel[summonID]+1);
    *str = buf;

    return isSummonUnlocked(summonID, vs[gameSettings->wellWidthID].size());
}

//--------------------------------------------------------------------

const char* LevelStats::getUnlockableName(int unlockableID)
{
    static const char* unlockableNames[NUM_UNLOCKABLES]=
    {
        "True Random",
        "Chaos",
        "Dark II",
        "Light II",
        "Patterned II",
        "Geometric II",
        "Match 4",
        "Match 5",
        "Match 6",
        "10x17",
        "12x21",
        "14x24",
        "16x28",
#ifdef EXTRA_GAME_MODES
        "TimeLimit Game",
        "ShotLimit Game",
        "ScoreLimit Game",
        "Level Editor"
#endif
    };
    return unlockableNames[unlockableID];
}

//--------------------------------------------------------------------
bool LevelStats::getUnlockableString(int unlockableID, std::string *str)
{
    int totalPassed=getTotalLevels();


    static const char* unlockables[NUM_UNLOCKABLES]=
    {
        "[ True Random ] Setting\n\n-Finish any %d (total: %d) levels\n\n-Pass any %d (%d) levels at\n   'Challenge' Randomness setting",
        "[ Chaos ] Pressure Setting\n\n-Finish any %d (total: %d) levels\n\n-Pass any %d (%d) levels at\n   'Haste' Pressure setting",
        "[ Dark II ] Theme\n\n-Finish any %d (total: %d) levels\n\n-Pass any %d (%d) levels using\n   the 'Dark' Theme",
        "[ Light II ] Theme\n\n-Finish any %d (total: %d) levels\n\n-Pass any %d (%d) levels using\n   the 'Light' Theme",
        "[ Patterned II ] Theme\n\n-Finish any %d (total: %d) levels\n\n-Pass any %d (%d) levels using\n   the 'Patterned' Theme",
        "[ Geometric II ] Theme\n\n-Finish any %d (total: %d) levels\n\n-Pass any %d (%d) levels using\n   the 'Geometric' Theme",
        "[ Match 4 ] GamePlay Setting\n\n-Finish any %d (total: %d) levels\n\n-Pass %d (%d) levels in\n    Match 3 Setting",
        "[ Match 5 ] GamePlay Setting\n\n-Finish any %d (total: %d) levels\n\n-Pass %d (%d) levels in\n    Match 4 Setting",
        "[ Match 6 ] GamePlay Setting\n\n-Finish any %d (total: %d) levels\n\n-Pass %d (%d) levels in\n    Match 5 Setting",
        "[ 10x17 ] Playing Area Size\n\n-Finish any %d (total: %d) levels\n\n-Pass %d (%d) levels in\n    Area size 8x14",
        "[ 12x21 ] Playing Area Size\n\n-Finish any %d (total: %d) levels\n\n-Pass %d (%d) levels in\n    Area size 10x17",
        "[ 14x24 ] Playing Area Size\n\n-Finish any %d (total: %d) levels\n\n-Pass %d (%d) levels in\n    Area size 12x21",
        "[ 16x28 ] Playing Area Size\n\n-Finish any %d (total: %d) levels\n\n-Pass any %d (%d) levels in\n    Area size 14x24",
#ifdef EXTRA_GAME_MODES
        "[ Play vs Time limit ]\n\n-Finish any %d (total: %d) levels\n\n-Pass any %d (%d) levels with\n      time <= [initial level balls]\n         (Time Limit)",
        "[ Play vs Shot limit ]\n\n-Finish any %d (total: %d) levels\n\n-Pass any %d (%d) levels with\n      shots <= [initial level balls]/2\n         (Shot Limit)",
        "[ Play vs Score limit ]\n\n-Finish any %d (total: %d) levels\n\n-Pass any %d (%d) levels with\n      score >= 4x[initial level balls]\n         (Score Limit)",
        "[ Level Editor ] \n\n-Finish any %d (total: %d) levels\n\n-Pass any\n %d (%d) levels under the Time Limit\n %d (%d) levels under the Shot Limit\n %d (%d) levels over the Score Limit",
#endif
    };
    char buffer[300];

#ifdef EXTRA_GAME_MODES
    if (unlockableID<UNL_EDITOR)
#endif
        sprintf(buffer,unlockables[unlockableID],minLevel[unlockableID],totalPassed,minRequirement[unlockableID],unlockableRequirements[unlockableID]);
#ifdef EXTRA_GAME_MODES
    else
        sprintf(buffer,unlockables[unlockableID],minLevel[unlockableID],totalPassed,
                minRequirement[unlockableID],unlockableRequirements[UNL_MODE_TIME],
                minRequirement[unlockableID],unlockableRequirements[UNL_MODE_SHOT],
                minRequirement[unlockableID],unlockableRequirements[UNL_MODE_SCORE]);
#endif
    *str=buffer;

    return  getUnlockableStatus(unlockableID);
}

bool LevelStats::hasBeatenModeStat(int level, int modeStat)
{
    if (level>=vs[gameSettings->wellWidthID].size() ) return false;

    if (modeStat==GM_NORMAL)
        return hasBeaten(level);

    return ( (vs[gameSettings->wellWidthID][level]->passed & ( 1 << (POFFSET_TIME_BEATEN+(modeStat-GM_TIMELIMIT) ) ))!=0);
}

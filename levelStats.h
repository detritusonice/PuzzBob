#ifndef LEVEL_STATS_H
#define LEVEL_STATS_H

#include <vector>
#include <string>

class LevelStats
{
    struct stats
    {
        unsigned int passed;
        unsigned int maxRoundScore;
        unsigned int minTime;
        unsigned int minShots;
        unsigned int summons;
        unsigned int matchStats;
    };

    bool dontSave;

    std::string playerName;

    int totalPlayTime;
    int totalShots;
    int maxTotalScore;

    std::vector< std::vector< stats*> > vs;//per width setting- per level

    int levelShots;

    int unlockableStatus[NUM_UNLOCKABLES];
    int unlockableRequirements[16];//take care, this number might change

    int getIndex(int randomness,int pressure);

    void zeroStats(stats* s);
    void setPlayerName(std::string name);
    void storeAndClear();

    void storeRandPressSettings(int level);
    void storeMatchComboData(int level, int match4,int match5,int match6);

    void storeMatchSetting(int level);
    void storeBeatenStatsData(int level,int roundScore,int playTime,int levelBalls);
    void storeSummons(int level,int summonUsed, bool usedTimestop);
    void storeSkin(int level,  int startSkin);


    std::string createFilename();

    void readStats();
    void writeStats();

    void markSummonAsUsed(int unl, int level);
    bool wasSummonUsed(int unl, int level);

    void initUnlockables();
    void recomputeUnlockablesData();
    int updateUnlockableStatus();

    void removeLevelFromRequirements(int level, int widthID);
    void addLevelToRequirements(int level, int widthID);

    void addLevel(int level, int widthID);
public:

    LevelStats(std::string name, bool dropAllChanges=false);
    ~LevelStats();


    bool changePlayer( std::string newName);
    std::string getPlayerName();



    void addAttempt(int level);
    void addPlayTime(int playtime);
    int addVictory(int level,int roundScore, int playTime, int summonUsed, bool usedTimestop, int match4,int match5,int match6,int levelBallNumber,int startSkin);
    void addShot();
    void setLevelShots(int shots);

    bool hasBeaten(int level,int randomness, int pressure);
    bool hasBeaten(int level);

    bool hasBeatenModeStat(int level, int modeStat);

    bool isSummonUnlocked( int unl ,int level);
    bool isSummonAvailable( int unl, int level, int* lockLevel=NULL);
    int getAvailableSummonsVector(int level);

    bool getUnlockableStatus(int unlockableID);

    bool getUnlockableString(int unlockableID, std::string *str);

    bool getSummonString(int summonID, std::string *str);

    const char* getUnlockableName(int unlockableID);
    const char* getSummonName(int summonID);


    int getMaxLevel();
    int getMaxLevel(int widthID);
    int getTotalLevels();


    int getTotalPlayTime();
    int getTotalShots();
    int getMinShots(int level);
    int getMinTime(int level);
    int getRoundScore(int level);
    int getMaxTotalScore();
    int getLevelShots();
    int getMaxScore(int level=-1);
};

#endif // LEVEL_STATS_H

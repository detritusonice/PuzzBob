#ifndef IN_GAME_H
#define IN_GAME_H

#include "constants.h"
#include "FPoint.h"
#include "timing.h"
#include "levelStats.h"
#include "states.h"


#include <list>

class TextureManager;
class SimpleButton;
class BaseTexture;
class LocalEffect;
class LevelButton;
class GameMenu;
class LevelSelector;
class EndGame;
class PauseScreen;
class SummonMenu;
class Flash;

class TrailsEffect;

const int MAX_COLLISIONS = 11;

const int MAX_BALL_INDEX_LENGTH = 2.5*MAX_WELL_HEIGHT;


enum BALL_STATE//level
{
    BALLSTATE_READY=0,
    BALLSTATE_MOVING,
    BALLSTATE_STATIC,
    BALLSTATE_FALLING,
    NUM_BALLSTATES
};
//=================================================
struct GridPoint
{
    int row;
    int col;

    GridPoint(int r=0, int c=0):row(r),col(c)
    {}
    ~GridPoint()
    {}
};
//=================================================
class BallInfo
{
    enum
    {
        EFFECT_SHINE=0,
        EFFECT_EXPLOSION,
        EFFECT_SPARKS,
        EFFECT_FUSE,
        NUM_BALL_EFFECTS
    };

    FPoint pos;
    FPoint vel;
    FPoint acceleration;

    int color;
    int state;
    int alpha;
    float radius;

public:
    //-----------------------------------------------------


    LocalEffect* effect[NUM_BALL_EFFECTS];


    BallInfo( float ballRadius, const FPoint& ballPos, int ballColor,int ballState,bool useEffects=true);
    ~BallInfo();

    BallInfo( const BallInfo&) =delete;
    BallInfo( const BallInfo&&) =delete;

    BallInfo& operator =( const BallInfo&) = delete;
    BallInfo& operator =( const BallInfo&&) =  delete;// no move or copy operations used, so suppress them

    void update(float dt);

    void setPosition(const FPoint& p);
    void setVelocity(const FPoint& v);
    void setAcceleration( const FPoint& a);
    void setState(int newState,int scoreWhenDropped=0);
    void setColor(int newColor);
    void setAlpha(int newAlpha);
    void setRadius(float newRadius);


    FPoint getPosition();
    FPoint getVelocity();
    int getState();
    int getColor();
    int getAlpha();
    float getRadius();

    void renderBall(int handle,const SDL_Rect& rc, const FPoint& offset=FPoint(0.,0.));
    bool updateEffects(float dt, float phase);
    void renderEffects(bool showShine=true, bool showOthers=true, const FPoint& offset=FPoint(0.,0.));
    void transformEffects(const FPoint& pt);

    void explode();
    void explodeWithSparks();
    void fade(float rate);

    void saveState(SDL_RWops* out);
    void loadState(SDL_RWops* in);
};

//=================================================
class InGame: public GameState
{

    friend class LevelButton;

    enum TEXTURE_TYPES//game
    {
        //TEX_BACKGROUND,
        TEX_WELL_BACKGROUND=NUM_BALLCOLORS+NUM_OBSTACLES,
        TEX_WELL_TOP,
        TEX_WELL_BOTTOM,
        TEX_WELL_LEFT,
        TEX_WELL_RIGHT,
        TEX_WELL_LCORNER,
        TEX_WELL_RCORNER,
        TEX_WELL_PISTONS,
        TEX_WELL_TOP_PLATE,
        TEX_WELL_ENDLINE,
        TEX_MACHINE,
        TEX_CANNON,
        TEX_WELL_PISTON_BACKGROUND,
        TEX_BALL_INDEX,
        TEX_TIMESTOP,
        NUM_TEXTURES
    };

    enum PLAY_MODES//level
    {
        MODE_PLAY=0,
        MODE_PAUSE,
        MODE_QUIT,
        MODE_EDITOR,
        MODE_ENDGAME,
        MODE_GAME_MENU,
        MODE_LEVEL_SELECT,
        MODE_CONTINUE,
        MODE_PREGAME,
        MODE_SUMMON_MENU
    };

    enum OBJECT_TYPE//well
    {
        TYPE_WELL_TOP=0,
        TYPE_WELL_SIDE,
        TYPE_BALL
    };

    enum STAT_TYPE
    {
        STAT_CURRENT=0,
        STAT_BEST,
        STAT_LEGEND
    };

    struct collisionInfo//level
    {
        float dt;
        int type;
        GridPoint gp;//for balls
    };

    int WELL_WIDTH;
    int WELL_HEIGHT;

    int BALL_INDEX_LENGTH;

    TTF_Font *largeMessageFont;
    TTF_Font *mediumMessageFont;
    TTF_Font *smallMessageFont;

    Flash* flashEffect;
    TrailsEffect* trailsEffect;


    //BaseTexture* texScore;
    BaseTexture* texLevel;

    BaseTexture* texRoundScore[3];
    BaseTexture* texRoundTime[3];
    BaseTexture* texRoundShots[3];

    BaseTexture *texMessage;
    BaseTexture *texCountDown;

    BaseTexture *texStaticBalls;
    BaseTexture *texShiningBalls;
    BaseTexture *texBallIndex;

    BaseTexture *texWellHardware;
    BaseTexture *texLauncher;
    BaseTexture *texPaused;


    bool invTexStatic;
    bool invTexShining;
    bool invTexBallIndex;
    bool invTexWellHardware;
    bool invTexLauncher;

    int handles[NUM_TEXTURES];
    SDL_Rect* recs[NUM_TEXTURES];

    bool soundPlaying[NUM_SOUND_EFFECTS];

    std::pair<FPoint,int> palette[NUM_BALLCOLORS-NUM_SPECIALBALLS+2];

    SDL_Rect rcWell;
    SDL_Rect rcBorder;

    BallInfo* well[MAX_WELL_HEIGHT+2][MAX_WELL_WIDTH];//stores static balls
    int conmap[MAX_WELL_HEIGHT+2][MAX_WELL_WIDTH];//connectivity map
    bool collisionTestMatrix[MAX_WELL_HEIGHT+2][MAX_WELL_WIDTH];

    collisionInfo collisions[MAX_COLLISIONS];//can't have more than 4 of any type at the same time

    int colorStats[NUM_BALLCOLORS+NUM_OBSTACLES];

    FPoint ballIndex[MAX_BALL_INDEX_LENGTH];
    bool indexCollides;
    int indexLength;

    FPoint lastVelocity;

    LocalEffect* summonEffect;
    BaseTexture *texSummon;
    FPoint summonPoint;
    int summonAlpha;

    std::list<BallInfo*> fallingBalls;
    std::list<GridPoint> ballTuple;


    BallInfo* currentBall;
    BallInfo* nextBall;

    FPoint launchPoint;

    Timer roundTimer, shotTimer, countDownTimer;
    int pistonDuration, shotDuration;

    float ball_radius;
    float topSurfaceY;
    float launchLimit;
    float collision_slack;
    //GAMEPLAY PARAMETERS
    int gameLevel;

    int playState;//PLAY_MODE
    int prevState;
    bool warnedForQuit;
    int editorSeq;

    int score,roundScore;
    int piston_step;
    int initial_step;

    int match4;
    int match5;
    int match6;

    int startSkin;


    //LAUNCHER ANGLE AND MOVEMENT
    float angle; //MIN-MAX
    float angleDiff;//dAngle
    float velocityMultiplier;

    int numCollisions;
    int numStaticBalls;
    int numInitialBalls;

    bool powerBallGenerated;
    bool isEffectActive;
    bool timeStopActive;
    bool timeStopUsed;
    bool ballSummoned;

    int ballToSummon;
#ifdef PLATFORM_ANDROID
    bool lastShotWasAuto;
    bool touchHappened;
    bool justSwitched;
    FPoint fingerVel;
#endif

    //GAME MENU
    GameMenu *gameMenu;
    LevelSelector *levelSelector;
    SummonMenu *summonMenu;

    //ENDGAME
    EndGame *endGame;

    //PAUSE AND QUIT
    PauseScreen *pauseScreen;

    //initialization
    void computeDimensions(SDL_Rect* displayArea = screenRect);
    void resizeComponents();

    void initGame(int action=0, int level=0);
    void initRound(bool loadSavedState=false);
    void initFonts();

    void buildEndgameMessages( bool hasWon );
    void endLevel( bool hasWon );

    void loadImages();
    void unloadImages();
    void endSound();
    void initCursor();
    void disableCursor();

    void initTimers();
    void stopTimers();
    void pauseTimers();
    void resumeTimers();
    void startCountDownTimer();
    void setDurations();
    void setVelocity();

    void emptyWell();
    void initWell();

    void initPalette();

//level and state----------------------------

    int generateRandomLevel(int levelmap[MAX_WELL_HEIGHT][MAX_WELL_WIDTH], RandomGenerator& levelGen, int MAXRAND);
    int generatePlasmaLevel(int levelmap[MAX_WELL_HEIGHT][MAX_WELL_WIDTH], RandomGenerator& levelGen);
    int generateBoxMapLevel(int levelmap[MAX_WELL_HEIGHT][MAX_WELL_WIDTH], RandomGenerator& levelGen);
    void generateLevelBlocks(int levelmap[MAX_WELL_HEIGHT][MAX_WELL_WIDTH], RandomGenerator& levelGen, int numLines);
    void generateLevel(int level,bool selectorMode=false);

    void makeBufferFromLevel( char* buffer);

    void readLevel(int level);
    void saveLevel();
    void saveGameState();
    bool loadGameState();

    void createBall(const GridPoint &pt, int color, bool useEffects=true);

    void initBalls(int currentColor=-1);
    int findLowestBall(GridPoint& gp);
    int chooseNextBallColor(bool checkCurrentColor=true);
    void switchColors();

    //controls
    void escapePressed();
    void quitSequence();
    void pauseGame(int mode,bool makeAggregates=true);

//----aiming-----------------------------
    void increaseAngle();
    void decreaseAngle();
    void accelAngle();
    void decelAngle();
    void setAngle( FPoint dp);
    void computeBallIndex();

    void launchBall();
    void slotNextBall();
    void setNextBallPos();

    void anchorBall(const GridPoint& gp);

//--------------------------------------------------------
    void activateTimeStop();
    void deactivateTimeStop();
    void setSummonIndicator();
    void displaySummonIndicator();


// anchor response----------------------------------------

    int findMaxTupleColor(const GridPoint& p);
    void initMap();
    void findColorTuple(const GridPoint &pt,int color);
    void DFS(const GridPoint &pt);
    void BFS(const GridPoint& pt,int maxDepth, int maxballs, std::vector<GridPoint> &pv);

    void dropBombAndNeighbors(const GridPoint &pt);
    void dropSameColored(const GridPoint &pt, int color=-1);

    void activateColorShifter( const GridPoint& gp);
    void activateLeveler( const GridPoint& gp);
    void activatePainter( const GridPoint& gp, int collisionIndex);

    bool dropUnconnectedBalls(bool doingLevelGeneration =false ,const GridPoint &dropper=GridPoint(-1,-1));
    void dropBall( const GridPoint &gp,int scoreType=0,const GridPoint &dropper=GridPoint(-1,-1));
    void deleteBall(const GridPoint &gp);

//falling balls--------------------------------------------
    void deleteFallingBalls();
    bool isBallInvisible(BallInfo* b);
    void moveFallingBalls(float dt);
    void removeVanishedBalls();
//-----------------------------collision detection----------------------
    GridPoint doCollision(int index);
    void resetCollisions();

    float findWallCollision(float timeLimit,const FPoint& ballPos, const FPoint& ballVel);
    void findBallCollisions(float timeLimit,const FPoint& ballPos, const FPoint& ballVel);

    bool testVertexNeighBors(const FPoint& pos, const FPoint& vel,
                             const GridPoint& gp, float timeLimit,bool realMode=true);

    bool testCollisionPoint(const FPoint& pos, const FPoint& vel,const GridPoint& gp, float timeLimit,bool realMode=true);
    bool collidesWith(const FPoint& pos, const FPoint& vel, const GridPoint &gp,float timeLimit);
    bool testBallCollision(FPoint b1, FPoint b2, float r1, float r2);
    float findCollisionTime(const FPoint& pos, const FPoint& vel,const GridPoint &pt);

    void addCollision(const FPoint& pos, const FPoint& vel,const GridPoint &pt,float dt);

    bool outOfBounds(const GridPoint& gp);
    void emptyCollisionTestMatrix();
    bool pointAlreadyTested(const GridPoint& gp);
    void markPointAsTested( const GridPoint& gp);

    GridPoint findNearestGridPos(FPoint realPos);

    //gameplay-------------------------------------
    void checkTopPlateAction();
    void resetTopPlate();
    bool lowerTopPlate(int numSteps=1);
    bool raiseTopPlate();
    bool adjustStaticPositions();

    //updates
    void updateLevelMessage();
    //void updateScoreMessage();

    void createBestMessages();
    void createLegendMessages();
    void updateRoundScoreMessage();
    void updateRoundTimeMessage(bool forcedUpdate=false);
    void updateRoundShotsMessage();

    void updateCountDownMessage()    ;
    void setGameMessage(std::string s);
    void updateBallEffects(float dt, float phase);
    void transformBallEffects(const FPoint &pt);

    //display optimizations-------------------------------
    void updateStaticBallImage();
    void updateShineEffectImage();
    void updateBallIndexImage();
    void updateWellHardwareImage();
    void updateLauncherImage();
    void updatePausedImage();
    //display----------------------------------------------
    void displayGameInfo();
    void renderLevelStats(const FPoint& offset=FPoint(0.f,0.f));

    void renderStaticObject(int handle,int alpha=255);
    void renderOffsetStaticObject(int handle, const FPoint& offset,int alpha=255);
    void displayMessages();
    void displayWellHardware();
    void displayWellBackground(float phase);
    void displayWellSides();
    void displayWellCorners( const FPoint& offset=FPoint(0.f,0.f));
    void displayWellCompressor();
    void displayWellCompressorBackground();

    void displayMatrix();// - debug -
    void displayApproximateGridPos(FPoint realPos);

    void displayStaticBalls();
    void displayFallingBalls();
    void displayBallIndex();
    void renderBallIndex(const FPoint& offset=FPoint(0.f,0.f));//, bool renderPath=true, bool renderGhost=true);
    void displayBall(BallInfo* ball);//for playing balls
    void displayLauncher();
    void displayEndLine(float phase);
    void displayBallEffects();
    void displayPalette(const FPoint& offset=FPoint(0.f,0.f));
    //helper

    int xIndex(const GridPoint &pt, int rowdiff, int  neighbor);
    int maxBalls(int row);
    int getWellMatrixHeight();

    float getXCoord(int row, int column);
    float getYCoord(int row );
    float getBallXCoord(int row, int column);
    float getBallYCoord(int row );

    void gameMenuInit();
    void levelSelectorInit(int level);
    void summonMenuInit();

    void gameRender(float phase=0.);
    void gameMenuRender(float phase=0.);
    void levelSelectorRender(float phase=0.);
    void endGameRender(float phase=0.);
    void pauseScreenRender(float phase=0.);
    void summonMenuRender(float phase=0.);

    void countDownLogic(float dt);
    void gameLogic(float dt, float phase=0.);
    void gameMenuLogic(float dt);
    void levelSelectorLogic(float dt);
    void endGameLogic(float dt);
    void pauseScreenLogic(float dt);
    void summonMenuLogic(float dt);

    bool gameHandleEvents(SDL_Event& event);
    bool editorHandleEvents(SDL_Event& event);
    bool preGameHandleEvents(SDL_Event& event);
    void gameMenuHandleEvents(SDL_Event& event);
    void levelSelectorHandleEvents(SDL_Event& event);
    void endGameHandleEvents(SDL_Event& event);
    void pauseScreenHandleEvents(SDL_Event& event);
    void summonMenuHandleEvents(SDL_Event& event);

    void editShiftBalls(int shift_type);

    void saveRawLevel(const char* filename,char* buffer);

    void resetEffectFlags();
    void soundEffect(int effect);
protected:
    //========FOR FRIENDS================
    bool loadRawLevel(const char* filename,char* buffer,bool selectorMode=false,int level=-1);
    void renderRawLevel(char* buffer);

 public:

    InGame();
    ~InGame();

    InGame( const InGame&) = delete;
    InGame( const InGame&& ) = delete;

    InGame& operator =( const InGame&) = delete;
    InGame& operator =( const InGame&&) = delete;

    void Logic(float dt, float phase=0.);
    void Render(float phase=0.);
    void HandleEvents();
};

#endif // IN_GAME_H

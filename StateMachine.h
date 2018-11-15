#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

class BaseTexture;

class GameState;

//==============STATEMACHINE IMPLEMENTATION=================

    enum GAME_STATES        //valid game states, used to switch between states
    {
        STATE_NULL=0,          //no state change necessary
        STATE_ENTRY,
        STATE_TITLE,        //title screen
        STATE_OPTIONS,      //game settings
        STATE_PROFILES,      //player profiles
        STATE_INGAME,       //playing phase
        STATE_ABOUT,
        STATE_PROGRESS,
        STATE_EXIT         //exit requested
    };

class StateMachine
{

    int stateID;
    static int nextState;

    BaseTexture *texBackground;

    GameState* currentState;

    void ChangeState();
    void renderBackground(float phase);
    void loadImages();


    public:

    StateMachine();
    ~StateMachine();

    StateMachine( const StateMachine&) = delete;
    StateMachine( const StateMachine&&) = delete;
    StateMachine& operator =(const StateMachine&) = delete;
    StateMachine& operator =(const StateMachine&&) = delete;// suppressing copy construction, assignment and move operations


    void Run();
    static void SetNextState(int newState);

};

#endif // STATE_MACHINE_H

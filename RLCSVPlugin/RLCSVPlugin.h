#pragma once
#pragma comment(lib, "BakkesMod.lib")

#include "bakkesmod/plugin/bakkesmodplugin.h"

#include <fstream>

typedef struct {
    float initialMMR;
    float newMMR;
} PlayerMMR;

class RLCSVPlugin : public BakkesMod::Plugin::BakkesModPlugin {
private:
    int teamNumber;
    map<unsigned long long, PlayerMMR> playerMMR;

    void writeCSV(std::ofstream& f, ArrayWrapper<TeamWrapper> teams);
    void writePlayersCSV(std::ofstream& f, ArrayWrapper<PriWrapper> players, int team);
    void getInitialMMR(ArrayWrapper<TeamWrapper> teams, int team);
    void getNewMMR(ArrayWrapper<TeamWrapper> teams, int team);
    float getPlayerMMR(PriWrapper player);
    void logCVarChange(std::string oldValue, CVarWrapper cvar);

public:
    virtual void onLoad();
    virtual void onUnload();

    void startGame(std::string eventName);
    void endGame(std::string eventName);
};

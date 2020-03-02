#pragma once
#pragma comment(lib, "BakkesMod.lib")

#include "bakkesmod/plugin/bakkesmodplugin.h"

#include <fstream>

typedef struct {
    float initialMMR, currentMMR;
    int tier, div;
} StatsStruct;

class RLCSVPlugin : public BakkesMod::Plugin::BakkesModPlugin {
private:
    int teamNumber;
    int currentPlaylist;
    std::map<int, StatsStruct> stats;
    SteamID mySteamID;

    void writeCSV(std::ofstream& f, ArrayWrapper<PriWrapper> players, int team);
    void logStatusToConsole(std::string oldValue, CVarWrapper cvar);

public:
    virtual void onLoad();
    virtual void onUnload();

    void StartGame(std::string eventName);
    void EndGame(std::string eventName);
};

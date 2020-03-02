#pragma once
#pragma comment(lib, "BakkesMod.lib")

#include "bakkesmod/plugin/bakkesmodplugin.h"

#include <fstream>

class RLCSVPlugin : public BakkesMod::Plugin::BakkesModPlugin {
private:
    int teamNumber;
    int playlist;
    float initialMMR;
    float newMMR;
    SteamID mySteamID;

    void writePlayersCSV(std::ofstream& f, ArrayWrapper<PriWrapper> players, int team);
    void logStatusToConsole(std::string oldValue, CVarWrapper cvar);

public:
    virtual void onLoad();
    virtual void onUnload();

    void StartGame(std::string eventName);
    void EndGame(std::string eventName);
};

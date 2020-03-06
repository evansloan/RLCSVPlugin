#pragma once
#pragma comment(lib, "BakkesMod.lib")

#include "bakkesmod/plugin/bakkesmodplugin.h"

#include <map>
#include <string>

typedef struct {
    int team;
    std::string name;
    int score;
    int goals;
    int assists;
    int saves;
    int shots;
    int teamScore;
    float mmr;
} Stats;

class RLCSVPlugin : public BakkesMod::Plugin::BakkesModPlugin {
private:
    void writeCSV();
    std::map<std::string, Stats> getPlayerStats(ArrayWrapper<TeamWrapper> teams, ArrayWrapper<PriWrapper> players);
    float getPlayerMMR(MMRWrapper mw, PriWrapper player);
    std::string getTimeStamp(bool includeTime);

public:
    virtual void onLoad();
    virtual void onUnload();

    void onMatchEnded(std::string eventName);
};

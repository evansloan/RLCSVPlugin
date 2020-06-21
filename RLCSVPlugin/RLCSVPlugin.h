#pragma once
#pragma comment(lib, "pluginsdk.lib")

#include <bakkesmod/plugin/bakkesmodplugin.h>

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
    int damage;
    int mvp;
    int teamScore;
    float mmr;
} Stats;

class RLCSVPlugin : public BakkesMod::Plugin::BakkesModPlugin {
private:
    void writeCSV();
    std::map<std::string, Stats> getPlayerStats(ArrayWrapper<TeamWrapper> teams, ArrayWrapper<PriWrapper> players);
    float getPlayerMMR(MMRWrapper mw, PriWrapper player);
    std::string getTimeStamp(std::string format);
    void createDirectory(std::string path);

public:
    virtual void onLoad();
    virtual void onUnload();

    void onMatchEnded(std::string eventName);
};

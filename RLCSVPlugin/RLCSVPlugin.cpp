#include "RLCSVPlugin.h"

#include <ctime>
#include <direct.h>
#include <fstream>
#include <iomanip>
#include <sstream>

BAKKESMOD_PLUGIN(RLCSVPlugin, "RLCSV Plugin", "0.1", 0)

const std::string saveLocation = "bakkesmod/data/RLCSV/";

void RLCSVPlugin::onLoad() {
    std::stringstream ss;
    ss << exports.pluginName << " version: " << exports.pluginVersion;
    cvarManager->log(ss.str());

    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchEnded", std::bind(&RLCSVPlugin::onMatchEnded, this, std::placeholders::_1));

    if (_mkdir(saveLocation.c_str()) == 0) {
        cvarManager->log("Save directory created at: " + saveLocation);
    }
}


void RLCSVPlugin::onUnload() {
    gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchEnded");
}

void RLCSVPlugin::onMatchEnded(std::string eventName) {
    cvarManager->log("Event " + eventName);

    ServerWrapper sw = gameWrapper->GetOnlineGame();
    if (sw.IsNull()) {
        cvarManager->log("ServerWrapper is null. CSV cannot be recorded");
        return;
    }

    writeCSV();
    cvarManager->log("CSV file successfully written");
}

void RLCSVPlugin::writeCSV() {
    ServerWrapper sw = gameWrapper->GetOnlineGame();
    if (sw.IsNull()) {
        cvarManager->log("ServerWrapper is null. CSV cannot be recorded");
        return;
    }

    std::stringstream ss;

    PriWrapper localPlayer = sw.GetLocalPrimaryPlayer().GetPRI();
    ArrayWrapper<TeamWrapper> teams = sw.GetTeams();

    int myScore = teams.Get(localPlayer.GetTeamNum()).GetScore();
    int otherScore;

    if (localPlayer.GetTeamNum() == 0) {
        otherScore = teams.Get(1).GetScore();
    } else {
        otherScore = teams.Get(0).GetScore();
    }

    std::string result = myScore > otherScore ? "WIN" : "LOSS";

    std::string folderName = saveLocation + getTimeStamp(false) + "/";
    if (_mkdir(folderName.c_str()) == 0) {
        cvarManager->log(folderName + " directory created");
    }

    ss << folderName << getTimeStamp(true) << "_" << result << "_" << myScore << "-" << otherScore << ".csv";

    std::string filename = ss.str();
    cvarManager->log("CSV file directory: " + filename);
    std::ofstream f(filename);

    ss.str(std::string());

    f << "Team,Player,Score,Goals,Assists,Saves,Shots,Team Score,MMR\n";

    map<std::string, Stats> playerStats = getPlayerStats(teams, sw.GetPRIs());

    for (auto const &pair : playerStats) {
        Stats player = pair.second;
        cvarManager->log("Writing player " + player.name);
        ss << player.team << "," << player.name << "," << player.score << "," << player.goals << "," << player.assists << ","
            << player.saves << "," << player.shots << "," << player.teamScore << "," << player.mmr << "\n";
        f << ss.str();
        ss.str(std::string());
    }

    f.close();
}

std::map<std::string, Stats> RLCSVPlugin::getPlayerStats(ArrayWrapper<TeamWrapper> teams, ArrayWrapper<PriWrapper> players) {
    cvarManager->log("Getting player stats");

    MMRWrapper mw = gameWrapper->GetMMRWrapper();
    std::map<std::string, Stats> playerStats;

    for (int i = 0; i < players.Count(); i++) {
        PriWrapper player = players.Get(i);
        std::string playerID = std::to_string(player.GetUniqueId().ID);
        int team = player.GetTeamNum2();
        std::string name = player.GetPlayerName().ToString();
        int score = player.GetMatchScore();
        int goals = player.GetMatchGoals();
        int assists = player.GetMatchAssists();
        int saves = player.GetMatchSaves();
        int shots = player.GetMatchShots();
        int teamScore = teams.Get(team).GetScore();
        float mmr = getPlayerMMR(mw, player);

        playerStats[playerID] = Stats{ team, name, score, goals, assists, saves, shots, teamScore, mmr };
    }
    return playerStats;
}

float RLCSVPlugin::getPlayerMMR(MMRWrapper mw, PriWrapper player) {
    int playlist = mw.GetCurrentPlaylist();
    SteamID playerID = player.GetUniqueId();
    return mw.GetPlayerMMR(playerID, playlist);
}

std::string RLCSVPlugin::getTimeStamp(bool includeTime) {
    const std::time_t now = std::time(nullptr);
    const std::tm currTimeLocal = *std::localtime(std::addressof(now));

    std::stringstream ss;
    std::string format = "%y%m%d";

    if (includeTime) {
        format += "T%H%M%S";
    }

    ss << std::put_time(std::addressof(currTimeLocal), format.c_str());

    return ss.str();
}
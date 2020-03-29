#include "RLCSVPlugin.h"

#include <ctime>
#include <direct.h>
#include <fstream>
#include <iomanip>
#include <sstream>

BAKKESMOD_PLUGIN(RLCSVPlugin, "RLCSV Plugin", "0.5", 0)

void RLCSVPlugin::onLoad() {
    std::stringstream ss;
    ss << exports.pluginName << " version: " << exports.pluginVersion;
    cvarManager->log(ss.str());

    cvarManager->registerCvar("rlcsv_save_path", "bakkesmod/data/RLCSV/", "Location to save CSV files. Use \"/\" as path seperator", true, false, 0, false, 0, true);
    cvarManager->registerCvar("rlcsv_save_structure", "0", "Save CSV files in folders based on date and game mode", true, true, 0, true, 1);

    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchEnded", std::bind(&RLCSVPlugin::onMatchEnded, this, std::placeholders::_1));

    createDirectory(cvarManager->getCvar("rlcsv_save_path").getStringValue());
}


void RLCSVPlugin::onUnload() {
    gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchEnded");
}

void RLCSVPlugin::onMatchEnded(std::string eventName) {
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

    std::string timestamp = getTimeStamp("%Y%m%dT%H%M%S");
    std::string result = myScore > otherScore ? "WIN" : "LOSS";
    std::string gameMode = sw.GetPlaylist().GetTitle().ToString();
    bool isRanked = sw.GetPlaylist().GetbRanked();
    gameMode = isRanked ? gameMode + "_Ranked" : gameMode;

    std::string baseFolder = cvarManager->getCvar("rlcsv_save_path").getStringValue();
    ss << baseFolder;

    if (cvarManager->getCvar("rlcsv_save_structure").getBoolValue()) {
        std::string dateFolder = getTimeStamp("%Y-%m-%d") + "/";
        createDirectory(baseFolder + dateFolder);
        std::string modeFolder = dateFolder + gameMode + "/";
        createDirectory(baseFolder + modeFolder);
        ss << modeFolder;
    }

    ss << timestamp << "_" << gameMode << "_"
       << result << "_" << myScore << "-" << otherScore << ".csv";
    std::string filename = ss.str();
    ss.str(std::string());

    std::ofstream f(filename);
    f << "Timestamp,Game mode,Team,Player,Score,Goals,Assists,Saves,Shots,Damage,MVP,Team Score,Win,MMR\n";

    map<std::string, Stats> playerStats = getPlayerStats(teams, sw.GetPRIs());

    for (auto const &pair : playerStats) {
        Stats player = pair.second;
        ss << timestamp << "," << gameMode << "," << player.team << "," << player.name << "," << player.score << "," << player.goals << ","
           << player.assists << "," << player.saves << "," << player.shots << "," << player.damage << "," << player.mvp << ","
           << player.teamScore << "," << (player.team == sw.GetWinningTeam().GetTeamNum()) << "," << player.mmr << "\n";
        f << ss.str();
        ss.str(std::string());
    }

    f.close();
}

std::map<std::string, Stats> RLCSVPlugin::getPlayerStats(ArrayWrapper<TeamWrapper> teams, ArrayWrapper<PriWrapper> players) {
    MMRWrapper mw = gameWrapper->GetMMRWrapper();
    std::map<std::string, Stats> playerStats;

    for (int i = 0; i < players.Count(); i++) {
        PriWrapper player = players.Get(i);
        std::string playerID = std::to_string(player.GetUniqueId().ID);
        int team = player.GetTeamNum();
        std::string name = player.GetPlayerName().ToString();
        int score = player.GetMatchScore();
        int goals = player.GetMatchGoals();
        int assists = player.GetMatchAssists();
        int saves = player.GetMatchSaves();
        int shots = player.GetMatchShots();
        int damage = player.GetMatchBreakoutDamage();
        int mvp = player.GetbMatchMVP();
        int teamScore = teams.Get(team).GetScore();
        float mmr = getPlayerMMR(mw, player);

        playerStats[playerID] = Stats{ team, name, score, goals, assists, saves, shots, damage, mvp, teamScore, mmr };
    }
    return playerStats;
}

float RLCSVPlugin::getPlayerMMR(MMRWrapper mw, PriWrapper player) {
    int playlist = mw.GetCurrentPlaylist();
    SteamID playerID = player.GetUniqueId();
    return mw.GetPlayerMMR(playerID, playlist);
}

std::string RLCSVPlugin::getTimeStamp(std::string format) {
    const std::time_t now = std::time(nullptr);
    const std::tm currTimeLocal = *std::localtime(&now);

    std::stringstream ss;
    ss << std::put_time(&currTimeLocal, format.c_str());

    return ss.str();
}

void RLCSVPlugin::createDirectory(std::string path) {
    if (_mkdir(path.c_str()) == 0) {
        cvarManager->log(path + " directory created");
    }
}
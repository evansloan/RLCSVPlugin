#include "RLCSVPlugin.h"

BAKKESMOD_PLUGIN(RLCSVPlugin, "RLCSV Plugin", "0.1", 0)

const std::string saveLocation = "data/RLCSV";

void RLCSVPlugin::onLoad() {
    std::stringstream ss;
    ss << exports.pluginName << " version: " << exports.pluginVersion;
    cvarManager->log(ss.str());

    cvarManager->registerCvar("cl_rlcsv_csv_directory", "bakkesmod/" + saveLocation + "/", "Directory to write CSV files to (use forward slash '/' as separator in path", true, false, (0.0F), false, (0.0F), true);
    cvarManager->getCvar("cl_rlcsv_csv_directory").addOnValueChanged(std::bind(&RLCSVPlugin::logCVarChange, this, std::placeholders::_1, std::placeholders::_2));

    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchEnded", std::bind(&RLCSVPlugin::onMatchEnded, this, std::placeholders::_1));
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

    ss << std::time(0) << "_" << result << "_" << myScore << "-" << otherScore << ".csv";

    std::string filename = cvarManager->getCvar("cl_rlcsv_csv_directory").getStringValue() + ss.str();
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

void RLCSVPlugin::logCVarChange(std::string oldValue, CVarWrapper cvar) {
    std::stringstream ss;
    ss << "cl_rlcsv_csv_directory: '" << cvarManager->getCvar("cl_rlcsv_csv_directory").getStringValue();
    cvarManager->log(ss.str());
}

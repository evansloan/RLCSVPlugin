#include "RLCSVPlugin.h"

#include <sstream>

BAKKESMOD_PLUGIN(RLCSVPlugin, "RLCSV Plugin", "0.1", 0)

void RLCSVPlugin::onLoad() {
    std::stringstream ss;
    ss << exports.pluginName << " version: " << exports.pluginVersion;
    cvarManager->log(ss.str());

    cvarManager->registerCvar("cl_rlcsv_csv_directory", "bakkesmod/data", "Directory to write CSV files to (use forward slash '/' as separator in path", true, false, (0.0F), false, (0.0F), true);
    cvarManager->getCvar("cl_rlcsv_csv_directory").addOnValueChanged(std::bind(&RLCSVPlugin::logStatusToConsole, this, std::placeholders::_1, std::placeholders::_2));

    gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", bind(&RLCSVPlugin::StartGame, this, std::placeholders::_1));
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet", bind(&RLCSVPlugin::EndGame, this, std::placeholders::_1));
}


void RLCSVPlugin::onUnload() {

}

void RLCSVPlugin::StartGame(std::string eventName) {
    if (!gameWrapper->IsInOnlineGame() || gameWrapper->IsInReplay()) return;

    mySteamID.ID = gameWrapper->GetSteamID();
    cvarManager->log("SteamID: " + std::to_string(mySteamID.ID));

    ServerWrapper sw = gameWrapper->GetOnlineGame();

    if (!sw.IsNull() && sw.IsOnlineMultiplayer()) {
        CarWrapper me = gameWrapper->GetLocalCar();

        if (!me.IsNull()) {
            teamNumber = me.GetTeamNum2();
        } else {
            teamNumber = -1;
        }

        MMRWrapper mw = gameWrapper->GetMMRWrapper();
        playlist = mw.GetCurrentPlaylist();
        initialMMR = mw.GetPlayerMMR(mySteamID, playlist);
    }
}

void RLCSVPlugin::EndGame(std::string eventName) {
    if (teamNumber == -1) {
        CarWrapper me = gameWrapper->GetLocalCar();
        if (!me.IsNull()) {
            teamNumber = me.GetTeamNum2();
        }
    }

    ServerWrapper sw = gameWrapper->GetOnlineGame();

    if (!sw.IsNull()) {
        ArrayWrapper<TeamWrapper> teams = sw.GetTeams();
        std::stringstream ss;

        if (teams.Count() == 2) {
            std::string guid = sw.GetMatchGUID();
            MMRWrapper mw = gameWrapper->GetMMRWrapper();
            newMMR = mw.GetPlayerMMR(mySteamID, playlist);

            int score0 = teams.Get(0).GetScore();
            int score1 = teams.Get(1).GetScore();

            if ((score0 > score1&& teamNumber == 0) || (score1 > score0&& teamNumber == 1)) {
                ss << "WIN: " << score0 << "-" << score1 << " - " << guid << ".csv";
            } else {
                ss << "LOSS: " << score0 << "-" << score1 << " - " << guid << ".csv";
            }

            std::string filename = cvarManager->getCvar("cl_rlcsv_csv_directory").getStringValue() + ss.str();
            std::ofstream f(filename, ios::app);

            ArrayWrapper<PriWrapper> players0 = teams.Get(0).GetMembers();
            ArrayWrapper<PriWrapper> players1 = teams.Get(1).GetMembers();

            f << "Initial MMR,New MMR\n";
            f << std::to_string(initialMMR) + "," + std::to_string(newMMR) + "\n";

            writePlayersCSV(f, players0, 0);
            writePlayersCSV(f, players1, 1);
        }
    }
}

void RLCSVPlugin::writePlayersCSV(std::ofstream& f, ArrayWrapper<PriWrapper> players, int team) {
    f << "Team: " + std::to_string(team) + "\n";
    f << "Player,Score,Goals,Assists,Saves,Shots\n";

    for (int i = 0; i < players.Count(); i++) {
        std::string name = players.Get(i).GetPlayerName().ToString();
        int score = players.Get(i).GetScore();
        int goals = players.Get(i).GetMatchGoals();
        int assists = players.Get(i).GetMatchAssists();
        int saves = players.Get(i).GetMatchSaves();
        int shots = players.Get(i).GetMatchShots();

        f << name + "," + std::to_string(score) + "," + std::to_string(goals) + "," + std::to_string(assists) + "," + std::to_string(saves) + "," + std::to_string(shots) + "\n";
    }

    f << "\n";
}

void RLCSVPlugin::logStatusToConsole(std::string oldValue, CVarWrapper cvar) {
    std::stringstream ss;
    ss << "cl_rlcsv_csv_directory: '" << cvarManager->getCvar("cl_rlcsv_csv_directory").getStringValue();
    cvarManager->log(ss.str());
}
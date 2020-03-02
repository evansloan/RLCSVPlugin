#include "RLCSVPlugin.h"

#include <sstream>

BAKKESMOD_PLUGIN(RLCSVPlugin, "RLCSV Plugin", "0.1", 0)

void RLCSVPlugin::onLoad() {
    std::stringstream ss;
    ss << exports.pluginName << " version: " << exports.pluginVersion;
    cvarManager->log(ss.str());

    cvarManager->registerCvar("cl_rlcsv_csv_directory", "bakkesmod/data", "Directory to write CSV files to (use forward slash '/' as separator in path", true, false, (0.0F), false, (0.0F), true);
    cvarManager->getCvar("cl_rlcsv_csv_directory").addOnValueChanged(std::bind(&RLCSVPlugin::logCVarChange, this, std::placeholders::_1, std::placeholders::_2));

    gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", bind(&RLCSVPlugin::startGame, this, std::placeholders::_1));
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet", bind(&RLCSVPlugin::endGame, this, std::placeholders::_1));
}


void RLCSVPlugin::onUnload() {

}

void RLCSVPlugin::startGame(std::string eventName) {
    if (!gameWrapper->IsInOnlineGame() || !gameWrapper->IsInGame() || gameWrapper->IsInReplay()) return;

    cvarManager->log("SteamID: " + std::to_string(gameWrapper->GetSteamID.ID));

    ServerWrapper sw = gameWrapper->GetOnlineGame();

    if (!sw.IsNull() && sw.IsOnlineMultiplayer()) {
        CarWrapper me = gameWrapper->GetLocalCar();

        if (!me.IsNull()) {
            teamNumber = me.GetTeamNum2();
        } else {
            teamNumber = -1;
        }

        ArrayWrapper<TeamWrapper> teams = sw.GetTeams();
        getInitialMMR(teams, 0);
        getInitialMMR(teams, 1);
    }
}

void RLCSVPlugin::endGame(std::string eventName) {
    if (teamNumber == -1) {
        CarWrapper me = gameWrapper->GetLocalCar();
        if (!me.IsNull()) {
            teamNumber = me.GetTeamNum2();
        }
    }

    ServerWrapper sw = gameWrapper->GetOnlineGame();

    if (!sw.IsNull()) {
        ArrayWrapper<TeamWrapper> teams = sw.GetTeams();
        getNewMMR(teams, 0);
        getNewMMR(teams, 1);

        if (teams.Count() == 2) {
            std::string guid = sw.GetMatchGUID();
            int score0 = teams.Get(0).GetScore();
            int score1 = teams.Get(1).GetScore();

            std::stringstream ss;

            if ((score0 > score1&& teamNumber == 0) || (score1 > score0&& teamNumber == 1)) {
                ss << "WIN: " << score0 << "-" << score1 << " - " << guid << ".csv";
            } else {
                ss << "LOSS: " << score0 << "-" << score1 << " - " << guid << ".csv";
            }

            std::string filename = cvarManager->getCvar("cl_rlcsv_csv_directory").getStringValue() + ss.str();
            std::ofstream f(filename, ios::app);

            ArrayWrapper<PriWrapper> players0 = teams.Get(0).GetMembers();
            ArrayWrapper<PriWrapper> players1 = teams.Get(1).GetMembers();

            writeCSV(f, teams);
        }
    }
}

void RLCSVPlugin::writeCSV(std::ofstream& f, ArrayWrapper<TeamWrapper> teams) {
    f << "Team,Player,Score,Goals,Assists,Saves,Shots,Demos,Damage,Initial MMR,New MMR\n";

    ArrayWrapper<PriWrapper> players0 = teams.Get(0).GetMembers();
    ArrayWrapper<PriWrapper> players1 = teams.Get(1).GetMembers();
    writePlayersCSV(f, players0, 0);
    writePlayersCSV(f, players1, 1);
}

void RLCSVPlugin::writePlayersCSV(std::ofstream& f, ArrayWrapper<PriWrapper> players, int team) {
    std::stringstream ss;

    for (int i = 0; i < players.Count(); i++) {
        PriWrapper player = players.Get(i);

        std::string name = player.GetPlayerName().ToString();
        int score = player.GetScore();
        int goals = player.GetMatchGoals();
        int assists = player.GetMatchAssists();
        int saves = player.GetMatchSaves();
        int shots = player.GetMatchShots();
        int demos = player.GetMatchDemolishes();
        int damage = player.GetMatchBreakoutDamage();
        float initialMMR = playerMMR[player.GetUniqueId().ID].initialMMR;
        float newMMR = playerMMR[player.GetUniqueId().ID].newMMR;

        ss << team << "," << name << "," << score << "," << goals << "," << assists << "," << saves << "," << shots << "," << demos << "," << damage
            << "," << initialMMR << "," << newMMR << "\n";
        f << ss.str();
        ss.clear();
    }
    f << "\n";
}

void RLCSVPlugin::getInitialMMR(ArrayWrapper<TeamWrapper> teams, int team) {
    ArrayWrapper<PriWrapper> players = teams.Get(team).GetMembers();

    for (int i = 0; i < players.Count(); i++) {
        SteamID playerID = players.Get(i).GetUniqueId();

        if (playerMMR.find(playerID.ID) == playerMMR.end()) {
            playerMMR[playerID.ID] = PlayerMMR{ getPlayerMMR(players.Get(i)), 0 };
        } else {
            playerMMR[playerID.ID].initialMMR = getPlayerMMR(players.Get(i));
        }
    }
}

void RLCSVPlugin::getNewMMR(ArrayWrapper<TeamWrapper> teams, int team) {
    ArrayWrapper<PriWrapper> players = teams.Get(team).GetMembers();

    for (int i = 0; i < players.Count(); i++) {
        SteamID playerID = players.Get(i).GetUniqueId();

        if (playerMMR.find(playerID.ID) == playerMMR.end()) {
            playerMMR[playerID.ID] = PlayerMMR{ 0, getPlayerMMR(players.Get(i)) };
        } else {
            playerMMR[playerID.ID].newMMR = getPlayerMMR(players.Get(i));
        }
    }

}

float RLCSVPlugin::getPlayerMMR(PriWrapper player) {
    MMRWrapper mw = gameWrapper->GetMMRWrapper();
    int playlist = mw.GetCurrentPlaylist();
    SteamID playerID = player.GetUniqueId();
    return mw.GetPlayerMMR(playerID, playlist);
}

void RLCSVPlugin::logCVarChange(std::string oldValue, CVarWrapper cvar) {
    std::stringstream ss;
    ss << "cl_rlcsv_csv_directory: '" << cvarManager->getCvar("cl_rlcsv_csv_directory").getStringValue();
    cvarManager->log(ss.str());
}
#include "Team.h"
#include <string>

using namespace std;

Team::Team(const TeamNumber::Enum& thisTeamNumber) {
    m_TeamNumber = thisTeamNumber;
}
Team::Team(const TeamNumber::Enum& thisTeamNumber, vector<TeamNumber::Enum>& allyTeams, vector<TeamNumber::Enum>& enemyTeams) {
    m_TeamNumber = thisTeamNumber;

    for (auto& allyTeam : allyTeams)
        addAllyTeam(allyTeam);
    for (auto& enemyTeam : enemyTeams)
        addEnemyTeam(enemyTeam);
}
Team::~Team() {

}

const TeamNumber::Enum& Team::getTeamNumber() const {
    return m_TeamNumber;
}
const unsigned int Team::getNumberOfPlayersOnTeam() const {
    return m_TeamPlayers.size();
}
string Team::getTeamNumberAsString() const {
    return to_string(static_cast<unsigned int>(m_TeamNumber));
}
unordered_set<string>& Team::getPlayers() {
    return m_TeamPlayers;
}
unordered_set<TeamNumber::Enum>& Team::getAllyTeams() {
    return m_AllyTeams;
}
unordered_set<TeamNumber::Enum>& Team::getEnemyTeams() {
    return m_EnemyTeams;
}

const bool Team::isPlayerOnTeam(const string& playerName) const {
    if (m_TeamPlayers.count(playerName))
        return true;
    return false;
}
const bool Team::addPlayerToTeam(const string& playerName) {
    if (m_TeamPlayers.count(playerName))
        return false;
    m_TeamPlayers.emplace(playerName);
    return true;
}

const bool Team::addEnemyTeam(const TeamNumber::Enum& teamNumber) {
    if (teamNumber == m_TeamNumber || m_EnemyTeams.count(teamNumber)) {
        return false;
    }
    m_EnemyTeams.emplace(teamNumber);
    return true;
}
const bool Team::addAllyTeam(const TeamNumber::Enum& teamNumber) {
    if (teamNumber == m_TeamNumber || m_AllyTeams.count(teamNumber)) {
        return false;
    }
    m_AllyTeams.emplace(teamNumber);
    return true;
}

const bool Team::isEnemyTeam(const TeamNumber::Enum& otherTeamNumber) const {
    if (m_TeamNumber == otherTeamNumber)
        return false;
    return (m_EnemyTeams.count(otherTeamNumber)) ? true : false;
}
const bool Team::isAllyTeam(const TeamNumber::Enum& otherTeamNumber) const {
    if (m_TeamNumber == otherTeamNumber)
        return true;
    return (m_AllyTeams.count(otherTeamNumber)) ? true : false;
}
const bool Team::isNeutralTeam(const TeamNumber::Enum& otherTeamNumber) const {
    return (!isEnemyTeam(otherTeamNumber) && !isAllyTeam(otherTeamNumber)) ? true : false;
}
const bool Team::isEnemyTeam(const Team& otherTeam) const {
    return isEnemyTeam(otherTeam.getTeamNumber());
}
const bool Team::isAllyTeam(const Team& otherTeam) const {
    return isAllyTeam(otherTeam.getTeamNumber());
}
const bool Team::isNeutralTeam(const Team& otherTeam) const {
    return isNeutralTeam(otherTeam.getTeamNumber());
}
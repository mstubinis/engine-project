#pragma once
#ifndef GAME_STATE_H
#define GAME_STATE_H

struct GameState {enum State {
    Main_Menu,
    Host_Server_Port_And_Name,
    Host_Server_Map_And_Ship,
    Join_Server_Port_And_Name_And_IP,
    Join_Server_Server_Info,
    Game,
};};

#endif
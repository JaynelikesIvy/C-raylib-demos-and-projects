#ifndef _MP_CLIENT_H_
#define _MP_CLIENT_H_
#include <enet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mp_networking.h"
#include "Third-Party/raymath.h"

#define HOST "127.0.0.1"
#define PORT 7000
  
typedef enum{
  Connecting,
  Playing,
  Disconnecting, 
  Disconnected
}GameConnectionState;

void InitEnetGameClient(const char* server_address); //Start Client for player to communicate with server
void EnetHandler(double now, bool shot, int shotDamage, int targetID); //Handle enet client, should be processed during main raylib window loop
int StopEnet(); //Kills enet user
int EnetSendKillPacketToServer(); //Kill the server
bool Connected();
int GetLocalPlayerID();
void Disconnect();
bool GetPlayerPos(int id, Vector3* pos);
void UpdateLocalplayer(Vector3 playerPosition, Vector3 playerDirection);
void InitEnetGameClient(const char* server_address);
bool GetPlayerHP(int id, int* health);
bool IsPlayerActive(int id);
bool GetPlayerKills(int id, int* killcount);
bool GetPlayerDamage(int id, int* damagecount);
#endif

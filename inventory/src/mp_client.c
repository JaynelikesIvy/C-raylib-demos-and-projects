#include "mp_client.h"

int local_playerID = -1;

bool canShootWhenReady = false;
int targetIDWhenReady = -1;

ENetHost *client = {0};
ENetAddress address = {0};
ENetEvent event = {0};
ENetPeer *server = {0};
ENetPacket *packet = {0};
int connected = 0;

double last_input_sent = -100;

double input_update_interval = 1.0f/20.0f;

double last_now = 0;

bool want_disconnect = false;

typedef struct RemotePlayer{
  bool active;
  Vector3 position;
  Vector3 direction;
  double update_time;
  Vector3 extrapolated_position;
  int damageDealt;
  int points;
  int health;
}RemotePlayer;

RemotePlayer players[MAX_PLAYER_COUNT] = {0};

void InitEnetGameClient(const char* server_address)
{
  if(want_disconnect) return;

  enet_initialize(); 

  client = enet_host_create(NULL, 1, 1, 0, 0);
  
  enet_address_set_host(&address, server_address);
  address.port = PORT;
  server = enet_host_connect(client, &address, 1, 0);
  if(server == NULL) printf("Failed to connect to server!\n");
  else printf("success\n");
}

Vector3 ReadPosition(ENetPacket* packet, size_t* offset)
{
  Vector3 pos = {0};
  pos.x = ReadFloat(packet, offset);
  pos.y = ReadFloat(packet, offset);
  pos.z = ReadFloat(packet, offset);

  return pos;
}


void HandleAddPlayer(ENetPacket* packet, size_t* offset)
{
  int remote_player = ReadByte(packet, offset);
  if(remote_player >= MAX_PLAYER_COUNT || remote_player == local_playerID) return;

  players[remote_player].active = true;
  players[remote_player].position = ReadPosition(packet, offset);
  players[remote_player].direction = ReadPosition(packet, offset);
  players[remote_player].health = 100;
  players[remote_player].update_time = last_now;
  players[remote_player].damageDealt = 0;
  players[remote_player].points = 0;
}

void HandleRemovePlayer(ENetPacket* packet, size_t* offset)
{
  int remote_player = ReadByte(packet, offset);
  if(remote_player >= MAX_PLAYER_COUNT || remote_player == local_playerID) return;

  players[remote_player].active = false;
}

void HandleMultiplayerUpdate(ENetPacket* packet, size_t* offset)
{
  int remote_player = ReadByte(packet, offset);
  if(remote_player >= MAX_PLAYER_COUNT || remote_player == local_playerID) return;

  players[remote_player].position = ReadPosition(packet, offset);
  players[remote_player].direction = ReadPosition(packet, offset);
  int targetID = ReadByte(packet, offset);
  if(targetID != 255)
  { 
    players[targetID].health = ReadByte(packet, offset);
    players[remote_player].damageDealt += ReadByte(packet, offset);
    players[remote_player].points += ReadByte(packet, offset);
  }

  if(players[local_playerID].health <= 0) players[targetID].health = 100;
  if(players[targetID].health <= 0) players[targetID].health = 100;

  players[remote_player].update_time = last_now;

}


void EnetHandler(double now, bool shot, int shotDamage, int targetID)
{
  last_now = now;
  if(shot && targetID != -1) canShootWhenReady = true;
  if(targetID != -1) targetIDWhenReady = targetID;

  if(server == NULL) return;

  if(!want_disconnect && local_playerID >= 0 && now - last_input_sent> input_update_interval)
  {
    uint8_t buffer[28] = {0};
    buffer[0] = (uint8_t)update_input;
		*(float*)(buffer+1) = players[local_playerID].position.x;
		*(float*)(buffer+5) = players[local_playerID].position.y;
		*(float*)(buffer+9) = players[local_playerID].position.z;
		*(float*)(buffer+13) = players[local_playerID].direction.x;
		*(float*)(buffer+17) = players[local_playerID].direction.y;
		*(float*)(buffer+21) = players[local_playerID].direction.z;
    buffer[25] = (uint8_t)canShootWhenReady;
    buffer[26] = (uint8_t)targetIDWhenReady;
    buffer[27] = (uint8_t)shotDamage;

    if(canShootWhenReady)
    { 
      players[targetIDWhenReady].health -= shotDamage; 
      players[local_playerID].damageDealt += shotDamage; 
      if(players[targetIDWhenReady].health <= 0){ players[local_playerID].points++; players[targetIDWhenReady].health = 100;}
    }


    packet = enet_packet_create(buffer, 28, ENET_PACKET_FLAG_RELIABLE);

    enet_peer_send(server, 0, packet);

    last_input_sent = now;
    if(canShootWhenReady) targetIDWhenReady = -1;
    canShootWhenReady = false;

  }

  while(enet_host_service(client, &event, 0) > 0)
  {
    switch(event.type)
    {
      case ENET_EVENT_TYPE_RECEIVE:
        if(event.packet->dataLength < 1 || want_disconnect)
        {
          enet_packet_destroy(event.packet);
          break;
        }

        size_t offset = 0;
        NetworkCommands command = (NetworkCommands)ReadByte(event.packet, &offset);

        if(local_playerID == -1)
        {
          if(command == accept_player)
          {
            local_playerID = ReadByte(event.packet, &offset);
            if(local_playerID < 0 || local_playerID > MAX_PLAYER_COUNT)
            {
              local_playerID = -1;
              break;
            }

            last_input_sent = -input_update_interval;

            players[local_playerID].active = true;

            players[local_playerID].health = 100;

            players[local_playerID].position = (Vector3){0, 0, 0};
          }
        }
        else
        {
          switch(command)
          {
            case add_player:
              HandleAddPlayer(event.packet, &offset);
              break;

            case remove_player:
              HandleRemovePlayer(event.packet, &offset);
              break;

            case update_player:
              HandleMultiplayerUpdate(event.packet, &offset);
              break;

            default:
              break;
          }
        }
        enet_packet_destroy(event.packet);
        break;

      case ENET_EVENT_TYPE_DISCONNECT:
        if(client != NULL) enet_host_destroy(client);

        server = NULL;
        enet_deinitialize();

        server = NULL;
        local_playerID = -1;

        want_disconnect = false;

        break;

      default:
        break;
    }
  }
  if(server == NULL) client = NULL; // remove client pointer after while loop because it crashes otherwise
  for(int i = 0; i < MAX_PLAYER_COUNT; i++)
  {
    if(i == local_playerID || !players[i].active) continue;

    double delta = last_now - players[i].update_time;
    players[i].extrapolated_position = Vector3Add(players[i].position, Vector3Scale(players[i].direction, (float)delta));
  }
}

void Disconnect()
{
  if(server != NULL)
  {
    want_disconnect = true;
    enet_peer_disconnect(server, 0);
  }
}

bool Connected()
{
  if(server == NULL)return false;
  else return true;
}

int GetLocalPlayerID()
{
  return local_playerID;
}

void UpdateLocalplayer(Vector3 playerPosition, Vector3 playerDirection)
{
  if(local_playerID < 0) return;

  players[local_playerID].position= playerPosition;

  players[local_playerID].direction = playerDirection;

}

bool GetPlayerPos(int id, Vector3* pos)
{
  if(id < 0 || id >= MAX_PLAYER_COUNT || !players[id].active)return false;

  if(id == local_playerID) *pos = players[id].position;
  else *pos = players[id].extrapolated_position;

  return true;
}

bool GetPlayerHP(int id, int* health)
{
  if(id < 0 || id >= MAX_PLAYER_COUNT || !players[id].active)return false;

  *health = players[id].health; 

  return true;
}

bool IsPlayerActive(int id)
{
  if(id < 0 || id >= MAX_PLAYER_COUNT || !players[id].active) return false;
  else return true;
}

bool GetPlayerDamage(int id, int* damagecount)
{
  if(id < 0 || id >= MAX_PLAYER_COUNT || !players[id].active)return false;

  *damagecount = players[id].damageDealt; 

  return true;
}


bool GetPlayerKills(int id, int* killcount)
{
  if(id < 0 || id >= MAX_PLAYER_COUNT || !players[id].active)return false;

  *killcount = players[id].points; 

  return true;
}

int StopEnet()
{
  enet_host_destroy(client);
  enet_deinitialize();
  return 0;
}
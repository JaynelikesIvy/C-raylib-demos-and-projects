#define ENET_IMPLEMENTATION
#include <enet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/mp_networking.h"

#define HOST "127.0.0.1"
#define PORT 7000

ENetAddress address = {0};
ENetHost *server = {0};
ENetEvent event = {0};
ENetPacket *packet = {0};

bool online = true;

int current_player_count = 0;

typedef struct PlayerData{
  bool active;
  bool validPosition;
  ENetPeer *peer;

  float X;
  float Y;
  float Z;

  float DX;
  float DY;
  float DZ;

  int health;
}PlayerData;

PlayerData players[MAX_PLAYER_COUNT] = {0};

int GetPlayerId(ENetPeer* peer)
{
	for (int i = 0; i < MAX_PLAYER_COUNT; i++)
	{
		if (players[i].active && players[i].peer == peer)
			return i;
	}
	return -1;
}

void SendToAllBut(ENetPacket* packet, int exceptPlayerId)
{
	for (int i = 0; i < MAX_PLAYER_COUNT; i++)
	{
		if (!players[i].active || i == exceptPlayerId)
			continue;

		enet_peer_send(players[i].peer, 0, packet);
	}
}


int main()
{
  if(enet_initialize() != 0)
  {
    printf("Could not initalize enet\n");
    return 1;
  }

  address.host = ENET_HOST_ANY;
  address.port = PORT;

  server = enet_host_create(&address, 10, 2, 0, 0);

  if(server == NULL)
  {
    printf("could not start server\n");
    return 1;
  }

  printf("Server Online...\n");

  while(online)
  {
    while(enet_host_service(server, &event, 1000) > 0)
    {
      switch(event.type)
      {
        case ENET_EVENT_TYPE_CONNECT:
        {
          current_player_count++;
          printf("%d\n", current_player_count);
          int playerID = 0;

          for(; playerID < MAX_PLAYER_COUNT; playerID++)
          {
            if(!players[playerID].active)
            {
              break;
            }
          }

          if(playerID == MAX_PLAYER_COUNT)
          {
            printf("Too many players, %d\n", playerID);
            enet_peer_disconnect(event.peer, 0);
            break;
          }
          
          players[playerID].active = true;
          players[playerID].validPosition = false;
          players[playerID].peer = event.peer;
          players[playerID].health = 100;

          uint8_t buffer[2] = {0};

          buffer[0] = (uint8_t)accept_player;
          buffer[1] = (uint8_t)playerID;

          ENetPacket *packet = enet_packet_create(buffer, 2, ENET_PACKET_FLAG_RELIABLE);
          enet_peer_send(event.peer, 0, packet);

          for(int i = 0; i < MAX_PLAYER_COUNT; i++)
          {
            if(i == playerID || !players[i].validPosition)continue;
            uint8_t addBuffer[27] = {0};
            addBuffer[0] = (uint8_t)add_player;
            addBuffer[1] = (uint8_t)i;
            *(float*)(addBuffer+2) = (float)players[i].X;
            *(float*)(addBuffer+6) = (float)players[i].Y;
            *(float*)(addBuffer+10) = (float)players[i].Z;
            *(float*)(addBuffer+14) = (float)players[i].DX;
            *(float*)(addBuffer+18) = (float)players[i].DY;
            *(float*)(addBuffer+22) = (float)players[i].DZ;
            addBuffer[26] = players[i].health;

            packet = enet_packet_create(addBuffer, 27, ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(event.peer, 0, packet);
          }
        }
          break;
        
        case ENET_EVENT_TYPE_RECEIVE:
        {
          int playerID = GetPlayerId(event.peer);
          int targetID = -1;
          if(playerID == -1)
          {
            printf("PlayerID is -1\n");
            enet_peer_disconnect(event.peer, 0);
            break;
          }
          size_t offset = 0;

          int damage = 0;
          
          NetworkCommands command = ReadByte(event.packet, &offset);

          if(command == update_input)
          {
            players[playerID].X = ReadFloat(event.packet, &offset);
            players[playerID].Y = ReadFloat(event.packet, &offset);
            players[playerID].Z = ReadFloat(event.packet, &offset);
            players[playerID].DX = ReadFloat(event.packet, &offset);
            players[playerID].DY = ReadFloat(event.packet, &offset);
            players[playerID].DZ = ReadFloat(event.packet, &offset);
            if(ReadByte(event.packet, &offset))
            {
              targetID = ReadByte(event.packet, &offset);
              damage = ReadByte(event.packet, &offset);
              if(players[targetID].health < damage) players[targetID].health = 0;
              else players[targetID].health -= damage;
            }

            NetworkCommands outbound_command = update_player;

            if(!players[playerID].validPosition) outbound_command = add_player;

            players[playerID].validPosition = true;

						uint8_t buffer[30] = { 0 };
						buffer[0] = (uint8_t)outbound_command;
						buffer[1] = (uint8_t)playerID;
						*(float*)(buffer+2) = players[playerID].X;
						*(float*)(buffer+6) = players[playerID].Y;
						*(float*)(buffer+10) = players[playerID].Z;
						*(float*)(buffer+14) = players[playerID].DX;
						*(float*)(buffer+18) = players[playerID].DY;
						*(float*)(buffer+22) = players[playerID].DZ;
            buffer[26] = (uint8_t)targetID;
            if(targetID != 255) 
            {
              buffer[27] = (uint8_t)players[targetID].health;
              buffer[28] = (uint8_t)damage;
              buffer[29] = (uint8_t)((players[targetID].health <= 0) ? 1:0);
            }
            else
            {
              buffer[27] = 0;
              buffer[28] = 0;
              buffer[29] = 0;
            }

						ENetPacket* packet = enet_packet_create(buffer, 30, ENET_PACKET_FLAG_RELIABLE);
						SendToAllBut(packet, playerID);
          }

          if(players[targetID].health <= 0) players[targetID].health = 100;
        
          enet_packet_destroy(event.packet);
          break;
        }

        case ENET_EVENT_TYPE_DISCONNECT:
        {
          current_player_count--;
          if(current_player_count <= 0)
          {
            online = false;
          }

          int playerID = GetPlayerId(event.peer);
          if(playerID == -1) break;

          players[playerID].active = false;
          players[playerID].peer = NULL;

          uint8_t buffer[2] = {0};
          buffer[0] = (uint8_t)remove_player;
          buffer[1] = (uint8_t)playerID;

          ENetPacket *packet = enet_packet_create(buffer, 2, ENET_PACKET_FLAG_RELIABLE);
          SendToAllBut(packet, -1);
          break;
        }

        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
        {
          current_player_count--;
          if(current_player_count == 0)
          {
            online = false;
          }

          int playerID = GetPlayerId(event.peer);
          if(playerID == -1) break;

          players[playerID].active = false;
          players[playerID].peer = NULL;

          uint8_t buffer[2] = {0};
          buffer[0] = (uint8_t)remove_player;
          buffer[1] = (uint8_t)playerID;

          ENetPacket *packet = enet_packet_create(buffer, 2, ENET_PACKET_FLAG_RELIABLE);
          SendToAllBut(packet, -1);
          break;
        }
          case ENET_EVENT_TYPE_NONE:
            break;
      }
    }
  }

  enet_host_destroy(server);
  enet_deinitialize();
  return 0;
}
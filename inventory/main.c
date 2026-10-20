#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RAYGUI_IMPLEMENTATION
#include "src/Third-Party/raygui.h"

#include <unistd.h>
#include <pthread.h>

#include "src/f_parser.h"
#include "src/p_player.h"
#include "terminal.h"

#define ENET_IMPLEMENTATION
#define RAYGUI_FONT_ICONS_BAKING
#include "src/mp_client.h"

#include "src/w_guns.h"

#define GAME_ITEM ".gitm"

#define TARGET_FPS 60

Vector2 lookRotation = { 0 };
float headTimer = 0.0f;
float walkLerp = 0.0f;
float headLerp = STAND_HEIGHT;
Vector2 lean = { 0 };

typedef struct ItemRegistry{
  ItemCollection* items;
  int count;
}ItemRegistry;

typedef struct InventoryItem{
  ItemCollection item;
  int amount;
  int max_amount;
}InventoryItem;

typedef struct Inventory{
  InventoryItem *items;
  int inventory_slots;
}Inventory;

FilePathList path_list = {0};

ItemRegistry item_registry = {0};

static Camera camera = { 0 };

static Body player = {0};
static Vector2 sensitivity = { 0.01f, 0.01f };

static GameConnectionState game_connection_state = Connecting;

pthread_t thread;

static bool run_game = true;
static bool started_server = false;

typedef enum{
  MAIN_MENU,
  SERVER_HOST,
  SERVER_SELECT,
  SETTINGS,
  GAME_MODE_SELECT,
  PLAYING_HOST,
  PLAYING
}GameState;

typedef enum{
  DEATHMATCH
}GameMode;

static GameState game_state = MAIN_MENU;

static GameMode game_mode = DEATHMATCH;

GameGunCollection game_guns = {0};

static clock_t waitTime = 0;

Model levelModel = {0};

void HandleLocalPlayerMovement()
{
  Vector2 mouseDelta = GetMouseDelta();
  lookRotation.x -= mouseDelta.x*sensitivity.x;
  lookRotation.y += mouseDelta.y*sensitivity.y;

  char sideway = (IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
  char forward = (IsKeyDown(KEY_W) - IsKeyDown(KEY_S));
  bool crouching = IsKeyDown(KEY_LEFT_CONTROL);
  UpdateBody(&player, lookRotation.x, sideway, forward, IsKeyPressed(KEY_SPACE), crouching, levelModel);

  float delta = GetFrameTime();
  headLerp = Lerp(headLerp, (crouching ? CROUCH_HEIGHT : STAND_HEIGHT), 20.0f*delta);
  camera.position = (Vector3){
      player.position.x,
      player.position.y + (BOTTOM_HEIGHT + headLerp),
      player.position.z,
  };

  if (player.is_on_floor && ((forward != 0) || (sideway != 0)))
  {
      headTimer += delta*3.0f;
      walkLerp = Lerp(walkLerp, 1.0f, 10.0f*delta);
      camera.fovy = Lerp(camera.fovy, 55.0f, 5.0f*delta);
  }
  else
  {
      walkLerp = Lerp(walkLerp, 0.0f, 10.0f*delta);
      camera.fovy = Lerp(camera.fovy, 60.0f, 5.0f*delta);
  }

  lean.x = Lerp(lean.x, sideway*0.02f, 10.0f*delta);
  lean.y = Lerp(lean.y, forward*0.015f, 10.0f*delta);

  UpdateCameraFPS(&camera, &lookRotation, &lean, headTimer, walkLerp);
}


void UpdateGame()
{
  bool shot = false;
  int targetID = -1;
  Vector3 pos;
  switch(game_guns.gameGuns[game_guns.current_gun].gunState)
  {
    case GUN_STATE_EQUIP:
      break; //TODO: animations and model bs

    case GUN_STATE_IDLE:
      if(IsKeyDown(KEY_TWO) && game_guns.current_gun != 1) game_guns.current_gun = 1;
      if(IsKeyDown(KEY_ONE) && game_guns.current_gun != 0) game_guns.current_gun = 0;
      if(IsKeyDown(KEY_R) && game_guns.gameGuns[game_guns.current_gun].currentAmmo < game_guns.gameGuns[game_guns.current_gun].maxAmmo && game_guns.gameGuns[game_guns.current_gun].totalAmmo > 0){ game_guns.gameGuns[game_guns.current_gun].gunState = GUN_STATE_RELOAD; waitTime = clock();}
      if(IsKeyDown(KEY_P) && CanShoot(game_guns.gameGuns[game_guns.current_gun]) && game_guns.gameGuns[game_guns.current_gun].currentAmmo > 0) game_guns.gameGuns[game_guns.current_gun].gunState = GUN_STATE_SHOOT;
      break;

    case GUN_STATE_SHOOT:
      for(int i = 0; i < MAX_PLAYER_COUNT; i++)
      {
        if(GetPlayerPos(i, &pos) && i != GetLocalPlayerID())
        {
          shot = ShootGun(camera, pos, levelModel);
          if(shot) targetID = i;
        }
      }
      game_guns.gameGuns[game_guns.current_gun].currentAmmo--;
      game_guns.gameGuns[game_guns.current_gun].gunState = GUN_STATE_IDLE;
      break;

    case GUN_STATE_RELOAD:
      int ammoDiff;
      if(waitTime + game_guns.gameGuns[game_guns.current_gun].reloadSpeed <= clock())
      {
        ammoDiff = game_guns.gameGuns[game_guns.current_gun].maxAmmo - game_guns.gameGuns[game_guns.current_gun].currentAmmo;
        for(int i = 0; i < ammoDiff; i++)
        {
          game_guns.gameGuns[game_guns.current_gun].currentAmmo++;
          game_guns.gameGuns[game_guns.current_gun].totalAmmo--;
          if(game_guns.gameGuns[game_guns.current_gun].totalAmmo <= 0) break;
        }
        game_guns.gameGuns[game_guns.current_gun].gunState = GUN_STATE_IDLE;
      }
      break;
  }


  EnetHandler(GetTime(), shot, game_guns.gameGuns[game_guns.current_gun].damage, targetID);
  switch(game_connection_state)
  {
    case Disconnected:
      run_game = false;

    default: 
      break;
    
    case Connecting:
      if(Connected() == true && GetLocalPlayerID() >= 0){ game_connection_state = Playing;}
      break;

    case Disconnecting:
      GuiTextBox((Rectangle){200, 200, 100, 100}, "Disconnecting...", 24, false);
      if(Connected() == false)
      {
        game_connection_state = Disconnected;
      }
      break;

    case Playing:
      if(WindowShouldClose())
      {
        Disconnect();
        game_connection_state = Disconnecting;
        break;
      }
      else if (Connected() == false)
      {
        InitEnetGameClient(HOST);
        game_connection_state = Connecting;
        break;
      }
      else
      {
        HandleLocalPlayerMovement();

        UpdateLocalplayer(player.position, player.dir);
        break;
      }
  }
}

void* server_thread(void* arg) // IGNORE THE COMPILE WARN, PTHREAD NEEDS FUNCTIONS TO HAVE A VOID ARG TO WORK, OR ELSE IT CRASHES
{
  started_server = true;
  #ifdef _WIN32
  char server_run_command[256] = "cd server && server.exe";
  #endif
  #ifdef __linux__
  char server_run_command[256] = "./server/server";
  #endif
  system(server_run_command);
  return NULL;
}


void GameUI()
{
  for(int i = 0; i < MAX_PLAYER_COUNT; i++)
  {
    if(IsPlayerActive(i))
    {
      int kills = 0;
      int damage = 0;
      GetPlayerKills(i, &kills);
      GetPlayerDamage(i, &damage);
      GuiDrawText(TextFormat("Player %d has %d kills and %d damage", i, kills, damage), (Rectangle){0, 50+i*10, 250, 100}, 0, WHITE);
    }
  }
}

int main(void)
{
  if(!DirectoryExists("items"))
  {
    printf("ERROR: No Item Path Exists, Adding Blank Item Path\n");
    MakeDirectory("items");
  }
  path_list = LoadDirectoryFiles("items");

  item_registry.items = malloc(sizeof(ItemCollection)*path_list.count);
  item_registry.count = path_list.count;

  game_guns.count = 0;

  for(unsigned int i = 0; i < path_list.count; i++)
  {
    if(strcmp(GetFileExtension(path_list.paths[i]), GAME_ITEM) == 0)
    {
      printf("Loading %s\n", path_list.paths[i]);
      item_registry.items[i] = GetFileData(path_list.paths[i]);
      item_registry.items[i].ID = i;

      for(int j = 0; j < item_registry.items[i].count; j++) 
      {
        printf("%s, %s\n", item_registry.items[i].item_data[j].item_name, item_registry.items[i].item_data[j].item_value);
        if(strcmp(item_registry.items[i].item_data[j].item_value, PARSE_ITEM_GUN_TYPE) == 0) game_guns.count++;
      }
    }
  }
  printf("Parsed %d Items with %d guns\n", path_list.count, game_guns.count);

  game_guns.gameGuns = malloc(sizeof(GameGun)*game_guns.count);

  for(int i = 0; i < game_guns.count; i++)
  {
    game_guns.gameGuns[i] = LoadGameGun(item_registry.items[i]);
  }

  game_guns.current_gun = 0;

  game_guns.gameGuns[game_guns.current_gun].gunState = GUN_STATE_IDLE;

  player.size = (Vector3){1, 2, 1};

  if(!DirectoryExists("server"))
  {
    printf("ERROR: No Server Path Found, Please Recompile Game To Fix This\n");
    return 1;
  }

  InitWindow(500, 500, "Game");

  levelModel = LoadModel("models/turret.obj");
  Texture2D texture = LoadTexture("models/turret_diffuse.png");
  levelModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

  Material mat = LoadMaterialDefault();
  Matrix matr = {0};
  SetTargetFPS(TARGET_FPS);

  DisableCursor();

  printf("%lld, %lld, %d\n", sizeof(*levelModel.meshes), sizeof(Mesh), levelModel.meshCount);
  printf("%d\n", levelModel.meshes[0].vertexCount);
  



  camera.fovy = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;
  camera.position = (Vector3){player.position.x, player.position.y + (BOTTOM_HEIGHT + headLerp),player.position.z};

  UpdateCameraFPS(&camera, &lookRotation, &lean, headTimer, walkLerp);

  player.health = 100;

  while(run_game)
  {
    BeginDrawing();
    switch(game_state)
    {
      case MAIN_MENU:
      
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        if(GuiButton((Rectangle){50, 100, 100, 50}, "Host Game"))game_state = SERVER_HOST; 
        if(GuiButton((Rectangle){50, 150, 100, 50}, "Join Game"))game_state = SERVER_SELECT; 
        if(GuiButton((Rectangle){50, 200, 100, 50}, "Quit Game")){run_game = false; Disconnect();}
        if(WindowShouldClose()) {run_game = false; Disconnect();}
        break;

      case SERVER_HOST:
        InitEnetGameClient(HOST);
        game_state = PLAYING_HOST;
        DisableCursor();
        break;



      case SERVER_SELECT:
          ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
          char textbuffer[256] = "127.0.0.1";
          int join_btn = -1;
          GuiTextInputBox((Rectangle){100, 100, 200, 200}, "Server Select.", "Please Input A Valid IP Address", textbuffer, 12, "Join Server", &join_btn, false);
          if(join_btn == 0) game_state = MAIN_MENU;
          if(join_btn == 1){ InitEnetGameClient(textbuffer); game_state = PLAYING; DisableCursor();}
          break;

      
      case PLAYING_HOST:
        if(!started_server) pthread_create(&thread, NULL, server_thread, NULL);
        game_state = PLAYING;
        break;
      
      case PLAYING:
        BeginMode3D(camera);
        UpdateGame();
        SetMousePosition(250, 250);
        ClearBackground(BLUE);
        DrawCubeV((Vector3){0, 0, 0}, (Vector3){10, 10, 10}, RED);
        DrawModel(levelModel, (Vector3){10, 0, 10}, 1.0, WHITE);
        DrawGrid(10, 1);
        for(int i = 0; i < MAX_PLAYER_COUNT; i++)
        {
          Vector3 pos = {0};
          if(GetPlayerPos(i, &pos))
          {
            DrawBoundingBox((BoundingBox){(Vector3){pos.x - player.size.x/2, pos.y - player.size.y/2, pos.z - player.size.z/2}, (Vector3){pos.x + player.size.x/2, pos.y + player.size.y, pos.z + player.size.z/2}}, BLACK);
            
          }
        }
        EndMode3D();



        if(game_connection_state == Connecting){ ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR))); GuiLabel((Rectangle){200, 200, 100, 100}, "Connecting...");}
        int hp = 0;
        if(GetPlayerHP(GetLocalPlayerID(), &hp))
        {
          DrawText(TextFormat("Player:%d, HP:%d", GetLocalPlayerID(), hp), 10, 10, 24, RED); 
          DrawText(TextFormat("Mag:%d, Total:%d", game_guns.gameGuns[game_guns.current_gun].currentAmmo, game_guns.gameGuns[game_guns.current_gun].totalAmmo), 10, 30, 24, RED);
          DrawText(TextFormat("%s", game_guns.gameGuns[game_guns.current_gun].name), 10, 50, 24, RED);
        }   
        GameUI();     
        DrawRectangle(248, 248, 4, 4, BLACK);
        //HandleTerminal();
        break;

      default:
        break;

      
    }
    EndDrawing();
  }
  UnloadModel(levelModel);
  StopEnet();
  CloseWindow();
  return 0;
}

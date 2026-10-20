#ifndef _GAME_GUNS_H_
#define _GAME_GUNS_H_
#include "raylib.h"
#include <stdio.h>
#include "Third-Party/raymath.h"
#include <time.h>
#include "f_parser.h"

#define PARSE_DAMAGE "Damage"
#define PARSE_FIRERATE "Firerate"
#define PARSE_RELOADSPEED "ReloadSpeed"
#define PARSE_MAX_AMMO "MaxAmmo"
#define PARSE_TOTAL_AMMO "TotalAmmo"
#define PARSE_NAME "Name"
#define PARSE_ITEM_GUN_TYPE "Gun"
#define PARSE_IS_PRIMARY "IsPrimary" //1 or 0

typedef enum{
  GUN_STATE_IDLE,
  GUN_STATE_SHOOT,
  GUN_STATE_RELOAD,
  GUN_STATE_EQUIP
}GunState;

typedef struct GameGun{
  Model model; //Gun Model
  ModelAnimation* modelAnimations; //Animations for the model
  int damage; //Gun damage
  int firerate; //Cooldown between shots (in ms)
  int reloadSpeed;//time to reload (in ms)
  int gunState;//Gun state
  int maxAmmo;//Maximum ammo allowed in mag
  int currentAmmo; //useable ammo
  int totalAmmo; // ammo in supply
  int maxTotalAmmo; //maximum ammo in supply
  char name[256]; //name
  bool isPrimary; //equip with 1 or 2
}GameGun;

typedef struct GameGunCollection{
  int count;
  int current_gun;
  int heldPrimary;
  int heldSecondary;
  GameGun* gameGuns;
}GameGunCollection;

bool ShootGun(Camera camera, Vector3 playerPos, Model world);
GameGun LoadGameGun(ItemCollection itemCollection);
bool CanShoot(GameGun currentGun);
#endif
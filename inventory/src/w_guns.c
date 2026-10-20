#include "w_guns.h"

static clock_t last_shot = 0;

GameGun LoadGameGun(ItemCollection itemCollection)
{
  GameGun temp_gun = {0};
  for(int i = 0; i < itemCollection.count; i++)
  {
    if(strcmp(itemCollection.item_data[i].item_name, PARSE_DAMAGE) == 0) {printf("getting gun damage\n"); temp_gun.damage = atoi(itemCollection.item_data[i].item_value); printf("done\n"); }
    if(strcmp(itemCollection.item_data[i].item_name, PARSE_FIRERATE) == 0) {printf("getting gun firerate\n"); temp_gun.firerate = atoi(itemCollection.item_data[i].item_value);printf("done\n");}
    if(strcmp(itemCollection.item_data[i].item_name, PARSE_RELOADSPEED) == 0) {printf("getting gun reload speed\n");temp_gun.reloadSpeed = atoi(itemCollection.item_data[i].item_value);printf("done\n");}
    if(strcmp(itemCollection.item_data[i].item_name, PARSE_MAX_AMMO) == 0) {printf("getting gun max ammo\n");temp_gun.maxAmmo = atoi(itemCollection.item_data[i].item_value); temp_gun.currentAmmo = temp_gun.maxAmmo; printf("done\n");}
    if(strcmp(itemCollection.item_data[i].item_name, PARSE_TOTAL_AMMO) == 0) {printf("getting gun total ammo\n");temp_gun.maxTotalAmmo = atoi(itemCollection.item_data[i].item_value); temp_gun.totalAmmo = temp_gun.maxTotalAmmo; printf("done\n");}
    if(strcmp(itemCollection.item_data[i].item_name, PARSE_NAME) == 0) {printf("getting gun name\n"); strcat(temp_gun.name, itemCollection.item_data[i].item_value);printf("done\n");}
  }
  return temp_gun;
}

bool ShootGun(Camera camera, Vector3 playerPos, Model world)
{
  Ray worldRay = GetScreenToWorldRay(GetMousePosition(), camera);
  worldRay.direction = Vector3Scale(worldRay.direction, 100);
  last_shot = clock();


  RayCollision playerCollision = GetRayCollisionBox(worldRay, (BoundingBox){(Vector3){playerPos.x - 2.5, playerPos.y - 5, playerPos.z - 2.5}, (Vector3){playerPos.x + 2.5, playerPos.y + 5, playerPos.z + 2.5}});
  for(int i = 0; i < world.meshes[0].triangleCount; i++)
  {
    Vector3 a, b, c;
    Vector3 *vertData = (Vector3*)world.meshes[0].vertices;

    if(world.meshes[0].indices)
    {
      a = vertData[world.meshes[0].indices[i*3]];
      b = vertData[world.meshes[0].indices[i*3+1]];
      c = vertData[world.meshes[0].indices[i*3+2]];
      a = Vector3Add(a, (Vector3){10, 0, 10});
      b = Vector3Add(b, (Vector3){10, 0, 10});
      c = Vector3Add(c, (Vector3){10, 0, 10});
    }
    else
    {
      a = vertData[i*3];
      b = vertData[i*3+1];
      c = vertData[i*3+2];

      a = Vector3Add(a, (Vector3){10, 0, 10});
      b = Vector3Add(b, (Vector3){10, 0, 10});
      c = Vector3Add(c, (Vector3){10, 0, 10});
    }

    RayCollision worldCollision = GetRayCollisionTriangle(worldRay, a, b, c);
    if(playerCollision.hit)
    {
      if(worldCollision.hit)
      {
        if(playerCollision.distance < worldCollision.distance) {return true;}
        else { return false;}
      }
    }
  }
  if(playerCollision.hit) return true;
  else return false;
}

bool CanShoot(GameGun currentGun)
{
  if(currentGun.gunState != GUN_STATE_RELOAD && (clock() - last_shot) >= currentGun.firerate)return true;
  else return false;
}


int GetGunIDByName(char* name, GameGunCollection gunCollection)
{
  for(int i = 0; i < gunCollection.count; i++)
  {
    if(strcmp(gunCollection.gameGuns[i].name, name)) return i;
  }
  fprintf(stderr, "ERROR: Invalid Gun name %s given in function GetGunIDByName", name);
  return -1;
}

char** GetGunNames(GameGunCollection gunCollection)
{
  char** buff = {0};
  buff = buff; // unused warning annoys me a lil but it helps in the long run
  for(int i = 0; i < gunCollection.count; i++)
  {
    strcat(buff[i], gunCollection.gameGuns[i].name);
  }
  return buff;
}
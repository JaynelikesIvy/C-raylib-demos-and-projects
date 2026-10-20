#include "p_player.h"
#include <stdio.h>

BoundingBox* GetLevelModelBoundingBox(Model model)
{
  BoundingBox* boundingBoxes = malloc(sizeof(BoundingBox)*model.meshCount);
  for(int i = 0; i < model.meshCount; i++)
  {
    boundingBoxes[i] = GetMeshBoundingBox(model.meshes[i]);
  } 
  return boundingBoxes;
}

bool CheckPlayerCollisionLevel(BoundingBox* levelBounds, BoundingBox playerBounds)
{
  for(size_t i = 0; i < (sizeof(*levelBounds)/sizeof(BoundingBox)); i++)
  {
      if(CheckCollisionBoxes(levelBounds[i], playerBounds)) return true;
  }
  return false;
}

void UpdateBody(Body *body, float rot, char side, char forward, bool jumpPressed, bool crouchHold, Model level)
{
    Vector2 input = (Vector2){ (float)side, (float)-forward };
    
    BoundingBox* levelBoxes = GetLevelModelBoundingBox(level);
    for(int i = 0; i < level.meshCount; i++)
    {
        levelBoxes[i].min = Vector3Add(levelBoxes[i].min, (Vector3){10, 0, 10});
        levelBoxes[i].max = Vector3Add(levelBoxes[i].max, (Vector3){10, 0, 10});     
    }


    if ((side != 0) && (forward != 0)) input = Vector2Normalize(input);

    float delta = GetFrameTime();

    if (!body->is_on_floor) body->velocity.y -= GRAVITY*delta;

    if (body->is_on_floor && jumpPressed)
    {
        body->velocity.y = JUMP_FORCE;
        body->is_on_floor = false;

    }

    Vector3 front = (Vector3){ sinf(rot), 0.f, cosf(rot) };
    Vector3 right = (Vector3){ cosf(-rot), 0.f, sinf(-rot) };

    Vector3 desiredDir = (Vector3){ input.x*right.x + input.y*front.x, 0.0f, input.x*right.z + input.y*front.z, };
    body->dir = Vector3Lerp(body->dir, desiredDir, CONTROL*delta);

    float decel = (body->is_on_floor ? FRICTION : AIR_DRAG);
    Vector3 hvel = (Vector3){ body->velocity.x*decel, 0.0f, body->velocity.z*decel };

    float hvelLength = Vector3Length(hvel);
    if (hvelLength < (MAX_SPEED*0.01f)) hvel = (Vector3){ 0 };

    float speed = Vector3DotProduct(hvel, body->dir);

    float maxSpeed = (crouchHold? CROUCH_SPEED : MAX_SPEED);
    float accel = Clamp(maxSpeed - speed, 0.f, MAX_ACCEL*delta);
    hvel.x += body->dir.x*accel;
    hvel.z += body->dir.z*accel;

    body->velocity.x = hvel.x;
    body->velocity.z = hvel.z;

    DrawModelWires(level, (Vector3){10, 0, 10}, 1.0f, WHITE);

    body->position.x += body->velocity.x*delta;
    BoundingBox playerBoundingBox = (BoundingBox){
        (Vector3){body->position.x - body->size.x/2, body->position.y - body->size.y/2, body->position.z - body->size.z/2}, 
        (Vector3){body->position.x + body->size.x/2, body->position.y + body->size.y, body->position.z + body->size.z/2}};
    if(CheckPlayerCollisionLevel(levelBoxes, playerBoundingBox)) body->position.x -= body->velocity.x*delta;
    body->position.y += body->velocity.y*delta;
    playerBoundingBox = (BoundingBox){
        (Vector3){body->position.x - body->size.x/2, body->position.y - body->size.y/2, body->position.z - body->size.z/2}, 
        (Vector3){body->position.x + body->size.x/2, body->position.y + body->size.y, body->position.z + body->size.z/2}};
    if(CheckPlayerCollisionLevel(levelBoxes, playerBoundingBox)){ body->position.y -= body->velocity.y*delta; body->is_on_floor = true;}
    body->position.z += body->velocity.z*delta;
    playerBoundingBox = (BoundingBox){
        (Vector3){body->position.x - body->size.x/2, body->position.y - body->size.y/2, body->position.z - body->size.z/2}, 
        (Vector3){body->position.x + body->size.x/2, body->position.y + body->size.y, body->position.z + body->size.z/2}};
    if(CheckPlayerCollisionLevel(levelBoxes, playerBoundingBox)) body->position.z -= body->velocity.z*delta;

    if (body->position.y <= body->size.y)
    {
        body->position.y = body->size.y;
        body->velocity.y = 0.0f;
        body->is_on_floor = true;
    }
}

void UpdateCameraFPS(Camera *camera, Vector2 *lookRotation, Vector2 *lean, float headTimer, float walkLerp)
{
    const Vector3 up = (Vector3){ 0.0f, 1.0f, 0.0f };
    const Vector3 targetOffset = (Vector3){ 0.0f, 0.0f, -1.0f };

    Vector3 yaw = Vector3RotateByAxisAngle(targetOffset, up, lookRotation->x);

    float maxAngleUp = Vector3Angle(up, yaw);
    maxAngleUp -= 0.001f;
    if ( -(lookRotation->y) > maxAngleUp) { lookRotation->y = -maxAngleUp; };

    float maxAngleDown = Vector3Angle(Vector3Negate(up), yaw);
    maxAngleDown *= -1.0f;
    maxAngleDown += 0.001f; 
    if ( -(lookRotation->y) < maxAngleDown) { lookRotation->y = -maxAngleDown; }

    Vector3 right = Vector3Normalize(Vector3CrossProduct(yaw, up));

    float pitchAngle = -lookRotation->y - lean->y;
    pitchAngle = Clamp(pitchAngle, -PI/2 + 0.0001f, PI/2 - 0.0001f);
    Vector3 pitch = Vector3RotateByAxisAngle(yaw, right, pitchAngle);

    float headSin = sinf(headTimer*PI);
    float headCos = cosf(headTimer*PI);
    const float stepRotation = 0.01f;
    camera->up = Vector3RotateByAxisAngle(up, pitch, headSin*stepRotation + lean->x);

    const float bobSide = 0.1f;
    const float bobUp = 0.15f;
    Vector3 bobbing = Vector3Scale(right, headSin*bobSide);
    bobbing.y = fabsf(headCos*bobUp);

    camera->position = Vector3Add(camera->position, Vector3Scale(bobbing, walkLerp));
    camera->target = Vector3Add(camera->position, pitch);
}


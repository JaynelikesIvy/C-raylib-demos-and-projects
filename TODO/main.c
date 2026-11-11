#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "src/perlin.h"

#define WINDOW_W 300
#define WINDOW_H 200

#define PERMUTATIONS 200


int squareSize = 1;

typedef struct Player{
  Camera2D playercam;
  Vector2 position;
}Player;


typedef struct Square{
  Color color;
  Vector2 position;
}Square;

Player player = {0};

Square squares[WINDOW_H*WINDOW_W] = {0};

int main(void)
{
  for(int i = 0; i < PERMUTATIONS; i++)
  {
    for(int j = 0; j < PERMUTATIONS; j++)
    {
      int seed = 2;
      float amplitude = 1.0f;
      float frequency = 0.01f;
      float noise = 0;
      for(int k = 0; k < 4; k++)
      {
        noise += amplitude * PerlinNoise2D(i*frequency, j*frequency, seed);

        amplitude*=0.5f;
        frequency*=2.0f;
        seed++;
      }
      noise+= 1.0f;
      noise/=2.0f;
      noise *= 255.0f;

      squares[j+(i*PERMUTATIONS)].color = (Color){0, noise, fabsf(noise - 255), 255};
      squares[j+(i*PERMUTATIONS)].position.x = j;
      squares[j+(i*PERMUTATIONS)].position.y = i;
    }
  }
  InitWindow(WINDOW_W, WINDOW_H, "Title");
  SetTargetFPS(60);


  player.position = (Vector2){0.0f,0.0f};

  while(!WindowShouldClose())
  {
    BeginDrawing();
    ClearBackground(BLUE);
    for(int y = 0; y < PERMUTATIONS; y++)
    {
      for(int x = 0; x < PERMUTATIONS; x++)
      {

        if(CheckCollisionRecs((Rectangle){player.position.x, player.position.y, WINDOW_W, WINDOW_H}, (Rectangle){squares[x+(y*PERMUTATIONS)].position.x*squareSize, squares[x+(y*PERMUTATIONS)].position.y*squareSize, squareSize, squareSize}))
        {
          if(CheckCollisionRecs((Rectangle){squares[x+(y*PERMUTATIONS)].position.x*squareSize, squares[x+(y*PERMUTATIONS)].position.y*squareSize, squareSize, squareSize}, (Rectangle){GetScreenToWorld2D(GetMousePosition(), player.playercam).x, GetScreenToWorld2D(GetMousePosition(), player.playercam).y, 0.5f, 0.5f})) 
          DrawRectangle(squares[x+(y*PERMUTATIONS)].position.x*squareSize - player.position.x, squares[x+(y*PERMUTATIONS)].position.y*squareSize- player.position.y, squareSize, squareSize, RED);
          else 
          DrawRectangle(squares[x+(y*PERMUTATIONS)].position.x*squareSize - player.position.x, squares[x+(y*PERMUTATIONS)].position.y*squareSize- player.position.y, squareSize, squareSize, squares[x+(y*PERMUTATIONS)].color);
        }
      }
    }

    if(IsKeyDown(KEY_A))
    {
      player.position.x -= (IsKeyDown(KEY_LEFT_SHIFT)? 3 : 1);
    }
  
    if(IsKeyDown(KEY_D))
    {
      player.position.x += (IsKeyDown(KEY_LEFT_SHIFT)? 3 : 1);
    }

    if(IsKeyDown(KEY_W))
    {
      player.position.y -= (IsKeyDown(KEY_LEFT_SHIFT)? 3 : 1);
    }
  
    if(IsKeyDown(KEY_S))
    {
      player.position.y += (IsKeyDown(KEY_LEFT_SHIFT)? 3 : 1);
    }

    if(IsKeyPressed(KEY_EQUAL) && IsKeyDown(KEY_LEFT_SHIFT))
    {
      squareSize++;
      player.position.x*=2;
      player.position.y*=2;
    }
    if(IsKeyPressed(KEY_MINUS) && squareSize > 1)
    {
      squareSize--;
      player.position.x*=0.5;
      player.position.y*=0.5;
    }

  player.playercam = (Camera2D){
    .target = (Vector2){player.position.x, player.position.y },
    .offset = (Vector2){0, 0 },
    .rotation = 0.0,
    .zoom = 1.0
  };
    EndDrawing();
  }
  CloseWindow();
  *squares = (Square){0};
  return 0;
}
#ifndef _MY_PERLIN_GEN_H_
#define _MY_PERLIN_GEN_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef RAYLIB_H
#include "raylib.h"
#endif

#define PERLIN_SIZE 600
#define FREQUENCY 0.01   
#define AMPLITUDE 1
#define SEED 1

Vector2 GetConstantVector(int v); //Get vector for each corner
float MyLerp(float t, float vector1, float vector2); //My version of linear interpolation for perlin noise
float DotProduct(Vector2 vector1, Vector2 vector2); //its the dot prouct :D
float PerlinNoise2D(float x, float y, int seed); //Get perlin noise value at a set coordinate and seed

#endif
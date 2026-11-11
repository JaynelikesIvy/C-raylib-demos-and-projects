#include "perlin.h"

//An implementation typically involves three steps: 

//defining a grid of random gradient vectors, 
//This means we need to determine vectors pointing in a random direction between -1 and 1 on both x and y
//We have to determine the size of a grid of squares and make each corner have one of these random vectors 
//The noise will also wrap every 256 squares in the grid on the x and y axis


//computing the dot product between the gradient vectors and their offsets, 
//Here, we need to find the vector gradients of each corner of a cube in our grid 
//Then, we take a point in the square (called candidate point) 
//we then find the offset between one gradient vector and the point
//Then we do the dot product of the gradient point and the offset point
//repeat 4 times


//and interpolation between these values.
// quite simple, interpolate the values of all 4 dot products to the position of the point
// too interpolate, we interpolate the top two first, call it v1, interpolate the bottom twp, call it v2, then interpolate v1 and v2

//oooh scary change ooh

const int permutation[] = { 151, 160, 137,  91,  90,  15, 131,  13, 201,  95,  96,  53, 194, 233, 7, 225,
                      140,  36, 103,  30,  69, 142,   8,  99,  37, 240,  21,  10,  23, 190,   6, 148,
                      247, 120, 234,  75,   0,  26, 197,  62,  94, 252, 219, 203, 117,  35,  11,  32,
                       57, 177,  33,  88, 237, 149,  56,  87, 174,  20, 125, 136, 171, 168,  68, 175,
                       74, 165,  71, 134, 139,  48,  27, 166,  77, 146, 158, 231,  83, 111, 229, 122,
                       60, 211, 133, 230, 220, 105,  92,  41,  55,  46, 245,  40, 244, 102, 143,  54,
                       65,  25,  63, 161,   1, 216,  80,  73, 209,  76, 132, 187, 208,  89,  18, 169,
                      200, 196, 135, 130, 116, 188, 159,  86, 164, 100, 109, 198, 173, 186,   3,  64,
                       52, 217, 226, 250, 124, 123,   5, 202,  38, 147, 118, 126, 255,  82,  85, 212,
                      207, 206,  59, 227,  47,  16,  58,  17, 182, 189,  28,  42, 223, 183, 170, 213,
                      119, 248, 152,   2,  44, 154, 163,  70, 221, 153, 101, 155, 167,  43, 172,   9,
                      129,  22,  39, 253,  19,  98, 108, 110,  79, 113, 224, 232, 178, 185, 112, 104,
                      218, 246,  97, 228, 251,  34, 242, 193, 238, 210, 144,  12, 191, 179, 162, 241,
                       81,  51, 145, 235, 249,  14, 239, 107,  49, 192, 214,  31, 181, 199, 106, 157,
                      184,  84, 204, 176, 115, 121,  50,  45, 127,   4, 150, 254, 138, 236, 205,  93,
                      222, 114,  67,  29,  24,  72, 243, 141, 128, 195,  78,  66, 215,  61, 156, 180 };

Vector2 GetConstantVector(int v) //Get vector for each corner
{
  int h = v % 3;
  if(h==0) return (Vector2){1.0, 1.0};
  if(h==1) return (Vector2){-1.0, 1.0};
  if(h==2) return (Vector2){-1.0, -1.0};
  else return (Vector2){1.0, -1.0};
}

float MyLerp(float t, float vector1, float vector2) //Linear interpolation (step 3)
{
  return vector1 + t*(vector2-vector1);
}

float MyFade(float t) //Easing curve to make borders smoother (step 3)
{
  return ((6*t - 15)*t + 10)*t*t*t;
}

float DotProduct(Vector2 vector1, Vector2 vector2) //its the dot prouct :D
{
  return vector1.x*vector2.x + vector1.y*vector2.y;
}

float PerlinNoise2D(float x, float y, int seed)
{
  int X = fmod(x, 255.0f); //deterministic value between 0 and 255 for permutations
  int Y = fmod(y, 255.0f);

  float xf = x - (int)x; // difference between x and rounded x, should be between 0 and 1
  float yf = y - (int)y; // Will be used to make random gradient vectors, again, deterministic

  Vector2 topright = {xf-1.0, yf-1.0}; //random gradient vector
  Vector2 topleft = {xf, yf-1.0};
  Vector2 bottomright = {xf-1.0, yf};
  Vector2 bottomleft = {xf, yf};

  int valuetopright = permutation[(permutation[permutation[X+1]+Y+1] + seed) % 255]; //Value for each corner for consistent permutation value
  int valuetopleft = permutation[(permutation[permutation[X]+Y+1] + seed) % 255];
  int valuebottomright = permutation[(permutation[permutation[X+1]+Y] + seed) % 255];
  int valuebottomleft = permutation[(permutation[permutation[X]+Y] + seed) % 255];

  float dottopright = DotProduct(topright, GetConstantVector(valuetopright)); //dot product between random gradient vector and its offset
  float dottopleft = DotProduct(topleft, GetConstantVector(valuetopleft));
  float dotbottomright = DotProduct(bottomright, GetConstantVector(valuebottomright));
  float dotbottomleft = DotProduct(bottomleft, GetConstantVector(valuebottomleft));


  float u = MyFade(xf); //smoothstep function getter
  float v = MyFade(yf);

  float lerp1 = MyLerp(v, dotbottomleft, dottopleft); //step 1 of lerping 
  float lerp2 = MyLerp(v, dotbottomright, dottopright);

  if(MyLerp(u, lerp1, lerp2) > 10 || MyLerp(u, lerp1, lerp2) < -10) printf("%f, %f, %d, %d, %f, %f\n", x, y, X, Y, xf, yf);
  float final = MyLerp(u, lerp1, lerp2);

  return final; //i summon thee, NOISE
}

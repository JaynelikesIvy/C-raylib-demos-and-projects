#include "raylib/raylib.h"

#include <stdio.h>

#define RAYGUI_IMPLEMENTATION
#include "raylib/raygui.h"

#define WINDOWSIZE 600

#define RECTANGLE_POS_X 50
#define RECTANGLE_POS_Y 50
#define BG_OFFSET 5
#define BG_RECTANGLE_POS_X RECTANGLE_POS_X - BG_OFFSET
#define BG_RECTANGLE_POS_Y RECTANGLE_POS_Y - BG_OFFSET
#define OFFSET 50

#define POSITION_ON_WEB

enum{
  MAINMENU,
  ABOUTME,
  LINKSTOMEDIA,
  BLOG,
  READBLOG,
};

typedef unsigned short int ui8;
typedef short int i8;
typedef unsigned int ui16;
typedef int i16;

ui16 currPage = 0;
ui16 currBlog = 0;


char blogBuff[2000] = "";

#if defined(PLATFORM_WEB)

#include <../emscripten.h>

EM_JS(ui16, canvas_get_width, (), {return document.getElementById('canvas').clientWidth;});

EM_JS(ui16, canvas_get_height, (), {return document.getElementById('canvas').clientHeight;});

#endif

typedef struct Gif{
  Texture2D GIFsheet;
  ui16 frames;
  ui16 xLimit;
  Vector2 FrameSize;
  ui16 currentFrame;
  ui16 subframe;
  ui8 framerate;
}Gif;

Gif gifs[5] = {0};
ui8 gifCount = 0;

ui8 websiteState = 0;

typedef struct String{
  char* c;
  i16 length;
  size_t size;
}String;

typedef struct ChatElement{
  String username;
  String message;
}ChatElement;

char chat[30];

FilePathList blogList = {0};

void HandleGifFrames()
{
  for(ui8 i = 0; i < gifCount; i++)
  {
    if(gifs[i].subframe * GetFrameTime() >= (float)(1/(float)(gifs[i].framerate))){gifs[i].currentFrame++; gifs[i].subframe = 0;}
    else gifs[i].subframe++;
    if(gifs[i].currentFrame >= gifs[i].frames-1) { gifs[i].currentFrame = 0;}
  }
}

ui16 GetFrameX(Gif gif)
{
  return ((gif.currentFrame % gif.xLimit) * gif.FrameSize.x);
}

ui16 GetFrameY(Gif gif)
{
  return ((gif.currentFrame / gif.xLimit) * gif.FrameSize.y);
}

void DrawMainMenu(ui16 w, ui16 h)
{
  //Seperated by ifdef because the majority of the text uses a web only variables (h and w) for spacing
  #ifdef PLATFORM_WEB 
  DrawText("Welcome stranger to my site!", w/3.5, 150, 45, LIGHTGRAY);

  DrawText("This is the main menu for this place, feel free to check out the different pages!", w/7.5, 250, 25, LIGHTGRAY);

  if(GuiButton((Rectangle){w/3, 300, 200, 50}, "About Me.")) websiteState = ABOUTME;

  if(GuiButton((Rectangle){w/2, 300, 200, 50}, "Links to media I use.")) websiteState = LINKSTOMEDIA;

/* Might make these workable buttons later, rn i cant be beothered
  if(GuiButton((Rectangle){ w/3, 400, 200, 50}, "Stuff I've made.")) websiteState = PROJECTS;

  if(GuiButton((Rectangle){w/2, 400, 200, 50}, "Minigames!!!")) websiteState = MINIGAMES;
*/
  if(GuiButton((Rectangle){w/2.35, 400, 200, 50}, "Blog")) websiteState = BLOG;


  DrawTexturePro(gifs[0].GIFsheet, (Rectangle){GetFrameX(gifs[0]), GetFrameY(gifs[0]), gifs[0].FrameSize.x, gifs[0].FrameSize.y}, (Rectangle){100, h - 400, 300, 300}, (Vector2){0, 0}, 0.0f, WHITE);

  DrawTexturePro(gifs[0].GIFsheet, (Rectangle){GetFrameX(gifs[0]), GetFrameY(gifs[0]), gifs[0].FrameSize.x, gifs[0].FrameSize.y}, (Rectangle){w - 400, h - 400, 300, 300}, (Vector2){0, 0}, 0.0f, WHITE);

  #endif
}

void DrawAboutMe(ui16 w, ui16 h)
{
  #ifdef PLATFORM_WEB 
  DrawText("About MEEEE!", w/2.5, 150, 45, LIGHTGRAY); 

  DrawRectangleGradientH(w/7, 200, w-400, 300, MAGENTA, PURPLE);

  DrawText("Name:\n Jayne :)", w/6, 250, 24, SKYBLUE); 

  DrawText("Age:\n 17", w/1.9, 250, 24, SKYBLUE); 

  DrawText("Likes:\n Video Games, My dog, \nfriend and family, and the C language.", w/6, 300, 24, PINK); 

  DrawText("Favorite Musicician:\n Gezebelle Gaburgably :D.", w/1.9, 300, 24, PINK);   

  if(GuiButton((Rectangle){w/1.3, 300, 150, 50}, "Link to her bandcamp!")) OpenURL("https://gezebellegaburgably.bandcamp.com/");   

  DrawText("Fun Fact:\n This website was written in C using raylib \nand then compilled for the web >:)", w/6, 400, 24, WHITE); 

  DrawText("Hobbies:\n Programming, Video games and \n(listening to) music.", w/1.9, 400, 24, WHITE);   

  DrawTexturePro(gifs[1].GIFsheet, (Rectangle){GetFrameX(gifs[1]), GetFrameY(gifs[1]), gifs[1].FrameSize.x, gifs[1].FrameSize.y}, (Rectangle){200, h - 350, 300, 300}, (Vector2){0, 0}, 0.0f, WHITE);


  DrawTexturePro(gifs[2].GIFsheet, (Rectangle){GetFrameX(gifs[2]), GetFrameY(gifs[2]), gifs[2].FrameSize.x, gifs[2].FrameSize.y},(Rectangle){w - 600, h - 350, 320, 240}, (Vector2){0, 0}, 0.0f, WHITE);

  if(GuiButton((Rectangle){w/2.35, h - 200, 200, 50}, "Return.")) websiteState = MAINMENU;
  
  #endif
}

void DrawMediaIUse(ui16 w, ui16 h)
{
  #ifdef PLATFORM_WEB
  DrawText("Link To Media I use!", w/3, 150, 45, LIGHTGRAY);

  DrawText("Ways to contact me.", w/8, 250, 25, LIGHTGRAY);

  DrawText("Email: jaynelikesivy@gmail.com.", w/8, 300, 20, BLACK);

  DrawText("Discord: Jaynelikesivy.", w/8, 350, 20, BLACK);

  DrawText("Fluxer: Jaynelikesivy.", w/8, 400, 20, BLACK);

  DrawText("Stuff I do.", w/1.5, 250, 25, LIGHTGRAY);

  if(GuiButton((Rectangle){w/1.5, 300, 200, 50}, "Youtube.")) OpenURL("https://youtube.com/jaynelikesivy");

  if(GuiButton((Rectangle){w/1.5, 350, 200, 50}, "Github.")) OpenURL("https://github.com/jaynelikesivy");

  if(GuiButton((Rectangle){w/1.5, 400, 200, 50}, "Steam.")) OpenURL("https://steamcommunity.com/id/Jaynelikeivy/");

  if(GuiButton((Rectangle){w/1.5, 450, 200, 50}, "Hypixel Skyblock Stats.")) OpenURL("https://sky.shiiyu.moe/stats/jayne_t/orange");

  if(GuiButton((Rectangle){w/1.5, 500, 200, 50}, "Wynncraft stats.")) OpenURL("https://wynncraft.com/stats/player/jayne_t");

  DrawTexturePro(gifs[3].GIFsheet, (Rectangle){GetFrameX(gifs[3]), GetFrameY(gifs[3]), gifs[3].FrameSize.x, gifs[3].FrameSize.y}, (Rectangle){200, h - 350, 300, 300}, (Vector2){0, 0}, 0.0f, WHITE);


  DrawTexturePro(gifs[4].GIFsheet, (Rectangle){GetFrameX(gifs[4]), GetFrameY(gifs[4]), gifs[4].FrameSize.x, gifs[4].FrameSize.y}, (Rectangle){w - 500, h - 300, 131, 240}, (Vector2){0, 0}, 0.0f, WHITE);

  if(GuiButton((Rectangle){w/2.35, h - 200, 200, 50}, "Return.")){ websiteState = MAINMENU;}

  #endif
}

void CacheNewBlog(void)
{
  blogBuff[0] = '\0';

  FILE *file = fopen(blogList.paths[currBlog], "r");

  char buff[2000] = "";
  char space[2] = " ";

  while(fgets(buff, sizeof(buff), file))
  {
    strcat(blogBuff, buff);
    blogBuff[strlen(blogBuff)-2] = space[0];
  }
  fclose(file);
}

void DrawBlog(ui16 w, ui16 h)
{
  #ifdef PLATFORM_WEB

    ui16 page = 0;
    ui16 row = 0;

    DrawText("Blog posts.", w/3, 150, 45, LIGHTGRAY);

    DrawText("This part will be used whenever I wanna write random bs for people to read.", w/8, 200, 25, LIGHTGRAY);

  for(ui16 i = 0; i < blogList.count; i++)
  {
    if(currPage == page)
    {
      if(GuiButton((Rectangle){w/3, 250 + (row*75), 200, 50}, TextFormat("%s", GetFileNameWithoutExt(blogList.paths[i]))))
      {
       websiteState = READBLOG; 
       currBlog = (row + (page*7));
       CacheNewBlog();
      }
    }
    row++;
    if(row >= 7){ row = 0; page++;}
  }

  if(currPage != 0){ if(GuiButton((Rectangle){w/2 - 50, h-200, 50, 50}, "<")) currPage--;}
  if(currPage != page){ if(GuiButton((Rectangle){w/2 + 200, h-200, 50, 50}, ">")) currPage++;}
  if(GuiButton((Rectangle){w/2, h-200, 200, 50}, "Return.")) websiteState = MAINMENU;

  #endif
}

void DrawBlogFromFile(ui16 w, ui16 h)
{
  #ifdef PLATFORM_WEB

  if(GuiButton((Rectangle){150, 100, 200, 50}, "Return.")){ websiteState = BLOG;}

  DrawText(TextFormat("%s", GetFileNameWithoutExt(blogList.paths[currBlog])), w/3, 150, 45, LIGHTGRAY);

  DrawText(blogBuff, 150, 250, 24, BLACK);
  #endif
}

void UpdateDrawFrame(void)
{
  
  #if defined(PLATFORM_WEB)
  static ui16 old_w=0,old_h=0;

  ui16 w = canvas_get_width();
  ui16 h = canvas_get_height();
  
  if(w!=old_w || h!=old_h) SetWindowSize(w,h);

  BeginDrawing();  

  ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

  DrawRectangle(BG_RECTANGLE_POS_X, BG_RECTANGLE_POS_Y, w - (BG_RECTANGLE_POS_X+OFFSET-BG_OFFSET), h - (BG_RECTANGLE_POS_Y+OFFSET-BG_OFFSET),SKYBLUE);

  DrawRectangleGradientEx((Rectangle){RECTANGLE_POS_X, RECTANGLE_POS_Y, w - RECTANGLE_POS_X-OFFSET, h - RECTANGLE_POS_Y-OFFSET}, PINK, WHITE, PURPLE, BLACK);

  HandleGifFrames();

  switch(websiteState)
  {
    case MAINMENU:
    {
      DrawMainMenu(w, h);
      break;
    }
    case ABOUTME:
    {
      DrawAboutMe(w, h);
      break;
    }
    
    case LINKSTOMEDIA:
    {
      DrawMediaIUse(w, h);
      break;
    }

    case BLOG:
    {
      DrawBlog(w, h);
      break;
    }

    case READBLOG:
    {
      DrawBlogFromFile(w, h);
      break;
    }
  }

  EndDrawing();

  #endif
}

int main(void)
{
  blogList = LoadDirectoryFiles("/blogs");
  InitWindow(WINDOWSIZE, WINDOWSIZE, "My website");
  GuiLoadStyle("lavanda/style_lavanda.rgs");

  gifCount = 5;


  //yeah yeah this isnt super effective but i have no way of accurately parsing how big a frame is supposed to be nor how long each frame is, its just a png
  // of a gif i took from https://loudoweb.fr/app/video-to-spritesheet/

  gifs[0].GIFsheet = LoadTexture("gifs/KonaDance.png");
  gifs[0].frames = 51;
  gifs[0].FrameSize = (Vector2){240, 240};
  gifs[0].xLimit = 8;
  gifs[0].currentFrame = 0;
  gifs[0].subframe = 0;
  gifs[0].framerate = 60; // This didnt work the way i wanted it to, since normally these usually run at 24fps, maybe ill fix later cuz rn it works fine enough

  gifs[1].GIFsheet = LoadTexture("gifs/GezebelleGaburgably.png");
  gifs[1].frames = 19;
  gifs[1].FrameSize = (Vector2){240, 240};
  gifs[1].xLimit = 5;
  gifs[1].currentFrame = 0;
  gifs[1].subframe = 0;
  gifs[1].framerate = 12;

  gifs[2].GIFsheet = LoadTexture("gifs/CMyBeloved.png");
  gifs[2].frames = 96;
  gifs[2].FrameSize = (Vector2){320, 240};
  gifs[2].xLimit = 10;
  gifs[2].currentFrame = 0;
  gifs[2].subframe = 0;
  gifs[2].framerate = 60;

  gifs[3].GIFsheet = LoadTexture("gifs/AlienCat.png");
  gifs[3].frames = 100;
  gifs[3].FrameSize = (Vector2){293, 240};
  gifs[3].xLimit = 10;
  gifs[3].currentFrame = 0;
  gifs[3].subframe = 0;
  gifs[3].framerate = 60;

  gifs[4].GIFsheet = LoadTexture("gifs/floof.png");
  gifs[4].frames = 176;
  gifs[4].FrameSize = (Vector2){131, 240};
  gifs[4].xLimit = 14;  
  gifs[4].currentFrame = 0;
  gifs[4].subframe = 0;
  gifs[4].framerate = 60;

  #if defined(PLATFORM_WEB)
  
  emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
  #else
  SetTargetFPS(60);
  
  while(!WindowShouldClose())
  {
    DrawText("THIS WONT WORK, THIS ONLY WORKS ON WEB!!!", 100, 0, 24, RED);
  }
  #endif
  
  CloseWindow();
  return 0;
}
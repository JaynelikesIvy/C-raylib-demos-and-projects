#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define SPACE " "

bool auto_run = false;
char name[32] = "main";

FilePathList file_path;

#define GET_TIME(time) (float)(clock()-time)/CLOCKS_PER_SEC


int CompileForLinux(int argc, char* argv[])
{
    clock_t time = clock();

  if(argc > 1)
  {
    for(int i = 1; i < argc; i++)
    {
      if(!strcmp(argv[i], "-auto-run"))
      {
        auto_run = true;
        printf("%f [INFO]: auto-run enabled! (Note, custom exe commands cannot be used)\n", GET_TIME(time));
      }
      else if(!strcmp(argv[i], "-name"))
      {
        i++;
        strcpy(name, argv[i]);
        printf("%f [INFO]: Executable now named '%s'\n", GET_TIME(time), argv[i]);
      }
      else printf("%f [WARNING]: argument '%s' not recognized\n",GET_TIME(time), argv[i]);
    }
  }
  printf("%f [INFO]: Checking for 'main.c' file...\n", GET_TIME(time));
  if(!FileExists("main.c")){ printf("%f [FATAL]: NO main.c SCRIPT FOUND, CANNOT COMPILE, ABORTING\n", GET_TIME(time)); return 1;}
  printf("%f [INFO]: Found 'main.c' file!\n", GET_TIME(time));

  char command[256] = "gcc main.c ";
  
  printf("%f [INFO]: checking for 'src', directory...\n", GET_TIME(time));
  if(!DirectoryExists("src")) printf("%f [WARNING]: No 'src' directory found, no additional scripts will be compiled, if this is not intended, please check your filepath\n", GET_TIME(time));

  else
  { 
    printf("%f [INFO]: Found 'src' directory!\n", GET_TIME(time));
    strcat(command, "-I src ");
    file_path = LoadDirectoryFiles("src");
    for(int i = 0; i < file_path.count; i++)
    {
      if(IsFileExtension(file_path.paths[i], ".c"))
      {
        strcat(command, file_path.paths[i]);
        strcat(command, SPACE);
      }
    }
  }
  strcat(command, "-L lib -lraylib -lm -Wall -Wextra -Wpedantic -o ");

  printf("%f [INFO]: Checking for 'server', directory...\n", GET_TIME(time));
  if(!DirectoryExists("server")){ printf("%f [FATAL]: No 'server' directory found, game will not compile as server is NEEDED to run\n", GET_TIME(time)); return 1;}
  else
  {
    printf("%f [INFO]: 'server' directory found! Compiling server. \n", GET_TIME(time));
    char server_compile[256] = "gcc server/mp_server.c src/mp_networking.c -lpthread -Wall -Wextra -Wpedantic -o server/server";
    printf("%f[INFO]: Compiling server with commands %s\n", GET_TIME(time), server_compile);
    clock_t c_time = clock();
    int success = system(server_compile);
    if(success != 0){printf("%f [FATAL]: Compilation failed for server, please check compilation error log\n", GET_TIME(time)); return 1;}
    else printf("%f [INFO]: Server succesfully compiled after %f seconds!\n", GET_TIME(time), GET_TIME(c_time));
  }
  
  strcat(command, name);

  printf("%f [INFO]: Compiling main.c with command: %s\n", GET_TIME(time), command);
  clock_t c_time = clock();
  int success = system(command);

  if(success == 0)printf("%f [INFO]: Build compiled succesfully after %f seconds!\n",GET_TIME(time), GET_TIME(c_time));
  
  else{ printf("%f [FATAL]: Compilation failed! Please check Compile error log\n", GET_TIME(time)); return 1;}
  

  if(auto_run == 1)
  {
    printf("%f [INFO]: Auto-running running %s\n", GET_TIME(time), name);
    char temp[32] = "./";
    strcat(temp, name);
    system(temp);
  }
  else printf("%f [INFO]: Build complete, exiting\n", GET_TIME(time));

  return 0;
}

int CompileForWindows(int argc, char* argv[])
{
  clock_t time = clock();

  if(argc > 1)
  {
    for(int i = 1; i < argc; i++)
    {
      if(!strcmp(argv[i], "-auto-run"))
      {
        auto_run = true;
        printf("%f [INFO]: auto-run enabled! (Note, custom exe commands cannot be used)\n", GET_TIME(time));
      }
      else if(!strcmp(argv[i], "-name"))
      {
        i++;
        strcpy(name, argv[i]);
        printf("%f [INFO]: Executable now named '%s'\n", GET_TIME(time), argv[i]);
      }
      else printf("%f [WARNING]: argument '%s' not recognized\n",GET_TIME(time), argv[i]);
    }
  }
  printf("%f [INFO]: Checking for 'main.c' file...\n", GET_TIME(time));
  if(!FileExists("main.c")){ printf("%f [FATAL]: NO main.c SCRIPT FOUND, CANNOT COMPILE, ABORTING\n", GET_TIME(time)); return 1;}
  printf("%f [INFO]: Found 'main.c' file!\n", GET_TIME(time));

  char command[256] = "gcc main.c ";
  
  printf("%f [INFO]: checking for 'src', directory...\n", GET_TIME(time));
  if(!DirectoryExists("src")) printf("%f [WARNING]: No 'src' directory found, no additional scripts will be compiled, if this is not intended, please check your filepath\n", GET_TIME(time));

  else
  { 
    printf("%f [INFO]: Found 'src' directory!\n", GET_TIME(time));
    strcat(command, "-I src ");
    file_path = LoadDirectoryFiles("src");
    for(unsigned int i = 0; i < file_path.count; i++)
    {
      if(IsFileExtension(file_path.paths[i], ".c"))
      {
        strcat(command, file_path.paths[i]);
        strcat(command, SPACE);
      }
    }
  }


  printf("%f [INFO]: Checking for 'lib', directory...\n", GET_TIME(time));
  if(!DirectoryExists("lib")) printf("%f [WARNING]: No 'lib' directory found, this will be needed to compile raylib on windows, if this is not intended, please check your filepath\n", GET_TIME(time));
  else
  {
    printf("%f [INFO]: 'lib' directory found!\n", GET_TIME(time));
    strcat(command, "-L lib -lraylib -lgdi32 -lwinmm -lws2_32 -lpthread ");
  }

  printf("%f [INFO]: Checking for 'server', directory...\n", GET_TIME(time));
  if(!DirectoryExists("server")){ printf("%f [FATAL]: No 'server' directory found, game will not compile as server is NEEDED to run\n", GET_TIME(time)); return 1;}
  else
  {
    printf("%f [INFO]: 'server' directory found! Compiling server. \n", GET_TIME(time));
    char server_compile[256] = "gcc server/mp_server.c src/mp_networking.c -lwinmm -lws2_32 -lpthread -Wall -Wextra -Wpedantic -o server/server";
    printf("%f[INFO]: Compiling server with commands %s\n", GET_TIME(time), server_compile);
    clock_t c_time = clock();
    int success = system(server_compile);
    if(success != 0){printf("%f [FATAL]: Compilation failed for server, please check compilation error log\n", GET_TIME(time)); return 1;}
    else printf("%f [INFO]: Server succesfully compiled after %f seconds!\n", GET_TIME(time), GET_TIME(c_time));
  }

  strcat(command, "-Wall -Wextra -Wpedantic -o ");
  
  strcat(command, name);

  printf("%f [INFO]: Compiling main.c with command: %s\n", GET_TIME(time), command);
  clock_t c_time = clock();
  int success = system(command);

  if(success != 0){printf("%f [FATAL]: Compilation failed! Please check Compile error log\n", GET_TIME(time)); return 1;}

  else printf("%f [INFO]: Build compiled succesfully after %f seconds!\n",GET_TIME(time), GET_TIME(c_time)); 

  if(auto_run)
  {
    printf("%f [INFO]: Auto-running running %s\n", GET_TIME(time), name);
    system(name);
  }
  else printf("%f [INFO]: Build complete, exiting\n", GET_TIME(time));

  return 0;
}

int main(int argc, char* argv[])
{
  #ifdef _WIN32
  return CompileForWindows(argc, argv);
  #endif

  #ifdef __linux__
  return CompileForLinux(argc, argv);
  #endif


}
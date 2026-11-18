#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>


clock_t app_time = 0;

char mainName[256] = "main.c";
char appName[256] = "main";
char srcPath[256] = "src/";
char libPath[256] = "lib/";
bool autoRun = false;
bool compile = false;
bool winRaylib = false;

#define GET_TIME(t) (float)(clock()-t)/CLOCKS_PER_SEC

enum FLAG{
  INFO,
  WARN,
  ERROR,
  FATAL,
};


void TraceLog(int logType, const char* format, ...)
{
  char buffer[256];
  sprintf(buffer, "%f ", GET_TIME(app_time));

  va_list args;
  va_start(args, format);
  
  switch(logType)
  {
    case INFO:  strcat(buffer, "INFO: ");  break;
    case WARN:  strcat(buffer, "WARN: ");  break;
    case ERROR: strcat(buffer, "ERROR: "); break;
    case FATAL: strcat(buffer, "FATAL: "); break;
    default: break;
  }

  strcat(buffer, format);
  strcat(buffer, "\n");

  vprintf(buffer, args);
  va_end(args);

  if(logType == FATAL) exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
  if(argc > 1)
  {
    for(int i = 0; i < argc; i++)
    {
      if(!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) printf("FLAGS:\n --compile -c: compile program.\n --main -m: name of main C file.\n --name -n: name of application.\n --autorun -a: autorun program once compilation is complete.\n --toggle-win-raylib -twr: toggle specific includes needed for raylib on windows.\n");
      else if(!strcmp(argv[i], "--compile") || !strcmp(argv[i], "-c")) compile = true;
      else if (!strcmp(argv[i], "--main") || !strcmp(argv[i], "-m")) { i++; strcpy(mainName, argv[i]);}
      else if (!strcmp(argv[i], "--name") || !strcmp(argv[i], "-n")) {i++; strcpy(appName, argv[i]);}
      else if (!strcmp(argv[i], "--autorun") || !strcmp(argv[i], "-a")) autoRun = true;
      else if (!strcmp(argv[i], "--toggle-win-raylib") || !strcmp(argv[i], "-twr")) winRaylib = true;
    }
  }
  if(!compile) return 0;

  app_time = clock();

  TraceLog(INFO, "Compiling program '%s' to '%s' (autorun = %d).", mainName, appName, autoRun);

  if(access(mainName, F_OK) == 0) TraceLog(INFO, "File %s found.", mainName);
  else TraceLog(FATAL, "File %s not found, aborting", mainName);

  return 0;
}

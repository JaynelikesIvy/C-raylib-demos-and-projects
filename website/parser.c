#include "parser.h"

ItemCollection GetFileData(char* fp)
{
  ItemCollection col;
  col.count = 0;

  FILE *file = fopen(fp, "r");
  if(file == NULL)
  {
    fprintf(stderr, "ERROR: FILE IS NULL, PLEASE CHECK FILEPATH (current filepath is %s)", fp); //How would this happen though
  }

  char char_buf[BUF_SIZE];

  int lines = countlines(fp);

  col.items = malloc(sizeof(ParseItem)*lines);

  while(fgets(char_buf, BUF_SIZE, file) != NULL)
  {
    col.items[col.count] = ParseFile(char_buf, col.items[col.count]);
    col.count++;
  }
  fclose(file);
  return col;
}

ParseItem ParseFile(char char_buf[BUF_SIZE], ParseItem item)
{
  int state = 0;

  char type_buf[BUF_SIZE] = ""; // Empty buffers because writing the information directly to the item data breaks stuff, I still don't get C but I adapt
  char val_buf[BUF_SIZE] = "";

  for(size_t i = 0; i < strlen(char_buf); i++)
  {
    switch(state)
    {
      case STATE_SEARCH:
        if(char_buf[i] == TYPE[0]) state = STATE_GET_TYPE; 
        
        else if(char_buf[i] == VALUE[0]) state = STATE_GET_VALUE;
        
        break;
        
      case STATE_GET_TYPE:
        if(char_buf[i] != TYPE[0]) type_buf[strlen(type_buf)] = char_buf[i];

        else state = STATE_SEARCH;
        
        break;

      case STATE_GET_VALUE:
        val_buf[strlen(val_buf)] = char_buf[i];
        break;
    }
  }
  val_buf[strlen(val_buf)-1] = 0; // Avoid unnecessary newlines, does break if there is no newline to begin with though
  strcpy(item.item_name, type_buf);
  strcpy(item.item_value, val_buf);
  return item;
}

int countlines(char *filename) // done in a seperate function because file needs to be open again else it can't fgets()
{                                    
  FILE *fp = fopen(filename,"r");
  int char_count = 0;
  int lines = 1; // By default there should be a line, we already checked if the file is NULL earlier (unless I end up using this elsewhere ill add it back)

  while ((char_count = fgetc(fp)) != EOF)
  {
      if (char_count == '\n') lines++; // Newlines means one extra line (wow thanks sherlock)
  }
  fclose(fp);
  return lines;
}
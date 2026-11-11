#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 6400 //Kinda abritrary icl, maybe will change later
#define NAME_SIZE 128

#define TYPE "'"
#define VALUE ":"

#define STATE_SEARCH 0
#define STATE_GET_TYPE 1
#define STATE_GET_VALUE 2

typedef struct {
  char item_name[NAME_SIZE]; //set size because otherwise fgets assumes this is a good place to store buffer data 
  char item_value[NAME_SIZE]; 
}ParseItem;

typedef struct{
  int count;
  ParseItem* items;
}ItemCollection;

ItemCollection GetFileData(char* fp); //gets file data for properly formatted txt files according to my shit parser
ParseItem ParseFile(char char_buf[BUF_SIZE], ParseItem item); //Parses the file and returns the name and value in the ParseItem struct form
int countlines(char *filename);//Gets the number of lines in the txt
#endif
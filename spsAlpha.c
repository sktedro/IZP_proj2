#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

enum cmds{irow, arow, drow, icol, acol, dcol, set, clear, swap, sum, avg, count, len/*, def, use, inc, set*/};

typedef struct{
  int row;
  int col;
  char* content;
  int len;
} cell_t;

typedef struct{
  int row;
} row_t;

typedef struct{
  int row1;
  int col1;
  int row2;
  int col2;
} cellSel_t;

typedef struct{
} tempSel_t;

typedef struct{
  cellSel_t cmdCellSel;
  enum cmds cmd;
} cmd_t;

int errFn(int errCode){
  switch (errCode){
    case -1:
      fprintf(stderr, "There are not enough aruments entered!");
      break;
    case -2:
      fprintf(stderr, "Too many arguments entered!");
      break;
    case -3:
      fprintf(stderr, "Did you enter the command sequence and the filename, too?");
      break;
    case -4:
      fprintf(stderr, "Sorry, memory allocation was unsuccessful.");
      break;
  }
  fprintf(stderr, " The program will now exit.\n");
  return -errCode;
}

int checkArgs(int argc, char *argv[]){
  if(argc < 3)
    return errFn(-1);
  else if(argc > 5)
    return errFn(-2);
  else if(argc == 4){
    if(!strcmp(argv[1], "-d"))
      return errFn(-3);
    return errFn(-2);
  }
  return 0;
}

char *getDel(char *argv[]){
  if(!strcmp(argv[1], "-d"))
    return argv[2];
  return " ";
}

char *getTab(char *argv[], long *tabSize){
  FILE *tab = fopen(argv[!strcmp(argv[1], "-d") ? 4 : 2], "r");
  //If there are delimiters entered, filename will be as the 4th argument
  fseek(tab, 0, SEEK_END);
  *tabSize = ftell(tab);
  fseek(tab, 0, SEEK_SET);
  char *tabStr = malloc(*tabSize + 1);
  if(!tabStr){
    fclose(tab);
    return NULL;
  }
  fread(tabStr, 1, *tabSize, tab);
  tabStr[*tabSize] = 0;
  fclose(tab);
  return tabStr;
}

void changeDels(char *tabStr, char *del){
  if(del[1])
    for(int i = 0; tabStr[i]; i++)
      for(int j = 1; del[j]; j++)
        if(tabStr[i] == del[j])
          tabStr[i] = del[0];
}

void addCols(long *tabSize, char *tabStr, char *del){
  int maxCols = 0;
  int cols = 0;
  for(int i = 0; tabStr[i]; i++){
    if(tabStr[i] == del[0])
      cols++;
    else if(tabStr[i] == '\n'){
      if(cols > maxCols)
        maxCols = cols;
      cols = 0;
    }
  }
  for(int i = 0; tabStr[i]; i++){
    if(tabStr[i] == del[0])
      cols++;
    else if(tabStr[i] == '\n'){
      if(maxCols - cols > 0){
        int colDiff = maxCols - cols;
        *tabSize += colDiff;
        tabStr = realloc(tabStr, *tabSize + 1);
        //if(!tabStr)
          //return;
        for(int j = *tabSize; j > (i + colDiff); j--)
          tabStr[j] = tabStr[j - colDiff];
        for(int j = 0; j < colDiff; j++){
          tabStr[i+j] = del[0];
          tabStr[i+j+1] = '\n';
        }
        i += colDiff;
      }
      cols = 0;
    }
  }
}

int main(int argc, char *argv[]){
  int errCode;
  errCode = checkArgs(argc, argv);
  if(errCode) return errCode;
  char *del = getDel(argv);


  long tabSize;
  char *tabStr = getTab(argv, &tabSize);
  if(!tabStr)
    return errFn(-4);

  changeDels(tabStr, del);
  addCols(&tabSize, tabStr, del);

  printf("%s", tabStr);



  return 0;
}


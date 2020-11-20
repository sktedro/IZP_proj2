#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define backslash 92

enum cmds{irow, arow, drow, icol, acol, dcol, set, clear, swap, sum, avg, count, len/*, def, use, inc, set*/};

typedef struct{
  char *cont;
  int len;
} cell_t;

typedef struct{
  cell_t *cell;
  int len;
} row_t;

typedef struct{
  row_t *row;
  int len;
} tab_t;


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
    case -5:
      fprintf(stderr, "Unknown error. Check your commands.");
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

int getCmdPlc(char *argv[]){
  if(!strcmp(argv[1], "-d"))
    return 3;
  return 1;
}

/*
bool tabCtor(tab_t *tab){
  tab->len = 0;
  tab->row = NULL;
}

bool rowCtor(tab_t *tab, int i){
  tab->row[i].len = 0;
  tab->row[i].cell = NULL;
  return true;
}
*/

bool cellCtor(tab_t *tab, int i, int j){
  tab->row[i].cell[j].cont = NULL;
  tab->row[i].cell[j].len = 0;
  return true;
}

bool firstMalloc(tab_t *tab){
  tab->row = malloc(sizeof(void*));
  if(!tab->row) return false;
  tab->row[0].cell = malloc(sizeof(void*));
  if(!tab->row[0].cell) return false;
  tab->row[0].cell[0].cont = malloc(sizeof(void*));
  if(!tab->row[0].cell[0].cont) return false;
  tab->len = tab->row[0].len = tab->row[0].cell[0].len = 0;
  return true;
}

/*char **/int getTab(char *argv[], tab_t *tab, char *del){
  FILE *tabFile = fopen(argv[!strcmp(argv[1], "-d") ? 4 : 2], "r");
  //If there are delimiters entered, filename will be as the 4th argument
  int rowN = 1, cellN = 1, cellcN = 1;
  char tempC;
  bool quoted = false, makeQuoted = false;
  /*
  tab = malloc(sizeof(void*) + sizeof(int));
  if(!tab) return 4;
  tab->row = malloc(rowN*sizeof(void*) + cellN*sizeof(int));
  if(!tab->row) return 4;
  tab->row[0].cell = malloc(cellN*sizeof(void*) + cellN*sizeof(int));
  if(!tab->row[0].cell) return 4;
  tab->row[0].cell[0].cont = malloc(cellcN*sizeof(char*));
  if(!tab->row[0].cell[0].cont) return 4;
  */
  if(!firstMalloc(tab)) return 4;

  for(int i = 0; tempC != EOF; i++){
    bool skip = false;
    tempC = fgetc(tabFile);
    if(tempC == '"') quoted = quoted ? false : true;
    if(!quoted){
      //if(tempC == backslash)
        //makeQuoted = true;
      if(tempC == '\n'){
        skip = true;
        rowN++;
        cellN = 1;
        cellcN = 1;
        row_t *p = realloc(tab->row, rowN*sizeof(row_t) + sizeof(int));
        if(!p) return 4;
        tab->row = p;
        tab->row[rowN-1].cell = malloc(cellN*sizeof(cell_t) + cellN*sizeof(int));
        tab->row[rowN-1].len = 0;
        continue;
      }
      else{
        for(int j = 0; del[j]; j++){
          if(tempC == del[j]){
            skip = true;
            cellcN = 1;
            cellN++;
            cell_t *p = realloc(tab->row[rowN-1].cell, cellN*sizeof(cell_t) + cellN*sizeof(int));
            if(!p) return 4;
            tab->row[rowN-1].cell = p;
            break;
          }
        }
      }
    }
    if(!skip){
      if(cellcN == 1){
        tab->row[rowN-1].cell[cellN-1].cont = malloc(sizeof(char*));
        //tab->row[rowN-1].cell[cellN-1].cont[0] = '\0';
      }
      cellcN++;
      char *p = realloc(tab->row[rowN-1].cell[cellN-1].cont, (cellcN)*sizeof(char*));
      if(!p) return 4;
      tab->row[rowN-1].cell[cellN-1].cont = p;
      tab->row[rowN-1].cell[cellN-1].cont[cellcN-2] = tempC;
      tab->row[rowN-1].cell[cellN-1].cont[cellcN-1] = '\0';
    }
    tab->len = rowN;
    tab->row[rowN-1].len = cellN;
    tab->row[rowN-1].cell[cellN-1].len = cellcN-1;
  }
  fclose(tabFile);
  return 0;
}

void freeTab(tab_t *tab){
  for(int i = 0; i < tab->len; i++){
    for(int j = 0; j < tab->row[i].len; j++)
      free(tab->row[i].cell[j].cont);
    free(tab->row[i].cell);
  }
  free(tab->row);
}

void printTab(tab_t *tab, char *del){
  for(int i = 0; i+2 < tab->len; i++){
    for(int j = 0; j < tab->row[i].len; j++){
      for(int k = 0; k < tab->row[i].cell[j].len; k++)
        printf("%c", tab->row[i].cell[j].cont[k]);
      if(j+1 != tab->row[i].len)
        printf("%c", del[0]);
    }
    printf("\n");
  }
}

/*
void changeDels(char *tabStr, char *del){
  if(del[1])
    for(int i = 0; tabStr[i]; i++)
      for(int j = 1; del[j]; j++)
        if(tabStr[i] == del[j])
          tabStr[i] = del[0];
}
*/

bool shiftAndInsert(long *tabSize, char *tabStr, int startPoint, int strSize, char *str){
  for(int j = *tabSize; j > (startPoint + strSize); j--){
    tabStr[j] = tabStr[j - strSize];
    tabStr[j - strSize] = '\0';
  }
  bool strSizeMatch = strSize == (int)strlen(str);
  for(int j = 0; j < strSize; j++){
    if(strSize != 1 && str[1] == '\0'){
      tabStr[startPoint+j] = str[0];
      tabStr[startPoint+j+1] = '\n';
    }
    else if(strSizeMatch)
      tabStr[startPoint+j] = str[j];
    else return false;
  }
  return true;
}

int addCols(long *tabSize, char **tabStr, char *del){
  int maxCols = 0, cols = 0;
  for(int i = 0; tabStr[0][i]; i++){
    if(tabStr[0][i] == del[0])
      cols++;
    else if(tabStr[0][i] == '\n'){
      if(cols > maxCols)
        maxCols = cols;
      cols = 0;
    }
  }
  for(int i = 0; tabStr[0][i]; i++){
    if(tabStr[0][i] == del[0])
      cols++;
    else if(tabStr[0][i] == '\n'){
      if(maxCols - cols > 0){
        int colDiff = maxCols - cols;
        *tabSize += colDiff;
        char *p = realloc(*tabStr, *tabSize + 1);
        if(!p){
          free(*tabStr);
          return -4;
        }
        *tabStr = p;
        char delToInsert[2] = {del[0], '\0'};
        if(!shiftAndInsert(tabSize, tabStr[0], i, colDiff, delToInsert))
          return -5;
        i += colDiff;
      }
      cols = 0;
    }
  }
  return 0;
}

int execCmds(char *argv[], int cmdPlc, long *tabSize, char *tabStr, char *del){
  (void) tabSize; (void) tabStr; (void) del;
  char *selCmd = 0;
  char *cmd, *cmdDel = ";";
  cmd = strtok(argv[cmdPlc], cmdDel);
  while(cmd){
    if(cmd[0] == '[' && cmd[1] != 's'){
      strcpy(selCmd, cmd);
      //cell selection
      continue;
    }
    cmd = strtok(NULL, cmdDel);
    //executing the command
  }
  return 0;
}

int main(int argc, char *argv[]){
  int errCode;
  errCode = checkArgs(argc, argv);
  if(errCode) return errCode;
  char *del = getDel(argv);
  int cmdPlc = getCmdPlc(argv);

  //long tabSize;
  tab_t tab;
  getTab(argv, &tab, del);
  printTab(&tab, del);
  freeTab(&tab);
  /*char *tabStr = getTab(argv, &tabSize);
  if(!tabStr)
    return errFn(-4);
    */

  /*changeDels(tabStr, del);
  errCode = addCols(&tabSize, &tabStr, del);
  if(errCode)
    return errFn(errCode);

  errCode = execCmds(argv, cmdPlc, &tabSize, tabStr, del);
  */
  //printf("%s", tabStr);

  //free(tabStr);
  return 0;
}


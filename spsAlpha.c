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
  cellSel_t tempSel[10];
} tempSel_t;

typedef struct{
  cellSel_t cmdSel;
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

bool firstMalloc(tab_t *tab){
  tab->row = malloc(sizeof(row_t));
  if(!tab->row) return false;
  tab->row[0].cell = malloc(sizeof(cell_t));
  if(!tab->row[0].cell) return false;
  tab->row[0].cell[0].cont = malloc(sizeof(void*));
  if(!tab->row[0].cell[0].cont) return false;
  tab->len = tab->row[0].len = tab->row[0].cell[0].len = 0;
  return true;
}

bool isDel(char *c, char *del){
  for(int i = 0; del[i]; i++)
    if(*c == del[i])
      return true;
  return false;
}

bool getTab(char *argv[], tab_t *tab, char *del){
  FILE *tabFile = fopen(argv[!strcmp(argv[1], "-d") ? 4 : 2], "r");
  //If there are delimiters entered, filename will be as the 4th argument
  int rowN = 1, cellN = 1, cellcN = 1;
  char tempC;
  bool quoted = false/*, makeQuoted = false*/;
  if(!firstMalloc(tab)) return false;
  for(int i = 0; tempC != EOF; i++){
    bool skip = false;
    tempC = fgetc(tabFile);
    if(tempC == '"') quoted = quoted ? false : true;
    if(!quoted){
      //if(tempC == backslash)
        //makeQuoted = true;
      if(tempC == '\n'){
        rowN++;
        skip = true;
        cellN = cellcN = 1;
        row_t *p = realloc(tab->row, rowN*sizeof(row_t));
        if(!p) return false;
        tab->row = p;
        tab->row[rowN-1].cell = malloc(cellN*sizeof(cell_t));
        tab->row[rowN-1].len = 0;
        tab->row[rowN-1].cell[cellN-1].cont = malloc(sizeof(void*));
      }else if(isDel(&tempC, del)){
        cellN++;
        skip = true;
        cellcN = 1;
        cell_t *p = realloc(tab->row[rowN-1].cell, cellN*sizeof(cell_t));
        if(!p) return false;
        tab->row[rowN-1].cell = p;
        tab->row[rowN-1].cell[cellN-1].cont = malloc(sizeof(void*));
      }
    }
    if(!skip){
      cellcN++;
      char *p = realloc(tab->row[rowN-1].cell[cellN-1].cont, (cellcN)*sizeof(void*));
      if(!p) return false;
      tab->row[rowN-1].cell[cellN-1].cont = p;
      tab->row[rowN-1].cell[cellN-1].cont[cellcN-2] = tempC;
      tab->row[rowN-1].cell[cellN-1].cont[cellcN-1] = '\0';
    }
    tab->len = rowN;
    tab->row[rowN-1].len = cellN;
    tab->row[rowN-1].cell[cellN-1].len = cellcN;
  }
  fclose(tabFile);
  return true;
}

void freeTab(tab_t *tab){
  for(int i = 0; i < tab->len; i++){
    for(int j = 0; j < tab->row[i].len; j++){
      free(tab->row[i].cell[j].cont);
    }
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

int getMaxCols(tab_t *tab){
  int max = 0;
  for(int i = 0; i < tab->len; i++)
    if(tab->row[i].len > max)
      max = tab->row[i].len;
  return max;
}

bool addCols(tab_t *tab){
  int maxCols = getMaxCols(tab);
  for(int i = 0; i < tab->len; i++){
    if(tab->row[i].len != maxCols){
      int diff = maxCols - tab->row[i].len;
      cell_t *p = realloc(tab->row[i].cell, maxCols*sizeof(cell_t));
      if(!p) return false;
      tab->row[i].cell = p;
      tab->row[i].len = maxCols;
      for(int j = 0; j < diff; j++){
        tab->row[i].cell[maxCols-diff+j].cont = malloc(sizeof(void*));
        if(!tab->row[i].cell[maxCols-diff+j].cont) return false;
        tab->row[i].cell[maxCols-diff+j].cont[0] = '\0';
        tab->row[i].cell[maxCols-diff+j].len = 1;
      }
    }
  }
  return true;
}

int getCellSelArg(char *cmd, int argNum){
  char *selArg = malloc(sizeof(char));
  if(!selArg) return -4;
  int i = 1, j = 1, k = 1;
  while(j != argNum){
    //if(cmd[i] == ']')
      //return -2;
    if(cmd[i] == ',')
      j++;
    i++;
  }
  while(cmd[i] != ',' && cmd[i] != ']'){
    char *p = realloc(selArg, (k+1)*sizeof(char));
    if(!p){ 
      free(selArg);
      return -4;
    }
    selArg[k-1] = cmd[i];
    selArg[k] = '\0';
    i++;
    k++;
  }
  char *tolptr = NULL;
  int arg = strtol(selArg, &tolptr, 10);
  if(!strcmp(selArg, "_")){
    free(selArg);
    return 0; //Whole row/column was selected
  }
  free(selArg);
  if(*tolptr)
    return -1;
  return arg;
}

int isCellSel(char *cmd, cellSel_t *cellSel){
  if(cmd[0] != '[' || cmd[1] == 'f' || cmd[1] == 'm')
    return 0; //Not a selection command
  if(cmd[1] == 's'){
    //TODO working with tempSels
    return 1;
  }
  int args = 1;
  for(int i = 0; cmd[i] != ']'; i++)
    if(cmd[i] == ',')
      args++;
  if(args != 2 && args != 4)
    return -6;
  int argsArr[4];
  for(int i = 0; i < args; i++){
    argsArr[i] = getCellSelArg(cmd, i+1);
    if(argsArr[i] == -4) return -4;
    if(argsArr[i] == -1) return 0; //Argument is not a number and neither '_'
  }
  cellSel->row1 = argsArr[0];
  cellSel->col1 = argsArr[1];
  //printf("%d, %d; ", argsArr[0], argsArr[1]); 
  if(args == 4){
    cellSel->row2 = argsArr[2];
    cellSel->col2 = argsArr[3];
    //printf("%d, %d", argsArr[2], argsArr[3]); 
  }
  //printf("\n");

  return 1;
}

bool isEscaped(char *str, int charPlc){
  int i = 0;
  //TODO PRECO  TO KURVA NEFUNGUJE? Program si mysli, ze je zadanych menej
  //backslashov, ako actually zadanych je... Napr zadam 6, program vidi ?3?
  while((charPlc - i) > 0 && str[charPlc - i - 1] == backslash){
    i++;
    //printf("<%d>\n", i);
  }
  if(i % 2)
    return true;
  return false;
}

char *getCmd(char *argv[], int cmdPlc, int cmdNum){
  int actCmdNum = 1;
  int i = 0;
  while(argv[cmdPlc][i]){
    bool isQuoted = false;
    bool isProperDelim = false;
    if(argv[cmdPlc][i] == '"' && !isEscaped(argv[cmdPlc], i))
      isQuoted = isQuoted ? false : true;
    //else if(argv[cmdPlc][i] == ';' && !isQuoted){
    else if(argv[cmdPlc][i] == ';' && !isQuoted && !isEscaped(argv[cmdPlc], i))
      isProperDelim = true;
    //TODO alebo to staci??
    if(isProperDelim)
      actCmdNum++;
    if(cmdNum == 1)
      return &argv[cmdPlc][0];
    if(actCmdNum == cmdNum)
      return &argv[cmdPlc][i+1];
    i++;
  }
  return NULL;
}

int execCmds(char *argv[], int cmdPlc, tab_t *tab){
        (void) tab;
  char *cmd;
  int cmdNum = 0;
  cellSel_t cellSel = {1, 1, -1, -1};
  while(1){
    //Getting next command:
    cmdNum++;
    cmd = getCmd(argv, cmdPlc, cmdNum);
    if(cmd == NULL)
      break;

    //Selection:
    int isCellSelRet = isCellSel(cmd, &cellSel);
    if(isCellSelRet < 0)
      return isCellSelRet; //Err
    else if(isCellSelRet > 0){
      continue; //The command is a selection command
    }

    //Exectution:
  }
  return 0;
}

int freeAndErr(tab_t *tab, int errCode){
  freeTab(tab);
  return errFn(errCode);
}

int main(int argc, char *argv[]){
  int errCode;
  errCode = checkArgs(argc, argv);
  if(errCode) return errCode;
  char *del = getDel(argv);
  int cmdPlc = getCmdPlc(argv);
  tab_t tab;
  if(!getTab(argv, &tab, del)) return freeAndErr(&tab, -4);
  if(!addCols(&tab)) return freeAndErr(&tab, -4);
  errCode = execCmds(argv, cmdPlc, &tab);
  if(errCode) return freeAndErr(&tab, errCode);

  printTab(&tab, del);
  freeTab(&tab);

  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define backslash         92

#define ROW               tab->row
#define SROW(X)           tab->row[X]
#define CELL(X)           tab->row[X].cell
#define SCELL(X, Y)       tab->row[X].cell[Y]
#define CONT(X, Y)        tab->row[X].cell[Y].cont
#define SCONT(X, Y, Z)    tab->row[X].cell[Y].cont[Z]
//SROW, SCELL, SCONT is a specific row/cell/cont

#define perr(X)           fprintf(stderr, X)

//TODO USE THOSE MACROS <3

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
  int r1;
  int c1;
  int r2;
  int c2;
} cellSel_t;

typedef struct{
  cellSel_t tempSel[10];
  int len;
} tempSel_t;

typedef struct{
  cellSel_t cmdSel;
  enum cmds cmd;
} cmd_t;

int errFn(int errCode){
  switch (errCode){
    case -1:
      perr("There are not enought arguments entered!");
      break;
    case -2:
      perr("Too many arguments entered!");
      break;
    case -3:
      perr("Did you enter the command sequence and the filename, too?");
      break;
    case -4:
      perr("Sorry, memory allocation was unsuccessful.");
      break;
    case -5:
      perr("Unknown error. Check your commands.");
      break;
    case -6:
      perr("Numbers in entered selection commands have to be bigger than 0.");
  }
  perr(" The program will now exit.\n");
  return -errCode;
}

bool allocNewCell(){

  return true;
}

bool allocNewRow(){

  return true;
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
  ROW = malloc(sizeof(row_t));
  if(!ROW) return false;
  CELL(0) = malloc(sizeof(cell_t));
  if(!CELL(0)) return false;
  CONT(0, 0) = malloc(1);
  if(!CONT(0, 0)) return false;
  tab->len = SROW(0).len = SCELL(0, 0).len = 0;
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
        row_t *p = realloc(ROW, rowN*sizeof(row_t));
        if(!p) return false;
        ROW = p;
        CELL(rowN-1) = malloc(cellN*sizeof(cell_t));
        SROW(rowN-1).len = 0;
        CONT(rowN-1, cellN-1) = malloc(1);
      }else if(isDel(&tempC, del)){
        cellN++;
        skip = true;
        cellcN = 1;
        cell_t *p = realloc(CELL(rowN - 1), cellN*sizeof(cell_t));
        if(!p) return false;
        CELL(rowN-1) = p;
        CONT(rowN-1, cellN-1) = malloc(1);
      }
    }
    if(!skip){
      cellcN++;
      char *p = realloc(CONT(rowN - 1, cellN - 1), cellcN);
      if(!p) return false;
      CONT(rowN - 1, cellN - 1) = p;
      SCONT(rowN - 1, cellN - 1, cellcN - 2) = tempC;
      SCONT(rowN - 1, cellN - 1, cellcN - 1) = '\0';
    }
    tab->len = rowN;
    SROW(rowN - 1).len = cellN;
    SCELL(rowN - 1, cellN - 1).len = cellcN;
  }
  fclose(tabFile);
  return true;
}

void freeTab(tab_t *tab){
  for(int i = 0; i < tab->len; i++){
    for(int j = 0; j < SROW(i).len; j++){
      free(CONT(i, j));
    }
    free(CELL(i));
  }
  free(ROW);
}

void printTab(tab_t *tab, char *del){
  for(int i = 0; i+1 < tab->len; i++){
    for(int j = 0; j < SROW(i).len; j++){
      for(int k = 0; k < SCELL(i, j).len; k++)
        printf("%c", SCONT(i, j, k));
      if(j+1 != SROW(i).len)
        printf("\t%c\t", del[0]);
    }
    printf("\n");
  }
}

int getMaxCols(tab_t *tab){
  int max = 0;
  for(int i = 0; i < tab->len - 1; i++)
    if(SROW(i).len > max)
      max = SROW(i).len;
  return max;
}

bool addCols(tab_t *tab){
  int maxCols = getMaxCols(tab);
  for(int i = 0; i < tab->len; i++){
    if(SROW(i).len != maxCols){
      int diff = maxCols - SROW(i).len;
      cell_t *p = realloc(SROW(i).cell, maxCols*sizeof(cell_t));
      if(!p) return false;
      SROW(i).cell = p;
      SROW(i).len = maxCols;
      for(int j = 0; j < diff; j++){
        SROW(i).cell[maxCols-diff+j].cont = malloc(1);
        if(!SROW(i).cell[maxCols-diff+j].cont) return false;
        SROW(i).cell[maxCols-diff+j].cont[0] = '\0';
        SROW(i).cell[maxCols-diff+j].len = 1;
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
      //TODO osetrit, ci tam nie je malo argumentov v prikaze
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
    selArg = p;
    selArg[k-1] = cmd[i];
    selArg[k] = '\0';
    i++;
    k++;
  }
  if(!strcmp(selArg, "_")){
    free(selArg);
    return 0; //Whole row/column was selected
  }
  char *tolptr = NULL;
  int arg = strtol(selArg, &tolptr, 10);
  if(*tolptr){
    free(selArg);
    return -1;
  }
  free(selArg);
  if(arg < 1)
    return -6;
  return arg;
}

int isCellSel(char *cmd, cellSel_t *sel, cellSel_t *tempSel){
  if(cmd[0] != '[' || cmd[1] == 'f' || cmd[1] == 'm')
    return 0; //Not a selection command
  if(cmd[1] == 's'){
    if(strstr(cmd, "[set]") == cmd && (cmd[5] == ';' || cmd[5] == '\0'))
      *tempSel = *sel;
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
    if(argsArr[i] == -6) return -6;
  }
  sel->r1 = argsArr[0];
  sel->c1 = argsArr[1];
  //printf("%d, %d; ", argsArr[0], argsArr[1]); 
  if(args == 4){
    sel->r2 = argsArr[2];
    sel->c2 = argsArr[3];
    //printf("%d, %d", argsArr[2], argsArr[3]); 
  }else{
    sel->r2 = -1;
    sel->c2 = -1;
  }
  //printf("\n");

  return 1;
}

bool isEscaped(char *str, int charPlc){
  int i = 0;
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
      return &argv[cmdPlc][i];
    i++;
  }
  if(cmdNum > actCmdNum + 1)
    return NULL;
  return &argv[cmdPlc][i]; 
}

bool tempVarFn(char *cmd, int cmdLen, char *tempVar[10], cellSel_t sel, tab_t *tab){
  int var;
  if(cmdLen < 6) return 0;
  /*
  printf("%s", cmd);
  printf("<%c>\n", cmd[6]);
  */
  if((cmd[6] == '\0' || cmd[6] == ';') && cmd[5] >= '0' && cmd[5] <= '9'){
    var = cmd[5] - '0';
    if(sel.r2 > 0 || sel.c2 > 0 
        || sel.r1 < 1 || sel.c1 < 1) 
      return 0;
    if(strstr(cmd, "def _") == cmd){
      char *p = realloc(tempVar[var], SCELL(sel.r1, sel.c1).len + 1);
      if(!p) return 0;
      tempVar[var] = p;
      strcpy(tempVar[var], CONT(sel.r1 - 1, sel.c1 - 1));
    }else if(strstr(cmd, "use _") == cmd){
      int tempVarLen = strlen(tempVar[var]) + 1;
      if(tempVarLen != tab->row[sel.r1 - 1].cell[sel.c1 - 1].len){
        char *p = realloc(tab->row[sel.r1 - 1].cell[sel.c1 - 1].cont, tempVarLen);
        if(!p) return 0;
        tab->row[sel.r1 - 1].cell[sel.c1 - 1].cont = p;
        tab->row[sel.r1 - 1].cell[sel.c1 - 1].len = tempVarLen;
        tab->row[sel.r1 - 1].cell[sel.c1 - 1].cont[tempVarLen - 1] = '\0';
      }
      strcpy(tab->row[sel.r1 - 1].cell[sel.c1 - 1].cont, tempVar[var]);
    }else if(strstr(cmd, "inc _") == cmd){
      char *todptr;
      double val = strtod(tempVar[var], &todptr);
      if(todptr && todptr[0] != '\0') 
        val = 0;
      sprintf(tempVar[var], "%g", ++val);
    }
  }
  return 0;
}

bool allocTempVars(char *tempVar[10]){
  for(int i = 0; i < 10; i++){
    tempVar[i] = malloc(1);
    if(!tempVar[i]) return false;
    tempVar[i][0] = '\0';
  }
  return true;
}

void freeTempVars(char *tempVar[10]){
  for(int i = 0; i < 10; i++)
    free(tempVar[i]);
}

//Deleting or adding rows
bool shiftRow(tab_t *tab, int from, int by){
  if(by > 0){ //Inserting or adding rows
    row_t *p = realloc(tab->row, (tab->len + by)*sizeof(row_t));
    if(!p) return false;
    tab->len += by; 
    ROW = p;
    for(int i = tab->len - 1; i > from; i--)
      SROW(i) = SROW(i - 1);
    for(int i = 0; i < by; i++){
      tab->row[from + i].cell = malloc(sizeof(cell_t));
      if(!tab->row[from + i].cell) return false;
      tab->row[from + i].len = 1;
      CONT(from + i, 0) = malloc(1);
      if(!CONT(from + i, 0)) return false;
      SCONT(from + i, 0, 0) = '\0';
      SCELL(from + i, 0).len = 1;
    }
  }else if(by < 0){ //Deleting rows
    for(int i = 0; i < -by; i++){
      for(int j = 0; j < tab->row[from + i - 1].len; j++)
        free(tab->row[from + i - 1].cell[j].cont);
      free(tab->row[from + i - 1].cell);
    }
    for(int i = from - 1; i < tab->len + by + 1; i++)
      SROW(i) = tab->row[i + (-by)];
    row_t *p = realloc(tab->row, (tab->len + by)*sizeof(row_t));
    if(!p) return false;
    tab->len += by;
    ROW = p;
  }
  return true;
}

bool shiftCol(tab_t *tab, int from, int by){
  if(by > 0){
    for(int i = 0; i < tab->len; i++){
      cell_t *p = realloc(SROW(i).cell, (SROW(i).len + by)*sizeof(cell_t));
      if(!p) 
        return false;
      SROW(i).len += by;
      SROW(i).cell = p;
      for(int j = (SROW(i).len - 1); j > from; j--)
        SCELL(i, j) = SROW(i).cell[j - by];
      for(int j = from; j < from + by; j++){
        SCELL(i, j).cont = malloc(1);
        if(!SCELL(i, j).cont)
          return false;
        SCONT(i, j, 0) = '\0';
        SCELL(i, j).len = 1;
      }
    }
  }else if(by < 0){
    by = -by; //Getting the absolute value
    for(int i = 0; i < (tab->len - 1); i++){
      for(int j = 0; j < by; j++)
        free(SROW(i).cell[from + j - 1].cont);
      for(int j = 0; j < (SROW(i).len - from); j++)
        SROW(i).cell[from + j - 1] = SROW(i).cell[from + j - 1 + by];
      cell_t *p = realloc(SROW(i).cell, (SROW(i).len - by)*sizeof(cell_t));
      if(!p)
        return false;
      SROW(i).cell = p;
      SROW(i).len -= by;
    }
  }
  return true;
}

int tabEdit(char *cmd, int cmdLen, tab_t *tab, cellSel_t sel){
  if(sel.r2 != -1 || sel.c2 != -1)
    return 0;
  if(cmdLen == 4 && (cmd[4] == '\0' || cmd[4] == ';')){
    if(strstr(cmd, "irow") == cmd){
      if(!shiftRow(tab, sel.r1 - 1, 1))
        return -1;
    }else if(strstr(cmd, "arow") == cmd){
      if(!shiftRow(tab, sel.r1, 1))
        return -1;
    }else if(strstr(cmd, "drow") == cmd){
      if(!shiftRow(tab, sel.r1, -1))
        return -1;
    }else if(strstr(cmd, "icol") == cmd){
      if(!shiftCol(tab, sel.c1 - 1, 1))
        return -1;
    }else if(strstr(cmd, "acol") == cmd){
      if(!shiftCol(tab, sel.c1, 1))
        return -1;
    }else if(strstr(cmd, "dcol") == cmd)
      if(!shiftCol(tab, sel.c1, -1))
        return -1;
    addCols(tab);
  }
  return 0;
}

int contEdit(char *cmd, int cmdLen, tab_t *tab, cellSel_t sel){
  (void) cmd; (void) cmdLen; (void) tab; (void) sel;

  return 0;
}

int execCmds(char *argv[], int cmdPlc, tab_t *tab){
  char *cmd;
  int cmdLen;
  int cmdNum = 0;
  cellSel_t sel = {1, 1, -1, -1};
  cellSel_t tempSel = {-1, -1, -1, -1};
  char *tempVar[10];
  if(!allocTempVars(tempVar)) return -4;
  while(1){
    //Getting next command:
    cmdNum++;
    cmd = getCmd(argv, cmdPlc, cmdNum);
    cmdLen = getCmd(argv, cmdPlc, cmdNum + 1) - cmd;
    if(cmd == NULL)
      break;
    if(cmd != argv[cmdPlc]){
      cmd++; //If this is not the first cmd, skip the delim
      cmdLen--; //And dont count the delim as part of the command
    }
    //Selection:
    int isCellSelRet = isCellSel(cmd, &sel, &tempSel);
    if(isCellSelRet < 0){
      freeTempVars(tempVar);
      return isCellSelRet; //Err
    }
    else if(isCellSelRet > 0){
      continue; //The command is a selection command
    }
    //Exectution:
    tempVarFn(cmd, cmdLen, tempVar, sel, tab);
    tabEdit(cmd, cmdLen, tab, sel);
    contEdit(cmd, cmdLen, tab, sel);
  }
  freeTempVars(tempVar);
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

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

void printTab(tab_t *tab, char *del);
int errFn(int errCode);
bool mallocCont(tab_t *tab, int rowN, int cellN, int cellcN);
bool mallocCell(tab_t *tab, int rowN, int cellN);
void freeCell(tab_t *tab, int rowN, int cellN);
void freeRow(tab_t *tab, int rowN);
void freeTab(tab_t *tab, char *tempVar[10]);
bool reallocCont(tab_t *tab, int rowN, int cellN, int by);
bool reallocRows(tab_t *tab, int nextSize);
bool editTableRows(tab_t *tab, int from, int by);
bool reallocCells(tab_t *tab, int rowN, int newSize);
bool editTableCols(tab_t *tab, int rowN, int from, int by);
int checkArgs(int argc, char *argv[]);
char *getDel(char *argv[]);
int getCmdPlc(char *argv[]);
bool isDel(char *c, char *del);
bool getTab(char *argv[], tab_t *tab, char *del);
int getMaxCols(tab_t *tab);
bool addCols(tab_t *tab);
int getCellSelArg(char *cmd, int argNum);
int isCellSel(char *cmd, cellSel_t *sel, cellSel_t *tempSel);
bool isEscaped(char *str, int charPlc);
char *getCmd(char *argv[], int cmdPlc, int cmdNum);
bool tempVarFn(char *cmd, int cmdLen, char *tempVar[10], cellSel_t sel, tab_t *tab);
bool allocTempVars(char *tempVar[10]);
int tabEdit(char *cmd, int cmdLen, tab_t *tab, cellSel_t sel);
int contEdit(char *cmd, int cmdLen, tab_t *tab, cellSel_t sel);
int execCmds(char *argv[], int cmdPlc, tab_t *tab, char *tempVar[10]);
int freeAndErr(tab_t *tab, int errCode, char *tempVar[10]);

/*
** "Main" function
*/

int main(int argc, char *argv[]){
  int errCode;
  errCode = checkArgs(argc, argv);
  if(errCode) 
    return errCode;
  char *del = getDel(argv);
  int cmdPlc = getCmdPlc(argv);
  char *tempVar[10] = {0};
  if(!allocTempVars(tempVar)) 
    return errFn(-4);
  tab_t tab;
  if(!getTab(argv, &tab, del)) 
    return freeAndErr(&tab, -4, tempVar);
  if(!addCols(&tab)) 
    return freeAndErr(&tab, -4, tempVar);
  errCode = execCmds(argv, cmdPlc, &tab, tempVar);
  if(errCode) 
    return freeAndErr(&tab, errCode, tempVar);
  printTab(&tab, del);
  freeTab(&tab, tempVar);
  return 0;
}

/*
** Free and allocation functions
*/

void freeTab(tab_t *tab, char *tempVar[10]){
  for(int i = tab->len - 1; i >= 0; i--)
    freeRow(tab, i);
  free(ROW);
  for(int i = 0; i < 10; i++)
    free(tempVar[i]);
}

void freeRow(tab_t *tab, int rowN){
  for(int j = SROW(rowN).len - 1; j >= 0; j--)
    freeCell(tab, rowN, j);
  free(CELL(rowN));
  tab->len--;
}

void freeCell(tab_t *tab, int rowN, int cellN){
  free(CONT(rowN, cellN));
  SROW(rowN).len--;
}

//Deleting or adding rows
bool editTableRows(tab_t *tab, int from, int by){
  if(from < 0)
    from = tab->len;
  if(by > 0){ //Inserting or adding rows
    if(!reallocRows(tab, (tab->len + by)))
      return false;
    for(int i = tab->len - 1; i > from; i--)
      SROW(i) = SROW(i - 1);
    for(int i = 0; i < by; i++)
      if(!mallocCell(tab, from + i + 1, 1))
        return false;
  }else if(by < 0){ //Deleting rows
    for(int i = 0; i < (-by); i++)
      freeRow(tab, from + i - 1);
    for(int i = from - 1; i < tab->len + by + 1; i++)
      SROW(i) = SROW(i + (-by));
    if(!reallocRows(tab, (tab->len - (-by) + 1)))
      return false;
  }
  return true;
}

//Deleting or adding columns
bool editTableCols(tab_t *tab, int rowN, int from, int by){
  int howManyRows = 1;
  if(rowN == 0){
    howManyRows = tab->len - 1;
    rowN = 1;
  }
  if(from == 0)
    from = SROW(0).len;
  for(int i = rowN; i < rowN + howManyRows; i++){
    int newCellN = SROW(i - 1).len + by;
    if(newCellN < 1){
      if(!editTableRows(tab, i, -1))
        return false;
    }else if(by > 0){
      if(from >= SROW(i - 1).len)
        from = SROW(i - 1).len + 1;
      if(!reallocCells(tab, i, newCellN))
        return false;
      if(from < SROW(i - 1).len - by)
        for(int j = (SROW(i - 1).len - 1); j + 1 > from; j--)
          SCELL(i - 1, j) = SCELL(i - 1, j - by);
      for(int j = from; j < from + by; j++)
        if(!mallocCont(tab, i, j, 1))
          return false;
    }
    else if(by < 0){
      for(int j = 0; j < (-by); j++)
        free(CONT(i - 1, from + j - 1));
      for(int j = from; j < (SROW(i - 1).len - (-by) + 1); j++)
        SCELL(i - 1, j - 1) = SCELL(i - 1, j - 1 + (-by));
      if(!reallocCells(tab, i, newCellN))
        return false;
    }
  }
  return true;
}


//Allocate space for cells of a new row
bool mallocCell(tab_t *tab, int rowN, int cellN){
  CELL(rowN - 1) = malloc(cellN * sizeof(cell_t));
  if(!CELL(rowN - 1))
    return false;
  SROW(rowN - 1).len = 1;
  for(int i = 0; i < cellN; i++)
    if(!mallocCont(tab, rowN, cellN, 1))
      return false;
  return true;
}

//Allocate space for contents of a new cell
bool mallocCont(tab_t *tab, int rowN, int cellN, int cellcN){
  CONT(rowN - 1, cellN - 1) = malloc(cellcN);
  if(!CONT(rowN - 1, cellN - 1))
    return false;
  SCONT(rowN - 1, cellN - 1, cellcN - 1) = '\0';
  SCELL(rowN - 1, cellN - 1).len = cellcN;
  return true;
}

//Reallocate space for rows in the table
bool reallocRows(tab_t *tab, int nextSize){
  row_t *p = realloc(ROW, (nextSize)*sizeof(row_t));
  if(!p) 
    return false;
  ROW = p;
  tab->len = nextSize; 
  return true;
}

//Reallocate space for columns in a row
bool reallocCells(tab_t *tab, int rowN, int newSize){
  cell_t *p = realloc(CELL(rowN - 1), newSize*sizeof(cell_t));
  if(!p) 
    return false;
  CELL(rowN - 1) = p;
  SROW(rowN - 1).len = newSize;
  return true;
}

//Reallocate space for contents of a cell
bool reallocCont(tab_t *tab, int rowN, int cellN, int by){
  int newCellcN = SCELL(rowN - 1, cellN - 1).len + by;
  if(newCellcN < 1)
    newCellcN = 1;
  char *p = realloc(CONT(rowN - 1, cellN - 1), newCellcN);
  if(!p) 
    return false;
  CONT(rowN - 1, cellN - 1) = p;
  SCELL(rowN - 1, cellN - 1).len = newCellcN;
  SCONT(rowN - 1, cellN - 1, newCellcN - 1) = '\0';
  return true;
}

bool allocTempVars(char *tempVar[10]){
  for(int i = 0; i < 10; i++){
    tempVar[i] = malloc(1);
    if(!tempVar[i]){
      for(int j = i; j >= 0; j--)
        free(tempVar[j]);
      return false;
    }
    tempVar[i][0] = '\0';
  }
  return true;
}

/*
** Miscellaneous functions
*/

//Print error message and return the error code
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

//Free the allocated memory and call the err function
int freeAndErr(tab_t *tab, int errCode, char *tempVar[10]){
  freeTab(tab, tempVar);
  return errFn(errCode);
}

//Checks if a certain character is a delimiter
bool isDel(char *c, char *del){
  for(int i = 0; del[i]; i++)
    if(*c == del[i])
      return true;
  return false;
}

//Checks, if a character is escaped or not
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

/*
** General working with the table
*/

//Allocates space for the whole table and writes the table into the memory
bool getTab(char *argv[], tab_t *tab, char *del){
  ROW = malloc(sizeof(row_t));
  if(!ROW) 
    return false;
  tab->len = 1;
  if(!mallocCell(tab, 1, 1))
    return false;
  FILE *tabFile = fopen(argv[!strcmp(argv[1], "-d") ? 4 : 2], "r");
  //If there are delimiters entered, filename will be as the 4th argument
  int rowN = 1, cellN = 1, cellcN = 1;
  char tempC = '\0';
  bool quoted = false/*, makeQuoted = false*/;
  for(int i = 0; tempC != EOF; i++){
    bool skip = false;
    tempC = fgetc(tabFile);
    if(tempC == '"') quoted = quoted ? false : true;
    if(!quoted){
      //if(tempC == backslash)
        //makeQuoted = true;
      if(tempC == '\n'){
        rowN++;
        cellN = 1;
        cellcN = 1;
        skip = true;
        if(!editTableRows(tab, -1, 1))
          return false;
      }else if(isDel(&tempC, del)){
        cellN++;
        cellcN = 1;
        skip = true;
        if(!editTableCols(tab, rowN, cellN, 1))
          return false;
      }
    }
    if(!skip){
      cellcN++;
      SCONT(rowN - 1, cellN - 1, cellcN - 2) = tempC;
      if(!reallocCont(tab, rowN, cellN, 1))
        return false;
    }
  }
  fclose(tabFile);
  return true;
}

//Prints the characters of the table one by one
void printTab(tab_t *tab, char *del){
  for(int i = 0; i+1 < tab->len; i++){
    for(int j = 0; j < SROW(i).len; j++){
      for(int k = 0; k < SCELL(i, j).len; k++)
        printf("%c", SCONT(i, j, k));
      if(j+1 != SROW(i).len)
        printf("\t%c\t", del[0]);
        //printf("%c", del[0]);
    }
    printf("\n");
  }
}

//After running this function, all rows will have the same amounts of columns
bool addCols(tab_t *tab){
  int maxCols = getMaxCols(tab);
  for(int i = 1; i < tab->len; i++)
    if(SROW(i - 1).len != maxCols)
      if(!editTableCols(tab, i, SROW(i - 1).len, maxCols - SROW(i - 1).len))
        return false;
  return true;
}

//Checks the table to get a number of how many columns there are in a row with
//the highest number of columns
int getMaxCols(tab_t *tab){
  int max = 0;
  for(int i = 1; i < tab->len; i++)
    if(SROW(i - 1).len > max)
      max = SROW(i - 1).len;
  return max;
}

/*
** Parsing the entered arguments
*/

//Check, if the entered arguments are valid
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

//Get delimiters. If there are none entered, space is a delimiter
char *getDel(char *argv[]){
  if(!strcmp(argv[1], "-d"))
    return argv[2];
  return " ";
}

//Get info about where the commands are in argv
int getCmdPlc(char *argv[]){
  if(!strcmp(argv[1], "-d"))
    return 3;
  return 1;
}

//Returns a pointer to the 'cmdNum'-th command in argv
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

/*
** Executing the entered commands
*/

//Reads commands and executes them
int execCmds(char *argv[], int cmdPlc, tab_t *tab, char *tempVar[10]){
  char *cmd;
  int cmdLen;
  int cmdNum = 0;
  cellSel_t sel = {1, 1, -1, -1};
  cellSel_t tempSel = {-1, -1, -1, -1};
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
      //freeTempVars(tempVar);
      return isCellSelRet; //Err
    }
    else if(isCellSelRet > 0)
      continue; //The command is a selection command
    //Exectution:
    tempVarFn(cmd, cmdLen, tempVar, sel, tab);
    tabEdit(cmd, cmdLen, tab, sel);
    contEdit(cmd, cmdLen, tab, sel);
  }
  //freeTempVars(tempVar);
  return 0;
}

//Checks, if the actual commdn is a selection command. If so, it parses it and
//writes it into the "sel" structure, which stands for "selection command"
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

//If there is a selection argument, this function parses it and returns it
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

//Function responsible for working with temporary variables that the user can
//work with
bool tempVarFn(char *cmd, int cmdLen, char *tempVar[10], cellSel_t sel, tab_t *tab){
  int var;
  if(cmdLen < 6) 
    return 0;
  if((cmd[6] == '\0' || cmd[6] == ';') && cmd[5] >= '0' && cmd[5] <= '9'){
    var = cmd[5] - '0';
    if(sel.r2 > 0 || sel.c2 > 0 || sel.r1 < 1 || sel.c1 < 1) 
      return 0;
    if(strstr(cmd, "def _") == cmd){
      char *p = realloc(tempVar[var], SCELL(sel.r1 - 1, sel.c1 - 1).len + 1);
      if(!p) 
        return 0;
      tempVar[var] = p;
      strcpy(tempVar[var], CONT(sel.r1 - 1, sel.c1 - 1));
    }else if(strstr(cmd, "use _") == cmd){
      int tempVarLen = strlen(tempVar[var]) + 1;
      if(tempVarLen != SCELL(sel.r1 - 1, sel.c1 - 1).len)
        reallocCont(tab, sel.r1, sel.c1, tempVarLen - SCELL(sel.r1 - 1, sel.c1 - 1).len);
      strcpy(CONT(sel.r1 - 1, sel.c1 - 1), tempVar[var]);
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

//Executing the table editing commands - arow, irow, drow, icol, acol, dcol
int tabEdit(char *cmd, int cmdLen, tab_t *tab, cellSel_t sel){
  if(sel.r2 != -1 || sel.c2 != -1)
    return 0;
  if(cmdLen == 4 && (cmd[4] == '\0' || cmd[4] == ';')){
    if(strstr(cmd, "irow") == cmd){
      if(!editTableRows(tab, sel.r1 - 1, 1))
        return -1;
    }else if(strstr(cmd, "arow") == cmd){
      if(!editTableRows(tab, sel.r1, 1))
        return -1;
    }else if(strstr(cmd, "drow") == cmd){
      if(!editTableRows(tab, sel.r1, -1))
        return -1;
    }else if(strstr(cmd, "icol") == cmd){
      if(!editTableCols(tab, 0, sel.c1, 1))
        return -1;
    }else if(strstr(cmd, "acol") == cmd){
      if(!editTableCols(tab, 0, sel.c1 + 1, 1))
        return -1;
    }else if(strstr(cmd, "dcol") == cmd)
      if(!editTableCols(tab, 0, sel.c1, -1))
        return -1;
    addCols(tab);
  }
  return 0;
}

//Executing the content editing commands
int contEdit(char *cmd, int cmdLen, tab_t *tab, cellSel_t sel){
  (void) cmd; (void) cmdLen; (void) tab; (void) sel;

  return 0;
}

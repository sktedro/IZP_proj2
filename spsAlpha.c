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

void printTab(tab_t *tab, char *del, char *fileName);
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
int getTab(char *argv[], tab_t *tab, char *del);
int getMaxCols(tab_t *tab);
bool addCols(tab_t *tab);
bool addSelectedCols(tab_t *tab, cellSel_t *sel);
int getCellSelArg(char *cmd, int argNum);
int isCellSel(char *cmd, tab_t *tab, cellSel_t *sel, cellSel_t *tempSel);
bool isEscaped(char *str, int charPlc);
char *getCmd(char *argv[], int cmdPlc, int cmdNum);
bool tempVarFn(char *cmd, int cmdLen, char *tempVar[10], cellSel_t sel, tab_t *tab);
bool allocTempVars(char *tempVar[10]);
int tabEdit(char *cmd, int cmdLen, tab_t *tab, cellSel_t sel);
int contEdit(char *cmd, tab_t *tab, cellSel_t *sel);
int execCmds(char *argv[], int cmdPlc, tab_t *tab, char *tempVar[10]);
int freeAndErr(tab_t *tab, int errCode, char *tempVar[10]);
void parseSel(tab_t *tab, cellSel_t *sel, cellSel_t *actSel);
void getMinOrMax(tab_t *tab, cellSel_t *sel, bool min);
bool findStr(char *cmd, tab_t *tab, cellSel_t *sel);
int digitsCt(int n);
bool setFn(char *cmd, tab_t *tab, cellSel_t sel);
bool clearFn(tab_t *tab, cellSel_t sel);
void swapFn(char *cmd, tab_t *tab, cellSel_t sel);
bool lenFn(char *cmd, tab_t *tab, cellSel_t sel);
bool sumAvgCt(char *cmd, tab_t *tab, cellSel_t *sel);

bool removeEmptyCols(tab_t *tab);
bool parseTheTab(tab_t *tab);
bool prepTabForPrint(tab_t *tab, char *del);

void checkTheTab(tab_t *tab){
  for(int i = 0; i < tab->len; i++){
    for(int j = 0; j < SROW(i).len; j++){
      if(SCONT(i, j, SCELL(i, j).len - 1) != '\0')
        printf("<no null byte at %d, %d>\n", i+1, j+1);
      if(!CONT(i, j))
        printf("<Cell pointer is NULL %d, %d>\n", i+1, j+1);
    }
  }
}

bool parseTheTab(tab_t *tab){
  for(int i = 0; i < tab->len - 1; i++){
    for(int j = 0; j < SROW(i).len; j++){
      for(int k = 0; k < SCELL(i, j).len; k++){
        if(SCONT(i, j, k) == backslash){
          for(int l = k; l < SCELL(i, j).len - 1; l++)
            SCONT(i, j, l) = SCONT(i, j, l + 1);
          if(!reallocCont(tab, i + 1, j + 1, -1))
            return false;
        }else if(SCONT(i, j, k) == '"'){
          for(int l = k; l < SCELL(i, j).len - 1; l++)
            SCONT(i, j, l) = SCONT(i, j, l + 1);
          if(!reallocCont(tab, i + 1, j + 1, -1))
            return false;
          k--;
        }
      }
    }
  }
  return true;
}

bool prepTabForPrint(tab_t *tab, char *del){
  for(int i = 0; i < tab->len - 1; i++){
    for(int j = 0; j < SROW(i).len; j++){
      bool makeQuoted = false;
      for(int k = 0; k < SCELL(i, j).len; k++){
        for(int l = 0; del[l]; l++)
          if(SCONT(i, j, k) == del[l])
            makeQuoted = true;
        if(SCONT(i, j, k) == '"')
          makeQuoted = true;
        if(SCONT(i, j, k) == backslash || (SCONT(i, j, k) == '"' )){
          if(!reallocCont(tab, i + 1, j + 1, 1))
            return false;
          for(int l = SCELL(i, j).len - 1; l > k; l--)
            SCONT(i, j, l) = SCONT(i, j, l - 1);
          SCONT(i, j, k) = backslash;
          k++;
        }
      }
      if(makeQuoted){
        if(!reallocCont(tab, i + 1, j + 1, 2))
          return false;
        for(int l = SCELL(i, j).len - 1; l > 0; l--)
          SCONT(i, j, l) = SCONT(i, j, l - 1);
        SCONT(i, j, 0) = SCONT(i, j, SCELL(i, j).len - 2) = '"';
        SCONT(i, j, SCELL(i, j).len - 1) = '\0';
        makeQuoted = false;
      }
    }
  }
  return true;
}


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
  errCode = getTab(argv, &tab, del);
  if(errCode) 
    return freeAndErr(&tab, errCode, tempVar);
  printf("Get the tab\n");
  checkTheTab(&tab);

  if(!addCols(&tab)) 
    return freeAndErr(&tab, -4, tempVar);
  printf("Add cols\n");
  checkTheTab(&tab);
  
  if(!parseTheTab(&tab))
    return freeAndErr(&tab, -4, tempVar);
  printf("Parse the tab\n");
  checkTheTab(&tab);

  errCode = execCmds(argv, cmdPlc, &tab, tempVar);
  if(errCode) 
    return freeAndErr(&tab, errCode, tempVar);
  printf("Exec cmds\n");
  checkTheTab(&tab);

  if(!addCols(&tab)) 
    return freeAndErr(&tab, -4, tempVar);
  printf("Add columns\n");
  checkTheTab(&tab);
/*
  if(!removeEmptyCols(&tab))
    return freeAndErr(&tab, -4, tempVar);
    */
  printf("Remove empty columns\n");
  checkTheTab(&tab);

  if(!prepTabForPrint(&tab, del))
    return freeAndErr(&tab, -4, tempVar);
  printf("Prep tab for print\n");
  checkTheTab(&tab);

  printTab(&tab, del, argv[cmdPlc + 1]);
  freeTab(&tab, tempVar);
  return 0;
}

/*
** Free and allocation functions
*/

//Frees the memory allocated for the table and temporary variables
void freeTab(tab_t *tab, char *tempVar[10]){
  for(int i = tab->len - 1; i >= 0; i--)
    freeRow(tab, i);
  free(ROW);
  if(tempVar != NULL)
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
  //printf("%d -> ", from);
  if(from < 0)
    from = tab->len;
  //printf("%d\n", from);
  if(by > 0){ //Inserting or adding rows
    if(!reallocRows(tab, (tab->len + by)))
      return false;
    for(int i = tab->len - 1; i > from; i--)
      SROW(i) = SROW(i - 1);
    for(int i = 0; i < by; i++)
      if(!mallocCell(tab, from + i + 1, 1))
        return false;
  }else if(by < 0){ //Deleting rows
    if(tab->len - (-by) + 1 < 1){
      //freeRow(tab, 1);
      freeTab(tab, NULL);
      ROW = malloc(sizeof(row_t));
      tab->len = 1;
      if(!mallocCell(tab, 1, 1))
        return -4;
      return true;
    }
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
    //printf("newCellN = %d + %d, because rowN = %d and i = %d\n", SROW(i - 1).len, by, rowN, i);
    if(SCONT(i - 1, 0, 0) == EOF)
      break;
    int newCellN = SROW(i - 1).len + by;
    if(newCellN < 1){
      if(!editTableRows(tab, i--, -1))
        return false;
    }else if(by > 0){
      if(!reallocCells(tab, i, newCellN))
        return false;
      if(from < SROW(i - 1).len - by + 1)
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
bool reallocRows(tab_t *tab, int newSize){
  row_t *p = realloc(ROW, newSize*sizeof(row_t));
  if(!p) 
    return false;
  ROW = p;
  tab->len = newSize; 
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
      break;
    case -7:
      perr("You cannot escape the EOL character.");
      break;
    case -8:
      perr("You are trying to use the temporary selection variable but it has not been set yet.");
      break;
    case -9:
      perr("Sorry, you cannot delete the whole table. You might as well type \"rm\", you know.");
      break;
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
  if(!str)
    return false;
  int i = 1;
  for(; (charPlc - i) > 0 && str[charPlc - i - 1] == backslash; i++){}
  return ((i - 1) % 2 != 0) ? true : false;
}

//Recursively counts the digits of a number
int digitsCt(int n){
  if(!n)
    return 0;
  return 1 + digitsCt(n / 10);
}

/*
** General working with the table
*/

//Allocates space for the whole table and writes the table into the memory
int getTab(char *argv[], tab_t *tab, char *del){
  ROW = malloc(sizeof(row_t));
  if(!ROW) 
    return -4;
  tab->len = 1;
  if(!mallocCell(tab, 1, 1))
    return -4;
  FILE *tabFile = fopen(argv[getCmdPlc(argv) + 1], "r");
  //If there are delimiters entered, filename will be as the 4th argument
  int rowN = 1, cellN = 1, cellcN = 1;
  char tempC = '\0';
  bool quoted = false;
  for(int i = 0; tempC != EOF; i++){
    tempC = fgetc(tabFile);
    if(tempC == '\n'){
      if(quoted)
        return -7;
      rowN++;
      cellN = cellcN = 1;
      if(!editTableRows(tab, -1, 1))
        return -4;
    }else if(!quoted && isDel(&tempC, del) 
        && !isEscaped(CONT(rowN - 1, cellN - 1), cellcN)){
      cellN++;
      cellcN = 1;
      if(!editTableCols(tab, rowN, cellN, 1))
        return -4;
    }else{
      if(tempC == '"' && !isEscaped(CONT(rowN - 1, cellN - 1), cellcN))
        quoted = !quoted;
      cellcN++;
      SCONT(rowN - 1, cellN - 1, cellcN - 2) = tempC;
      if(!reallocCont(tab, rowN, cellN, 1))
        return -4;
    }
  }
  fclose(tabFile);
  return 0;
}

//Repeatedly searches and removes last columns that have all the cells empty
bool removeEmptyCols(tab_t *tab){
  bool done = false;
  bool isEmpty = true;
  for(int i = SROW(0).len; !done && SROW(0).len > 1; i--){
    for(int j = 1; j < tab->len; j++){
      if(SCELL(j - 1, i - 1).len != 1){
        isEmpty = false;
        return true;
      }
    }
    if(isEmpty)
      if(!editTableCols(tab, 0, i, -1))
        return false;
  }
  return true;
}

//Prints the characters of the table one by one
void printTab(tab_t *tab, char *del, char *fileName){
  (void) fileName;
  for(int i = 0; i < tab->len - 1; i++){
    for(int j = 0; j < SROW(i).len; j++){
      for(int k = 0; k < SCELL(i, j).len; k++){
        printf("%c", SCONT(i, j, k));
      }
      if(j+1 != SROW(i).len){
        printf("\t%c\t", del[0]);
        //printf("%c", del[0]);
      }
    }
    printf("\n");
  }
  /*
  FILE *f = fopen(fileName, "w");
  for(int i = 0; i < tab->len - 1; i++){
    for(int j = 0; j < SROW(i).len; j++){
      for(int k = 0; k < SCELL(i, j).len; k++){
        fputc(SCONT(i, j, k), f);
      }
      if(j+1 != SROW(i).len){
        fputc(del[0], f);
      }
    }
    fputc('\n', f);
  }
  fclose(f);
  */
}

//After running this function, all rows will have the same amounts of columns
bool addCols(tab_t *tab){
  int maxCols = getMaxCols(tab);
  for(int i = 1; i < tab->len; i++)
    if(SROW(i - 1).len != maxCols)
      if(!editTableCols(tab, i, SROW(i - 1).len + 1, maxCols - SROW(i - 1).len))
        //TODO
        return false;
  return true;
}

//If a selection lands on a cell that isn't a part of the table, this functions
//adds necessary rows and columns
bool addSelectedCols(tab_t *tab, cellSel_t *sel){
  cellSel_t actSel;
  parseSel(tab, sel, &actSel);
  int maxr = (actSel.r1 > actSel.r2) ? actSel.r1 : actSel.r2;
  int maxc = (actSel.c1 > actSel.c2) ? actSel.c1 : actSel.c2;
  /*
  int maxr = (sel->r1 > sel->r2) ? sel->r1 : sel->r2;
  int maxc = (sel->c1 > sel->c2) ? sel->c1 : sel->c2;
  */
  if(tab->len - 1 < maxr)
    if(!editTableRows(tab, tab->len - 1, maxr - tab->len + 1))
      return false;
  addCols(tab);
  if(SROW(0).len < maxc)
    if(!editTableCols(tab, 0, SROW(0).len + 1, maxc - SROW(0).len))
      return false;
  return true;
}

//Returns the biggest amount of columns in a row
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

//Get delimiters. If there are none entered, space is the delimiter
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
    bool quoted = false;
    bool isProperDelim = false;
    if(argv[cmdPlc][i] == '"')
      quoted = !quoted;
    else if(argv[cmdPlc][i] == ';' && !quoted && !isEscaped(argv[cmdPlc], i))
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

//Parses the selection arguments so they can be worked with (for example, if
//"sel->r1" == 0, "r1" will be set to 1 and r2 will be set to number of rows
void parseSel(tab_t *tab, cellSel_t *sel, cellSel_t *actSel){
  actSel->r1 = sel->r1;
  actSel->c1 = sel->c1;
  actSel->r2 = (sel->r2 == -1) ? sel->r1 : sel->r2;
  actSel->c2 = (sel->c2 == -1) ? sel->c1 : sel->c2;
  if(actSel->r1 == 0 || sel->r2 == -2)
    actSel->r2 = tab->len - 1;
  if(actSel->c1 == 0 || sel->c2 == -2)
    actSel->c2 = SROW(actSel->r1 - 1).len;
  if(actSel->r1 == 0)
    actSel->r1 = 1;
  if(actSel->c1 == 0)
    actSel->c1 = 1;
  printf("%d, %d, %d, %d\n", actSel->r1, actSel->c1, actSel->r2, actSel->c2);
}

/*
** Executing the entered commands
*/

//Reads commands and executes them
int execCmds(char *argv[], int cmdPlc, tab_t *tab, char *tempVar[10]){
  char *cmd;
  int cmdLen, cmdNum = 0;
  int errCode;
  cellSel_t sel = {1, 1, -1, -1};
  cellSel_t tempSel = {-1, -1, -1, -1};
  while(1){
    //Getting next command:
    addCols(tab);
    cmdNum++;
    cmd = getCmd(argv, cmdPlc, cmdNum);
    cmdLen = getCmd(argv, cmdPlc, cmdNum + 1) - cmd;
    if(!cmd)
      break;
    if(cmd != argv[cmdPlc]){
      cmd++; //If this is not the first cmd, skip the delim
      cmdLen--; //And dont count the delim as part of the command
    }
    //Selection:
    int isCellSelRet = isCellSel(cmd, tab, &sel, &tempSel);
    if(isCellSelRet < 0){
      //freeTempVars(tempVar);
      return isCellSelRet; //Err
    }
    else if(isCellSelRet > 0)
      continue; //The command is a selection command
    //Exectution:
    tempVarFn(cmd, cmdLen, tempVar, sel, tab);
    errCode = tabEdit(cmd, cmdLen, tab, sel);
    if(errCode)
      return errCode;
    errCode = contEdit(cmd, tab, &sel);
    if(errCode)
      return errCode;
  }
  //freeTempVars(tempVar);
  return 0;
}

//Checks, if the actual command is a selection command. If so, it parses it and
//writes it into the "sel" structure, which stands for "selection command"
int isCellSel(char *cmd, tab_t *tab, cellSel_t *sel, cellSel_t *tempSel){
  if(cmd[0] != '[')
    return 0; //Not a selection command
  if(cmd == strstr(cmd, "[set]")){// == cmd && (cmd[5] == ';' || cmd[5] == '\0')){
    memcpy(tempSel, sel, sizeof(cellSel_t));
  }else if(cmd == strstr(cmd, "[_]")){
    if(tempSel->r1 < 1 || tempSel->c1 < 1)
      return -8;
    memcpy(sel, tempSel, sizeof(cellSel_t));
  }else if(cmd == strstr(cmd, "[min]") || cmd == strstr(cmd, "[max]")){
    getMinOrMax(tab, sel, cmd == strstr(cmd, "[min]"));
  }else if(cmd == strstr(cmd, "[find ")){
    if(!findStr(cmd, tab, sel))
      return -4;
  }else{
    int args = 1;
    for(int i = 0; cmd[i] != ']' && cmd[i]; i++)
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
    }else
      sel->r2 = sel->c2 = -1;
    //printf("\n");
    if(!addSelectedCols(tab, sel))
      return -4;
  }
  return 1;
}

//Function responsible for working with temporary variables that the user can
//work with
bool tempVarFn(char *cmd, int cmdLen, char *tempVar[10], cellSel_t sel, tab_t *tab){
  int var;
  if(cmdLen < 6) 
    return 0;
  if((cmd[6] == '\0' || cmd[6] == ';') && cmd[5] >= '0' && cmd[5] <= '9'){
    var = cmd[5] - '0';
    cellSel_t actSel;
    parseSel(tab, &sel, &actSel);
    if(strstr(cmd, "def _") == cmd){
      char *p = realloc(tempVar[var], SCELL(actSel.r1 - 1, actSel.c1 - 1).len + 1);
      if(!p) 
        return 0;
      tempVar[var] = p;
      strcpy(tempVar[var], CONT(actSel.r1 - 1, actSel.c1 - 1));
    }else if(strstr(cmd, "use _") == cmd){
      for(int i = actSel.r1; i <= actSel.r2; i++){
        for(int j = actSel.c1; j <= actSel.c2; j++){
          int len = strlen(tempVar[var]) + 1;
          if(len != SCELL(i - 1, j - 1).len)
            reallocCont(tab, i, j, len - SCELL(i - 1, j - 1).len);
          strcpy(CONT(i - 1, j - 1), tempVar[var]);
        }
      }
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
  cellSel_t actSel;
  parseSel(tab, &sel, &actSel);
  if(cmdLen == 4 && (cmd[4] == '\0' || cmd[4] == ';')){
    for(int j = 0, i = actSel.r1; i <= actSel.r2; i++){
      if(strstr(cmd, "irow") == cmd){
        if(!editTableRows(tab, i - 1 + j++, 1))
          return -4;
      }else if(strstr(cmd, "arow") == cmd){
        if(!editTableRows(tab, i + j++, 1))
          return -4;
      }else if(strstr(cmd, "drow") == cmd)
        if(!editTableRows(tab, i + j--, -1))
          return -4;
    }
    for(int j = 0, i = actSel.c1; i <= actSel.c2; i++){
      if(strstr(cmd, "icol") == cmd){
        if(!editTableCols(tab, 0, i + j++, 1))
          return -4;
      }else if(strstr(cmd, "acol") == cmd){
        if(!editTableCols(tab, 0, i + 1 + j++, 1))
          return -4;
      }else if(strstr(cmd, "dcol") == cmd){
        if(SROW(0).len == 0)
          return 0;
        if(!editTableCols(tab, 0, i + j--, -1))
          return -4;
      }
    }
  }
  return 0;
}

//Executing the content editing commands
int contEdit(char *cmd, tab_t *tab, cellSel_t *sel){
  cellSel_t actSel;
  parseSel(tab, sel, &actSel);
  if(cmd == strstr(cmd, "set ")){
    if(!setFn(cmd, tab, actSel))
      return -4;
  }else if(cmd == strstr(cmd, "clear") && (cmd[5] == '\0' || cmd[5] == ';')){
    if(!clearFn(tab, actSel))
      return -4;
  }else if(cmd == strstr(cmd, "swap [")){
    swapFn(cmd, tab, *sel);
  }else if(cmd == strstr(cmd, "len ")){
    if(!lenFn(cmd, tab, *sel))
      return -4;
  }else
    if(!sumAvgCt(cmd, tab, sel))
      return -4;
  return 0;
}

/*
** Additional functions for executing entered commands
*/

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
  }else if(!strcmp(selArg, "-")){
    free(selArg);
    return -2;
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

//bool min is false for "max", true for "min"
void getMinOrMax(tab_t *tab, cellSel_t *sel, bool min){
  cellSel_t actSel;
  parseSel(tab, sel, &actSel);
  //printf("%d, %d, %d, %d\n", actSel.r1, actSel.c1, actSel.r2, actSel.c2);
  int nextr1, nextc1;
  double val;
  bool init = false;
  for(int i = actSel.r1; i <= actSel.r2; i++){
    for(int j = actSel.c1; j <= actSel.c2; j++){
      char *todptr = NULL;
      double temp = strtod(CONT(i - 1, j - 1), &todptr);
      if(!todptr[0] && (!init || (min && temp < val) || (!min && temp > val))){
        init = true;
        val = temp;
        nextr1 = i;
        nextc1 = j;
      }
    }
  }
  if(init){
    sel->r1 = nextr1;
    sel->c1 = nextc1;
    sel->r2 = sel->c2 = -1;
  }
}

//Searches for a string in selected cells
bool findStr(char *cmd, tab_t *tab, cellSel_t *sel){
  //printf("%d, %d : %d, %d\n", sel->r1, sel->r2, sel->c1, sel->c2);
  bool isQuoted = false;
  int i = strlen("[find "), j = 0;
  char *str = malloc(1);
  if(!str)
    return false;
  while(cmd[i]){
    if(cmd[i] == '"')
      isQuoted = !isQuoted;
    else if((cmd[i] == ']' || cmd[i] == ';') && !isQuoted && !isEscaped(cmd, i + 1))
      break;
    else{
      char *p = realloc(str, ++j);
      if(!p){
        free(str);
        return false;
      }
      str = p;
      str[j - 1] = cmd[i];
    }
    i++;
  }
  str[j] = '\0';
  bool done = false;
  cellSel_t actSel;
  parseSel(tab, sel, &actSel);
  for(int i = actSel.r1; i <= actSel.r2 && !done; i++){
    for(int j = actSel.c1; j <= actSel.c2 && !done; j++){
      if(strstr(CONT(i - 1, j - 1), str)){
        sel->r1 = i;
        sel->c1 = j;
        sel->r2 = sel->c2 = -1;
        done = true;
      }
    }
  }
  free(str);
  return true;
}

//Writes string into a selected cell
bool setFn(char *cmd, tab_t *tab, cellSel_t sel){
  for(int i = sel.r1; i <= sel.r2; i++){
    for(int j = sel.c1; j <= sel.c2; j++){
      bool isQuoted = false;
      int k = strlen("set "), l = 0;
      if(!reallocCont(tab, i, j, 1))
        return false;
      while(cmd[k]){
        if(cmd[k] == '"' && !isEscaped(cmd, k + 1))
          isQuoted = !isQuoted;
        else if((cmd[k] == ']' || cmd[k] == ';') && !isQuoted && !isEscaped(cmd, k + 1))
          break;
        else{
          if(!reallocCont(tab, i, j, ++l + 1))
            return false;
          SCONT(i - 1, j - 1, l - 1) = cmd[k];
        }
        k++;
      }
      SCONT(i - 1, j - 1, l) = '\0';
      SCELL(i - 1, j - 1).len = l + 1;
    }
  }
  return true;
}

//Removes the content of a selected cell
bool clearFn(tab_t *tab, cellSel_t sel){ 
  for(int i = sel.r1; i <= sel.r2; i++){
    for(int j = sel.c1; j <= sel.c2; j++){
      int prevLen = SCELL(i - 1, j - 1).len;
      if(prevLen != 1)
        if(!reallocCont(tab, i, j, -(prevLen) + 1))
          return -4;
      SCONT(i - 1, j - 1, 0) = '\0';
    }
  }
  return 0;
}

//Swaps two cells
void swapFn(char *cmd, tab_t *tab, cellSel_t sel){
  cellSel_t actSel, swapSel;
  if(isCellSel(cmd + strlen("swap "), tab, &swapSel, NULL)){
    parseSel(tab, &sel, &actSel);
    for(int i = actSel.r1; i <= actSel.r2; i++){
      for(int j = actSel.c1; j <= actSel.c2; j++){
        //printf("Swapping cell [%d,%d] with [%d,%d]\n", i, j, swapSel.r1, swapSel.c1);
        cell_t temp = SCELL(i - 1, j - 1);
        SCELL(i - 1, j - 1) = SCELL(swapSel.r1 - 1, swapSel.c1 - 1);
        SCELL(swapSel.r1 - 1, swapSel.c1 - 1) = temp;
      }
    }
  }
  /*
  if(isCellSel(cmd + strlen("swap "), tab, &swapSel, NULL)){
    parseSel(tab, &sel, &actSel);
    cell_t temp = SCELL(actSel.r1 - 1, actSel.c1 - 1);
    SCELL(actSel.r1 - 1, actSel.c1 - 1) = SCELL(swapSel.r1 - 1, swapSel.c1 - 1);
    SCELL(swapSel.r1 - 1, swapSel.c1 - 1) = temp;
  }
  */
}

//Gets the length of a string in a cell
bool lenFn(char *cmd, tab_t *tab, cellSel_t sel){
  cellSel_t lenSel;
  if(isCellSel(cmd + strlen("len "), tab, &lenSel, NULL)){
    int len = strlen(CONT(sel.r1 - 1, sel.c1 - 1));
    int digits = digitsCt(len);
    if(len != digits + 1)
      if(!reallocCont(tab, lenSel.r1, lenSel.c1, -(len + digits + 1)))
        return false;
    sprintf(CONT(lenSel.r1 - 1, lenSel.c1 - 1), "%d", len);
    SCELL(lenSel.r1 - 1, lenSel.c1 - 1).len = digitsCt(digits) + 1;
  }
  return true;
}

//Function for getting sum of, average of or count of all the selected cells,
//which only contain numbers
bool sumAvgCt(char *cmd, tab_t *tab, cellSel_t *sel){
  int cmdParsed = 0;
  if(cmd == strstr(cmd, "sum ["))
    cmdParsed = 1;
  else if(cmd == strstr(cmd, "avg ["))
    cmdParsed = 2;
  else if(cmd == strstr(cmd, "count ["))
    cmdParsed = 3;
  else
    return true;
  cellSel_t actSel;
  parseSel(tab, sel, &actSel);
  double val = 0;
  double count = 0;
  for(int i = actSel.r1; i <= actSel.r2; i++){
    for(int j = actSel.c1; j <= actSel.c2; j++){
      char *todptr = NULL;
      double temp = strtod(CONT(i - 1, j - 1), &todptr);
      if(!todptr[0] && SCONT(i - 1, j - 1, 0)){
        if(cmdParsed != 3)
          val += temp;
        if(cmdParsed != 1)
          count++;
      }
    }
  }
  cellSel_t cmdSel;
  if(cmdParsed == 2){
    val /= count;
    if(!isCellSel(cmd + strlen("avg "), tab, &cmdSel, NULL))
      return false;
  }else if(cmdParsed == 3){
    val = count;
    if(!isCellSel(cmd + strlen("count "), tab, &cmdSel, NULL))
      return false;
  }else if(cmdParsed == 1)
    if(!isCellSel(cmd + strlen("sum "), tab, &cmdSel, NULL))
      return false;
  sprintf(CONT(cmdSel.r1 - 1, cmdSel.c1 - 1), "%g", val);
  SCELL(cmdSel.r1 - 1, cmdSel.c1 - 1).len = 
    strlen(CONT(cmdSel.r1 - 1, cmdSel.c1 - 1)) + 1;
  /*
  for(int i = r1; i <= r2; i++){
    for(int j = c1; j <= c2; j++){
      char *todptr = NULL;
      double temp = strtod(CONT(i - 1, j - 1), &todptr);
      if(!todptr[0] && SCONT(i - 1, j - 1, 0)){
        if(cmdParsed != 3)
          val += temp;
        if(cmdParsed != 1)
          count++;
      }
    }
  }
  cellSel_t cmdSel;
  if(cmdParsed == 2){
    val /= count;
    if(!isCellSel(cmd + strlen("avg "), tab, &cmdSel, NULL))
      return false;
  }else if(cmdParsed == 3){
    val = count;
    if(!isCellSel(cmd + strlen("count "), tab, &cmdSel, NULL))
      return false;
  }else if(cmdParsed == 1)
    if(!isCellSel(cmd + strlen("sum "), tab, &cmdSel, NULL))
      return false;
  sprintf(CONT(cmdSel.r1 - 1, cmdSel.c1 - 1), "%g", val);
  SCELL(cmdSel.r1 - 1, cmdSel.c1 - 1).len = 
    strlen(CONT(cmdSel.r1 - 1, cmdSel.c1 - 1)) + 1;
    */
  return true;
}

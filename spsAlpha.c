#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define backslash         92
//SROW, SCELL, SCONT is a specific row/cell/cont
#define ROW               tab->row
#define SROW(X)           tab->row[X]
#define CELL(X)           tab->row[X].cell
#define SCELL(X, Y)       tab->row[X].cell[Y]
#define CONT(X, Y)        tab->row[X].cell[Y].cont
#define SCONT(X, Y, Z)    tab->row[X].cell[Y].cont[Z]
#define perr(X)           fprintf(stderr, X)

#define print 0
#define deb 1

enum cmdEnum {sel = 11, sqSel, minSel, maxSel, findSel, tmpSel, 
              irow = 21, arow, drow, icol, acol, dcol,
              set = 31, clear, swap, sum, avg, count, len,
              defTmp = 41, useTmp, incTmp, setTmp};

typedef struct{
  int r1;
  int c1;
  int r2;
  int c2;
} cellSel_t;

typedef struct{
  cellSel_t tmpSel[10];
  int len;
} tmpSel_t;

typedef struct cmds{
  int cmdType;
  cellSel_t selArgs;
  char *str;
  int tmpSelN;
  struct cmds *next;
} cmds_t;

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

//TODO remove this
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

/*
** Free and allocation functions
*/

//Frees the memory allocated for the table and tmporary variables
void freeCell(tab_t *tab, int rowN, int cellN){
  free(CONT(rowN, cellN));
  SROW(rowN).len--;
}
void freeRow(tab_t *tab, int rowN){
  for(int j = SROW(rowN).len - 1; j >= 0; j--)
    freeCell(tab, rowN, j);
  free(CELL(rowN));
  tab->len--;
}
void freeTab(tab_t *tab, char *tmpVar[10]){
  for(int i = tab->len - 1; i >= 0; i--)
    freeRow(tab, i);
  free(ROW);
  for(int i = 0; i < 10; i++)
    if(tmpVar[i])
      free(tmpVar[i]);
}

//Allocate space for contents of a new cell
bool mlcCont(tab_t *tab, int rowN, int cellN, int charN){
  CONT(rowN - 1, cellN - 1) = malloc(charN);
  if(!CONT(rowN - 1, cellN - 1))
    return false;
  SCELL(rowN - 1, cellN - 1).len = charN;
  SCONT(rowN - 1, cellN - 1, charN - 1) = '\0';
  return true;
}

//Allocate space for cells of a new row
bool mlcCell(tab_t *tab, int rowN, int cellN){
  CELL(rowN - 1) = malloc(cellN * sizeof(cell_t));
  if(!CELL(rowN - 1))
    return false;
  //SROW(rowN - 1).len = 1;
  SROW(rowN - 1).len = cellN;
  for(int i = 0; i < cellN; i++)
    if(!mlcCont(tab, rowN, cellN, 1))
      return false;
  return true;
}

bool rlcChar(char *p, int newSz){
  char *tmp = realloc(p, newSz);
  if(!tmp) 
    return false;
  p = tmp;
  return true;
}

//Reallocate space for contents of a cell
bool rlcCont(tab_t *tab, int rowN, int cellN, int by){
  int newCharN = SCELL(rowN - 1, cellN - 1).len + by;
  if(newCharN < 1)
    newCharN = 1;
  char *p = realloc(CONT(rowN - 1, cellN - 1), newCharN);
  if(!p) 
    return false;
  CONT(rowN - 1, cellN - 1) = p;
  SCELL(rowN - 1, cellN - 1).len = newCharN;
  SCONT(rowN - 1, cellN - 1, newCharN - 1) = '\0';
  return true;
}

//Reallocate space for columns in a row
bool rlcCells(tab_t *tab, int rowN, int newSz){
  cell_t *p = realloc(CELL(rowN - 1), newSz*sizeof(cell_t));
  if(!p) 
    return false;
  CELL(rowN - 1) = p;
  SROW(rowN - 1).len = newSz;
  return true;
}

//Reallocate space for rows in the table
bool rlcRows(tab_t *tab, int newSz){
  row_t *p = realloc(ROW, newSz*sizeof(row_t));
  if(!p) 
    return false;
  ROW = p;
  tab->len = newSz; 
  return true;
}

bool mlcTmpVars(char *tmpVar[10]){
  for(int i = 0; i < 10; i++){
    tmpVar[i] = malloc(1);
    if(!tmpVar[i]){
      for(int j = i; j >= 0; j--)
        free(tmpVar[j]);
      return false;
    }
    tmpVar[i][0] = '\0';
  }
  return true;
}

//Deleting or adding rows
bool editRows(tab_t *tab, int from, int by){
  if(from < 0)
    from = tab->len;
  if(by > 0){ //Inserting or adding rows
    if(!rlcRows(tab, (tab->len + by)))
      return false;
    for(int i = tab->len - 1; i > from; i--)
      SROW(i) = SROW(i - 1);
    for(int i = 0; i < by; i++)
      if(!mlcCell(tab, from + i + 1, 1))
        return false;
  }else if(by < 0){ //Deleting rows
    if(tab->len - (-by) + 1 < 1){
      freeTab(tab, NULL);
      ROW = malloc(sizeof(row_t));
      tab->len = 1;
      if(!mlcCell(tab, 1, 1))
        return -4;
      return true;
    }
    for(int i = 0; i < (-by); i++)
      freeRow(tab, from + i - 1);
    for(int i = from - 1; i < tab->len + by + 1; i++)
      SROW(i) = SROW(i + (-by));
    if(!rlcRows(tab, (tab->len - (-by) + 1)))
      return false;
  }
  return true;
}

//Deleting or adding columns
bool editCols(tab_t *tab, int rowN, int from, int by){
  int howManyRows = 1;
  if(rowN == 0){
    howManyRows = tab->len - 1;
    rowN = 1;
  }
  if(from == 0)
    from = SROW(0).len;
  for(int i = rowN; i < rowN + howManyRows; i++){
    if(SCONT(i - 1, 0, 0) == EOF)
      break;
    int newCellN = SROW(i - 1).len + by;
    if(newCellN < 1){
      if(!editRows(tab, i--, -1))
        return false;
    }else if(by > 0){
      if(!rlcCells(tab, i, newCellN))
        return false;
      if(from < SROW(i - 1).len - by + 1)
        for(int j = (SROW(i - 1).len - 1); j + 1 > from; j--)
          SCELL(i - 1, j) = SCELL(i - 1, j - by);
      for(int j = from; j < from + by; j++)
        if(!mlcCont(tab, i, j, 1))
          return false;
    }
    else if(by < 0){
      for(int j = 0; j < (-by); j++)
        free(CONT(i - 1, from + j - 1));
      for(int j = from; j < (SROW(i - 1).len - (-by) + 1); j++)
        SCELL(i - 1, j - 1) = SCELL(i - 1, j - 1 + (-by));
      if(!rlcCells(tab, i, newCellN))
        return false;

    }
  }
  return true;
}

/*
** Miscellaneous functions
*/

//Print error message and return the error code
int errFn(int errCode){
  if(errCode == -1)
    perr("There are not enought arguments entered!");
  else if(errCode == -2)
    perr("Too many arguments entered!");
  else if(errCode == -3)
    perr("Did you enter the command sequence and the filename, too?");
  else if(errCode == -4)
    perr("Sorry, memory allocation was unsuccessful.");
  else if(errCode == -5)
    perr("Unknown error. Check your commands.");
  else if(errCode == -6)
    perr("Numbers in entered selection commands have to be bigger than 0.");
  else if(errCode == -7)
    perr("You cannot escape the EOL character.");
  else if(errCode == -8)
    perr("Please enter a command.");
  perr(" The program will now exit.\n");
  return -errCode;
}

//Free the allocated memory and call the err function
int freeAndErr(tab_t *tab, int errCode, char *tmpVar[10]){
  freeTab(tab, tmpVar);
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
  while((charPlc - i) > 0 && str[charPlc - i - 1] == backslash){i++;}
  return ((i - 1) % 2 != 0) ? true : false;
}

//Recursively counts the digits of a number
int digitCt(int n){
  if(!n)
    return 0;
  return 1 + digitCt(n / 10);
}

/*
** Parsing the entered arguments
*/

//Returns a pointer to the 'cmdNum'-th command in argv
char *getCmd(char *argv[], int cmdPlc, int cmdNum){
  if(cmdNum == 1)
    return &argv[cmdPlc][0];
  int actCmdNum = 1, i = 0;
  while(argv[cmdPlc][i]){
    bool quoted = false, isProperDelim = false;
    if(argv[cmdPlc][i] == '"')
      quoted = !quoted;
    else if(argv[cmdPlc][i] == ';' && !quoted && !isEscaped(argv[cmdPlc], i))
      isProperDelim = true;
    //TODO alebo to staci??
    if(isProperDelim)
      actCmdNum++;
    if(actCmdNum == cmdNum)
      return &argv[cmdPlc][i];
    i++;
  }
  if(cmdNum > actCmdNum + 1)
    return NULL;
  return &argv[cmdPlc][i]; 
}

//If there is a selection argument, this function parses it and returns it
int getCellSelArg(char *cmd, int argNum){
  char *selArg = malloc(1);
  if(!selArg) 
    return -4;
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
    if(!rlcChar(selArg, (k + 1))){
      free(selArg);
      return -4;
    }
    /*
    char *p = realloc(selArg, (k + 1)*sizeof(char));
    if(!p){ 
      free(selArg);
      return -4;
    }
    selArg = p;
    */
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
  char *tolPtr = NULL;
  int arg = strtol(selArg, &tolPtr, 10);
  free(selArg);
  if(*tolPtr)
    return -1;
  if(arg < 1)
    return -6;
  return arg;
}

//When a string is entered (with "find" command or "set" command), it needs to
//be parsed - same as the table. Backslashes escaping a character are removed
//and quotes around the string as well.
char *getCmdStr(char *cmd, int i){
  bool isQuoted = false;
  int j = 1;
  char *str = malloc(1);
  if(!str)
    return NULL;
  while(cmd[i]){
    if(cmd[i] == '"' && !isEscaped(cmd, i + 1))
      isQuoted = !isQuoted;
    if((cmd[i] == ']' || cmd[i] == ';') && !isQuoted && !isEscaped(cmd, i + 1))
      break;
    else{
      if(!rlcChar(str, j + 1)){
        free(str);
        return NULL;
      }
      /*
      char *p = realloc(str, j + 1);
      if(!p){
        free(str);
        return NULL;
      }
      str = p;
      */
      str[j - 1] = cmd[i];
      str[j++] = '\0';
    }
    i++;
  }
  int len = strlen(str) + 1;
  for(int k = 0; k < len; k++){
    if(str[k] == '"'){
      for(int l = k; l < len - 1; l++)
        str[l] = str[l + 1];
      k--;
    }else if(str[k] == backslash)
      for(int l = k; l < len - 1; l++)
        str[l] = str[l + 1];
    len = strlen(str) + 1;
    str[len - 1] = '\0';
  }
  return str;
}

//Parses the selection arguments so they can be worked with (for example, if
//"sel->r1" == 0, "r1" will be set to 1 and r2 will be set to number of rows
void parseSel(tab_t *tab, cellSel_t *sel, cellSel_t *aSel){
  aSel->r1 = sel->r1;
  aSel->c1 = sel->c1;
  aSel->r2 = (sel->r2 == -1) ? sel->r1 : sel->r2;
  aSel->c2 = (sel->c2 == -1) ? sel->c1 : sel->c2;
  if(aSel->r1 == 0 || sel->r2 == -2)
    aSel->r2 = tab->len - 1;
  if(aSel->r1 == 0)
    aSel->r1 = 1;
  if(aSel->c1 == 0 || sel->c2 == -2)
    aSel->c2 = SROW(aSel->r1 - 1).len;
  if(aSel->c1 == 0)
    aSel->c1 = 1;
}

/*
** General working with the table
*/

//Allocates space for the whole table and writes the table into the memory
int getTab(char *argv[], tab_t *tab, char *del, int filePlc){
  ROW = malloc(sizeof(row_t));
  if(!ROW) 
    return -4;
  tab->len = 1;
  if(!mlcCell(tab, 1, 1))
    return -4;
  FILE *tabFile = fopen(argv[filePlc], "r");
  //If there are delimiters entered, filename will be as the 4th argument
  int rowN = 1, cellN = 1, charN = 1;
  char tmpC = '\0';
  bool quoted = false;
  for(int i = 0; tmpC != EOF; i++){
    tmpC = fgetc(tabFile);
    if(tmpC == '\n'){
      rowN++;
      cellN = charN = 1;
      if(quoted)
        return -7;
      if(!editRows(tab, -1, 1))
        return -4;
    }else if(!quoted && isDel(&tmpC, del) 
        && !isEscaped(CONT(rowN - 1, cellN - 1), charN)){
      cellN++;
      charN = 1;
      if(!editCols(tab, rowN, cellN, 1))
        return -4;
    }else{
      if(tmpC == '"' && !isEscaped(CONT(rowN - 1, cellN - 1), charN))
        quoted = !quoted;
      charN++;
      SCONT(rowN - 1, cellN - 1, charN - 2) = tmpC;
      if(!rlcCont(tab, rowN, cellN, 1))
        return -4;
    }
  }
  fclose(tabFile);
  return 0;
}

//Converts the tab to pure text - gets rid of backslashes that "are escaping a
//character" and quotes that are not escaped
bool parseTheTab(tab_t *tab){
  for(int i = 0; i < tab->len - 1; i++){
    for(int j = 0; j < SROW(i).len; j++){
      for(int k = 0; k < SCELL(i, j).len; k++){
        if(SCONT(i, j, k) == backslash){
          for(int l = k; l < SCELL(i, j).len - 1; l++)
            SCONT(i, j, l) = SCONT(i, j, l + 1);
          if(!rlcCont(tab, i + 1, j + 1, -1))
            return false;
        }else if(SCONT(i, j, k) == '"'){
          for(int l = k; l < SCELL(i, j).len - 1; l++)
            SCONT(i, j, l) = SCONT(i, j, l + 1);
          if(!rlcCont(tab, i + 1, j + 1, -1))
            return false;
          k--;
        }
      }
    }
  }
  return true;
}

//After running this function, all rows will have the same amounts of columns
bool addCols(tab_t *tab){
  int maxCols = 0;
  for(int i = 1; i < tab->len - 1; i++)
    if(SROW(i - 1).len > maxCols)
      maxCols = SROW(i - 1).len;
  for(int i = 1; i < tab->len && tab->len > 2; i++) //Only if there is more than 1 row
    if(SROW(i - 1).len != maxCols)
      if(!editCols(tab, i, SROW(i - 1).len + 1, maxCols - SROW(i - 1).len))
        return false;
  return true;
}

//If a selection lands on a cell that isn't a part of the table, this functions
//adds necessary rows and columns
bool addSelectedCols(tab_t *tab, cellSel_t *sel){
  cellSel_t aSel;
  parseSel(tab, sel, &aSel);
  int maxr = (aSel.r1 > aSel.r2) ? aSel.r1 : aSel.r2;
  int maxc = (aSel.c1 > aSel.c2) ? aSel.c1 : aSel.c2;
  if(tab->len - 1 < maxr)
    if(!editRows(tab, tab->len - 1, maxr - tab->len + 1))
      return false;
  //addCols(tab);
  if(SROW(0).len < maxc)
    if(!editCols(tab, 0, SROW(0).len + 1, maxc - SROW(0).len))
      return false;
  addCols(tab);
  return true;
}

//Repeatedly searches and removes last columns that have all the cells empty
bool removeEmptyCols(tab_t *tab){
  bool done = false, isEmpty = true;
  for(int i = SROW(0).len; !done && SROW(0).len > 1; i--){
    for(int j = 1; j < tab->len; j++){
      if(SCELL(j - 1, i - 1).len != 1){
        isEmpty = false;
        return true;
      }
    }
    if(isEmpty)
      if(!editCols(tab, 0, i, -1))
        return false;
  }
  return true;
}

//All the backslashes "that were escaping a character" and quotes that were not
//escaped, were deleted. This functions inserts a backslash before characters
//that need to be escaped and puts quotes around a cell if it's necessary
bool prepTabForPrint(tab_t *tab, char *del){
  for(int i = 0; i < tab->len - 1; i++){
    for(int j = 0; j < SROW(i).len; j++){
      bool makeQuoted = false;
      for(int k = 0; k < SCELL(i, j).len; k++){
        for(int l = 0; del[l] && !makeQuoted; l++)
          if(SCONT(i, j, k) == del[l] || SCONT(i, j, k) == '"')
            makeQuoted = true;
        if(SCONT(i, j, k) == backslash || (SCONT(i, j, k) == '"')){
          if(!rlcCont(tab, i + 1, j + 1, 1))
            return false;
          for(int l = SCELL(i, j).len - 1; l > k; l--)
            SCONT(i, j, l) = SCONT(i, j, l - 1);
          SCONT(i, j, k) = backslash;
          k++;
        }
      }
      if(makeQuoted){
        if(!rlcCont(tab, i + 1, j + 1, 2))
          return false;
        for(int l = SCELL(i, j).len - 1; l > 0; l--)
          SCONT(i, j, l) = SCONT(i, j, l - 1);
        SCONT(i, j, 0) = SCONT(i, j, SCELL(i, j).len - 2) = '"';
        SCONT(i, j, SCELL(i, j).len - 1) = '\0';
        //makeQuoted = false;
      }
    }
  }
  return true;
}

//Prints the characters of the table one by one
void printTab(tab_t *tab, char *del, char *fileName){
  //TODO remove
  if(!print){
    (void) fileName;
    for(int i = 0; i < tab->len - 1; i++){
      for(int j = 0; j < SROW(i).len; j++){
        for(int k = 0; k < SCELL(i, j).len; k++){
          printf("%c", SCONT(i, j, k));
        }
        if(j+1 != SROW(i).len){
          //printf("\t%c\t", del[0]);
          printf("%c", del[0]);
        }
      }
      printf("\n");
    }
  }
  else{
  FILE *f = fopen(fileName, "w");
  for(int i = 0; i < tab->len - 1; i++){
    for(int j = 0; j < SROW(i).len; j++){
      for(int k = 0; k < SCELL(i, j).len - 1; k++)
        fputc(SCONT(i, j, k), f);
      if(j+1 != SROW(i).len){
        fputc(del[0], f);
      }
    }
    fputc('\n', f);
  }
  fclose(f);
  }
}

/*
** Additional functions for selecting cells
*/

//bool min is false for "max", true for "min"
void getMinOrMaxFn(tab_t *tab, cellSel_t *sel, bool min){
  cellSel_t aSel;
  parseSel(tab, sel, &aSel);
  int nextr1, nextc1;
  double val;
  bool init = false;
  for(int i = aSel.r1; i <= aSel.r2; i++){
    for(int j = aSel.c1; j <= aSel.c2; j++){
      char *todPtr = NULL;
      double tmp = strtod(CONT(i - 1, j - 1), &todPtr);
      if(!todPtr[0] && (!init || (min && tmp < val) || (!min && tmp > val))){
        init = true;
        val = tmp;
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
bool findFn(char *cmd, tab_t *tab, cellSel_t *sel){
  char *str = getCmdStr(cmd, strlen("[find "));
  if(!str)
    return false;
  bool done = false;
  cellSel_t aSel;
  parseSel(tab, sel, &aSel);
  for(int i = aSel.r1; i <= aSel.r2 && !done; i++){
    for(int j = aSel.c1; j <= aSel.c2 && !done; j++){
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

//Checks, if the actual command is a selection command. If so, it parses it and
//writes it into the "sel" structure, which stands for "selection command"
int isSel(char *cmd, tab_t *tab, cellSel_t *sel, cellSel_t *tmpSel){
  if(cmd[0] != '[')
    return 0; //Not a selection command
  if(cmd == strstr(cmd, "[set]")){
    memcpy(tmpSel, sel, sizeof(cellSel_t));
  }else if(cmd == strstr(cmd, "[_]")){
    memcpy(sel, tmpSel, sizeof(cellSel_t));
  }else if(cmd == strstr(cmd, "[min]") || cmd == strstr(cmd, "[max]")){
    getMinOrMaxFn(tab, sel, cmd == strstr(cmd, "[min]"));
  }else if(cmd == strstr(cmd, "[find ")){
    if(!findFn(cmd, tab, sel))
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
    if(args == 4){
      sel->r2 = argsArr[2];
      sel->c2 = argsArr[3];
    }else
      sel->r2 = sel->c2 = -1;
    if(!addSelectedCols(tab, sel))
      return -4;
  }
  return 1;
}

/*
** Additional functions for executing entered commands
*/

//Writes string into a selected cell
bool setFn(char *cmd, tab_t *tab, cellSel_t sel){
  char *str = getCmdStr(cmd, strlen("set "));
  if(!str)
    return false;
  for(int i = sel.r1; i <= sel.r2; i++){
    for(int j = sel.c1; j <= sel.c2; j++){
      if(!rlcCont(tab, i, j, (-SCELL(i - 1, j - 1).len + strlen(str) + 1)))
        return false;
      strcpy(CONT(i - 1, j - 1), str);
      //SCONT(i - 1, j - 1, strlen(str)) = '\0';
    }
  }
  free(str);
  return true;
}

//Removes the content of a selected cell
bool clearFn(tab_t *tab, cellSel_t sel){ 
  for(int i = sel.r1; i <= sel.r2; i++)
    for(int j = sel.c1; j <= sel.c2; j++)
      if(SCELL(i - 1, j - 1).len != 1)
        if(!rlcCont(tab, i, j, -(SCELL(i - 1, j - 1).len) + 1))
          return false;
  return true;
}

//Swaps two cells
int swapFn(char *cmd, tab_t *tab, cellSel_t sel){
  cellSel_t aSel, swpSel;
  int isSelRet = isSel(cmd + strlen("swap "), tab, &swpSel, NULL);
  if(isSelRet < 0)
    return isSelRet;
  if(isSelRet != 0){
    parseSel(tab, &sel, &aSel);
    for(int i = aSel.r1; i <= aSel.r2; i++){
      for(int j = aSel.c1; j <= aSel.c2; j++){
        cell_t tmp = SCELL(i - 1, j - 1);
        SCELL(i - 1, j - 1) = SCELL(swpSel.r1 - 1, swpSel.c1 - 1);
        SCELL(swpSel.r1 - 1, swpSel.c1 - 1) = tmp;
      }
    }
  }
  return 0;
}

//Function for getting sum of, average of or count of all the selected cells,
//which only contain numbers
int sumAvgCtFn(char *cmd, tab_t *tab, cellSel_t *sel){
  int cmdParsed;
  if(cmd == strstr(cmd, "sum ["))
    cmdParsed = 1;
  else if(cmd == strstr(cmd, "avg ["))
    cmdParsed = 2;
  else if(cmd == strstr(cmd, "count ["))
    cmdParsed = 3;
  else
    return 0;
  cellSel_t aSel, cmdSel;
  parseSel(tab, sel, &aSel);
  double val = 0, count = 0;
  for(int i = aSel.r1; i <= aSel.r2; i++){
    for(int j = aSel.c1; j <= aSel.c2; j++){
      char *todPtr = NULL;
      double tmp = strtod(CONT(i - 1, j - 1), &todPtr);
      if(!todPtr[0] && SCONT(i - 1, j - 1, 0)){
        if(cmdParsed != 3)
          val += tmp;
        if(cmdParsed != 1)
          count++;
      }
    }
  }
  int isSelRet = 0;
  if(cmdParsed == 2){
    isSelRet = isSel(cmd + strlen("avg "), tab, &cmdSel, NULL);
    val /= count;
  }else if(cmdParsed == 3){
    isSelRet = isSel(cmd + strlen("count "), tab, &cmdSel, NULL);
    val = count;
  }else if(cmdParsed == 1)
    isSelRet = isSel(cmd + strlen("sum "), tab, &cmdSel, NULL);
  if(isSelRet < 0)
    return isSelRet;
  if(isSelRet != 0){
    char *str = malloc((digitCt((int)val) + 16 + 1) * sizeof(int));
    int len = sprintf(str, "%g", val) + 1;
    int prevLen = SCELL(cmdSel.r1 - 1, cmdSel.c1 - 1).len;
    if(!rlcCont(tab, cmdSel.r1, cmdSel.c1, (len + 1 - prevLen)))
      return -4;
    strcpy(CONT(cmdSel.r1 - 1, cmdSel.c1 - 1), str);
    //SCELL(cmdSel.r1 - 1, cmdSel.c1 - 1).len = len;
    free(str);
  }
  return 0;
}

//Gets the length of a string in a cell
int lenFn(char *cmd, tab_t *tab, cellSel_t sel){
  cellSel_t lenSel, aSel;
  int isSelRet = isSel(cmd + strlen("len "), tab, &lenSel, NULL);
  if(isSelRet < 0)
    return isSelRet;
  if(isSelRet != 0){
    parseSel(tab, &sel, &aSel);
    int len = SCELL(aSel.r2 - 1, aSel.c2 - 1).len - 1;
    int prevLen = SCELL(lenSel.r2 - 1, lenSel.c2 - 1).len - 1;
    //int digits = digits(len);
    if(!rlcCont(tab, lenSel.r1, lenSel.c1, (digitCt(len) + 1 - prevLen)))
      return -4;
    sprintf(CONT(lenSel.r1 - 1, lenSel.c1 - 1), "%d", len);
    //SCELL(lenSel.r1 - 1, lenSel.c1 - 1).len = digits + 1;
  }
  return 0;
}

/*
** Executing the entered commands
*/

//Function responsible for working with tmporary variables that the user can
//work with
bool tmpVarFn(char *cmd, int cmdLen, char *tmpVar[10], cellSel_t sel, tab_t *tab){
  if(cmdLen < 6) 
    return true;
  if((cmd[6] == '\0' || cmd[6] == ';') && cmd[5] >= '0' && cmd[5] <= '9'){
    int var = cmd[5] - '0';
    cellSel_t aSel;
    parseSel(tab, &sel, &aSel);
    if(strstr(cmd, "def _") == cmd){
      if(!rlcChar(tmpVar[var], SCELL(aSel.r1 - 1, aSel.c1 - 1).len + 1))
        return false;
      /*
      char *p = realloc(tmpVar[var], SCELL(aSel.r1 - 1, aSel.c1 - 1).len + 1);
      if(!p) 
        return false;
      tmpVar[var] = p;
      */
      strcpy(tmpVar[var], CONT(aSel.r1 - 1, aSel.c1 - 1));
    }else if(strstr(cmd, "use _") == cmd){
      for(int i = aSel.r1; i <= aSel.r2; i++){
        for(int j = aSel.c1; j <= aSel.c2; j++){
          int len = strlen(tmpVar[var]) + 1;
          if(len != SCELL(i - 1, j - 1).len)
            if(!rlcCont(tab, i, j, len - SCELL(i - 1, j - 1).len))
              return false;
          strcpy(CONT(i - 1, j - 1), tmpVar[var]);
        }
      }
    }else if(strstr(cmd, "inc _") == cmd){
      char *todPtr = NULL;
      double val = strtod(tmpVar[var], &todPtr);
      if(todPtr && todPtr[0] != '\0') 
        val = 0;
      if(!rlcChar(tmpVar[var], digitCt(val + 1) + 1))
        return false;
      /*
      char *p = realloc(tmpVar[var], digitCt(val + 1) + 1);
      if(!p) 
        return false;
      tmpVar[var] = p;
      */
      sprintf(tmpVar[var], "%g", ++val);
    }
  }
  return true;
}

//Executing the table editing commands - arow, irow, drow, icol, acol, dcol
int tabEdit(char *cmd, int cmdLen, tab_t *tab, cellSel_t sel){
  cellSel_t aSel;
  parseSel(tab, &sel, &aSel);
  if(cmdLen == 4 && (cmd[4] == '\0' || cmd[4] == ';')){
    for(int j = 0, i = aSel.r1; i <= aSel.r2; i++){
      if(strstr(cmd, "irow") == cmd){
        if(!editRows(tab, i - 1 + j++, 1))
          return -4;
      }else if(strstr(cmd, "arow") == cmd){
        if(!editRows(tab, i + j++, 1))
          return -4;
      }else if(strstr(cmd, "drow") == cmd){
        if(!editRows(tab, i + j--, -1))
          return -4;
      }
    }
    for(int j = 0, i = aSel.c1; i <= aSel.c2; i++){
      if(strstr(cmd, "icol") == cmd){
        if(!editCols(tab, 0, i + j++, 1))
          return -4;
      }else if(strstr(cmd, "acol") == cmd){
        if(!editCols(tab, 0, i + 1 + j++, 1))
          return -4;
      }else if(strstr(cmd, "dcol") == cmd){
        if(SROW(0).len == 0)
          break;
        if(!editCols(tab, 0, i + j--, -1))
          return -4;
      }
    }
  }
  return 0;
}

//Executing the content editing commands
int contEdit(char *cmd, tab_t *tab, cellSel_t *sel){
  cellSel_t aSel;
  parseSel(tab, sel, &aSel);
  int errCode = 0;
  if(cmd == strstr(cmd, "set ")){
    if(!setFn(cmd, tab, aSel))
      errCode = -4;
  }else if(cmd == strstr(cmd, "clear")){// && (cmd[5] == '\0' || cmd[5] == ';')){
    if(!clearFn(tab, aSel))
      errCode = -4;
  }else if(cmd == strstr(cmd, "swap [")){
    errCode = swapFn(cmd, tab, *sel);
  }else if(cmd == strstr(cmd, "len ")){
    errCode = lenFn(cmd, tab, *sel);
  }else
    errCode = sumAvgCtFn(cmd, tab, sel);
  if(errCode)
    return errCode;
  return 0;
}

/*
** Main "Executing the commands" function
*/

//Reads commands and executes them
int execCmds(char *argv[], int cmdPlc, tab_t *tab, char *tmpVar[10]){
  char *cmd;
  int cmdLen, cmdNum = 0, errCode = 0;
  cellSel_t sel = {1, 1, -1, -1}, tmpSel = {1, 1, -1, -1};
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
    int isSelRet = isSel(cmd, tab, &sel, &tmpSel);
    if(isSelRet < 0)
      return isSelRet; //Err
    else if(isSelRet > 0)
      continue; //The command is a selection command
    //Exectution:
    if(!tmpVarFn(cmd, cmdLen, tmpVar, sel, tab))
      return -4;
    errCode = tabEdit(cmd, cmdLen, tab, sel);
    if(errCode)
      return errCode;
    errCode = contEdit(cmd, tab, &sel);
    if(errCode)
      return errCode;
  }
  return 0;
}



char *getCmd2(char *argv[], char *prevCmd, int cmdPlc){
  bool quoted = false;
  for(int i = (&prevCmd[0] - &argv[cmdPlc][0]); ; i++){
    if(argv[cmdPlc][i] == '"' && !isEscaped(argv[cmdPlc], i))
      quoted = !quoted;
    if(!argv[cmdPlc][i])
      return &argv[cmdPlc][i];
    if(argv[cmdPlc][i] == ';' && !quoted && !isEscaped(argv[cmdPlc], i))
      return &argv[cmdPlc][i + 1];
  }
  return NULL;
}

bool parseCmds(cmds_t *curr, char *cmd, int cmdLen){
  if(cmdLen == 4){
    if(strstr(cmd, "irow") == cmd) curr->cmdType = irow;
    else if(strstr(cmd, "arow") == cmd) curr->cmdType = arow;
    else if(strstr(cmd, "drow") == cmd) curr->cmdType = drow;
    else if(strstr(cmd, "icol") == cmd) curr->cmdType = icol;
    else if(strstr(cmd, "acol") == cmd) curr->cmdType = acol;
    else if(strstr(cmd, "dcol") == cmd) curr->cmdType = dcol;
  }else if(cmdLen == 5){
    if(strstr(cmd, "[min]") == cmd) curr->cmdType = minSel;
    else if(strstr(cmd, "[max]") == cmd) curr->cmdType = maxSel;
    else if(strstr(cmd, "clear") == cmd) curr->cmdType = clear;
    else if(strstr(cmd, "[set]") == cmd) curr->cmdType = setTmp;
  }else if(cmdLen == 6){
    if(strstr(cmd, "def _") == cmd) curr->cmdType = defTmp;
    else if(strstr(cmd, "use _") == cmd) curr->cmdType = useTmp;
    else if(strstr(cmd, "inc _") == cmd) curr->cmdType = incTmp;
  }
  if(strstr(cmd, "[find ") == cmd) curr->cmdType = findSel;
  else if(strstr(cmd, "set ") == cmd) curr->cmdType = set;
  else if(strstr(cmd, "swap [") == cmd) curr->cmdType = swap;
  else if(strstr(cmd, "sum [") == cmd) curr->cmdType = sum;
  else if(strstr(cmd, "avg [") == cmd) curr->cmdType = avg;
  else if(strstr(cmd, "count [") == cmd) curr->cmdType = count;
  else if(strstr(cmd, "len [") == cmd) curr->cmdType = len;
  if(curr->cmdType)
    return true;
  return false;
}

int getCmds(char *argv[], int cmdPlc, cmds_t *firstCmd, tab_t *tab){
  //nacitat prikaz, klasifikovat prikaz, zapisat do pola prikazov
  cmds_t *curr = firstCmd;
  char *actCmd = &argv[cmdPlc][0];
  int cmdLen = getCmd2(argv, actCmd, cmdPlc) - actCmd;
  while(cmdLen){
    if(actCmd[cmdLen - 1] == ';')
      cmdLen--;
    curr->cmdType = cmdLen;
    if(parseCmds(curr, actCmd, cmdLen)){
      if(curr->cmdType >= sel && curr->cmdType <= tmpSel){

      }else if(curr->cmdType >= swap && curr->cmdType <= len){
        cellSel_t tmpSel;
        int isSelRet = isSel(strchr(actCmd, '['), tab, &tmpSel, NULL);
        if(isSelRet < 0)
          return isSelRet;
        curr->selArgs = tmpSel;


      }else if(curr->cmdType >= defTmp && curr->cmdType <= incTmp){
        char *tolPtr = NULL;
        curr->tmpSelN = strtol(actCmd + sizeof("def _"), &tolPtr, 1);
        if(tolPtr)
          curr->tmpSelN = -1;
      }

    }

    actCmd = getCmd2(argv, actCmd, cmdPlc);
    cmdLen = getCmd2(argv, actCmd, cmdPlc) - actCmd;
    curr->next = malloc(sizeof(cmds_t));
    curr = curr->next;
    curr->cmdType = 0;
  }
  curr->next = NULL;
  curr = firstCmd;
  while(curr->next){
    printf("%d\n", curr->cmdType);
    curr = curr->next;
  }
  return 0;
}



/*
** "Main" function
*/

int main(int argc, char *argv[]){
  int errCode = 0;
  char *del = " ";
  int cmdPlc = 1, filePlc = 2;
  if(argc < 3){
    return errFn(-1);
  }else if(argc == 3 && !strcmp(argv[1], "-d")){
    return errFn(-3);
  }else if(argc == 4){
    return errFn(-3);
  }else if(argc > 5)
    return errFn(-2);
  if(!strcmp(argv[1], "-d")){
    del = argv[2];
    cmdPlc = 3;
    filePlc = 4;
  }
  if(argv[cmdPlc][0] == '\0')
    return errFn(-8);
  char *tmpVar[10] = {0};
  if(!mlcTmpVars(tmpVar)) 
    return errFn(-4);
  tab_t tab;
  cmds_t *firstCmd = malloc(sizeof(cmds_t));

  if(deb){
  errCode = getTab(argv, &tab, del, filePlc);
  if(errCode) 
    return freeAndErr(&tab, errCode, tmpVar);
  printf("Get the tab\n");
  checkTheTab(&tab);

  errCode = getCmds(argv, cmdPlc, firstCmd, &tab);
  if(errCode)
    return freeAndErr(&tab, errCode, tmpVar);
  printf("Get the cmds\n");
  checkTheTab(&tab);
  return 0;

  if(!addCols(&tab)) 
    return freeAndErr(&tab, -4, tmpVar);
  printf("Add cols\n");
  checkTheTab(&tab);
  
  if(!parseTheTab(&tab))
    return freeAndErr(&tab, -4, tmpVar);
  printf("Parse the tab\n");
  checkTheTab(&tab);

  errCode = execCmds(argv, cmdPlc, &tab, tmpVar);
  if(errCode) 
    return freeAndErr(&tab, errCode, tmpVar);
  printf("Exec cmds\n");
  checkTheTab(&tab);

  if(!addCols(&tab)) 
    return freeAndErr(&tab, -4, tmpVar);
  printf("Add columns\n");
  checkTheTab(&tab);

  if(!removeEmptyCols(&tab))
    return freeAndErr(&tab, -4, tmpVar);
  printf("Remove empty columns\n");
  checkTheTab(&tab);

  if(!prepTabForPrint(&tab, del))
    return freeAndErr(&tab, -4, tmpVar);
  printf("Prep tab for print\n");
  checkTheTab(&tab);


  }else{

  errCode = getTab(argv, &tab, del, filePlc);
  if(errCode) 
    return freeAndErr(&tab, errCode, tmpVar);
  if(!parseTheTab(&tab))
    return freeAndErr(&tab, -4, tmpVar);
  if(!addCols(&tab)) 
    return freeAndErr(&tab, -4, tmpVar);
  errCode = execCmds(argv, cmdPlc, &tab, tmpVar);
  if(errCode) 
    return freeAndErr(&tab, errCode, tmpVar);
  if(!addCols(&tab)) 
    return freeAndErr(&tab, -4, tmpVar);
  if(!removeEmptyCols(&tab))
    return freeAndErr(&tab, -4, tmpVar);
  if(!prepTabForPrint(&tab, del))
    return freeAndErr(&tab, -4, tmpVar);
  }

  printTab(&tab, del, argv[cmdPlc + 1]);
  freeTab(&tab, tmpVar);
  return 0;
}

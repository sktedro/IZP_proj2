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

//Frees the memory allocated for the table and temporary variables
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
void freeTab(tab_t *tab, char *tempVar[10]){
  for(int i = tab->len - 1; i >= 0; i--)
    freeRow(tab, i);
  free(ROW);
  for(int i = 0; i < 10; i++)
    if(tempVar[i])
      free(tempVar[i]);
}

//Allocate space for contents of a new cell
bool mallocCont(tab_t *tab, int rowN, int cellN, int cellcN){
  CONT(rowN - 1, cellN - 1) = malloc(cellcN);
  if(!CONT(rowN - 1, cellN - 1))
    return false;
  SCELL(rowN - 1, cellN - 1).len = cellcN;
  SCONT(rowN - 1, cellN - 1, cellcN - 1) = '\0';
  return true;
}

//Allocate space for cells of a new row
bool mallocCell(tab_t *tab, int rowN, int cellN){
  CELL(rowN - 1) = malloc(cellN * sizeof(cell_t));
  if(!CELL(rowN - 1))
    return false;
  //SROW(rowN - 1).len = 1;
  SROW(rowN - 1).len = cellN;
  for(int i = 0; i < cellN; i++)
    if(!mallocCont(tab, rowN, cellN, 1))
      return false;
  return true;
}

bool reallocAndCheck(char *p, int newSize){
  char *temp = realloc(p, newSize);
  if(!temp) 
    return false;
  p = temp;
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

//Reallocate space for columns in a row
bool reallocCells(tab_t *tab, int rowN, int newSize){
  cell_t *p = realloc(CELL(rowN - 1), newSize*sizeof(cell_t));
  if(!p) 
    return false;
  CELL(rowN - 1) = p;
  SROW(rowN - 1).len = newSize;
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
    if(tab->len - (-by) + 1 < 1){
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
  while((charPlc - i) > 0 && str[charPlc - i - 1] == backslash){i++;}
  return ((i - 1) % 2 != 0) ? true : false;
}

//Recursively counts the digits of a number
int digitsCt(int n){
  if(!n)
    return 0;
  return 1 + digitsCt(n / 10);
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
    if(!reallocAndCheck(selArg, (k + 1))){
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
  char *tolptr = NULL;
  int arg = strtol(selArg, &tolptr, 10);
  free(selArg);
  if(*tolptr)
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
      if(!reallocAndCheck(str, j + 1)){
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
void parseSel(tab_t *tab, cellSel_t *sel, cellSel_t *actSel){
  actSel->r1 = sel->r1;
  actSel->c1 = sel->c1;
  actSel->r2 = (sel->r2 == -1) ? sel->r1 : sel->r2;
  actSel->c2 = (sel->c2 == -1) ? sel->c1 : sel->c2;
  if(actSel->r1 == 0 || sel->r2 == -2)
    actSel->r2 = tab->len - 1;
  if(actSel->r1 == 0)
    actSel->r1 = 1;
  if(actSel->c1 == 0 || sel->c2 == -2)
    actSel->c2 = SROW(actSel->r1 - 1).len;
  if(actSel->c1 == 0)
    actSel->c1 = 1;
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
  if(!mallocCell(tab, 1, 1))
    return -4;
  FILE *tabFile = fopen(argv[filePlc], "r");
  //If there are delimiters entered, filename will be as the 4th argument
  int rowN = 1, cellN = 1, cellcN = 1;
  char tempC = '\0';
  bool quoted = false;
  for(int i = 0; tempC != EOF; i++){
    tempC = fgetc(tabFile);
    if(tempC == '\n'){
      rowN++;
      cellN = cellcN = 1;
      if(quoted)
        return -7;
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

//Converts the tab to pure text - gets rid of backslashes that "are escaping a
//character" and quotes that are not escaped
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

//After running this function, all rows will have the same amounts of columns
bool addCols(tab_t *tab){
  int maxCols = 0;
  for(int i = 1; i < tab->len - 1; i++)
    if(SROW(i - 1).len > maxCols)
      maxCols = SROW(i - 1).len;
  for(int i = 1; i < tab->len && tab->len > 2; i++) //Only if there is more than 1 row
    if(SROW(i - 1).len != maxCols)
      if(!editTableCols(tab, i, SROW(i - 1).len + 1, maxCols - SROW(i - 1).len))
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
  if(tab->len - 1 < maxr)
    if(!editTableRows(tab, tab->len - 1, maxr - tab->len + 1))
      return false;
  //addCols(tab);
  if(SROW(0).len < maxc)
    if(!editTableCols(tab, 0, SROW(0).len + 1, maxc - SROW(0).len))
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
      if(!editTableCols(tab, 0, i, -1))
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
  cellSel_t actSel;
  parseSel(tab, sel, &actSel);
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
bool findFn(char *cmd, tab_t *tab, cellSel_t *sel){
  char *str = getCmdStr(cmd, strlen("[find "));
  if(!str)
    return false;
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

//Checks, if the actual command is a selection command. If so, it parses it and
//writes it into the "sel" structure, which stands for "selection command"
int isCellSel(char *cmd, tab_t *tab, cellSel_t *sel, cellSel_t *tempSel){
  if(cmd[0] != '[')
    return 0; //Not a selection command
  if(cmd == strstr(cmd, "[set]")){
    memcpy(tempSel, sel, sizeof(cellSel_t));
  }else if(cmd == strstr(cmd, "[_]")){
    memcpy(sel, tempSel, sizeof(cellSel_t));
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
      if(!reallocCont(tab, i, j, (-SCELL(i - 1, j - 1).len + strlen(str) + 1)))
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
        if(!reallocCont(tab, i, j, -(SCELL(i - 1, j - 1).len) + 1))
          return false;
  return true;
}

//Swaps two cells
int swapFn(char *cmd, tab_t *tab, cellSel_t sel){
  cellSel_t actSel, swapSel;
  int isCellSelRet = isCellSel(cmd + strlen("swap "), tab, &swapSel, NULL);
  if(isCellSelRet < 0)
    return isCellSelRet;
  if(isCellSelRet != 0){
    parseSel(tab, &sel, &actSel);
    for(int i = actSel.r1; i <= actSel.r2; i++){
      for(int j = actSel.c1; j <= actSel.c2; j++){
        cell_t temp = SCELL(i - 1, j - 1);
        SCELL(i - 1, j - 1) = SCELL(swapSel.r1 - 1, swapSel.c1 - 1);
        SCELL(swapSel.r1 - 1, swapSel.c1 - 1) = temp;
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
  cellSel_t actSel, cmdSel;
  parseSel(tab, sel, &actSel);
  double val = 0, count = 0;
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
  int isCellSelRet = 0;
  if(cmdParsed == 2){
    isCellSelRet = isCellSel(cmd + strlen("avg "), tab, &cmdSel, NULL);
    val /= count;
  }else if(cmdParsed == 3){
    isCellSelRet = isCellSel(cmd + strlen("count "), tab, &cmdSel, NULL);
    val = count;
  }else if(cmdParsed == 1)
    isCellSelRet = isCellSel(cmd + strlen("sum "), tab, &cmdSel, NULL);
  if(isCellSelRet < 0)
    return isCellSelRet;
  if(isCellSelRet != 0){
    char *str = malloc((digitsCt((int)val) + 16 + 1) * sizeof(int));
    int len = sprintf(str, "%g", val) + 1;
    int prevLen = SCELL(cmdSel.r1 - 1, cmdSel.c1 - 1).len;
    if(!reallocCont(tab, cmdSel.r1, cmdSel.c1, (len + 1 - prevLen)))
      return -4;
    strcpy(CONT(cmdSel.r1 - 1, cmdSel.c1 - 1), str);
    //SCELL(cmdSel.r1 - 1, cmdSel.c1 - 1).len = len;
    free(str);
  }
  return 0;
}

//Gets the length of a string in a cell
int lenFn(char *cmd, tab_t *tab, cellSel_t sel){
  cellSel_t lenSel, actSel;
  int isCellSelRet = isCellSel(cmd + strlen("len "), tab, &lenSel, NULL);
  if(isCellSelRet < 0)
    return isCellSelRet;
  if(isCellSelRet != 0){
    parseSel(tab, &sel, &actSel);
    int len = SCELL(actSel.r2 - 1, actSel.c2 - 1).len - 1;
    int prevLen = SCELL(lenSel.r2 - 1, lenSel.c2 - 1).len - 1;
    //int digits = digits(len);
    if(!reallocCont(tab, lenSel.r1, lenSel.c1, (digitsCt(len) + 1 - prevLen)))
      return -4;
    sprintf(CONT(lenSel.r1 - 1, lenSel.c1 - 1), "%d", len);
    //SCELL(lenSel.r1 - 1, lenSel.c1 - 1).len = digits + 1;
  }
  return 0;
}

/*
** Executing the entered commands
*/

//Function responsible for working with temporary variables that the user can
//work with
bool tempVarFn(char *cmd, int cmdLen, char *tempVar[10], cellSel_t sel, tab_t *tab){
  if(cmdLen < 6) 
    return true;
  if((cmd[6] == '\0' || cmd[6] == ';') && cmd[5] >= '0' && cmd[5] <= '9'){
    int var = cmd[5] - '0';
    cellSel_t actSel;
    parseSel(tab, &sel, &actSel);
    if(strstr(cmd, "def _") == cmd){
      if(!reallocAndCheck(tempVar[var], SCELL(actSel.r1 - 1, actSel.c1 - 1).len + 1))
        return false;
      /*
      char *p = realloc(tempVar[var], SCELL(actSel.r1 - 1, actSel.c1 - 1).len + 1);
      if(!p) 
        return false;
      tempVar[var] = p;
      */
      strcpy(tempVar[var], CONT(actSel.r1 - 1, actSel.c1 - 1));
    }else if(strstr(cmd, "use _") == cmd){
      for(int i = actSel.r1; i <= actSel.r2; i++){
        for(int j = actSel.c1; j <= actSel.c2; j++){
          int len = strlen(tempVar[var]) + 1;
          if(len != SCELL(i - 1, j - 1).len)
            if(!reallocCont(tab, i, j, len - SCELL(i - 1, j - 1).len))
              return false;
          strcpy(CONT(i - 1, j - 1), tempVar[var]);
        }
      }
    }else if(strstr(cmd, "inc _") == cmd){
      char *todptr = NULL;
      double val = strtod(tempVar[var], &todptr);
      if(todptr && todptr[0] != '\0') 
        val = 0;
      if(!reallocAndCheck(tempVar[var], digitsCt(val + 1) + 1))
        return false;
      /*
      char *p = realloc(tempVar[var], digitsCt(val + 1) + 1);
      if(!p) 
        return false;
      tempVar[var] = p;
      */
      sprintf(tempVar[var], "%g", ++val);
    }
  }
  return true;
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
      }else if(strstr(cmd, "drow") == cmd){
        if(!editTableRows(tab, i + j--, -1))
          return -4;
      }
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
          break;
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
  int errCode = 0;
  if(cmd == strstr(cmd, "set ")){
    if(!setFn(cmd, tab, actSel))
      errCode = -4;
  }else if(cmd == strstr(cmd, "clear")){// && (cmd[5] == '\0' || cmd[5] == ';')){
    if(!clearFn(tab, actSel))
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
int execCmds(char *argv[], int cmdPlc, tab_t *tab, char *tempVar[10]){
  char *cmd;
  int cmdLen, cmdNum = 0, errCode = 0;
  cellSel_t sel = {1, 1, -1, -1}, tempSel = {1, 1, -1, -1};
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
    if(isCellSelRet < 0)
      return isCellSelRet; //Err
    else if(isCellSelRet > 0)
      continue; //The command is a selection command
    //Exectution:
    if(!tempVarFn(cmd, cmdLen, tempVar, sel, tab))
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
  char *tempVar[10] = {0};
  if(!allocTempVars(tempVar)) 
    return errFn(-4);
  tab_t tab;

  if(deb){
  errCode = getTab(argv, &tab, del, filePlc);
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

  if(!removeEmptyCols(&tab))
    return freeAndErr(&tab, -4, tempVar);
  printf("Remove empty columns\n");
  checkTheTab(&tab);

  if(!prepTabForPrint(&tab, del))
    return freeAndErr(&tab, -4, tempVar);
  printf("Prep tab for print\n");
  checkTheTab(&tab);


  }else{

  errCode = getTab(argv, &tab, del, filePlc);
  if(errCode) 
    return freeAndErr(&tab, errCode, tempVar);
  if(!parseTheTab(&tab))
    return freeAndErr(&tab, -4, tempVar);
  if(!addCols(&tab)) 
    return freeAndErr(&tab, -4, tempVar);
  errCode = execCmds(argv, cmdPlc, &tab, tempVar);
  if(errCode) 
    return freeAndErr(&tab, errCode, tempVar);
  if(!addCols(&tab)) 
    return freeAndErr(&tab, -4, tempVar);
  if(!removeEmptyCols(&tab))
    return freeAndErr(&tab, -4, tempVar);
  if(!prepTabForPrint(&tab, del))
    return freeAndErr(&tab, -4, tempVar);
  }

  printTab(&tab, del, argv[cmdPlc + 1]);
  freeTab(&tab, tempVar);
  return 0;
}

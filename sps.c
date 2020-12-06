/* IZP project #2
** xskalo01, Patrik Skalos
** 4.12.2020
** 
** I recommend reading error codes in errFn at line TODO
** Note: invalid commands are ignored and error is not returned
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define ROW               tab->row                    //Ptr to rows in the tab
#define SROW(X)           tab->row[X]                 //SROW = specific row
#define CELL(X)           tab->row[X].cell            //Ptr to cells in a row
#define SCELL(X, Y)       tab->row[X].cell[Y]         //SCELL = specific cell
#define CONT(X, Y)        tab->row[X].cell[Y].cont    //Content of a cell
#define SCONT(X, Y, Z)    tab->row[X].cell[Y].cont[Z] //SCONT = specific cont
#define err(X)            fprintf(stderr, X)
#define maxNumLen         32    //No number will have more digits than 32...

#define print 1
#define deb 1

//row1, col1, row2 and col2. r1 and c1 can be zero or negative if '-' or '_'
//was entered. These values are then converted to actual row/col numbers by
//parseSel function 
typedef struct{
  int r1;
  int c1;
  int r2;
  int c2;
} cSel_t;

typedef struct{
  cSel_t tmpSel[10];
  int len;
} tmpSel_t;

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
  for(int i = 0; i < tab->len - 1; i++){
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
      free(tmpVar[i]); //Also, free the user's temporary variables
}

//Allocate space for contents of a new cell
bool mlcCont(tab_t *tab, int rowN, int cellN){
  CONT(rowN - 1, cellN - 1) = malloc(1);
  if(!CONT(rowN - 1, cellN - 1))
    return false;
  SCELL(rowN - 1, cellN - 1).len = 1;
  SCONT(rowN - 1, cellN - 1, 0) = '\0';
  return true;
}

//Allocate space for cells of a new row
bool mlcCell(tab_t *tab, int rowN){
  CELL(rowN - 1) = malloc(sizeof(cell_t));
  if(!CELL(rowN - 1))
    return false;
  SROW(rowN - 1).len = 1;
  if(!mlcCont(tab, rowN, 1))
    return false;
  return true;
}

//When a temporary string is used, this function is uset to realloc it
bool rlcChar(char **p, int newSz){
  char *tmp = realloc(*p, newSz);
  if(!tmp) 
    return false;
  *p = tmp;
  return true;
}

//Reallocate space for contents of a cell (mostly adding one more character)
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

//Reallocate space for columns in a row, if a columns is to be added or removed
bool rlcCells(tab_t *tab, int rowN, int newSz){
  cell_t *p = realloc(CELL(rowN - 1), newSz*sizeof(cell_t));
  if(!p) 
    return false;
  CELL(rowN - 1) = p;
  SROW(rowN - 1).len = newSz;
  return true;
}

//Reallocate space for rows in the table, if a row is to be added or removed
bool rlcRows(tab_t *tab, int newSz){
  row_t *p = realloc(ROW, newSz*sizeof(row_t));
  if(!p) 
    return false;
  ROW = p;
  tab->len = newSz;
  return true;
}

//Simply allocate 10 user's temporary variables
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

//Deleting or adding rows starting at row 'from'. 'by' is negative for deletion
bool editRows(tab_t *tab, int from, int by){
  if(from < 0) //If from is negative, last row is selected
    from = tab->len;
  if(by > 0){
    //Create space for 'by' new rows, shift all rows on the right from 'from'
    //and allocate space for a cell and a character in added rows
    if(!rlcRows(tab, (tab->len + by))) 
      return false;
    for(int i = tab->len - 1; i > from + by - 1; i--)
      SROW(i) = SROW(i - by);
    for(int i = 0; i < by; i++)
      if(!mlcCell(tab, from + i + 1))
        return false;
  }else if(by < 0){
    if(tab->len - (-by) + 1 < 1){
      //If all rows are to be deleted, free it and allocate place for one char
      freeTab(tab, NULL);
      ROW = malloc(sizeof(row_t));
      tab->len = 1;
      if(!mlcCell(tab, 1))
        return -4;
      return true;
    }
    //Free the allocated rows that are to be deleted, shift all rows on the
    //right from 'from' to the left by 'by' and update the table length
    for(int i = 0; i < (-by); i++)
      freeRow(tab, from + i - 1);
    for(int i = from - 1; i < tab->len + by + 1; i++)
      SROW(i) = SROW(i + (-by));
    if(!rlcRows(tab, (tab->len - (-by) + 1)))
      return false;
  }
  return true;
}

//Deleting or adding columns. Pretty much the same as editRows
bool editCols(tab_t *tab, int rowN, int from, int by){
  int howManyRows = 1; //Normally, this function works for one row
  if(rowN == 0){ //But if 'rowN' == 0, all the rows are selected
    howManyRows = tab->len - 1;
    rowN = 1;
  }
  if(from == 0) //If 'from' is zero, last column is selected
    from = SROW(0).len;
  for(int i = rowN; i < rowN + howManyRows; i++){
    if(SCONT(i - 1, 0, 0) == EOF)
      break;
    int newCellN = SROW(i - 1).len + by;
    //If all columns are to be deleted, call editRows to delete all rows:
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
        if(!mlcCont(tab, i, j))
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
    err("There are not enought arguments entered!");
  else if(errCode == -2)
    err("Too many arguments entered!");
  else if(errCode == -3)
    err("Did you enter the command sequence and the filename, too?");
  else if(errCode == -4)
    err("Sorry, memory allocation was unsuccessful.");
  else if(errCode == -5)
    err("Unknown error. Check your commands.");
  else if(errCode == -6)
    err("Numbers in entered selection commands have to be bigger than 0.");
  else if(errCode == -7)
    err("You cannot escape the EOL character.");
  else if(errCode == -8)
    err("Please enter a command.");
  else if(errCode == -9)
    err("The specified file does not exist.");
  else if(errCode == -10)
    err("One of you commands is too long. Maximum length is 1000 characters.");
  err(" The program will now exit.\n");
  return -errCode;
}

//Free the allocated memory and call the err function
int freeAndErr(tab_t *tab, int errCode, char *tmpVar[10]){
  freeTab(tab, tmpVar);
  return errFn(errCode);
}

//Checks if a certain character is a delimiter
bool isDel(char c, char *del){
  for(int i = 0; del[i]; i++)
    if(c == del[i])
      return true;
  return false;
}

//Checks, if a character is escaped or not (if there is odd number of
//backslashes before the character)
bool isEscaped(char *str, int charPlc){
  /*
  if(!str)
    return false;
    */
  int i = 1;
  while((charPlc - i) > 0 && str[charPlc - i - 1] == '\\'){i++;}
  return ((i - 1) % 2 != 0) ? true : false;
}

/*
** Parsing the entered arguments
*/

//Returns a pointer to the next command in argv
char *getCmd(char *argv[], char *prevCmd, int cmdPlc){
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

//If there is a selection argument, this function parses it and returns it
int getCellSelArg(char *cmd, int argNum){
  int i = 1, j = 1, k = 1;
  while(j != argNum)
    if(cmd[i++] == ',')
      j++;
  char *selArg = malloc(maxNumLen + 1);
  if(!selArg) 
    return -4;
  while(cmd[i] != ',' && cmd[i] != ']'){
    selArg[k-1] = cmd[i];
    selArg[k] = '\0';
    i++;
    k++;
  }
  char *tolPtr = NULL;
  int arg = strtol(selArg, &tolPtr, 10);
  if(!strcmp(selArg, "_")){
    free(selArg);
    return 0; //Whole row/column was selected
  }else if(!strcmp(selArg, "-")){
    free(selArg);
    return -2; //Last row/column was selected
  }else if(tolPtr[0]){
    free(selArg);
    return -1; //Not a number an neither a '_' or '-'
  }
  free(selArg);
  if(arg < 1)
    return -6; //Number is lower than 1..
  return arg;
}

//When a string is entered (with "find" command or "set" command), it needs to
//be parsed - same as the table. Backslashes escaping a character are removed
//and quotes around the string as well.
char *getCmdStr(char *cmd, int i){
  bool quoted = false;
  int j = 1;
  char *str = malloc(1);
  if(!str)
    return NULL;
  str[0] = '\0';
  while(cmd[i]){
    if(cmd[i] == '"' && !isEscaped(cmd, i + 1))
      quoted = !quoted;
    if((cmd[i] == ']' || cmd[i] == ';') && !quoted && !isEscaped(cmd, i + 1))
      break;
    else{
      if(!rlcChar(&str, j + 1)){
        free(str);
        return NULL;
      }
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
    }else if(str[k] == '\\')
      for(int l = k; l < len - 1; l++)
        str[l] = str[l + 1];
    len = strlen(str) + 1;
    str[len - 1] = '\0';
  }
  return str;
}

//Parses the selection arguments so they can be worked with (for example, if
//"sel->r1" == 0, "r1" will be set to 1 and r2 will be set to number of rows
void parseSel(tab_t *tab, cSel_t sel, cSel_t *aSel){
  aSel->r1 = sel.r1;
  aSel->c1 = sel.c1;
  aSel->r2 = (sel.r2 == -1) ? sel.r1 : sel.r2;
  aSel->c2 = (sel.c2 == -1) ? sel.c1 : sel.c2;
  if(aSel->r1 == 0 || sel.r2 == -2) //All rows were selected
    aSel->r2 = tab->len - 1;
  if(aSel->r1 == 0)
    aSel->r1 = 1;
  if(aSel->c1 == 0 || sel.c2 == -2) //All columns were selected
    aSel->c2 = SROW(aSel->r1 - 1).len;
  if(aSel->c1 == 0)
    aSel->c1 = 1;
}

/*
** General working with the table
*/

//Allocates space for the whole table and writes the table into the memory
int getTab(char *argv[], tab_t *tab, char *del, int filePlc){
  //First, allocate space for one character in one cell in one row:
  ROW = malloc(sizeof(row_t));
  if(!ROW) 
    return -4;
  tab->len = 1;
  if(!mlcCell(tab, 1))
    return -4;
  FILE *tabFile = fopen(argv[filePlc], "r");
  if(!tabFile)
    return -9;
  int rowN = 1, cellN = 1, charN = 1;
  bool quoted = false, escaped = false;
  char tmpC = '\0';
  do{
    tmpC = fgetc(tabFile);
    escaped = isEscaped(CONT(rowN - 1, cellN - 1), charN);
    if(tmpC == '\n'){
      rowN++;
      cellN = charN = 1;
      if(quoted || escaped)
        return -7;
      if(!editRows(tab, -1, 1)) //Append one row after the last one
        return -4;
    }else if(isDel(tmpC, del) && !quoted && !escaped){
      cellN++;
      charN = 1;
      if(!editCols(tab, rowN, cellN, 1))
        return -4;
    }else{
      if(tmpC == '"' && !escaped)
        quoted = !quoted;
      charN++;
      SCONT(rowN - 1, cellN - 1, charN - 2) = tmpC;
      if(!rlcCont(tab, rowN, cellN, 1))
        return -4;
    }
  }while(tmpC != EOF);
  fclose(tabFile);
  return 0;
}

//Converts the table to pure text - gets rid of backslashes that 
//"are escaping a character" and quotes that are not escaped
bool parseTheTab(tab_t *tab){
  for(int i = 0; i < tab->len - 1; i++){
    for(int j = 0; j < SROW(i).len; j++){
      for(int k = 0; k < SCELL(i, j).len; k++){
        if(SCONT(i, j, k) == '\\'){
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
  for(int i = 1; i < tab->len && tab->len > 2; i++) 
    if(SROW(i - 1).len != maxCols)
      if(!editCols(tab, i, SROW(i - 1).len + 1, maxCols - SROW(i - 1).len))
        return false;
  return true;
}

//If a selection lands on a cell that isn't a part of the table, this functions
//adds necessary rows and columns
bool addSeldCols(tab_t *tab, cSel_t aSel){
  if(SROW(0).len < aSel.c2)
    if(!editCols(tab, 1, SROW(0).len + 1, aSel.c2 - SROW(0).len))
      return false;
  if(tab->len - 1 < aSel.r2)
    if(!editRows(tab, tab->len - 1, aSel.r2 - tab->len + 1))
      return false;
  if(!addCols(tab))
    return false;
  return true;
}

//Repeatedly searches and removes last columns that have all the cells empty
bool removeEmptyCols(tab_t *tab){
  for(int i = SROW(0).len; SROW(0).len > 1; i--){
    for(int j = 1; j < tab->len; j++)
      if(SCELL(j - 1, i - 1).len != 1)
        return true;
    if(!editCols(tab, 0, i, -1))
      return false;
  }
  return true;
}
//Same thing for rows
bool removeEmptyRows(tab_t *tab){
  for(int i = tab->len - 1; i > 1; i--){
    for(int j = 1; j < SROW(i - 1).len + 1; j++)
      if(SCELL(i - 1, j - 1).len != 1)
        return true;
    if(!editRows(tab, i, -1))
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
            //If there's a delim or quote in the cell, make it quoted
        if(SCONT(i, j, k) == '\\' || (SCONT(i, j, k) == '"')){
          //If there's a backslash or quote, escape it with a backslash
          if(!rlcCont(tab, i + 1, j + 1, 1))
            return false;
          for(int l = SCELL(i, j).len - 1; l > k; l--)
            SCONT(i, j, l) = SCONT(i, j, l - 1);
          SCONT(i, j, k) = '\\';
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
      }
    }
  }
  return true;
}

//Prints the cells of the table one by one
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
      fputs(CONT(i, j), f);
      /*
      for(int k = 0; k < SCELL(i, j).len - 1; k++)
        fputc(SCONT(i, j, k), f);
        */
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

//Searches for the cell with min or max value
//'min' (bool) is false for "max", true for "min"
void getMinOrMaxFn(tab_t *tab, cSel_t *sel, cSel_t aSel, bool min){
  int nextr1, nextc1;
  double val;
  bool init = false;
  for(int i = aSel.r1; i <= aSel.r2; i++){
    for(int j = aSel.c1; j <= aSel.c2; j++){
      char *todPtr = NULL;
      double tmp = strtod(CONT(i - 1, j - 1), &todPtr);
      if(!todPtr[0] && SCONT(i - 1, j - 1, 0)
          && (!init || (min && tmp < val) || (!min && tmp > val))){
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
bool findFn(char *cmd, tab_t *tab, cSel_t *sel, cSel_t aSel){
  char *str = getCmdStr(cmd, strlen("[find "));
  if(!str)
    return -4;
  for(int i = aSel.r1; i <= aSel.r2; i++){
    for(int j = aSel.c1; j <= aSel.c2; j++){
      if(strstr(CONT(i - 1, j - 1), str)){
        sel->r1 = i;
        sel->c1 = j;
        sel->r2 = sel->c2 = -1;
        free(str);
        return 1;
      }
    }
  }
  free(str);
  return 0;
}

//Checks, if a command is a selection command. If so, it parses it and
//writes it into the "sel" structure, which stands for "selection command"
int isSel(char *cmd, tab_t *tab, cSel_t *sel, cSel_t *tmpSel){
  cSel_t actSel;
  parseSel(tab, *sel, &actSel);
  if(cmd[0] != '[')
    return 0; //Not a selection command
  if(cmd == strstr(cmd, "[set]")){
    memcpy(tmpSel, sel, sizeof(cSel_t));
  }else if(cmd == strstr(cmd, "[_]")){
    memcpy(sel, tmpSel, sizeof(cSel_t));
  }else if(cmd == strstr(cmd, "[min]") || cmd == strstr(cmd, "[max]")){
    getMinOrMaxFn(tab, sel, actSel, cmd == strstr(cmd, "[min]"));
  }else if(cmd == strstr(cmd, "[find ")){
    return findFn(cmd, tab, sel, actSel);
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
      if(argsArr[i] == -1) return 0; //Argument is not a number and neither '_'
      if(argsArr[i] == -4) return -4;
      if(argsArr[i] == -6) return -6;
    }
    sel->r1 = argsArr[0];
    sel->c1 = argsArr[1];
    if(args == 4){
      sel->r2 = argsArr[2];
      sel->c2 = argsArr[3];
    }else
      sel->r2 = sel->c2 = -1;
    parseSel(tab, *sel, &actSel);
    if(!addSeldCols(tab, actSel))
      return -4;
  }
  return 1;
}

/*
** Additional functions for executing entered commands
*/

//Writes string into a selected cell
int setFn(char *cmd, tab_t *tab, cSel_t sel){
  char *str = getCmdStr(cmd, strlen("set "));
  if(!str)
    return -4;
  for(int i = sel.r1; i <= sel.r2; i++){
    for(int j = sel.c1; j <= sel.c2; j++){
      if(!rlcCont(tab, i, j, (-SCELL(i - 1, j - 1).len + strlen(str) + 1)))
        return -4;
      strcpy(CONT(i - 1, j - 1), str);
    }
  }
  free(str);
  return 0;
}

//Removes the content of a selected cell
int clearFn(tab_t *tab, cSel_t sel){ 
  for(int i = sel.r1; i <= sel.r2; i++)
    for(int j = sel.c1; j <= sel.c2; j++)
      if(SCELL(i - 1, j - 1).len != 1)
        if(!rlcCont(tab, i, j, -(SCELL(i - 1, j - 1).len) + 1))
          return -4;
  return 0;
}

//Swaps pointers of two cells. Simple enough 
int swapFn(char *cmd, tab_t *tab, cSel_t aSel){
  cSel_t swpSel = {1, 1, -1, -1};
  int isSelRet = isSel(cmd + strlen("swap "), tab, &swpSel, NULL);
  if(isSelRet < 0)
    return isSelRet;
  if(isSelRet != 0){
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

//Function for getting length of all the selected cells or sum of, average of 
//or count of all the selected cells that only contain numbers
int lenSumAvgCtFn(char *cmd, tab_t *tab, cSel_t aSel){
  int cmdParsed;
  if(cmd == strstr(cmd, "sum ["))
    cmdParsed = 1;
  else if(cmd == strstr(cmd, "avg ["))
    cmdParsed = 2;
  else if(cmd == strstr(cmd, "count ["))
    cmdParsed = 3;
  else if(cmd == strstr(cmd, "len ["))
    cmdParsed = 4;
  else
    return 0;
  cSel_t cmdSel = {1, 1, -1, -1};
  double val = 0, count = 0;
  for(int i = aSel.r1; i <= aSel.r2; i++){
    for(int j = aSel.c1; j <= aSel.c2; j++){
      if(cmdParsed == 4)
        val += strlen(CONT(i - 1, j - 1));
      else{
        char *todPtr = NULL;
        double tmp = strtod(CONT(i - 1, j - 1), &todPtr);
        if(!todPtr[0] && SCONT(i - 1, j - 1, 0)){
          val += tmp;
          count++;
        }
      }
    }
  }
  int isSelRet = 0;
  if(cmdParsed == 3){
    isSelRet = isSel(cmd + strlen("count "), tab, &cmdSel, NULL);
    //Skip 6 characters and get the selection in '[...]'
    val = count;
  }else if(cmdParsed == 1 || cmdParsed == 2 || cmdParsed == 4)
    isSelRet = isSel(cmd + strlen("sum "), tab, &cmdSel, NULL);
    //Sum, avg and len commands + space all have 4 characters
  if(cmdParsed == 2)
    val /= count;
  if(isSelRet < 1)
    return isSelRet;
  if(isSelRet != 0){
    char *str = malloc(maxNumLen + 1);
    int len = sprintf(str, "%g", val) + 1;
    int prevLen = SCELL(cmdSel.r1 - 1, cmdSel.c1 - 1).len;
    if(!rlcCont(tab, cmdSel.r1, cmdSel.c1, (len - prevLen))){
      free(str);
      return -4;
    }
    strcpy(CONT(cmdSel.r1 - 1, cmdSel.c1 - 1), str);
    free(str);
  }
  return 0;
}

/*
** Executing the entered commands
*/

//Working with user's temporary variables
bool tmpVarFn(char *cmd, char *tmpVar[10], cSel_t aSel, tab_t *tab){
  if((cmd[6] == '\0' || cmd[6] == ';') && cmd[5] >= '0' && cmd[5] <= '9'){
    int var = cmd[5] - '0';
    if(strstr(cmd, "def _") == cmd){
      if(!rlcChar(&tmpVar[var], SCELL(aSel.r1 - 1, aSel.c1 - 1).len + 1))
        return false;
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
      if(todPtr[0])
        val = 0;
      if(!rlcChar(&tmpVar[var], maxNumLen + 1))
        return false;
      sprintf(tmpVar[var], "%g", ++val);
      if(!rlcChar(&tmpVar[var], strlen(tmpVar[var]) + 1))
        return false;
    }
  }
  return true;
}

//Executing the table editing commands - arow, irow, drow, icol, acol, dcol
int tabEdit(char *cmd, int cmdLen, tab_t *tab, cSel_t aSel){
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
int contEdit(char *cmd, tab_t *tab, cSel_t aSel){
  int errCode = 0;
  if(cmd == strstr(cmd, "set "))
    errCode = setFn(cmd, tab, aSel);
  else if(cmd == strstr(cmd, "clear"))
    errCode = clearFn(tab, aSel);
  else if(cmd == strstr(cmd, "swap ["))
    errCode = swapFn(cmd, tab, aSel);
  else
    errCode = lenSumAvgCtFn(cmd, tab, aSel);
  return errCode;
}

/*
** Main "Executing the commands" function
*/

//Reads commands and executes them
int execCmds(char *argv[], int cmdPlc, tab_t *tab, char *tmpVar[10]){
  char *actCmd = &argv[cmdPlc][0];
  int cmdLen = getCmd(argv, actCmd, cmdPlc) - actCmd;
  int errCode = 0;
  cSel_t sel = {1, 1, -1, -1}, tmpSel = {1, 1, -1, -1}, actSel;
  while(cmdLen){
    if(cmdLen > 1000)
      return -10;
    if(actCmd[cmdLen - 1] == ';')
      cmdLen--;
    //Checking, if the command is a selection command
    int isSelRet = isSel(actCmd, tab, &sel, &tmpSel);
    if(isSelRet < 0)
      return isSelRet; //Err
    else if(isSelRet == 0){
      //If the command is not a selection command, execute it
      parseSel(tab, sel, &actSel);
      if(!tmpVarFn(actCmd, tmpVar, actSel, tab))
        return -4;
      errCode = tabEdit(actCmd, cmdLen, tab, actSel);
      if(errCode)
        return errCode;
      errCode = contEdit(actCmd, tab, actSel);
      if(errCode)
        return errCode;
      if(!addCols(tab))
        return false;
    }
    actCmd = getCmd(argv, actCmd, cmdPlc);
    cmdLen = getCmd(argv, actCmd, cmdPlc) - actCmd;
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

  if(deb){
  errCode = getTab(argv, &tab, del, filePlc);
  if(errCode) 
    return freeAndErr(&tab, errCode, tmpVar);
  //printf("Get the tab\n");
  checkTheTab(&tab);

  if(!addCols(&tab)) 
    return freeAndErr(&tab, -4, tmpVar);
  //printf("Add cols\n");
  checkTheTab(&tab);
  
  if(!parseTheTab(&tab))
    return freeAndErr(&tab, -4, tmpVar);
  //printf("Parse the tab\n");
  checkTheTab(&tab);

  errCode = execCmds(argv, cmdPlc, &tab, tmpVar);
  if(errCode) 
    return freeAndErr(&tab, errCode, tmpVar);
  //printf("Exec cmds\n");
  checkTheTab(&tab);

  if(!addCols(&tab)) 
    return freeAndErr(&tab, -4, tmpVar);
  //printf("Add columns\n");
  checkTheTab(&tab);

  if(!removeEmptyCols(&tab) || !removeEmptyRows(&tab))
    return freeAndErr(&tab, -4, tmpVar);
  //printf("Remove empty columns\n");
  checkTheTab(&tab);

  if(!prepTabForPrint(&tab, del))
    return freeAndErr(&tab, -4, tmpVar);
  //printf("Prep tab for print\n");
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
  if(!removeEmptyCols(&tab) || !removeEmptyRows(&tab))
    return freeAndErr(&tab, -4, tmpVar);
  if(!prepTabForPrint(&tab, del))
    return freeAndErr(&tab, -4, tmpVar);
  }

  printTab(&tab, del, argv[cmdPlc + 1]);
  freeTab(&tab, tmpVar);
  return 0;
}

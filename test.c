#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct{
  char *cont;
  char *row_t;
  long len;
} tab_t;

typedef struct{
  char *cell_t;
  int len;
} row_t;

typedef struct{
  char *cont;
  int len;
} cell_t;

int main(int argc, char *argv[]){
  char *p = malloc(10);
  for(int i = 0; i < 20; i++)
    p = realloc(p, 10);
  free(p);
  /*
  tab_t tab;
  FILE *tabFile;
  tabFile = fopen(argv[1], "r");
  if(!tabFile)
    return 1;
  fseek(tabFile, 0, SEEK_END);
  tab.len = ftell(tabFile);
  fseek(tabFile, 0, SEEK_SET);
  tab.cont = malloc(1);
  for(int i = 0; i < tab.len; i++){
    tab.cont = realloc(tab.cont, i+1);
    tab.cont[i] = fgetc(tabFile);
  }
  
  printf("%s", tab.cont);

  free(tab.cont);
 
  fclose(tabFile);
  */
  return 0;
}

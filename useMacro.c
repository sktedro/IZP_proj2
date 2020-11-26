#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main(int argc, char *argv[]){
  if(argc < 2) return 1;
  FILE *f = fopen(argv[1], "r+");
  char *code = malloc(20000);
  char *codeOut = malloc(20000);

  char tempC = fgetc(f);
  int i = 0;
  while(tempC != EOF){
    code[i++] = tempC;
    tempC = fgetc(f);
  }
  fclose(f);
  FILE *out = fopen("spsBravo.c", "w");
  char line[200] = {0};
  i = 0;
  while(code[i] != EOF){
    int start = i;
    while(code[i] != '\n'){
      line[i-start] = code[i];
      i++;
    }
    strstr(line, "tab->row");


    i++;
  }



  fclose(out);
  free(code);
  free(codeOut);
  return 0;
}

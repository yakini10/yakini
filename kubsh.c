#include <stdio.h>
#include <string.h>

int main(){
   printf("$");
   char input[100];
   input[strlen(input) - 1] = '\0';
   fgets(input, 100, stdin);
   printf("%s: command not found\n", input);

   return 0;
}

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




#include <stdio.h>
#include <string.h>

int main(){
   char input[100];

while(1){  
   printf("$");
 if(fgets(input, 100, stdin)==NULL){
   printf("\nSortir par Ctrl+D\n");
   break;
}
   input[strlen(input) - 1] = '\0';
   
  if(strcmp(input, "exit")==0){
     printf("Sortir du programme\n");
break;} 
   if(strcmp(input, "\\q")==0){
     printf("Sortir du programme par \\q\n");
  printf("%s: command not found\n", input);
if(strcmp(input, "echo")==0){
     printf("123\n");
}
   return 0;
}



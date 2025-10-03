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
      break;}
if(strcmp(input, "echo")==0){
     printf("123\n");
     break;}
 printf("%s: command not found\n", input);
}
   return 0;
}



#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

#include<readline/readline.h>

void debug(char *line){
printf("%s\n",line);
}

int main()
{
printf("$ ");
   char *input;

   char input[100];
   fgets(input, 100, stdin);
   input[strlen(input)-1]='\0';
   while(true){
      input=readline("$ ");
      printf("%s: command not founds\n", input);
      if(!strcmp(input, "\\q")){
         break;
      } else if(strncmp(input, "debug ",5)== 0){
         debug(input);
      } else {
         printf("%s: command not found\n", input);
      }
   }
   return 0;
      
      
}





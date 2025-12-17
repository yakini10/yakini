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


#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<signal.h>

#include<readline/readline.h>
#include<readline/history.h>
#define HISTORY_FILE  ".kubsh_history"
sig_atomic_t signal_received = 0;

void debug(char *line){
printf("%s\n",line);
}
void sig_handler(int signum){
signal_received = signum;
printf("Configuration reloaded");
}

void print_env_var(const char *var_name){
if(var_name == NULL || strlen(var_name)==0){
printf("Usage:  \\e $VARNAME\n");
return;
}
if(var_name[0]=='$'){var_name++;
}
const char *value = getenv(var_name);
if(value==NULL){
printf("Variable'%s'not found.\n",var_name);
return;
}
char*copy=strdup(value);
if(!copy){
perror("strdup");
return;
}
printf("%s =\n",var_name);
char *token = strtok(copy,":");
if(token &&strchr(value,':')){
while(token != NULL){
printf("-%s\n",token);
token= strtok(NULL,":");
}
} else {printf("%s\n",copy);
}
free (copy);
}
int main()
{
signal(SIGSTOP, sig_handler);
read_history(HISTORY_FILE);

printf("Kubsh started.\n ");
   char *input;

   while(1){
      input=readline("$ ");
if(signal_received) {
signal_received=0;
continue;
}

      if(input == NULL) {

      break;
      }
      add_history(input);

      if(!strcmp(input, "\\q")){
         break;
      } else if(strncmp(input, "debug ",5)== 0){
         debug(input);
      } else if (strncmp(input,"\\e $",4)==0){
        print_env_var(input+3);
      } else {
         printf("%s: command not founds\n", input);
      }
      free(input);
      }

   write_history(HISTORY_FILE);

     return 0;
}
Code pour l'exo 8 
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<signal.h>
#include<unistd.h>
#include<sys/wait.h>

#include<readline/readline.h>
#include<readline/history.h>
#define HISTORY_FILE  ".kubsh_history"
sig_atomic_t signal_received = 0;

void debug(char *line){
printf("%s\n",line);
}
void sig_handler(int signum){
signal_received = signum;
printf("Configuration reloaded");
}

void print_env_var(const char *var_name){
if(var_name == NULL || strlen(var_name)==0){
printf("Usage:  \\e $VARNAME\n");
return;
}
if(var_name[0]=='$')
var_name++;

const char *value = getenv(var_name);
if(value==NULL){
printf("Variable'%s'not found.\n",var_name);
return;
}
char*copy=strdup(value);
if(!copy){
perror("strdup");
return;
}
printf("%s =\n",var_name);
if(strchr(copy,':')){
char*copy=strdup(value);
char *token = strtok(copy,":");
while(token){
printf("-%s\n",token);
token= strtok(NULL,":");
}
} else {printf("%s\n",copy);
}
free (copy);
}
void excute_command(char *input){
char*args[11];
int i=0;
char * token = strtok(input, " ");
while(token!=NULL && i<10){
args[i++]=token;
token = strtok(NULL, " ");
}
args[i]=NULL;
if(args[0]==NULL)
return;
pid_t pid = fork();
if(pid==0){
execvp(args[0],args);
exit(EXIT_FAILURE);
} else if (pid>0) {
int status;
waitpid(pid,&status,0);
} else {
perror("fork");
}
}
int main()
{
signal(SIGSTOP, sig_handler);
read_history(HISTORY_FILE);

printf("Kubsh started.\n ");
   char *input;

   while(1){
      input=readline("$ ");
if(signal_received) {
signal_received=0;
continue;
}

      if(input == NULL) {

      break;
      }
      add_history(input);

      if(!strcmp(input, "\\q")){
         break;
      } else if(strncmp(input, "debug ",5)== 0){
         debug(input);
      } else if (strncmp(input,"\\e $",4)==0){
        print_env_var(input+3);
      } else {
         excute_command(input);
      }
      free(input);
      }

   write_history(HISTORY_FILE);

     return 0;
}


On avait commence les commandes  maisons du coup je dois le terminer a la  maison




# 1. Installer les dépendances
sudo apt-get update
sudo apt-get install -y libreadline-dev

# 2. Compiler
make

# 3. Tester toutes les fonctionnalités
make run

# Tests à faire dans kubsh:
# 1. Tapez "hello" -> doit afficher "hello" (Point 1)
# 2. Ctrl+D pour quitter (Point 2)
# 3. Tapez "\q" pour quitter (Point 3)
# 4. Tapez plusieurs commandes, elles doivent être sauvegardées (Point 4)
# 5. Tapez "echo "test"" -> doit afficher "test" (Point 5)
# 6. Tapez "cd /tmp" puis "pwd" -> doit fonctionner (Point 6)
# 7. Tapez "\e $PATH" -> doit afficher PATH en liste (Point 7)
# 8. Tapez "ls" -> doit exécuter ls (Point 8)
# 9. Dans un autre terminal: "kill -HUP <pid_kubsh>" -> doit afficher "Configuration reloaded" (Point 9)
# 10. Tapez "\l /dev/sda" -> doit lister les partitions (Point 10)



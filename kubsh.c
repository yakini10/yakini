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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <readline/readline.h>
#include <readline/history.h>

#define HISTORY_FILE ".kubsh_history"
#define BUFFER_SIZE 1024

sig_atomic_t signal_received = 0;

// ============ SIGNAL HANDLER (POINT 9) ============
void sig_handler(int signum) {
    if (signum == SIGHUP) {
        printf("\nConfiguration reloaded\n");
        fflush(stdout);
    }
    signal_received = signum;
}

// ============ DEBUG ============
void debug(char *line) {
    printf("[DEBUG] %s\n", line + 6);
}

// ============ ECHO COMMAND (POINT 5) ============
void echo_command(char *input) {
    char *text = input + 5; // Skip "echo "
    
    // Supprimer les guillemets si présents
    if (text[0] == '"' && text[strlen(text)-1] == '"') {
        text[strlen(text)-1] = '\0';
        text++;
    }
    
    printf("%s\n", text);
}

// ============ PRINT ENVIRONMENT VARIABLE (POINT 7) ============
void print_env_var(const char *var_name) {
    if(var_name == NULL || strlen(var_name) == 0) {
        printf("Usage:  \\e $VARNAME\n");
        return;
    }
    
    // Passer le $ si présent
    const char *var_ptr = var_name;
    if(var_ptr[0] == '$')
        var_ptr++;
    
    const char *value = getenv(var_ptr);
    if(value == NULL) {
        printf("Variable '%s' not found.\n", var_ptr);
        return;
    }
    
    printf("%s =\n", var_ptr);
    
    // Si contient ':', afficher en liste
    if(strchr(value, ':')) {
        char *copy = strdup(value);
        if(!copy) {
            perror("strdup");
            return;
        }
        
        char *token = strtok(copy, ":");
        while(token) {
            printf("  - %s\n", token);
            token = strtok(NULL, ":");
        }
        free(copy);
    } else {
        printf("  %s\n", value);
    }
}

// ============ LIST PARTITIONS (POINT 10) ============
void list_partitions(const char *device) {
    char command[512];
    
    if (device && strlen(device) > 0) {
        // Commande avec device spécifique
        snprintf(command, sizeof(command), "fdisk -l %s 2>/dev/null || lsblk %s", device, device);
    } else {
        // Tous les devices
        snprintf(command, sizeof(command), "lsblk");
    }
    
    system(command);
}

// ============ EXECUTE COMMAND (POINTS 6 & 8) ============
void execute_command(char *input) {
    char *args[64];
    int i = 0;
    
    // Faire une copie pour ne pas modifier l'original
    char *input_copy = strdup(input);
    if (!input_copy) {
        perror("strdup");
        return;
    }
    
    char *token = strtok(input_copy, " ");
    while(token != NULL && i < 63) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
    
    if(args[0] == NULL) {
        free(input_copy);
        return;
    }
    
    // POINT 6: Vérification de commande
    if (strcmp(args[0], "cd") == 0) {
        // Commande interne cd
        if (args[1]) {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        } else {
            // cd sans argument = home
            chdir(getenv("HOME"));
        }
        free(input_copy);
        return;
    }
    
    // POINT 8: Exécuter le binaire (recherche dans $PATH automatique par execvp)
    pid_t pid = fork();
    
    if(pid == 0) {
        // Processus enfant
        execvp(args[0], args);
        
        // Si execvp échoue
        fprintf(stderr, "Command not found: %s\n", args[0]);
        exit(EXIT_FAILURE);
        
    } else if (pid > 0) {
        // Processus parent
        int status;
        waitpid(pid, &status, 0);
        
    } else {
        perror("fork");
    }
    
    free(input_copy);
}

// ============ MAIN SHELL LOOP ============
int main() {
    // POINT 9: Configuration du signal SIGHUP
    signal(SIGHUP, sig_handler);
    
    // POINT 4: Charger l'historique depuis le fichier
    char *home = getenv("HOME");
    char history_path[512];
    snprintf(history_path, sizeof(history_path), "%s/%s", home, HISTORY_FILE);
    
    // Vérifier si le fichier existe avant de le charger
    FILE *test = fopen(history_path, "r");
    if (test) {
        fclose(test);
        read_history(history_path);
    }
    
    printf("Kubsh started. Type \\q to quit.\n");
    
    char *input;
    
    // POINT 2: Boucle principale (sortie par Ctrl+D)
    while(1) {
        input = readline("kubsh> ");
        
        // Vérifier les signaux
        if(signal_received) {
            signal_received = 0;
            if (input) free(input);
            continue;
        }
        
        // POINT 2: Ctrl+D détecté (input == NULL)
        if(input == NULL) {
            printf("\n");
            break;
        }
        
        // Ignorer les lignes vides
        if(strlen(input) == 0) {
            free(input);
            continue;
        }
        
        // POINT 4: Ajouter à l'historique
        add_history(input);
        
        // POINT 1: Afficher l'entrée (démo)
        // POINT 3: Commande \q pour quitter
        if(!strcmp(input, "\\q")) {
            free(input);
            break;
        }
        // POINT 5: Commande echo
        else if(strncmp(input, "echo ", 5) == 0) {
            echo_command(input);
        }
        // POINT 7: Afficher variable d'environnement
        else if(strncmp(input, "\\e $", 4) == 0) {
            print_env_var(input + 4);
        }
        // POINT 10: Lister les partitions
        else if(strncmp(input, "\\l", 2) == 0) {
            // Extraire le device s'il est spécifié
            char *device = input + 2;
            while(*device == ' ') device++; // Supprimer les espaces
            list_partitions(device);
        }
        // Commande debug
        else if(strncmp(input, "debug ", 6) == 0) {
            debug(input);
        }
        // POINT 6 & 8: Commandes normales
        else {
            execute_command(input);
        }
        
        free(input);
    }
    
    // POINT 4: Sauvegarder l'historique
    write_history(history_path);
    
    printf("Goodbye!\n");
    return 0;
}


# 1. Installer les dépendances
sudo apt-get update
sudo apt-get install -y libreadline-dev

# 2. Compiler
make














La derniere mise a jour
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <pwd.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64
#define HISTORY_SIZE 100
#define HISTORY_FILE ".kubsh_history"

// Structure pour l'historique
typedef struct {
    char *commands[HISTORY_SIZE];
    int count;
    int pos;
} History;

// Variables globales
static History history = {0};
static volatile sig_atomic_t reload_config = 0;

// Prototypes
void handle_sighup(int sig);
void history_init(void);
void history_add(const char *cmd);
void history_save(void);
void history_load(void);
void history_print(void);
int is_builtin(const char *cmd);
int execute_builtin(char **args);
int execute_external(char **args);
char* find_in_path(const char *cmd);

// Gestion des signaux
void handle_sighup(int sig) {
    (void)sig;
    reload_config = 1;
    printf("\nConfiguration reloaded\n");
}

// Historique
void history_init(void) {
    for (int i = 0; i < HISTORY_SIZE; i++) {
        history.commands[i] = NULL;
    }
    history.count = 0;
    history.pos = 0;
}

void history_add(const char *cmd) {
    if (!cmd || strlen(cmd) == 0) return;
    
    if (history.commands[history.pos]) {
        free(history.commands[history.pos]);
    }
    
    history.commands[history.pos] = strdup(cmd);
    history.pos = (history.pos + 1) % HISTORY_SIZE;
    
    if (history.count < HISTORY_SIZE) {
        history.count++;
    }
}

void history_save(void) {
    char *home = getenv("HOME");
    if (!home) return;
    
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", home, HISTORY_FILE);
    
    FILE *f = fopen(path, "w");
    if (!f) return;
    
    for (int i = 0; i < history.count; i++) {
        int idx = (history.pos - history.count + i + HISTORY_SIZE) % HISTORY_SIZE;
        if (history.commands[idx]) {
            fprintf(f, "%s\n", history.commands[idx]);
        }
    }
    
    fclose(f);
}

void history_load(void) {
    char *home = getenv("HOME");
    if (!home) return;
    
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", home, HISTORY_FILE);
    
    FILE *f = fopen(path, "r");
    if (!f) return;
    
    char line[MAX_INPUT];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;
        history_add(line);
    }
    
    fclose(f);
}

void history_print(void) {
    printf("Command history:\n");
    for (int i = 0; i < history.count; i++) {
        int idx = (history.pos - history.count + i + HISTORY_SIZE) % HISTORY_SIZE;
        printf("%3d: %s\n", i + 1, history.commands[idx]);
    }
}

// Commandes intégrées
int cmd_echo(char **args) {
    for (int i = 1; args[i]; i++) {
        printf("%s", args[i]);
        if (args[i + 1]) printf(" ");
    }
    printf("\n");
    return 0;
}

int cmd_env(char **args) {
    if (!args[1]) {
        extern char **environ;
        for (int i = 0; environ[i]; i++) {
            printf("%s\n", environ[i]);
        }
    } else {
        char *val = getenv(args[1]);
        if (val) {
            if (strchr(val, ':')) {
                char *token = strtok(val, ":");
                while (token) {
                    printf("%s\n", token);
                    token = strtok(NULL, ":");
                }
            } else {
                printf("%s\n", val);
            }
        } else {
            printf("Undefined variable: %s\n", args[1]);
            return 1;
        }
    }
    return 0;
}

int cmd_exec(char **args) {
    if (!args[1]) {
        fprintf(stderr, "Usage: exec <command>\n");
        return 1;
    }
    
    char *path = find_in_path(args[1]);
    if (!path) {
        fprintf(stderr, "Command not found: %s\n", args[1]);
        return 1;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        execv(path, &args[1]);
        perror("execv");
        free(path);
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        free(path);
        return WEXITSTATUS(status);
    } else {
        perror("fork");
        free(path);
        return 1;
    }
}

int cmd_disk_info(char **args) {
    if (!args[1]) {
        fprintf(stderr, "Usage: \\l <device>\n");
        return 1;
    }
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "fdisk -l %s 2>/dev/null || lsblk %s", args[1], args[1]);
    return system(cmd);
}

// Chercher dans PATH
char* find_in_path(const char *cmd) {
    if (strchr(cmd, '/')) {
        if (access(cmd, X_OK) == 0) return strdup(cmd);
        return NULL;
    }
    
    char *path = getenv("PATH");
    if (!path) return NULL;
    
    char *path_copy = strdup(path);
    char *dir = strtok(path_copy, ":");
    
    while (dir) {
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);
        
        if (access(full_path, X_OK) == 0) {
            free(path_copy);
            return strdup(full_path);
        }
        
        dir = strtok(NULL, ":");
    }
    
    free(path_copy);
    return NULL;
}

// Vérifier si commande intégrée
int is_builtin(const char *cmd) {
    const char *builtins[] = {
        "echo", "\\e", "\\l", "\\h", "\\q", "\\vfs", NULL
    };
    
    for (int i = 0; builtins[i]; i++) {
        if (strcmp(cmd, builtins[i]) == 0) return 1;
    }
    
    return 0;
}

// Exécuter commande intégrée
int execute_builtin(char **args) {
    if (!args[0]) return 1;
    
    if (strcmp(args[0], "echo") == 0) return cmd_echo(args);
    if (strcmp(args[0], "\\e") == 0) return cmd_env(args);
    if (strcmp(args[0], "\\l") == 0) return cmd_disk_info(args);
    if (strcmp(args[0], "\\h") == 0) { history_print(); return 0; }
    if (strcmp(args[0], "\\q") == 0) return -1; // Signal pour quitter
    
    // Pour \vfs, appeler la fonction du fichier vfs.c
    if (strcmp(args[0], "\\vfs") == 0) {
        // Fonction déclarée dans vfs.c
        extern int vfs_command(char **args);
        return vfs_command(args);
    }
    
    return 1;
}

// Exécuter commande externe
int execute_external(char **args) {
    pid_t pid = fork();
    
    if (pid == 0) {
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    } else {
        perror("fork");
        return 1;
    }
}

// Parser l'entrée
int parse_input(char *input, char **args) {
    int i = 0;
    char *token = strtok(input, " \t\n");
    
    while (token && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    
    args[i] = NULL;
    return i;
}

// Boucle principale
void shell_loop(void) {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    
    signal(SIGHUP, handle_sighup);
    history_init();
    history_load();
    
    // Créer ~/users
    char *home = getenv("HOME");
    if (home) {
        char path[256];
        snprintf(path, sizeof(path), "%s/users", home);
        mkdir(path, 0755);
    }
    
    printf("kubsh v1.0 - Type \\q to quit, \\h for history\n");
    
    while (1) {
        if (reload_config) {
            reload_config = 0;
            // Recharger configuration si nécessaire
        }
        
        printf("kubsh> ");
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break;
        }
        
        input[strcspn(input, "\n")] = 0;
        
        if (strlen(input) == 0) continue;
        
        history_add(input);
        
        int argc = parse_input(input, args);
        if (argc == 0) continue;
        
        if (is_builtin(args[0])) {
            int result = execute_builtin(args);
            if (result == -1) break; // \q command
        } else {
            execute_external(args);
        }
    }
    
    history_save();
}

int main(void) {
    shell_loop();
    return 0;
}

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



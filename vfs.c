#define FUSE_USE_VERSION 31
#define MAX_USERS 1000
#include <fuse3/fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

static int vfs_pid = -1;

// === ВСТАВКА: Структура для хранения информации о пользователях ===
typedef struct {
    char name[256];
    uid_t uid;
    char home[1024];
    char shell[1024];
} user_info_t;

static user_info_t *users_list = NULL;
static int users_count = 0;
// === КОНЕЦ ВСТАВКИ ===

int get_users_list() {
    // === ВСТАВКА: Реализация получения списка пользователей ===
    setpwent();
    
    users_count = 0;
    struct passwd *pwd;
    
    // Подсчет пользователей
    while ((pwd = getpwent()) != NULL) {
        if (pwd->pw_uid >= 1000 || pwd->pw_uid == 0) {
            users_count++;
        }
    }
    endpwent();
    
    // Выделение памяти
    users_list = malloc(users_count * sizeof(user_info_t));
    if (!users_list) {
        perror("malloc");
        return -1;
    }
    
    // Заполнение данных
    setpwent();
    int index = 0;
    while ((pwd = getpwent()) != NULL && index < users_count) {
        if (pwd->pw_uid >= 1000 || pwd->pw_uid == 0) {
            strncpy(users_list[index].name, pwd->pw_name, sizeof(users_list[index].name) - 1);
            users_list[index].uid = pwd->pw_uid;
            strncpy(users_list[index].home, pwd->pw_dir, sizeof(users_list[index].home) - 1);
            strncpy(users_list[index].shell, pwd->pw_shell, sizeof(users_list[index].shell) - 1);
            index++;
        }
    }
    endpwent();
    
    return users_count;
    // === КОНЕЦ ВСТАВКИ ===
}

void free_users_list() {
    // === ВСТАВКА: Освобождение памяти ===
    if (users_list) {
        free(users_list);
        users_list = NULL;
    }
    users_count = 0;
    // === КОНЕЦ ВСТАВКИ ===
}

static int users_readdir(
    const char *path, 
    void *buf, 
    fuse_fill_dir_t filler,
    off_t offset,
    struct fuse_file_info *fi,
    enum fuse_readdir_flags flags
) {
    // === ВСТАВКА: Реализация чтения директории ===
    printf("readdir: %s\n", path);
    
    if (strcmp(path, "/") == 0) {
        filler(buf, ".", NULL, 0, 0);
        filler(buf, "..", NULL, 0, 0);
        
        for (int i = 0; i < users_count; i++) {
            filler(buf, users_list[i].name, NULL, 0, 0);
        }
        return 0;
    }
    
    char username[256];
    if (sscanf(path, "/%255[^/]", username) == 1) {
        filler(buf, ".", NULL, 0, 0);
        filler(buf, "..", NULL, 0, 0);
        filler(buf, "id", NULL, 0, 0);
        filler(buf, "home", NULL, 0, 0);
        filler(buf, "shell", NULL, 0, 0);
        return 0;
    }
    
    return -ENOENT;
    // === КОНЕЦ ВСТАВКИ ===
}

static int users_open(const char *path, struct fuse_file_info *fi) {
    printf("open: %s\n", path);
    return 0;
}

static int users_read(
    const char *path, 
    char *buf, 
    size_t size,
    off_t offset,
    struct fuse_file_info *fi
) {
    // === ВСТАВКА: Реализация чтения файлов ===
    printf("read: %s, size: %zu, offset: %ld\n", path, size, offset);
    
    char username[256], filename[256];
    if (sscanf(path, "/%255[^/]/%255s", username, filename) == 2) {
        for (int i = 0; i < users_count; i++) {
            if (strcmp(users_list[i].name, username) == 0) {
                char content[1024];
                size_t content_len = 0;
                
                if (strcmp(filename, "id") == 0) {
                    content_len = snprintf(content, sizeof(content), "%d\n", users_list[i].uid);
                } else if (strcmp(filename, "home") == 0) {
                    content_len = snprintf(content, sizeof(content), "%s\n", users_list[i].home);
} else if (strcmp(filename, "shell") == 0) {
                    content_len = snprintf(content, sizeof(content), "%s\n", users_list[i].shell);
                } else {
                    return -ENOENT;
                }
                
                if (offset >= content_len) {
                    return 0;
                }
                
                if (offset + size > content_len) {
                    size = content_len - offset;
                }
                
                memcpy(buf, content + offset, size);
                return size;
            }
        }
    }
    
    return -ENOENT;
    // === КОНЕЦ ВСТАВКИ ===
}

static int users_getattr(const char *path, struct stat *stbuf,
                         struct fuse_file_info *fi) {
    // === ВСТАВКА: Реализация получения атрибутов ===
    printf("getattr: %s\n", path);
    memset(stbuf, 0, sizeof(struct stat));
    
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }
    
    char username[256];
    if (sscanf(path, "/%255[^/]", username) == 1 && strchr(path + 1, '/') == NULL) {
        for (int i = 0; i < users_count; i++) {
            if (strcmp(users_list[i].name, username) == 0) {
                stbuf->st_mode = S_IFDIR | 0755;
                stbuf->st_nlink = 2;
                return 0;
            }
        }
    }
    
    char user_dir[256], filename[256];
    if (sscanf(path, "/%255[^/]/%255s", user_dir, filename) == 2) {
        for (int i = 0; i < users_count; i++) {
            if (strcmp(users_list[i].name, user_dir) == 0) {
                if (strcmp(filename, "id") == 0 || 
                    strcmp(filename, "home") == 0 || 
                    strcmp(filename, "shell") == 0) {
                    stbuf->st_mode = S_IFREG | 0644;
                    stbuf->st_nlink = 1;
                    
                    if (strcmp(filename, "id") == 0) {
                        stbuf->st_size = snprintf(NULL, 0, "%d\n", users_list[i].uid);
                    } else if (strcmp(filename, "home") == 0) {
                        stbuf->st_size = snprintf(NULL, 0, "%s\n", users_list[i].home);
                    } else if (strcmp(filename, "shell") == 0) {
                        stbuf->st_size = snprintf(NULL, 0, "%s\n", users_list[i].shell);
                    }
                    return 0;
                }
            }
        }
    }
    
    return -ENOENT;
    // === КОНЕЦ ВСТАВКИ ===
}

// === ВСТАВКА: Функция для выполнения системных команд ===
int execute_system_command(const char *command) {
    printf("Executing: %s\n", command);
    int result = system(command);
    if (result == -1) {
        perror("system");
        return -1;
    }
    return WEXITSTATUS(result);
}
// === КОНЕЦ ВСТАВКИ ===

// === ВСТАВКА: Функция для создания реального пользователя ===
int create_system_user(const char *username) {
    char command[512];
    snprintf(command, sizeof(command), "sudo adduser --disabled-password --gecos '' %s", username);
    
    if (execute_system_command(command) != 0) {
        fprintf(stderr, "Failed to create user: %s\n", username);
        return -1;
    }
    
    printf("User %s created successfully\n", username);
    return 0;
}
// === КОНЕЦ ВСТАВКИ ===

// === ВСТАВКА: Функция для удаления реального пользователя ===
int delete_system_user(const char *username) {
    char command[256];
    snprintf(command, sizeof(command), "sudo userdel -r %s", username);
    
    if (execute_system_command(command) != 0) {
        fprintf(stderr, "Failed to delete user: %s\n", username);
        return -1;
    }
    
    printf("User %s deleted successfully\n", username);
    return 0;
}
// === КОНЕЦ ВСТАВКИ ===

static int users_mkdir(const char *path, mode_t mode) {

derniere mise a jour aussi

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <errno.h>

#define VFS_BASE "users"

// Créer répertoire utilisateur
static void create_user_dir(const char *username) {
    char *home = getenv("HOME");
    if (!home) return;
    
    struct passwd *pw = getpwnam(username);
    if (!pw) return;
    
    char dir_path[512];
    snprintf(dir_path, sizeof(dir_path), "%s/%s/%s", home, VFS_BASE, username);
    
    mkdir(dir_path, 0755);
    
    // Fichier id
    char id_path[512];
    snprintf(id_path, sizeof(id_path), "%s/id", dir_path);
    FILE *f = fopen(id_path, "w");
    if (f) {
        fprintf(f, "%d\n", pw->pw_uid);
        fclose(f);
    }
    
    // Fichier home
    char home_path[512];
    snprintf(home_path, sizeof(home_path), "%s/home", dir_path);
    f = fopen(home_path, "w");
    if (f) {
        fprintf(f, "%s\n", pw->pw_dir);
        fclose(f);
    }
    
    // Fichier shell
    char shell_path[512];
    snprintf(shell_path, sizeof(shell_path), "%s/shell", dir_path);
    f = fopen(shell_path, "w");
    if (f) {
        fprintf(f, "%s\n", pw->pw_shell);
        fclose(f);
    }
}

// Initialiser VFS
int vfs_init(void) {
    char *home = getenv("HOME");
    if (!home) return -1;
    
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", home, VFS_BASE);
    
    if (mkdir(path, 0755) != 0 && errno != EEXIST) {
        perror("mkdir");
        return -1;
    }
    
    // Créer répertoires pour utilisateurs existants
    struct passwd *pw;
    setpwent();
    while ((pw = getpwent()) != NULL) {
        create_user_dir(pw->pw_name);
    }
    endpwent();
    
    return 0;
}

// Gérer commandes VFS
int vfs_command(char **args) {
    if (!args[1]) {
        // Lister utilisateurs
        char *home = getenv("HOME");
        if (!home) return -1;
        
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", home, VFS_BASE);
        
        DIR *dir = opendir(path);
        if (!dir) return -1;
        
        printf("Users in VFS:\n");
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            
            char user_path[512];
            snprintf(user_path, sizeof(user_path), "%s/%s", path, entry->d_name);
            
            struct stat st;
            if (stat(user_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                printf("  %s\n", entry->d_name);
            }
        }
        closedir(dir);
        return 0;
    }
    
    if (strcmp(args[1], "add") == 0 && args[2]) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "sudo adduser %s", args[2]);
        int ret = system(cmd);
        
        if (ret == 0) {
            create_user_dir(args[2]);
            printf("User %s added\n", args[2]);
        }
        return ret;
    }
    
    if (strcmp(args[1], "remove") == 0 && args[2]) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "sudo userdel %s", args[2]);
        int ret = system(cmd);
        
        if (ret == 0) {
            char *home = getenv("HOME");
            if (home) {
                char path[512];
                snprintf(path, sizeof(path), "%s/%s/%s", home, VFS_BASE, args[2]);
                char rm_cmd[512];
                snprintf(rm_cmd, sizeof(rm_cmd), "rm -rf %s", path);
                system(rm_cmd);
            }
            printf("User %s removed\n", args[2]);
        }
        return ret;
    }
    
    fprintf(stderr, "Usage: \\vfs [add <user>|remove <user>]\n");
    return 1;
}




#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/inotify.h>

#include <readline/readline.h>
#include <readline/history.h>

#define HISTORY_FILE ".kubsh_history"
#define VFS_USERS_DIR "users"
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))
#define MAX_PATH 512

sig_atomic_t signal_received = 0;
static char vfs_base_path[MAX_PATH];
static int inotify_fd = -1;
static int watch_fd = -1;

// ============ FONCTIONS VFS ============

static void create_user_files(const char *username, struct passwd *pwd) {
char user_dir[MAX_PATH];
snprintf(user_dir, sizeof(user_dir), "%s/%s", vfs_base_path, username);

if (mkdir(user_dir, 0755) != 0 && errno != EEXIST) {
perror("mkdir user_dir");
return;
}

char id_file[MAX_PATH];
snprintf(id_file, sizeof(id_file), "%s/id", user_dir);
FILE *f = fopen(id_file, "w");
if (f) {
fprintf(f, "%d\n", pwd->pw_uid);
fclose(f);
}

char home_file[MAX_PATH];
snprintf(home_file, sizeof(home_file), "%s/home", user_dir);
f = fopen(home_file, "w");
if (f) {
fprintf(f, "%s\n", pwd->pw_dir);
fclose(f);
}

char shell_file[MAX_PATH];
snprintf(shell_file, sizeof(shell_file), "%s/shell", user_dir);
f = fopen(shell_file, "w");
if (f) {
fprintf(f, "%s\n", pwd->pw_shell);
fclose(f);
}

printf("[VFS] User %s synchronized\n", username);
}

static void remove_user_files(const char *username) {
char user_dir[MAX_PATH];
snprintf(user_dir, sizeof(user_dir), "%s/%s", vfs_base_path, username);

char id_file[MAX_PATH], home_file[MAX_PATH], shell_file[MAX_PATH];
snprintf(id_file, sizeof(id_file), "%s/id", user_dir);
snprintf(home_file, sizeof(home_file), "%s/home", user_dir);
snprintf(shell_file, sizeof(shell_file), "%s/shell", user_dir);

unlink(id_file);
unlink(home_file);
unlink(shell_file);

if (rmdir(user_dir) != 0) {
perror("rmdir user_dir");
} else {
printf("[VFS] User %s removed\n", username);
}
}

static void system_add_user(const char *username) {
char command[MAX_PATH * 2];

struct passwd *pwd = getpwnam(username);
if (pwd != NULL) {
printf("[VFS] User %s already exists\n", username);
return;
}

snprintf(command, sizeof(command), 
         "sudo adduser --disabled-password --gecos '' %s 2>/dev/null", 
         username);

int result = system(command);
if (result == 0) {
printf("[VFS] User %s created\n", username);

pwd = getpwnam(username);
if (pwd) {
create_user_files(username, pwd);
}
} else {
fprintf(stderr, "[VFS] Failed to create user %s\n", username);
}
}

static void system_del_user(const char *username) {
char command[MAX_PATH * 2];

struct passwd *pwd = getpwnam(username);
if (pwd == NULL) {
printf("[VFS] User %s doesn't exist\n", username);
return;
}

snprintf(command, sizeof(command), "sudo userdel -r %s 2>/dev/null", username);

int result = system(command);
if (result == 0) {
printf("[VFS] User %s removed\n", username);
} else {
fprintf(stderr, "[VFS] Failed to remove user %s\n", username);
}
}

static int init_inotify(void) {
inotify_fd = inotify_init();
if (inotify_fd < 0) {
perror("inotify_init");
return -1;
}

watch_fd = inotify_add_watch(inotify_fd, vfs_base_path, 
                             IN_CREATE | IN_DELETE | IN_MOVED_TO | IN_MOVED_FROM);
if (watch_fd < 0) {
perror("inotify_add_watch");
close(inotify_fd);
inotify_fd = -1;
return -1;
}

return 0;
}

void vfs_init(void) {
char *home = getenv("HOME");
if (!home) {
fprintf(stderr, "[VFS] HOME not set\n");
return;
}

snprintf(vfs_base_path, sizeof(vfs_base_path), "%s/%s", home, VFS_USERS_DIR);

printf("[VFS] Creating: %s\n", vfs_base_path);

if (mkdir(vfs_base_path, 0755) != 0 && errno != EEXIST) {
perror("mkdir vfs_base_path");
return;
}

if (init_inotify() < 0) {
fprintf(stderr, "[VFS] Failed inotify\n");
return;
}

struct passwd *pwd;
setpwent();

while ((pwd = getpwent()) != NULL) {
if (pwd->pw_uid >= 1000) {
create_user_files(pwd->pw_name, pwd);
}
}

endpwent();

printf("[VFS] Ready\n");
}

void vfs_check_events(void) {
if (inotify_fd < 0) {
return;
}

fd_set read_fds;
struct timeval timeout;

FD_ZERO(&read_fds);
FD_SET(inotify_fd, &read_fds);

timeout.tv_sec = 0;
timeout.tv_usec = 0;

int ret = select(inotify_fd + 1, &read_fds, NULL, NULL, &timeout);
if (ret > 0 && FD_ISSET(inotify_fd, &read_fds)) {
char buffer[EVENT_BUF_LEN];
int length = read(inotify_fd, buffer, EVENT_BUF_LEN);

if (length < 0) {
perror("read inotify");
return;
}

int i = 0;
while (i < length) {
struct inotify_event *event = (struct inotify_event *)&buffer[i];

if (event->len > 0) {
if (event->mask & IN_ISDIR) {
if (event->mask & (IN_CREATE | IN_MOVED_TO)) {
printf("[VFS] New: %s\n", event->name);
system_add_user(event->name);
}
else if (event->mask & (IN_DELETE | IN_MOVED_FROM)) {
printf("[VFS] Removed: %s\n", event->name);
system_del_user(event->name);
}
}
}

i += EVENT_SIZE + event->len;
}
}
}

void vfs_cleanup(void) {
DIR *dir;
struct dirent *entry;

dir = opendir(vfs_base_path);
if (!dir) {
perror("opendir vfs_base_path");
return;
}

while ((entry = readdir(dir)) != NULL) {
if (entry->d_type == DT_DIR && 
    strcmp(entry->d_name, ".") != 0 && 
    strcmp(entry->d_name, "..") != 0) {
    
    remove_user_files(entry->d_name);
}
}

closedir(dir);

if (inotify_fd >= 0) {
if (watch_fd >= 0) {
inotify_rm_watch(inotify_fd, watch_fd);
}
close(inotify_fd);
}

printf("[VFS] Cleanup done\n");
}

// ============ FONCTIONS SHELL ============

void sig_handler(int signum){
if (signum == SIGHUP) {
printf("Configuration reloaded\n");
}
signal_received = signum;
}

void debug(char *line){
printf("%s\n",line);
}

void echo_command(char *input){
char *text = input + 5;
if(text[0]=='"'&&text[strlen(text)-1]=='"'){
text[strlen(text)-1]='\0';
text++;
}
printf("%s\n",text);
}

void print_env_var(const char *var_name){
if(var_name==NULL||strlen(var_name)==0){
printf("Usage: \\e $VARNAME\n");
return;
}
if(var_name[0]=='$')
var_name++;

const char *value=getenv(var_name);
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
char*token=strtok(copy,":");
while(token){
printf("-%s\n",token);
token=strtok(NULL,":");
}
}else{printf("%s\n",copy);}
free(copy);
}

void list_partitions(const char *device){
char command[512];
if(device&&strlen(device)>0){
snprintf(command,sizeof(command),"sudo fdisk -l %s 2>/dev/null||sudo lsblk %s",device,device);
}else{
snprintf(command,sizeof(command),"sudo lsblk");
}
system(command);
}

void execute_command(char *input){
char*args[64];
int i=0;
char*copy=strdup(input);
char*token=strtok(copy," ");
while(token!=NULL&&i<63){
args[i++]=token;
token=strtok(NULL," ");
}
args[i]=NULL;

if(args[0]==NULL){
free(copy);
return;
}

if(strcmp(args[0],"cd")==0){
if(args[1]){
if(chdir(args[1])!=0){
perror("cd");
}
}else{
chdir(getenv("HOME"));
}
free(copy);
return;
}

pid_t pid=fork();
if(pid==0){
execvp(args[0],args);
perror(args[0]);
exit(EXIT_FAILURE);
}else if(pid>0){
int status;
waitpid(pid,&status,0);
}else{
perror("fork");
}
free(copy);
}

// ============ MAIN ============

int main()
{
signal(SIGHUP,sig_handler);
signal(SIGINT,SIG_IGN);

char*home=getenv("HOME");
char history_path[512];
snprintf(history_path,sizeof(history_path),"%s/%s",home,HISTORY_FILE);

FILE*test=fopen(history_path,"r");
if(test){
fclose(test);
read_history(history_path);
}

vfs_init();

printf("Kubsh started.\n");
char*input;

while(1){
vfs_check_events();

input=readline("kubsh> ");

if(signal_received){
signal_received=0;
if(input)free(input);
continue;
}

if(input==NULL){
printf("\n");
break;
}

if(strlen(input)==0){
free(input);
continue;
}

add_history(input);

if(!strcmp(input,"\\q")){
free(input);
break;
}
else if(strncmp(input,"debug ",6)==0){
debug(input);
}
else if(strncmp(input,"echo ",5)==0){
echo_command(input);
}
else if(strncmp(input,"\\e $",4)==0){
print_env_var(input+4);
}
else if(strncmp(input,"\\l",2)==0){
char*device=input+2;
while(*device==' ')device++;
list_partitions(device);
}
else{
execute_command(input);
}
free(input);
}

write_history(history_path);
vfs_cleanup();
printf("Goodbye!\n");
return 0;
}






#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/inotify.h>

// ============ CONFIGURATION VFS ============
#define VFS_USERS_DIR "users"
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))
#define MAX_PATH 512

// ============ VARIABLES VFS ============
static char vfs_base_path[MAX_PATH];
static int inotify_fd = -1;
static int watch_fd = -1;

// ============ FONCTIONS VFS ============

// Créer les fichiers pour un utilisateur
static void create_user_files(const char *username, struct passwd *pwd) {
    char user_dir[MAX_PATH];
    snprintf(user_dir, sizeof(user_dir), "%s/%s", vfs_base_path, username);
    
    if (mkdir(user_dir, 0755) != 0 && errno != EEXIST) {
        perror("mkdir user_dir");
        return;
    }
    
    char id_file[MAX_PATH];
    snprintf(id_file, sizeof(id_file), "%s/id", user_dir);
    FILE *f = fopen(id_file, "w");
    if (f) {
        fprintf(f, "%d\n", pwd->pw_uid);
        fclose(f);
    }
    
    char home_file[MAX_PATH];
    snprintf(home_file, sizeof(home_file), "%s/home", user_dir);
    f = fopen(home_file, "w");
    if (f) {
        fprintf(f, "%s\n", pwd->pw_dir);
        fclose(f);
    }
    
    char shell_file[MAX_PATH];
    snprintf(shell_file, sizeof(shell_file), "%s/shell", user_dir);
    f = fopen(shell_file, "w");
    if (f) {
        fprintf(f, "%s\n", pwd->pw_shell);
        fclose(f);
    }
    
    printf("[VFS] User %s synchronized\n", username);
}

// Supprimer les fichiers d'un utilisateur
static void remove_user_files(const char *username) {
    char user_dir[MAX_PATH];
    snprintf(user_dir, sizeof(user_dir), "%s/%s", vfs_base_path, username);
    
    char id_file[MAX_PATH], home_file[MAX_PATH], shell_file[MAX_PATH];
    snprintf(id_file, sizeof(id_file), "%s/id", user_dir);
    snprintf(home_file, sizeof(home_file), "%s/home", user_dir);
    snprintf(shell_file, sizeof(shell_file), "%s/shell", user_dir);
    
    unlink(id_file);
    unlink(home_file);
    unlink(shell_file);
    
    if (rmdir(user_dir) != 0) {
        perror("rmdir user_dir");
    } else {
        printf("[VFS] User %s removed\n", username);
    }
}

// Ajouter un utilisateur système
static void system_add_user(const char *username) {
    char command[MAX_PATH * 2];
    
    struct passwd *pwd = getpwnam(username);
    if (pwd != NULL) {
        printf("[VFS] User %s already exists\n", username);
        return;
    }
    
    snprintf(command, sizeof(command), 
             "sudo adduser --disabled-password --gecos '' %s 2>/dev/null", 
             username);
    
    int result = system(command);
    if (result == 0) {
        printf("[VFS] User %s created\n", username);
        
        pwd = getpwnam(username);
        if (pwd) {
            create_user_files(username, pwd);
        }
    } else {
        fprintf(stderr, "[VFS] Failed to create user %s\n", username);
    }
}

// Supprimer un utilisateur système
static void system_del_user(const char *username) {
    char command[MAX_PATH * 2];
    
    struct passwd *pwd = getpwnam(username);
    if (pwd == NULL) {
        printf("[VFS] User %s doesn't exist\n", username);
        return;
    }
    
    snprintf(command, sizeof(command), "sudo userdel -r %s 2>/dev/null", username);
    
    int result = system(command);
    if (result == 0) {
        printf("[VFS] User %s removed\n", username);
    } else {
        fprintf(stderr, "[VFS] Failed to remove user %s\n", username);
    }
}

// Initialiser inotify
static int init_inotify(void) {
    inotify_fd = inotify_init();
    if (inotify_fd < 0) {
        perror("inotify_init");
        return -1;
    }
    
    watch_fd = inotify_add_watch(inotify_fd, vfs_base_path, 
                                 IN_CREATE | IN_DELETE | IN_MOVED_TO | IN_MOVED_FROM);
    if (watch_fd < 0) {
        perror("inotify_add_watch");
        close(inotify_fd);
        inotify_fd = -1;
        return -1;
    }
    
    return 0;
}

// ============ FONCTIONS PUBLIQUES VFS ============

// Initialiser la VFS
void vfs_init(void) {
    char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "[VFS] HOME not set\n");
        return;
    }
    
    snprintf(vfs_base_path, sizeof(vfs_base_path), "%s/%s", home, VFS_USERS_DIR);
    
    printf("[VFS] Creating: %s\n", vfs_base_path);
    
    if (mkdir(vfs_base_path, 0755) != 0 && errno != EEXIST) {
        perror("mkdir vfs_base_path");
        return;
    }
    
    if (init_inotify() < 0) {
        fprintf(stderr, "[VFS] Failed inotify\n");
        return;
    }
    
    struct passwd *pwd;
    setpwent();
    
    while ((pwd = getpwent()) != NULL) {
        if (pwd->pw_uid >= 1000) {
            create_user_files(pwd->pw_name, pwd);
        }
    }
    
    endpwent();
    
    printf("[VFS] Ready\n");
}

// Vérifier les événements VFS
void vfs_check_events(void) {
    if (inotify_fd < 0) {
        return;
    }
    
    fd_set read_fds;
    struct timeval timeout;
    
    FD_ZERO(&read_fds);
    FD_SET(inotify_fd, &read_fds);
    
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    
    int ret = select(inotify_fd + 1, &read_fds, NULL, NULL, &timeout);
    if (ret > 0 && FD_ISSET(inotify_fd, &read_fds)) {
        char buffer[EVENT_BUF_LEN];
        int length = read(inotify_fd, buffer, EVENT_BUF_LEN);
        
        if (length < 0) {
            perror("read inotify");
            return;
        }
        
        int i = 0;
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            
            if (event->len > 0) {
                if (event->mask & IN_ISDIR) {
                    if (event->mask & (IN_CREATE | IN_MOVED_TO)) {
                        printf("[VFS] New: %s\n", event->name);
                        system_add_user(event->name);
                    }
                    else if (event->mask & (IN_DELETE | IN_MOVED_FROM)) {
                        printf("[VFS] Removed: %s\n", event->name);
                        system_del_user(event->name);
                    }
                }
            }
            
            i += EVENT_SIZE + event->len;
        }
    }
}

// Synchroniser un utilisateur
void vfs_sync_user(const char *username) {
    struct passwd *pwd = getpwnam(username);
    if (!pwd) {
        fprintf(stderr, "[VFS] User %s not found\n", username);
        return;
    }
    
    create_user_files(username, pwd);
}

// Nettoyer la VFS
void vfs_cleanup(void) {
    DIR *dir;
    struct dirent *entry;
    
    dir = opendir(vfs_base_path);
    if (!dir) {
        perror("opendir vfs_base_path");
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && 
            strcmp(entry->d_name, ".") != 0 && 
            strcmp(entry->d_name, "..") != 0) {
            
            remove_user_files(entry->d_name);
        }
    }
    
    closedir(dir);
    
    if (inotify_fd >= 0) {
        if (watch_fd >= 0) {
            inotify_rm_watch(inotify_fd, watch_fd);
        }
        close(inotify_fd);
    }
    
    printf("[VFS] Cleanup done\n");
}

// Obtenir le chemin de la VFS
const char *vfs_get_path(void) {
    return vfs_base_path;
}

// Lister les utilisateurs
void vfs_list_users(void) {
    DIR *dir;
    struct dirent *entry;
    
    printf("\n=== Users ===\n");
    
    dir = opendir(vfs_base_path);
    if (!dir) {
        perror("opendir vfs_base_path");
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && 
            strcmp(entry->d_name, ".") != 0 && 
            strcmp(entry->d_name, "..") != 0) {
            
            char user_dir[MAX_PATH];
            snprintf(user_dir, sizeof(user_dir), "%s/%s", vfs_base_path, entry->d_name);
            
            char id_file[MAX_PATH], home_file[MAX_PATH], shell_file[MAX_PATH];
            char id[64], home[MAX_PATH], shell[MAX_PATH];
            
            snprintf(id_file, sizeof(id_file), "%s/id", user_dir);
            snprintf(home_file, sizeof(home_file), "%s/home", user_dir);
            snprintf(shell_file, sizeof(shell_file), "%s/shell", user_dir);
            
            FILE *f;
            
            f = fopen(id_file, "r");
            if (f) {
                fgets(id, sizeof(id), f);
                id[strcspn(id, "\n")] = 0;
                fclose(f);
            } else {
                strcpy(id, "N/A");
            }
            
            f = fopen(home_file, "r");
            if (f) {
                fgets(home, sizeof(home), f);
                home[strcspn(home, "\n")] = 0;
                fclose(f);
            } else {
                strcpy(home, "N/A");
            }
            
            f = fopen(shell_file, "r");
            if (f) {
                fgets(shell, sizeof(shell), f);
                shell[strcspn(shell, "\n")] = 0;
                fclose(f);
            } else {
                strcpy(shell, "N/A");
            }
            
            printf("User: %s\n", entry->d_name);
            printf("  UID: %s\n", id);
            printf("  Home: %s\n", home);
            printf("  Shell: %s\n", shell);
            printf("  ---\n");
        }
    }
    
    closedir(dir);
}

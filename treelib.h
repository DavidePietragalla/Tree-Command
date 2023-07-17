#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "treesort.h"

//le impostazioni trovo sia comodo salvarle globalmente
bool a, d, f, g, l, p, r, s, t, u, D, dirsfirst, inodes = false;
int L;//eventuale limite in altezza dell'albero
int F, C = 0;//rispettivamente il numero di file e cartelle riscontrati

//verifica alune eccezioni e salva le impostazioni
void save_set(char* set){
    if(strlen(set)<2){
        printf("Sintassi errata ritentare...\n");
        exit(1);
    }
    if(set[0]==set[1]){
        if(strcmp(set, "--dirsfirst")==0) dirsfirst=true;
        else if(strcmp(set, "--inodes")==0) inodes=true;
        else if(strcmp(set, "--help")==0){
                printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                        "Impostazioni: [-adfgprstuDL] [--dirsfirst] [--inodes] [--help]\n",
                        "-a: Tutti i file vengono considerati.\n",
                        "-d: Solo le cartelle vengono considerate.\n",
                        "-D: Mostra la data dell'ultima modifica.\n",
                        "-f: Print dell'intero percorso per ogni file.\n",
                        "-g: Mostra il nome del gruppo oppure il GID.\n",
                        "-L: Determina il limite in altezza dell'albero.\n",
                        "-p: Mostra il tipo di file e i permessi per ogni file.\n",
                        "-r: Ordina il risultato in ordine alfabetico al contrario.\n",
                        "-s: Mostra la taglia in byte per ogni file insieme al nome.\n",
                        "-t: Ordina il risultato in base alle ultime modifiche.\n",
                        "-u: Mostra l'username oppure UID.\n",
                        "--inodes: Mostra il numero inode di file e cartelle.\n",
                        "--dirsfirst: Ordina prima le cartelle.\n",
                        "--help: Mostra delle indicazioni riguardo il programma.\n");
                exit(0);
        }else{
            printf("%s%s%s", "Il programma treeHW non prevede l'impostazione ", set, ";\nRitentare...\n");
            exit(1);
        }
    }else{
        for(int c=1; c<strlen(set); c++){
            switch(set[c]){
                case 'a':
                    a=true;
                    break;
                case 'd':
                    d=true;
                    break;
                case 'f':
                    f=true;
                    break;
                case 'g':
                    g=true;
                    break;
                case 'L':
                    l=true;
                    break;
                case 'p':
                    p=true;
                    break;
                case 'r':
                    r=true;
                    t=false;
                    break;
                case 's':
                    s=true;
                    break;
                case 't':
                    t=true;
                    r=false;
                    break;
                case 'u':
                    u=true;
                    break;
                case 'D':
                    D=true;
                    break;
                default:
                    printf("%s%c%s", "Il programma treeHW non prevede l'impostazione -", set[c], ";\nRitentare...\n");
                    exit(1);
            }
        }
    }
}

//salva le impostazioni ed eventuale percorso
//utilizza save_set()
char* setup(int argc, char** argv){
    L = -1;
    bool path = false;
    int s;
    for(s=1; s<argc; s++){
        if(argv[s][0]=='-'){
            save_set(argv[s]);
        }else if(l){
            l = false;
            for(int c = 0; c<strlen(argv[s]); c++){
                if(!isdigit(argv[s][c])){
                    printf("L'impostazione -L necessita di un parametro intero.\n");
                    exit(1);
                }
            }
            L = atoi(argv[s]);
        }else{
            path = true;
        }
    }
    return path ? argv[s-1] : getcwd(NULL, 1024);
}

//si assicura che il percorso sia valido
void check_path(char* path){
    DIR* dir = opendir(path);
    if(dir == NULL){
        printf("%s\n", "Impossibile trovare il percorso indicato");
        exit(1);
    }
    closedir(dir);
}

//scorre la lista dei file nel percorso indicato
//salva in un array i file di interessa e restituisce l'array
struct dirent** get_list_file(char* path){
    DIR* dir = opendir(path);
    struct dirent** list_dir = malloc(sizeof(struct dirent**));
    list_dir[0] = NULL;
    if(dir != NULL){
        struct dirent* ent;
        int size_ls = 1;
        chdir(path);
        while((ent = readdir(dir)) != NULL){
            char* name = ent->d_name;
            if(!strcmp(name,".") || !strcmp(name, "..")) continue;
            if(!a && (name[0] == '.')) continue;
            struct stat filestat;
            stat(name, &filestat);
            if(d && !(S_ISDIR(filestat.st_mode))) continue;
            list_dir = realloc(list_dir, (size_ls*sizeof(struct dirent**))+1);
            list_dir[size_ls-1] = ent;
            size_ls++;
        }
        list_dir[size_ls-1] = NULL;
    }
    return list_dir;
}

//gestisce l'ordinamento della lista
//utilizza treesort.h
void sort_list(struct dirent** list_file, int size){
    if(!d && dirsfirst){
        int mid = dirsfst(list_file);
        if(r){
            quick_sort(list_file, 0, mid-1, -1);
            quick_sort(list_file, mid, size-1, -1);
        }else if(t){
            quick_sort(list_file, 0, mid-1, 1);
            quick_sort(list_file, mid, size-1, 1);
        }else{
            quick_sort(list_file, 0, mid-1, 0);
            quick_sort(list_file, mid, size-1, 0);
        }
    }
    else if(r) quick_sort(list_file, 0, size-1, -1);//ordine alfabetico decrescente
    else if(t) quick_sort(list_file, 0, size-1, 1);//ordine di ultima modifica
    else quick_sort(list_file, 0, size-1, 0);//ordine alfabetico
}

//produce la spaziatura adeguata da stampare
//in modo da produrre i rami di un albero
char* make_space(char* sp, int i, int size){
    char* space;
    char* newspace = malloc(strlen(sp)+6);
    newspace[0] = '\0';
    if(i<size-1) space = "│  ";
    else space = "   ";
    strcat(newspace, sp);
    strcat(newspace, space);
    return newspace;
}

//unisce percorso e cartella
//in modo da creare un nuovo percorso utilizzabile
char* make_path(char* path, char*name){
    int size = strlen(path)+strlen(name)+2;
    char* newpath = malloc(size);
    newpath[0] = '\0';
    strcat(newpath, path);
    strcat(newpath, "/");
    strcat(newpath, name);
    return newpath;

}

//stampa la data di ultima modifica di un file
char* print_date(struct stat filestat){
    char date[20];
    strftime(date, 20, "%b %d %H:%M", localtime(&filestat.st_mtime));
    printf("%s", date);
}

//gestisce la stampa delle informazioni aggiuntive
//in caso siano state richieste
//utilizza print_date()
void print_stats(struct stat filestat){
    int n = 0;
    if(inodes){
        printf("%s", " [");
        printf("%lu", filestat.st_ino);
        n++;
    }
    if(p){
        if(n==0) printf("%s", " [");
        else printf("%s", "  ");
        mode_t mode = filestat.st_mode;
        char perm[] = "----------";
        if(S_ISDIR(mode)) perm[0] = 'd';
        if(S_ISLNK(mode)) perm[0] = 'l';
        if(mode & S_IRUSR) perm[1] = 'r';
        if(mode & S_IWUSR) perm[2] = 'w';
        if(mode & S_IXUSR) perm[3] = 'x';
        if(mode & S_IRGRP) perm[4] = 'r';
        if(mode & S_IWGRP) perm[5] = 'w';
        if(mode & S_IXGRP) perm[6] = 'x';
        if(mode & S_IROTH) perm[7] = 'r';
        if(mode & S_IWOTH) perm[8] = 'w';
        if(mode & S_IXOTH) perm[9] = 'x';
        if(mode & S_ISUID) perm[3] = perm[3] == 'x' ? 's' : 'S';
        if(mode & S_ISGID) perm[6] = perm[6] == 'x' ? 's' : 'S';
        if(mode & 01000) perm[9] = perm[9] == 'x' ? 't' : 'T';
        printf("%s", perm);
        n++;
    }
    if(u){
        if(n==0) printf("%s", " [");
        else printf("%s", "  ");
        struct passwd *pwd = getpwuid(filestat.st_uid);
        if(pwd == NULL) printf("%d", filestat.st_uid);
        else printf("%s", pwd->pw_name);
        n++;
    }
    if(g){
        if(n==0) printf("%s", " [");
        else printf("%s", "  ");
        struct group *grp = getgrgid(filestat.st_gid);
        if(grp == NULL) printf("%d", filestat.st_gid);
        else printf("%s", grp->gr_name);
        n++;
    }
    if(s){
        if(n==0) printf("%s", " [");
        else printf("%s", "  ");
        printf("%ld", filestat.st_size);
        n++;
    }
    if(D){
        if(n==0) printf("%s", " [");
        else printf("%s", "  ");
        print_date(filestat);
        n++;
    }
    if(n>0) printf("%s", "] ");
}

//gestisce il caso dei link
void print_lnk(char* path){
    char lnk [1024];
    int len = readlink(path, lnk, 1024);
    lnk[len] = '\0';
    printf("-> %s ", lnk);
    struct stat filestat;
    if(stat(lnk, &filestat)==0){
        if(S_ISDIR(filestat.st_mode)) C++;
        else F++;
    }else F++;
}

//stampa tramite ricorsione l'albero
//ottiene la lista di file tramite il percorso indicato
//ordina la lista in base alle impostazioni
//stampa l'i-esimo elemento
//avvia la ricorsione su di esso
void print_tree(char* path, char* space, int lvl){
    if((L>-1) && (lvl>=L)) return; //clausola del limite -L
    struct dirent** list_file = get_list_file(path);//ottengo la lista
    int size = get_size(list_file);//recupero la taglia della lista
    sort_list(list_file, size);//ordino la lista
    int i = 0;
    while(list_file[i] != NULL){//scorro la lista di file
        char* name = list_file[i]->d_name;//ottengo il nome del file
        char* newspace = make_space(space, i, size);//calcolo la spaziatura del ramo
        if(!strcmp(name, "")) break;
        char* newpath = make_path(path, name);//calcolo il percorso della prossima chiamata
        struct stat filestat;
        if(i<size-1) printf("%s%s", space, "├── ");//stampo il ramo
        else printf("%s%s", space, "└── ");//stampo il ramo
        if(lstat(newpath, &filestat)==0) print_stats(filestat);//recupero e stampo le informazioni richieste
        if(f) printf("%s ", newpath);//clausola -f
        else printf("%s ", name);//stampo il nome del file
        if(S_ISLNK(filestat.st_mode)) print_lnk(newpath);//stampo il percorso del lnk
        else if(S_ISDIR(filestat.st_mode)) C++;//conto cartelle
        else F++;//conto file
        printf("\n");
        print_tree(newpath, newspace, lvl+1);//ricorsione sul nuovo percorso
        i++;
        //libero memoria dinamica
        free(newpath);
        free(newspace);
    }
    free(list_file);
}

//stampa il numero di cartelle e file in un albero
void print_result(){
    if(C>0){
        printf("%d ", C);
        if(C>1) printf("%s", "directories");
        else printf("%s", "directory");
        if(F>0) printf("%s", ", ");
    }
    if(F>0){
        printf("%d ", F);
        if(F>1) printf("%s\n", "files");
        else printf("%s\n", "file");
    }
}
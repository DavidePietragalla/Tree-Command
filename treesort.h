#include <stdio.h>
#include <dirent.h>
#include <time.h>

//scambia 2 elementi nella lista
void swap(struct dirent* *a, struct dirent* *b){
    struct dirent* c = *a;
    *a = *b;
    *b = c;
}

//ordina la partizione
int partition(struct dirent** list_file, int low, int high, int mode){
    char* pivot = list_file[high]->d_name;
    int i = low-1;
    for(int j = low; j<high; j++){
        char* name = list_file[j]->d_name;
        if(mode==0){//ordine alfabetico
            if(strcasecmp(name, pivot)<0){
                i++;
                if(i==j) continue;
                swap(&list_file[i], &list_file[j]);
            }
        }else if(mode==-1){//ordine alfabetico al  contrario
            if(strcasecmp(name, pivot)>0){
                i++;
                if(i==j) continue;
                swap(&list_file[i], &list_file[j]);
            }
        }else{//ordine di tempo
            struct stat filestatj;
            struct stat filestatp;
            stat(name, &filestatj);
            stat(pivot, &filestatp);
            if(filestatj.st_mtime<filestatp.st_mtime){
                i++;
                if(i==j) continue;
                swap(&list_file[i], &list_file[j]);
            }
        }
    }
    swap(&list_file[i+1], &list_file[high]);
    return i+1;
}

//gestisce le partizioni
void quick_sort(struct dirent** list_file, int low, int high, int mode){
    if(low < high){
        int pi = partition(list_file, low, high, mode);
        quick_sort(list_file, low, pi-1, mode);
        quick_sort(list_file, pi+1, high, mode);
    }
}

//prende la taglia della lista
int get_size(struct dirent** list_file){
    int size = 0;
    while(list_file[size] != NULL){
        size++;
    }
    return size;
}

//posiziona le cartelle prima dei file
int dirsfst(struct dirent** list_file){
    int i = 1;
    int j = 0;
    int mid = 0;
    struct stat filestat;
    char* name;
    while(list_file[i]!= NULL){
        name = list_file[i]->d_name;
        stat(name, &filestat);
        if(S_ISDIR(filestat.st_mode)){
            swap(&list_file[i], &list_file[j]);
            j++;
        }
        i++;
    }
    return j;
}
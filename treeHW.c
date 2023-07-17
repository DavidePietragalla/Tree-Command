#include <stdio.h>
#include "treelib.h"

int main(int argc, char **argv){
    //salvo le impostazioni e recupero la radice dell'albero
    char* path = setup(argc, argv);
    //controllo che il percorso sia valido
    check_path(path);
    //avvio la stampa ricorsiva dell'albero
    puts(".");
    print_tree(path, "", 0);
    //stampo il numero di cartelle e di file
    print_result();
    return 0;
}
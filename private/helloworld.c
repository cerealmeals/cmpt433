#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
    char *username = getlogin();
    if (username == NULL){
        perror("getlogin");
        exit(EXIT_FAILURE);
    }

    printf("Hello embedded world, from %s\n", username);
    return 0;
}
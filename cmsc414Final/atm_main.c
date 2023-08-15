/* 
 * The main program for the ATM.
 *
 * You are free to change this as necessary.
 */

#include "atm.h"
#include <stdio.h>
#include <stdlib.h>

static const char prompt[] = "ATM: ";

int main(int argc, char *argv[])
{
    char user_input[1000];

    ATM *atm = atm_create();
    
    FILE *atm_file = fopen(argv[1], "r");
    if (!atm_file) {
        printf("Error opening ATM initialization file");
        return 64;
    }

    printf("%s", prompt);
    // setvbuf(stdout, user_input, _IOFBF, sizeof(user_input));
    // fflush(stdout);

    while (fgets(user_input, 10000,stdin) != NULL)
    {
        atm_process_command(atm, user_input);
        // == 1 바꿈
        if (atm->logged_in){
            printf("ATM (%s): ", atm->user);
        } else {
            printf("%s", prompt);
        }

        fflush(stdout);

    }
   return EXIT_SUCCESS;
}

atm-main.c
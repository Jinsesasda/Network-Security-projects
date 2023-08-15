#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define MAX_STR_LEN 255

int main(int argc, char *argv[])
{   
    /* If user provides one argument, return value 62.*/
    if (argc != 2) {
        printf("Usage: init <filename>\n");
        return 62;
    }

    char atm_fname[MAX_STR_LEN + 1] = ""; 
    char bank_fname[MAX_STR_LEN + 1] = "";

    sprintf(bank_fname, "%s.bank", argv[1]);
    sprintf(atm_fname, "%s.atm", argv[1]);

    if ((access(atm_fname, F_OK) == 0) || (access(bank_fname, F_OK) == 0)) {
        printf("Error: one of the files already exists\n");
        return 63;
    }

    FILE *atm_file = fopen(atm_fname, "w");
    FILE *bank_file = fopen(bank_fname, "w");

    if (!atm_file) {
        printf("Error creating initialization files\n");
        fclose(bank_file);
        return 64;
    }

    if (!bank_file) {
        printf("Error creating initialization files\n");
        fclose(atm_file);
        return 64;
    }
    printf("Successfully initialized bank state\n");
    fclose(atm_file);
    fclose(bank_file);
    return 0;
}
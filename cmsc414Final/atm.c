#include "atm.h"
#include "ports.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <regex.h>

ATM* atm_create()
{
    ATM *atm = (ATM*) malloc(sizeof(ATM));
    if(atm == NULL)
    {
        perror("Could not allocate ATM");
        exit(1);
    }

    // Set up the network state
    // UDP socket -- to comunicate with servier
    atm->sockfd=socket(AF_INET,SOCK_DGRAM,0);

    // initialize router address
    // port number and router ip
    bzero(&atm->rtr_addr,sizeof(atm->rtr_addr));
    atm->rtr_addr.sin_family = AF_INET;
    atm->rtr_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    atm->rtr_addr.sin_port=htons(ROUTER_PORT);

    // Initialize the ATM address 'atm_addr' with the IP address and port
    // number of the ATM and bind the socket to the ATM address.
    bzero(&atm->atm_addr, sizeof(atm->atm_addr));
    atm->atm_addr.sin_family = AF_INET;
    atm->atm_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    atm->atm_addr.sin_port = htons(ATM_PORT);
    bind(atm->sockfd,(struct sockaddr *)&atm->atm_addr,sizeof(atm->atm_addr));

    // Set up the protocol state
    // TODO set up more, as needed
    atm->user = NULL;
    atm->logged_in = 0;
    // prompt
    return atm;
}

void atm_free(ATM *atm)
{
    if(atm != NULL)
    {
        close(atm->sockfd);
        free(atm);
    }
}

ssize_t atm_send(ATM *atm, char *data, size_t data_len)
{
    // Returns the number of bytes sent; negative on error
    return sendto(atm->sockfd, data, data_len, 0,
                  (struct sockaddr*) &atm->rtr_addr, sizeof(atm->rtr_addr));
}

ssize_t atm_recv(ATM *atm, char *data, size_t max_data_len)
{
    // Returns the number of bytes received; negative on error
    return recvfrom(atm->sockfd, data, max_data_len, 0, NULL, NULL);
}

void atm_process_command(ATM *atm, char *command)
{
    // TODO: Implement the ATM's side of the ATM-bank protocol

	/*
	 * The following is a toy example that simply sends the
	 * user's command to the bank, receives a message from the
	 * bank, and then prints it to stdout.
	 */

	/*
    char recvline[10000];
    int n;

    atm_send(atm, command, strlen(command));
    n = atm_recv(atm,recvline,10000);
    recvline[n]=0;
    fputs(recvline,stdout);
	*/
    char *token = strtok(command, " \t\n");

    if (!strcmp("begin-session", token)) {
        token = strtok(NULL, " \t\n");
        if (token == NULL || strlen(token) > 250){
            printf("Usage: begin-session <user-name>\n\n");
            return;
        }

        for (int i=0; token[i] != '\0'; i++){
            if(!isalpha(token[i])) {
                printf("Usage: begin-session <user-name>\n\n");
                printf("%d", atm->logged_in);
                return;
            }
        }

        if (atm->logged_in) {
            printf("A user is already logged in\n\n");
            return;
        }
        /*똑같음*/
        /* bank 에서 받을때 get-user -> registered */
        int n;
        char sendline[1000];
        sprintf(sendline, "registered %s", token);

        char recvline[10000];
        atm_send(atm, sendline, strlen(sendline));
        n = atm_recv(atm,recvline,10000);
        recvline[n] = 0;

        if (!strcmp(recvline, "missing")) {
            printf("No such user\n\n");
            return;
        }
        char card[257];
        char temp[10];
        char temp2[10];
        strcpy(card, token);
        strcpy(temp, "./");
        strcpy(temp2, ".card");
        strcat(temp, card);
        strcat(card, temp2);

        FILE *fptr = fopen(card, "r");
        if (fptr == NULL) {
            printf("Unable to access %s's card\n\n", token);
            return;
        }

        printf("PIN? ");
        char user_pin[100];
        regex_t regex_expression;
        regcomp(&regex_expression, "^[0-9]{4}$", REG_EXTENDED);

        if (fgets(user_pin, 100, stdin) == NULL || !regexec(&regex_expression, user_pin, 0, NULL, 0)) {
            printf("Not authorized\n\n");
            return;
        }
        regfree(&regex_expression);

        sprintf(sendline, "pin %s %s", token, user_pin);
        atm_send(atm, sendline, strlen(sendline));
        n = atm_recv(atm,recvline,10000);
        recvline[n] = 0;

        // If the bank responds with "error" deny entry to the account.
        if (!strcmp(recvline, "incorrect")) {
            printf("Not authorized\n\n");
            return;
        }

        printf("Authorized\n");
        atm->logged_in = 1;

        char *user_copy = (char *) malloc(strlen(token + 1));
        strcpy(user_copy, token);

        atm->user = user_copy;

    } else if (!strcmp("withdraw", token)) {
        token = strtok(NULL, " \t\n");
        int amount;

        if (!atm -> logged_in) {
            printf("No user logged in\n\n");
            return;
        }
        
        if (token == NULL) {
            printf("Usage: withdraw <amt>\n\n");
            return;
        } else {
            int found_amount = 0;
            while (token != NULL) {
                if (isdigit(token[0])) {
                    amount = atoi(token);
                    found_amount = 1;
                    break;
                }
                token = strtok(NULL, " \t\n");
            }
            // add later: if (strtok(NULL, " \t\n") != NULL)
            if (!found_amount || amount < 1) {
                printf("Usage: withdraw <amt>\n\n");
                return;
            } else {
                int n;
                int bank_amount;
                char sendline[1000];
                sprintf(sendline, "balance %s", atm -> user);

                char recvline[10000];
                atm_send(atm, sendline, strlen(sendline));
                n = atm_recv(atm,recvline,10000);
                recvline[n] = 0;
                bank_amount = atoi(recvline);

                if (bank_amount < amount) {
                    printf("Insufficient funds\n\n");
                    return;
                } else {
                    sprintf(sendline, "withdraw %s %s", atm -> user, token);
                    atm_send(atm, sendline, strlen(sendline));
                    n = atm_recv(atm, recvline, 10000);
                    recvline[n] = 0;
                    printf("$%s dispensed\n\n", token);
                }
            }
        }
    } else if (!strcmp("balance", token)) {
        token = strtok(NULL, " \t\n");


        if (!atm -> logged_in) {
            printf("No user logged in\n\n");
            return;
        }

        if (token == NULL || !strcmp(token, atm->user)) {
            int amount_size;
            int bank_amount;
            char sendline[1000];
            sprintf(sendline, "balance %s", atm->user);

            char recvline[10000];
            atm_send(atm, sendline, strlen(sendline));
            amount_size = atm_recv(atm,recvline,10000);
            recvline[0] = 0;
            printf("$%s\n\n", recvline);
        } else {
            printf("Usage: balance <amt>\n\n");
            return;
        }

    } else if (!strcmp("end-session", token)) {
        if (!atm->logged_in){
            printf("No user logged in\n\n");
            return;
        }

        atm->user = NULL;
        atm->logged_in = 0;
        printf("User logged out\n\n");

    } else {
        printf("Invalid command\n\n");
    }
}
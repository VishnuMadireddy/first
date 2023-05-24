#ifndef unix
#define WIN32
#include <windows.h>
#include <winsock.h>
#else
#define closesocket close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#define PROTOPORT 8888
extern int errno;
char localhost[] = "localhost";

const int BUFFERSIZE = 1024;

void uploadFile(unsigned int, char *);
void deleteFile(unsigned int, char *);
void renameFile(unsigned int, char *);
void downloadFile(unsigned int, char *);
void menu(unsigned int);

int main(int argc, char *argv[])
{

    struct hostent *ptrh;
    struct protoent *ptrp;
    struct sockaddr_in sad;
    int sd;
    int port;
    char *host;
#ifdef WIN32
    WSADATA wsaData;
    WSAStartup(0x0101, &wsaData);
#endif
    memset((char *)&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;

    if (argc > 2)
    {
        port = atoi(argv[2]);
    }
    else
    {
        port = PROTOPORT;
    }
    if (port > 0)
        sad.sin_port = htons((u_short)port);
    else
    {
        fprintf(stderr, "bad port number please chceck  %s\n", argv[2]);
        exit(1);
    }

    if (argc > 1)
    {
        host = argv[1];
    }
    else
    {
        host = localhost;
    }

    ptrh = gethostbyname(host);
    if (((char *)ptrh) == NULL)
    {
        fprintf(stderr, "Invalid host identifier please check: %s\n", host);
        exit(1);
    }
    memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

    if (((int)(ptrp = getprotobyname("tcp"))) == 0)
    {
        fprintf(stderr, "cannot map \"tcp\" to protocol number");
        exit(1);
    }

    sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sd < 0)
    {
        fprintf(stderr, "Unable to create socket connection\n");
        exit(1);
    }

    if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0)
    {
        fprintf(stderr, "connection failed\n");
        exit(1);
    }

    menu(sd);

    closesocket(sd);

    exit(0);
}

void menu(unsigned int socket)
{
    int choice = 0;
    char *allParams;
    char *input;

    allParams = (char *)malloc(BUFFERSIZE);
    input = (char *)malloc(256);

    while (choice != 5)
    {

        printf("Choose Action to performe:\n1 - DOWNLOAD_FILE\n2 - DELETE_FILE\n3 - RENAME_FILE\n4 - EXIT\n");

        fgets(input, 3, stdin);
        choice = atoi(input);

        switch (choice)
        {
        case 1:

            strcpy(allParams, "DOWNLOAD_FILE ");

            printf("Enter filename to download from the server: ");
            fgets(input, 256, stdin);

            if (strlen(input) > 0 && input[strlen(input) - 1] == '\n')
                input[strlen(input) - 1] = '\0';

            strcat(allParams, input);

            downloadFile(socket, allParams);
            break;
        case 2:

            strcpy(allParams, "DELETE_FILE ");

            printf("Enter filename to delete file in the server: ");
            fgets(input, 256, stdin);

            if (strlen(input) > 0 && input[strlen(input) - 1] == '\n')
                input[strlen(input) - 1] = '\0';

            strcat(allParams, input);

            deleteFile(socket, allParams);
            break;
        case 3:

            strcpy(allParams, "RENAME_FILE ");

            printf("Enter orginal filename: ");
            fgets(input, 256, stdin);

            if (strlen(input) > 0 && input[strlen(input) - 1] == '\n')
                input[strlen(input) - 1] = '\0';

            strcat(allParams, input);
            strcat(allParams, " ");

            printf("Enter new filename: ");
            fgets(input, 256, stdin);

            if (strlen(input) > 0 && input[strlen(input) - 1] == '\n')
                input[strlen(input) - 1] = '\0';

            strcat(allParams, input);

            renameFile(socket, allParams);
            break;
        }
    }

    free(allParams);
    free(input);
}

void deleteFile(unsigned int socket, char *params)
{
    char *buffer;
    buffer = (char *)malloc(BUFFERSIZE);

    write(socket, params, strlen(params) + 1);

    read(socket, buffer, BUFFERSIZE);

    printf("%s\n", buffer);

    free(buffer);
}

void renameFile(unsigned int socket, char *params)
{
    char *buffer;
    buffer = (char *)malloc(BUFFERSIZE);

    write(socket, params, strlen(params) + 1);

    read(socket, buffer, BUFFERSIZE);

    printf("%s\n", buffer);
    free(buffer);
}

void downloadFile(unsigned int socket, char *params)
{
    char *buffer;
    char *filename;
    int size;
    buffer = (char *)malloc(BUFFERSIZE);

    write(socket, params, strlen(params) + 1);

    read(socket, buffer, sizeof(int));
    size = (int)buffer[0];

    strcpy(buffer, "ACK");
    write(socket, buffer, strlen(buffer) + 1);

    if (size == -1)
    {

        read(socket, buffer, BUFFERSIZE);
    }
    else
    {

        filename = strtok(params, " ");
        filename = strtok(NULL, " ");

        FILE *file = fopen(filename, "w");

        if (file == NULL)
        {
            printf("unable to create %s\n", filename);

            free(buffer);
            buffer = (char *)malloc(size);
            read(socket, buffer, size);
        }
        else
        {

            free(buffer);
            buffer = (char *)malloc(size);
            read(socket, buffer, size);

            fwrite(buffer, size, 1, file);

            fclose(file);
        }

        free(buffer);
        buffer = (char *)malloc(BUFFERSIZE);
        strcpy(buffer, "ACK");
        write(socket, buffer, strlen(buffer) + 1);

        read(socket, buffer, BUFFERSIZE);
    }

    printf("%s\n", buffer);
    free(buffer);
}


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define COMMAND_SIZE 100
#define MAXIMUM_FILE 100
#define FILE_NAMES 100
#define MAXIMUM_CLIENTS 3

char **reNames;
char **deleteName;
char **downloadName;

int renameName = 0,dName = 0,dwdName = 0;

int GetCommandRequest(char* request)
{
	char cmd[COMMAND_SIZE];
	strcpy(cmd, request);
	int x = 0;
	while(request[x] != ' ' && request[x] != '\0')
		x++;
	if(request[x] == '\0')
		return 4;
	else
	{
		strncpy(cmd, request, x-1);
		cmd[x] = '\0';
	}

	if(!strcmp(cmd, "DOWNLD")){
		return 1;
	}else if(!strcmp(cmd, "Delete")){
		return 2;
	}else if(!strcmp(cmd, "Rename")){
		return 3;
	}
	return 0;
}

bool  SendFileToClient(int sd, char* f_name)
{
	struct stat	obj;
	int f_desc, size_f;

	printf("Sending File\n");
	stat(f_name, &obj);
	f_desc = open(f_name, O_RDONLY);
	size_f = obj.st_size;
	write(sd, &size_f, sizeof(int));
	sendfile(sd, f_desc, NULL, size_f);
	printf("File sent successfully\n");
	return true;
}

void DownloadFile(char *f_name, int socket)
{
	char response_server[BUFSIZ];
	printf(" download request of clientt\n");

	strcpy(response_server, "downld");
	write(socket, response_server, strlen(response_server));
		
	SendFileToClient(socket, f_name);
}

char* GetArgumentFromRequest(char* request)
{
	char *arg = strchr(request, ' ');
	return arg + 1;
}

void *HandlingConnection(void *sd)
{
	int	option, f_desc, size_f;
	int socket = *(int*)sd;

	char reply[BUFSIZ], file_ext[BUFSIZ],response_server[BUFSIZ], request_client[BUFSIZ], f_name[BUFSIZ];
	char *data_buff;
	while(1)
	{	printf("\nWaiting for a command\n");
		int l = recv(socket, request_client, BUFSIZ, 0);
		if(l == 0){
			printf("client exited\n");
			return NULL;
		}else if(l == -1){
			printf("connection lost\n");
			return NULL;
		}
		request_client[l]='\0';
		printf("Command has Recieved %s\n",request_client );
		option = GetCommandRequest(request_client);
		switch(option)
		{
			case 1:
				strcpy(f_name, GetArgumentFromRequest(request_client));
				if(access(f_name, F_OK) != -1){
					int x = 0,flag = 0;
					for(x = 0; x < renameName; x++){
						if(strcmp(reNames[x], f_name) == 0){
							flag = 1;
							printf("%s file is under in renaming process\n", f_name);
							char* temp = "RENAME";
							write(socket, temp, 6);
						}
					}
					for(x = 0; x < dName; x++){
						if(strcmp(deleteName[x], f_name) == 0){
							flag = 1;
							printf("%s file is under in deleting process\n", f_name);
							char* temp = "DELETE";
							write(socket, temp, 6);
						}
					}
					if(flag == 0){
						downloadName[dwdName] = f_name;
						dwdName++;
						DownloadFile(f_name, socket);
						dwdName--;
					}
				}else{
					char* temp = "NOTEXT";
					write(socket, temp, 6);
				}
				break;
			case 2:
				strcpy(f_name, GetArgumentFromRequest(request_client));
				if(access(f_name, F_OK) != -1){
					int x = 0;
					int flag = 0;
					for(x = 0; x < renameName; x++){
						if(strcmp(reNames[x], f_name) == 0){
							flag = 1;
							printf("%s file is under in renaming process \n", f_name);
							char* temp = "RENAME";
							write(socket, temp, 6);
						}
					}
					for(x = 0; x < dwdName; x++){
						if(strcmp(downloadName[x], f_name) == 0){
							flag = 1;
							printf("%s file is under in downloading process\n", f_name);
							char* temp = "DOWNLD";
							write(socket, temp, 6);
						}
					}
					if(flag == 0){
						deleteName[dName] = f_name;
						dName++;
						if(remove(f_name) == 0){
							printf("File removed successfully\n");
							char *temp = "delete";
							write(socket, temp, 6);
						}else{
							printf("Unable to remove file\n");
							char *temp = "EXISTS";
							write(socket, temp, 6);
						}
						dName--;
					}
				}else{
					char *temp = "NOTEXT";
					write(socket, temp, 6);
				}
				break;
			case 3:
				strcpy(file_ext, GetArgumentFromRequest(request_client));
				char temp[100];
				int x = 0;
				while(file_ext[x] != ' '){
				       temp[x] = file_ext[x];
				       x++;
				}	       
				temp[x] = '\0';
				x++;
				char file_n[100];
				int y = 0;
				while(file_ext[x] != '\0'){
					file_n[y] = file_ext[x];
					y++;x++;
				}
				file_n[y] = '\0';
				if(access(temp, F_OK) != -1){
					int k = 0,flag = 0;
					for(k = 0; k < dName; k++){
						if(strcmp(deleteName[k], f_name) == 0){
							flag = 1;
							printf("%s file is under deleting process\n", f_name);
							char* temp = "DELETE";
							write(socket, temp, 6);
						}
					}	
					for(k = 0; k < dwdName; k++){
						if(strcmp(downloadName[k], f_name) == 0){
							flag = 1;
							printf("%s file is under downloading process\n", f_name);
							char* temp = "DOWNLD";
							write(socket, temp, 6);
						}
					}
					if(flag == 0){
						reNames[renameName] = temp;
						renameName++;
						if(rename(temp, file_n ) == 0){
							printf("File renamed successfully\n");
							char *temp = "rename";
							write(socket, temp, 6);
						}else{
							printf("Unable to remove file\n");
							char *temp = "EXISTST";
							write(socket, temp, 6);
						}
						renameName--;
					}
				}else{
					char *temp = "NOTEXT";
					write(socket, temp, 6);
				}
				break;
		}
	}
	free(sd);   
	return 0;
}

int main(int argc, char **argv)
{
	
	if(argc!=2){
		printf("Arguments passed are not valid\n");
		return 0;
	}
	
	int x = 0;
	reNames = (char **)malloc(sizeof(char *) * MAXIMUM_CLIENTS);
	deleteName = (char **)malloc(sizeof(char *) * MAXIMUM_CLIENTS);
	downloadName = (char **)malloc(sizeof(char *) * MAXIMUM_CLIENTS);

	for(x = 0; x < MAXIMUM_CLIENTS; x++){
		reNames[x] = (char *)malloc(sizeof(char) * FILE_NAMES);
		deleteName[x] = (char *)malloc(sizeof(char) * FILE_NAMES);
		downloadName[x] = (char *)malloc(sizeof(char) * FILE_NAMES);
	}

	int sd, socket_client[MAXIMUM_CLIENTS], *new_s, client_no = 0;
	int c = sizeof(struct sockaddr_in);
	struct  sockaddr_in	ftp_server, clientt;

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1)
	{
		perror("Error occured while creating socket");
		return 1;
	}
	int SERVER_PORT = atoi(argv[1]);
	ftp_server.sin_family = AF_INET;
	ftp_server.sin_addr.s_addr = INADDR_ANY;
	ftp_server.sin_port = htons(SERVER_PORT);

	if (bind(sd, (struct sockaddr *)&ftp_server, sizeof(ftp_server)) < 0)
	{
		perror("Bind failed to achive");
		return 1;
	}

	listen(sd, MAXIMUM_CLIENTS);

	while (socket_client[client_no] = accept(sd, (struct sockaddr *)&clientt, (socklen_t*)&c))
	{
		pthread_t s_th;
		new_s = malloc(1);
		*new_s = socket_client[client_no];
		client_no++;
		printf("client %d", client_no);
		pthread_create(&s_th, NULL, HandlingConnection, (void*) new_s);
	}

	if (socket_client<0)
	{
		perror("Failed to accept");
		return 1;
	}
	pthread_exit(NULL);

	return 0;
}








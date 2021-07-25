#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <pthread.h>

#include <stdlib.h>   
#include <unistd.h>   
#include <fcntl.h>   
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netdb.h>

#include <time.h>

char* now(){
 
	time_t timep; 
	time (&timep); 
	printf("%s\n",asctime(gmtime(&timep))); 
	return asctime(gmtime(&timep));
}
#define BUF_SIZE 500

int file_exists(char* filename){
	return (access(filename, F_OK) == 0);
}

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_accept  = PTHREAD_MUTEX_INITIALIZER;

typedef struct product_list{
	char name[256];
	struct product_list* next;
}product_list;

typedef struct quote_client{
	int num;
	char name[256];
	product_list pro_list;
	struct quote_client* next;
} quote_client;

typedef struct quote_client_list{
	quote_client* head;
	int count;
}quote_client_list;

typedef struct req{
	char stock_name[256];
	int client_num;
	struct req* next;
}req;

typedef struct req_list{
	req* head;
	int count;
}req_list;


quote_client_list QClist;
req_list R_list;

void add_req(req_list* list, int num, char name[256]){
	req** indirect = &list -> head;
	while(*indirect != NULL)
		indirect = &(*indirect) -> next;
	*indirect = (req*)malloc(sizeof(req));
	(*indirect) -> next = NULL;
	(*indirect) -> client_num = num;
	strcpy((*indirect) -> stock_name, name); 
	
	pthread_mutex_lock(&mutex1);
	list -> count++;
	pthread_mutex_unlock(&mutex1);
}

void remove_req(req_list* list, req* target){
	req** indirect = &list -> head;
	while(*indirect != target)
		indirect = &(*indirect) -> next;
	*indirect = target -> next;

	pthread_mutex_lock(&mutex1);
	list -> count--;
	pthread_mutex_unlock(&mutex1);

}

int Count(quote_client_list* list){
	int count = 0;
	quote_client* p = list -> head;
	while(p != NULL){
		pthread_mutex_lock(&mutex1);
		count++;
		pthread_mutex_unlock(&mutex1);
		p = p -> next;
	}
	return count;
}

void add(quote_client_list* list ,int client_num){
	quote_client** p = &list -> head;
	while((*p) != NULL){
		p = &(*p) -> next;
	}

	pthread_mutex_lock(&mutex1);
	(*p) = (quote_client*)malloc(sizeof(quote_client) * 1);
	(*p) -> num = client_num; 
	(*p) -> next = NULL;
	pthread_mutex_unlock(&mutex1);

	printf("tail\n");
	
	list -> count = Count(list);
}

void remove_list_node(quote_client_list* list, quote_client* target){
	quote_client** indirect = &list -> head;
	
	while(*indirect != target)
		indirect = &(*indirect) -> next;
	pthread_mutex_lock(&mutex1);
	*indirect = target -> next;
	pthread_mutex_unlock(&mutex1);

	list -> count = Count(list);
}


void SendAllClients(quote_client_list* list, char* msg, req_list* Rlist){
	quote_client* p = list -> head; 

	while(p != NULL){
		long int err;

		//history tick
/*		if (strlen(msg) > 3){
			if (strncmp(msg, "MDS", 3) != 0 && Rlist -> count > 0){
				req** p = &Rlist -> head;
				while(*p != NULL)
				{
					err = send((*p) -> client_num, msg, strlen(msg), MSG_NOSIGNAL);
					printf("%d  -> %ld history\n", Rlist -> count, err);
					if (err == -1)
						remove_req(Rlist, *p);
					if (strstr(msg, "history end:TXF") != NULL){
						printf("delete\n");
						remove_req(Rlist, (*p));
					}
					if (*p != NULL)
						p = &(*p) -> next;
					req* x = Rlist -> head;
					while(x != NULL){
						printf("%d (%s) -> ", x -> client_num, x -> stock_name);
						x = x -> next;
					}

					//p = p -> next;
					//continue;
				}
				return;
			}
		}
*/		printf("normal\n");
		err = send(p -> num, msg, strlen(msg), MSG_NOSIGNAL);
		if (err == -1)
			remove_list_node(list, p);
		printf("%d  -> %ld\n", list -> count, err);	
		p = p -> next;
	}
}


int server_socket, client, server_id;

void* Transfer (void* pa){
//	int client_socket = *(int*) pa;
	char msg[1024 * 1024 * 4] = "";
	
	while(1){
	//	scanf("%s", msg);
	//	send(client_socket, msg, sizeof(msg), 0);
		recv(server_id, msg, sizeof(msg), 0);

		if (strcmp(msg, "") != 0){
			//printf("recv:%d %s  client id:%d\n", server_id, msg, client_socket);
		//	printf("%s\n", msg);
//			pthread_mutex_lock(&mutex1);
			pthread_mutex_lock(&mutex_accept);
			SendAllClients(&QClist, msg, &R_list);
			pthread_mutex_unlock(&mutex_accept);			
			if (QClist.head == NULL)
				printf("%s\n", msg);
//printf("%ld\n",send(client, msg, strlen(msg), MSG_NOSIGNAL));
//			pthread_mutex_unlock(&mutex1);
		}
		memset(msg, 0, strlen(msg));
	}
}

void FileWrite(char path[256], char msg[1024 * 1024]){
	FILE* fp = fopen(path, "a");
	fprintf(fp, msg, strlen(msg) + 1);
	fclose(fp);
}

void FileRead(char path[256]){
	printf("hello\n");

	FILE* fp = fopen(path, "r+");
	char buff[1024] = "";
	char str[1024 * 1024];
//	fscanf(fp, "%s", buff);
	while(fgets(buff, 256, (FILE*)fp) != NULL){
		strcpy(&str[strlen(str)], buff);
		printf("%s", str);
	}fclose(fp);
//	return buff;
}

int main(){
//check file
	printf("HELLO\n");

//	FileWrite("history.txt", "second.\n");
//	FileRead("history.txt");	

	//char server_message[1024] = "聯絡資訊hisopfun@gmail.com歡迎乾爹們斗內贊助 ^_^ https://p.ecpay.com.tw/9B8FEA9      祝各位操盤順利\n";
	R_list.count = 0;
	//create the server socket
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(12002);
	server_address.sin_addr.s_addr = INADDR_ANY;

//	//check ip-------------------------------
//	// hints 參數，設定 getaddrinfo() 的回傳方式
//	struct addrinfo hints; 
//
//	// getaddrinfo() 執行結果的 addrinfo 結構指標
//	struct addrinfo *result; 
//
//	// 以 memset 清空 hints 結構
//	memset(&hints, 0, sizeof(struct addrinfo));
//	hints.ai_family = AF_UNSPEC; // 使用 IPv4 or IPv6
//	hints.ai_socktype = SOCK_STREAM; // 串流 Socket
//	hints.ai_flags = AI_NUMERICSERV; // 將 getaddrinfo() 第 2 參數 (PORT_NUM) 視為數字
//	    
//	getaddrinfo(NULL, "12002", &hints, &result);
//	//check p--------------------------------


	
	bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

	listen(server_socket, 5);
	
	char client_message[256] = "";
	
	while(1){

		int client_socket = accept(server_socket, NULL, NULL);
		if (fork() == 0){

			//Recive client Request
			recv(client_socket, client_message, sizeof(client_message), 0);
		
			//Write to Log
			char _now[256]; 
			strcpy(_now, now());
			strcpy(&_now[strlen(_now) - 1], "\0");
			FileWrite("./record.TXT", _now);
			FileWrite("./record.TXT", ",");				
			FileWrite("./record.TXT", client_message);	
			FileWrite("./record.TXT", "\n");	

			struct stat filestat;
			int numbytes;
			char buf[100000];
			FILE *fp;	

			//Sending file
			printf("Sending msg\n");
			fp = fopen("./msg.TXT", "rb");
			while(!feof(fp)){
				numbytes = fread(buf, sizeof(char), sizeof(buf), fp);
				printf("%d ", numbytes);
				numbytes = write(client_socket, buf, numbytes);
			}
			printf("Msg Done\n");

			
			if (strlen(client_message) >= 17){
	
				//Absolute Path
				char str[1024] = "./tick/";
				strncpy(&str[strlen(str)], &client_message[7], 10);
				strcpy(&str[strlen(str)], "TXF.TXT");
				
				//Check if the data exist
				int exist = file_exists(str);
				printf("%s %d %s\n\n", client_message, exist, str);
		
				if (exist == 1) 
				{
					//Get file stat
					if ( lstat(str, &filestat) < 0){
						exit(1);
					}

					//Sending file
					printf("Sending ticks %d\n", file_exists(str));
					fp = fopen(str, "rb");
					while(!feof(fp)){
						numbytes = fread(buf, sizeof(char), sizeof(buf), fp);
						printf("%d ", numbytes);
						numbytes = write(client_socket, buf, numbytes);
					//	printf("%d ",numbytes);
					}
					write(client_socket, "DONE Tick", 9);
					printf("Tick Done\n");


					//Sending DayK
					printf("Sending DayK %d\n", file_exists("./tick/TX00.TXT"));
					fp = fopen("./tick/TX00.TXT", "rb");
					while(!feof(fp)){
						numbytes = fread(buf, sizeof(char), sizeof(buf), fp);
						numbytes = write(client_socket, buf, numbytes);
						printf("%d ",numbytes);
					}
					write(client_socket, "DONE DayK", 9);
					printf("Day DONE\n");

					//Sending Finish
					write(client_socket, "Finish", 6);

					printf("\n\n\n\n\n");
					return 0;
				}
	//			FileRead(str);
			}
	//			pthread_mutex_lock(&mutex1);
				//client = client_socket;
	//			pthread_mutex_unlock(&mutex1);
	//		}
			
			write(client_socket, "NO DATA", 7);
			printf("\n\n\n\n\n");
			memset(client_message, 0, strlen(client_message));
			return 0;
	//		pthread_mutex_unlock(&mutex_accept);
		}
		signal(SIGCHLD,SIG_IGN);
	}
		

	while(1);	
		
	/*
	recv(client_socket, &client_msg, sizeof(client_msg), 0) ;
	printf("%d %s\n", client_socket, client_msg);
	close(server_socket);i
	*/
	return 0;
}

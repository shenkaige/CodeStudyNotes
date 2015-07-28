#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>

#define SERVER_PORT 0
typedef enum {
	TRUE = 1, FALSE = 0
} Bool;

typedef enum {
	LOG_INFO, LOG_DEBUG, LOG_ERROR
} LOG_V;

#define LOG_I(msg,...) 	inner_log(0, 0, LOG_INFO, msg, ##__VA_ARGS__)
#define LOG_D(msg,...) inner_log(0, 0, LOG_DEBUG, msg, ##__VA_ARGS__)
#define LOG_E(msg,...) inner_log(__FILE__, __LINE__, LOG_ERROR, msg, ##__VA_ARGS__)

void handlerRequest(int req_sock, struct sockaddr_in* req_attr);

void inner_log(const char* filename, int line, LOG_V logv, const char* msg, ...) {
	if (msg == 0) {
		msg = "null";
	}
	if (filename != 0) {
		printf("at %s,line%d\n", filename, line);
	}
	switch (logv) {
	case LOG_INFO:
		printf("[INFO] ");
		break;
	case LOG_ERROR:
		printf("[ERROR] ");
		break;
	case LOG_DEBUG:
	default:
		printf("[DEBUG] ");
		break;
	}
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
	printf("\n");
}

void handlerRequest(int req_sock, struct sockaddr_in* req_addr) {
	printf("handler reqeust good\n");
//char msg[]="handlerRequest";
//send(req_sock,msg,sizeof(msg),0);
//close(req_sock);
//read data
	char buffer[2048];
	memset(buffer, 0, sizeof(buffer));
	int readCount = 0;
	printf("read req content:------>\n");
	while ((readCount = recv(req_sock, buffer, sizeof(buffer), 0)) > 0) {
		printf("%s", buffer);
		memset(buffer, 0, sizeof(buffer));
	}
	printf("\n<------\n");
//
	char header[] = "HTTP/1.0 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"\r\n";
	send(req_sock, header, strlen(header), 0);
//send(req_sock, buffer, strlen(buffer), 0);
}

int main(int argc, char* argv[]) {
//create server socket
//1,make socket
//2,bind socket to local port
//3,start listen port
//4,accept
	int server_s = socket(PF_INET, SOCK_STREAM, 0);
	if (server_s < 0) {
		LOG_E("open socket failed");
	}
	LOG_I("openg server socket fd:%d", server_s);
//make sock address
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
//addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(SERVER_PORT);
	socklen_t addr_len = (socklen_t) sizeof(addr);
//---bind
	if (bind(server_s, (struct sockaddr*) &addr, addr_len) < 0) {
		LOG_E("bind socket failed!");
	}
	getsockname(server_s, (struct sockaddr*) &addr, &addr_len);
	LOG_I("bind server port:%d", ntohs(addr.sin_port));
//--listen
	if (listen(server_s, 100) < 0) {
		LOG_E("listen failed!");
		exit(-1);
	}
//--accept
	int client_s;
	struct sockaddr_in client_s_addr;
	socklen_t client_s_len = sizeof(client_s_addr);
	pid_t pid;
	while (1) {
		LOG_I("server listening request...");
		client_s = accept(server_s, (struct sockaddr*) &client_s_addr,
				&client_s_len);
		if (client_s < 0) {
			LOG_E("accept error!");
			exit(-1);
		}
		if ((pid = fork()) < 0) {
			LOG_E("fork failed");
			exit(-1);
		}
		if (pid == 0) {
			//child
			LOG_I("child process start handler request");
			close(server_s);
			handlerRequest(client_s, &client_s_addr);
			close(client_s);
			LOG_I("child process work finish");
			exit(0);
		} else {
			//parent
			LOG_I("make child process pid:%d", pid);
			close(client_s);
		}
		memset(&client_s_addr, 0, sizeof(struct sockaddr_in));
	}
	return 0;
}

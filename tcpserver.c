#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/poll.h>
#include <sys/epoll.h>


#include <pthread.h>
 
#define MAXLNE  4096

#define POLL_SIZE	1024

//8m * 4G = 128 , 512
//C10k
void *client_routine(void *arg) { //

	int connfd = *(int *)arg;

	char buff[MAXLNE];

	while (1) {

		int n = recv(connfd, buff, MAXLNE, 0);
        if (n > 0) {
            buff[n] = '\0';
            printf("recv msg from client: %s\n", buff);

	    	send(connfd, buff, n, 0);
        } else if (n == 0) {
            close(connfd);
			break;
        }

	}

	return NULL;
}


int main(int argc, char **argv) 
{
    int listenfd, connfd, n;
    struct sockaddr_in servaddr;//Internet环境下的套接字的地址形式
    char buff[MAXLNE];
 
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
// int socket(int domain, int type, int protocol);return a file descriptor for the new socket, or -1 for errors;
// domain指定使用何种类型的地址，此函数中指的是IP地址
// type指建立哪种连接流，此函数中参数指的是TCP；
// protocol指定所用的传输协议的编号。通常不用管他，设为0即可。
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);//char *stererror(int errnum);通过errnum的错误代码来查询其错误的原因，然后返回该字符串的头指针；系统调用错误的编号存储在errnum中
        return 0;
    }
 
    memset(&servaddr, 0, sizeof(servaddr));//sizeof() return XX byte
    servaddr.sin_family = AF_INET;//协议族；在socket中它只能是AF_INET(IP协议族)
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//IP address in network byte order;htonl()将32位主机字符顺序转换成网络字符顺序
    servaddr.sin_port = htons(9999);//Port number(must is 网络字符顺序) htons()将数字转化为网络字符顺序
 

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
//把协议族中的一个特定地址赋给listenfd代表的socket。例如AF_INET就是把一个ipv4地址和端口号组合赋给socket。第三个参数对应的是套接字的地址的长度
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
 
    if (listen(listenfd, 10) == -1) {//成功返回0，失败返回-1；第一个参数即为要监听的socket套接字，第二个参数为相应socket可以排队的最大连接个数。socket()函数创建的socket默认是主动类型的
	//，listen()函数将socket变为被动类型，等待客户的连线请求
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
#if 0
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
		// 客户端调用了connect()函数之后，TCP服务器监听到这个请求后，就会调用accept()函数去接收请求
		// 此函数的第一个参数是服务器的socket套接字，第二个参数是用于返回客户端地址协议的指针，第三个参数是这个协议地址的长度
		// 如果函数成功，其返回值是一个由内核重新生成的全新的套接字，代表与返回客户的TCP连接

		// accept的第一个参数为服务器的socket描述字，是服务器开始调用socket()函数生成的，称为监听socket描述字；
		// 而accept函数返回的是已连接的socket描述字。一个服务器通常通常仅仅只创建一个监听socket描述字，它在该服
		// 务器的生命周期内一直存在。内核为每个由服务器进程接受的客户连接创建了一个已连接socket描述字，当服务器完
		// 成了对某个客户的服务，相应的已连接socket描述字就被关闭
        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    printf("========waiting for client's request========\n");
    while (1) {

        n = recv(connfd, buff, MAXLNE, 0);
		//接收由远端主机经指定的socket传来的数据，并把数据存到由参数buff只向的内存空间，第三个参数为可接收数据的最大长度，第四个参数一般设为0；
        if (n > 0) {
            buff[n] = '\0';
            printf("recv msg from client: %s\n", buff);

	    	send(connfd, buff, n, 0);
		// 将数据由指定的socket传送给对方的主机
        } else if (n == 0) {
            close(connfd);
        }
        
        //close(connfd);
    }

#elif 0


    printf("========waiting for client's request========\n");
    while (1) {

		struct sockaddr_in client;
	    socklen_t len = sizeof(client);
	    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
	        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
	        return 0;
	    }

        n = recv(connfd, buff, MAXLNE, 0);
        if (n > 0) {
            buff[n] = '\0';
            printf("recv msg from client: %s\n", buff);

	    	send(connfd, buff, n, 0);
        } else if (n == 0) {
            close(connfd);
        }
        
        //close(connfd);
    }

#elif 0

	while (1) {

		struct sockaddr_in client;
	    socklen_t len = sizeof(client);
	    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
	        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
	        return 0;
	    }

		pthread_t threadid;
		pthread_create(&threadid, NULL, client_routine, (void*)&connfd);//为何是void *？？

    }

#elif 0

	// 
	fd_set rfds, rset, wfds, wset;

	FD_ZERO(&rfds);//对rds清零
	FD_SET(listenfd, &rfds);//将这个套接字加入rfds

	FD_ZERO(&wfds);

	int max_fd = listenfd;

	while (1) {

		rset = rfds;
		wset = wfds;

		int nready = select(max_fd+1, &rset, &wset, NULL, NULL);


		if (FD_ISSET(listenfd, &rset)) { //

			struct sockaddr_in client;
		    socklen_t len = sizeof(client);
		    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {//block
		        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
		        return 0;
		    }

			FD_SET(connfd, &rfds);

			if (connfd > max_fd) max_fd = connfd;

			if (--nready == 0) continue;

		}

		int i = 0;
		for (i = listenfd+1;i <= max_fd;i ++) {

			if (FD_ISSET(i, &rset)) { // 

				n = recv(i, buff, MAXLNE, 0);//block
		        if (n > 0) {
		            buff[n] = '\0';
		            printf("recv msg from client: %s\n", buff);

					FD_SET(i, &wfds);

					//reactor
					//send(i, buff, n, 0);
		        } else if (n == 0) { //

					FD_CLR(i, &rfds);
					//printf("disconnect\n");
		            close(i);
					
		        }
				if (--nready == 0) break;
			} else if (FD_ISSET(i, &wset)) {

				send(i, buff, n, 0);//如何发生；为何放在这里？
				FD_SET(i, &rfds);
			
			}

		}
		

	}

#elif 0


	struct pollfd fds[POLL_SIZE] = {0};
	fds[0].fd = listenfd;
	fds[0].events = POLLIN;

	int max_fd = listenfd;
	int i = 0;
	for (i = 1;i < POLL_SIZE;i ++) {
		fds[i].fd = -1;
	}

	while (1) {

		int nready = poll(fds, max_fd+1, -1);

	
		if (fds[0].revents & POLLIN) {

			struct sockaddr_in client;
		    socklen_t len = sizeof(client);
		    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
		        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
		        return 0;
		    }

			printf("accept \n");
			fds[connfd].fd = connfd;
			fds[connfd].events = POLLIN;

			if (connfd > max_fd) max_fd = connfd;

			if (--nready == 0) continue;
		}

		//int i = 0;
		for (i = listenfd+1;i <= max_fd;i ++)  {

			if (fds[i].revents & POLLIN) {
				
				n = recv(i, buff, MAXLNE, 0);
		        if (n > 0) {
		            buff[n] = '\0';
		            printf("recv msg from client: %s\n", buff);

					send(i, buff, n, 0);
		        } else if (n == 0) { //

					fds[i].fd = -1;

		            close(i);
					
		        }
				if (--nready == 0) break;

			}

		}

	}

#else

	//poll/select --> 
	// epoll_create 
	// epoll_ctl(ADD, DEL, MOD)
	// epoll_wait

	int epfd = epoll_create(1); //int size里面的参数大于0就行

	struct epoll_event events[POLL_SIZE] = {0};
	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.fd = listenfd;

	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

	while (1) {

		int nready = epoll_wait(epfd, events, POLL_SIZE, 5);
		if (nready == -1) {
			continue;
		}

		int i = 0;
		for (i = 0;i < nready;i ++) {

			int clientfd =  events[i].data.fd;
			if (clientfd == listenfd) {

				struct sockaddr_in client;
			    socklen_t len = sizeof(client);
			    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
				// 客户端调用了connect()函数之后，TCP服务器监听到这个请求后，就会调用accept()函数去接收请求
				// 此函数的第一个参数是服务器的socket descriptor，第二个参数是用于返回客户端地址协议的指针，第三个参数是这个协议地址的长度
				// 如果函数成功，其返回值是一个由内核重新生成的全新的套接字，代表与返回客户的TCP连接

				// accept的第一个参数为服务器的socket描述字，是服务器开始调用socket()函数生成的，称为监听socket描述字；
				// 而accept函数返回的是已连接的socket描述字。一个服务器通常通常仅仅只创建一个监听socket描述字，它在该服
				// 务器的生命周期内一直存在。内核为每个由服务器进程接受的客户连接创建了一个已连接socket描述字，当服务器完
				// 成了对某个客户的服务，相应的已连接socket描述字就被关闭
			        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
			        return 0;
			    }

				printf("accept\n");
				ev.events = EPOLLIN;
				ev.data.fd = connfd;
				epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);

			} else if (events[i].events & EPOLLIN) {

				printf("recv\n");
				n = recv(clientfd, buff, MAXLNE, 0);
		        if (n > 0) {
		            buff[n] = '\0';
		            printf("recv msg from client: %s\n", buff);

					send(clientfd, buff, n, 0);
		        } else if (n == 0) { //


					ev.events = EPOLLIN;
					ev.data.fd = clientfd;

					epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, &ev);

		            close(clientfd);
					
		        }

			}

		}

	}
	

#endif
 
    close(listenfd);
    return 0;
}


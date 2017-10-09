#include "Epoller.h"
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <stddef.h>

//#include "Logger.h"

Epoller::Epoller(int max_load, int max_epoll_event){
	this->epollfd_=epoll_create(max_load);
	if(this->epollfd_==0)
		printf("Epoller::Epoller - fail to epoll_create.\n");
	this->max_epoll_event_=max_epoll_event;
}

Epoller::~Epoller(){
	if(this->epollfd_!=0){
		close(this->epollfd_);
		this->epollfd_=0;
	}
}

void Epoller::add(int sockfd){
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET;//EPOLLONESHOT;// | EPOLLERR | EPOLLHUP;
	event.data.fd = sockfd;
	if(epoll_ctl(this->epollfd_, EPOLL_CTL_ADD, sockfd, &event) == -1)
		printf("fail to add sockfd to epoll, sockfd=%d\n",sockfd);
}

void Epoller::remove(int sockfd){
	struct epoll_event event;
	if(epoll_ctl(this->epollfd_, EPOLL_CTL_DEL, sockfd, &event) == -1)
		printf("fail to remove sockfd from epoll, sockfd=%d\n",sockfd);
}

void Epoller::getSockfds(list<int> &sockfds){
	sockfds.clear();
	struct epoll_event events[this->max_epoll_event_];
	int nfd = epoll_wait(this->epollfd_, events, this->max_epoll_event_, -1);
	if(nfd == -1){
		if (errno != EINTR)
			printf("epoll wait failed: %d\n", errno);
		return;
	}
	for(int i = 0; i < nfd; i++)
		sockfds.push_back(events[i].data.fd);
}

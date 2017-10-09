//encoding: utf-8
//author: yangsl
//last modified: 2017-01-22

//Epoller管理epoll，方法add和remove为增加、删除管理的sockfd
//方法getSockfds获取有接收请求或者是断开的sockfds，需要专门开一个线程来处理。
//Epoller需要两个配置参数：max_load和max_epoll_event，前者为管理的最大sockfd数，后者为getSockfds返回的最大sockfd数。

#pragma once

#include <list>
#include<stdio.h>
using std::list;

class Epoller {
public:
	Epoller(int max_load,int max_epoll_event);
	~Epoller();
	void add(int sockfd);
	void remove(int sockfd);
	void getSockfds(list<int> &sockfds);

private:
	int epollfd_,max_epoll_event_;
};


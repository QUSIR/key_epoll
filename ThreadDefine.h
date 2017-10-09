#pragma once

#ifdef _WIN32
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>

	#define THREAD_ID DWORD
	#define CREATE_THREAD(id,thread,argv) (CreateThread(NULL,0,thread,argv,0,id)!=0)
	#define THREAD_TYPE DWORD WINAPI
	#define SLEEP(n) Sleep(n*1000)
#else
	#include <pthread.h>
	#define THREAD_ID pthread_t
//	#define CREATE_THREAD_PREPARE 	pthread_attr_t attr;pthread_attr_init(&attr);pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED)
	#define CREATE_THREAD(id,thread,argv) (pthread_create(id,NULL,thread,argv)==0)
	#define THREAD_TYPE void*
	#define SLEEP(n) sleep(n)
#endif

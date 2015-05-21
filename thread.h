// Author:onebird
// Date 2006.08.22
// For Cross Thread Pool Class
//////////////////////////////////////////////////////////////////////
#ifndef _THREAD_H_
#define _THREAD_H_
#include "mthread.h"
#include "mutex.h"
//#include <pthread.h>

#include "threadjob.h"
#include "threadpool.h"


class ThreadPool;
class Thread
{
public:
////	static long get_threadcount(bool finc, bool finit = false);
	bool Create();
	inline  ThreadPool * get_threadpool() {
		return m_pthreadPool;
	}
	void run();
	Thread( ThreadPool * pPool );
	virtual ~Thread();

protected:
	//unsigned int m_threadid;
	//pthread_t m_thread;
	ThreadPool * m_pthreadPool ;
	MThread m_thread;
	MMutex  m_mutex;
	bool m_fquit;
};

#endif 

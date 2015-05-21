// Thread.cpp: implementation of the Thread class.
//
//////////////////////////////////////////////////////////////////////

#include "thread.h"
#ifndef  WIN32
#include <unistd.h>
#endif
unsigned long  __thread_proc(void *p) {
	((Thread*)p)->run();
	return 0;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Thread::Thread( ThreadPool *  pPool )
{
	m_fquit = false;
	// bool Create(unsigned long (*start_address)(void*), void* p)
	//m_thread.Create( __thread_proc, this);
	m_pthreadPool = pPool ;
	

}

bool Thread::Create(){
     return m_thread.Create( __thread_proc, this);
}

Thread::~Thread()
{
	while (!m_fquit) 
	{ 
		MThread::Sleep(80); 
	}
	
}

void Thread::run()
{
	while (true) {

	//pthread_cond_wait(get_threadpool().get_object(), &m_mutex);
		
		if (get_threadpool()->is_stop() ) {
			break;
		}
		if (get_threadpool()->is_pause() ) {
			MThread::Sleep(3000);
			continue;
		}	
		ThreadJobPtr pjob = get_threadpool()->get_next();
		while (pjob != NULL) {
			pjob->execute();
			if(pjob->IsRetJob()){
				get_threadpool()->add_done(pjob);		
			}
			pjob = get_threadpool()->get_next();
		}
	}
	m_fquit = true;
}


//Thread* Thread::create_thread(  )
//{
//	long threads = get_threadcount(false);
//	if (threads < ->get_threadcount()) {
//		Thread* pthread = new Thread(pPool);
//		get_threadpool().add_thread(pthread);
//		get_threadcount(true);
//		return pthread;
//	}
//	return NULL;
//}

//long Thread::get_threadcount(bool finc, bool finit)
//{
//	static long nthreads;
//	if (finit)
//		nthreads = 0;
//	if (finc) {
//		nthreads++;
//	}
//	return nthreads;
//}


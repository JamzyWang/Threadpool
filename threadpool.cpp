#include "threadjob.h"
#include "threadpool.h"
#include <algorithm>
#include "thread.h"
#include <stdio.h>

void delete_thread(Thread* pthread) { 	delete pthread; }



ThreadPool::ThreadPool(JobPool * jobPoolPtr)
{
	/*Thread::get_threadcount(false, true);*/
	m_fstop = false;
	m_bPause = false ;
	m_pjobs = jobPoolPtr;
	
}

ThreadPool::~ThreadPool()
{
	if(m_pjobs != NULL){
		delete m_pjobs ;
		m_pjobs = NULL ;
	}
}

void ThreadPool::stop() {
	
	m_fstop = true;
	m_mutex.Lock();
	size_t jobSize = m_pjobs->size(); //清除 任务池中的优先队列
	for( size_t k = 0 ; k< jobSize ; k++){
		ThreadJobPtr pjob = m_pjobs->front();
		//if(pjob != NULL){
		//	remove_job(pjob);
		//}
		m_pjobs->pop();
	}
	


	for(size_t i = 0 ; i < jobSize ; i++){
		m_signal.Signal() ; //temp code 需要给MSemaphore 类中封装 boradcast 	
	}
	std::for_each(m_threads.begin(), m_threads.end(), delete_thread);
	m_threads.erase(m_threads.begin(), m_threads.end());

	m_mutex.Unlock();	
}

void ThreadPool::pause(){
	m_bPause  = true ;

}
bool ThreadPool::is_pause(){
	bool b = m_bPause;
	return b ;
}

void ThreadPool::un_pause(){
	
	m_bPause = false ;
}

void ThreadPool::add_thread(Thread *pthread) {
	
        m_mutex.Lock();	
	m_threads.push_back(pthread);
	m_mutex.Unlock();
}

void ThreadPool::set_threadcount(int nthreads) {
	
	m_nthreadcount = nthreads;
	for (int index = 0; index < nthreads; ++index) {
		Thread  * pThread = new Thread(this);;
		if ( pThread == NULL)
		{
			continue ;
		}

		if(!pThread->Create()){
			printf("ERROR:: create thread faluse\n");
		        delete pThread;
			continue;	

		}
		add_thread(pThread);
	}
}


// Author:onebird
// Date 2006.08.22
// For Cross Thread Pool Class
//////////////////////////////////////////////////////////////////////

#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_
#include <queue>
#include <vector>
#include "mthread.h"
#include "mutex.h"
#include <assert.h>
#include <boost/regex.hpp>
#include "threadjob.h"
//#include <unistd.h>

class Thread;
class ThreadPool;
typedef boost::shared_ptr<ThreadPool> ThreadPoolPtr ;
class ThreadPool  
{
public:

	void pause();
	bool is_pause();
	void un_pause();

	inline bool is_stop() { return m_fstop; }
	void add_thread(Thread* pthread);
	inline long get_threadcount() {return m_nthreadcount;}
	
	inline long get_jobcount() 
	{
		m_mutex.Lock();
		int count = m_pjobs->size();
		m_mutex.Unlock();
		return count;
	}

	void stop();
	void set_threadcount(int nthreads);

	inline void add_job(ThreadJobPtr pjob) 
	{
		if (m_fstop) 
		{
			//delete pjob;
			return;
		}
		m_mutex.Lock();
		m_pjobs->push(pjob);
		m_mutex.Unlock();
		m_signal.Signal();
	}
	
	inline void add_done(ThreadJobPtr pjob) 
	{
	
		m_DoneJobsMutex.Lock();
		m_DoneJobs.push(pjob);
		m_DoneJobsMutex.Unlock();
		m_DoneJobsSignal.Signal();
	}

	inline ThreadJobPtr get_done(){
		ThreadJobPtr p;
		while(1){
			m_DoneJobsSignal.Wait(5000);	
			m_DoneJobsMutex.Lock();	
			if (m_DoneJobs.size() <= 0) 
			{
				m_DoneJobsMutex.Unlock();
				continue ;
				
			}else{
				p = m_DoneJobs.front();
				assert(p != NULL);
				m_DoneJobs.pop();
		
				m_DoneJobsMutex.Unlock();
				break;
				
			}
		}
		
		return p;


	}


	ThreadPool( JobPool * JobPoolPtr = new DefaultJobPool );
	virtual ~ThreadPool();
	
	inline ThreadJobPtr get_next() 
	{
		
		ThreadJobPtr p; 
		m_signal.Wait(5000);
   	        m_mutex.Lock();
               		
		if (m_pjobs->size() > 0) 
		{
			p = m_pjobs->front();
			m_pjobs->pop();
			m_mutex.Unlock();
			
		}else{
			m_mutex.Unlock();
			MThread::Sleep(500);
		}
	
		return p;
	}

protected:
	MSemaphore m_signal;

//	std::deque< ThreadJob* > m_jobs;
	JobPool * m_pjobs;
	std::queue<ThreadJobPtr> m_DoneJobs ;
	std::vector< Thread* > m_threads;
	
	MMutex m_mutex;
	MMutex m_DoneJobsMutex;
	int m_nthreadcount;
	bool m_fstop;
	bool m_bPause ;
	MSemaphore m_DoneJobsSignal;	
};
#endif

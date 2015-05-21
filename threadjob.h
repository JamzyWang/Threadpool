/*
Author onebird
Date 20060822
For describe a job for thread pool
*/

#ifndef _THREADJOB_H_
#define _THREADJOB_H_
#include <time.h>
#include <queue>
#include <boost/shared_ptr.hpp>

class ThreadJob  
{
public:
	virtual bool execute()=0;
	//{
	//	//delete this;
	//	return true;
	//}
	inline bool IsRetJob(){
		return m_bIsRetJob ;
	}
	inline void SetIsRetJob(bool bol){
		m_bIsRetJob = bol ;
	}
	ThreadJob():m_bIsRetJob(false) {
		time(&m_ts);

	}
	virtual ~ThreadJob() {}
protected:
    bool m_bIsRetJob ;
	time_t m_ts;
};
typedef boost::shared_ptr<ThreadJob>  ThreadJobPtr ;

class JobPool{
public:
	virtual void  push(ThreadJobPtr jobPtr)=0;
	virtual ThreadJobPtr front()=0;
	virtual void  pop()=0;
	virtual size_t  size()=0 ;
	virtual ~JobPool() {}
};
class DefaultJobPool:public JobPool{
public:
	virtual void  push(ThreadJobPtr jobPtr);
	virtual ThreadJobPtr front();
	virtual void  pop();
	virtual size_t  size();
	virtual ~DefaultJobPool() {}
private:
	std::queue<ThreadJobPtr> jobs ;
};




#endif

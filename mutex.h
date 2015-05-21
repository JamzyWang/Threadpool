

#ifndef _MUTEX_H_
#define _MUTEX_H_

#ifdef WIN32

#include <windows.h>
#else
#include <sys/time.h>
#include <pthread.h>
#endif


class MMutex
{
public:

    MMutex();
    ~MMutex();
	
    inline void Lock() 
	{
#ifdef WIN32
		::EnterCriticalSection(&fMutex);
#else
		(void)pthread_mutex_lock(&fMutex);
#endif
	};

    inline void Unlock() 
	{
#ifdef WIN32
	    ::LeaveCriticalSection(&fMutex);
#else
		 pthread_mutex_unlock(&fMutex);
#endif
	};

#ifdef WIN32
	inline CRITICAL_SECTION& getMutex(){return fMutex;}
#else
	inline pthread_mutex_t& getMutex(){return fMutex;}
#endif


private:
	MMutex & operator=(MMutex & mtx){return mtx;};  //锁不允许拷贝 和 赋值  故私有化以下两函数
	MMutex(MMutex & mtx){};

#ifdef WIN32
    CRITICAL_SECTION fMutex;                     
#else
    pthread_mutex_t fMutex;       
#endif       
};

class MLock
{
public:
	MLock(MMutex& cs) : _cs(cs)
	{
		_cs.Lock();
	}
	~MLock()
	{
		_cs.Unlock();
	}
private:
	MMutex& _cs;
};

class MSemaphore
{
#ifdef WIN32
public:
	MSemaphore()
	{
		h = CreateSemaphore(NULL, 0, MAXLONG, NULL);
	}
	void Signal() 
	{
		ReleaseSemaphore(h, 1, NULL);
	}

	bool Wait()
	{
		return WaitForSingleObject(h, INFINITE) == WAIT_OBJECT_0;
	}

	bool Wait(unsigned int msecond)
	{
		return WaitForSingleObject(h, msecond) == WAIT_OBJECT_0;
	}

	~MSemaphore()
	{
		CloseHandle(h);
	}

private:
	HANDLE h;
#else
public:
	MSemaphore() : count(0)
	{
		pthread_cond_init(&cond, NULL);
	}
	void Signal() 
	{
		MLock l(cs);
		count++;
		pthread_cond_signal(&cond);
	}

	bool Wait()
	{ 
		MLock l(cs);
		if(count == 0) {
			pthread_cond_wait(&cond, &cs.getMutex());
		}
		count--;
		return true;
	}

	bool Wait(unsigned int msecond)
	{ 
		MLock l(cs);
		if(count == 0) {
			timeval timev;
			timespec t;
			gettimeofday(&timev, NULL);
			t.tv_sec = timev.tv_sec + (msecond/1000);
			t.tv_nsec = (msecond%1000)*1000*1000;
			int ret = pthread_cond_timedwait(&cond, &cs.getMutex(), &t);
			if(ret != 0) {
				return false;
			}
		}
		count--;
		return true;
	}

private:
	pthread_cond_t cond;
	MMutex cs;
	int count;
#endif
public:
	MSemaphore(const MSemaphore&);
	MSemaphore& operator=(const MSemaphore&);
};

class MSafeSemaphore
{
#ifdef WIN32
public:
	MSafeSemaphore()
	{
		h = CreateSemaphore(NULL, 0, MAXLONG, NULL);
	}
	void Signal() 
	{
		ReleaseSemaphore(h, 1, NULL);
	}

	bool Wait()
	{
		return WaitForSingleObject(h, INFINITE) == WAIT_OBJECT_0;
	}

	bool Wait(unsigned int msecond)
	{
		return WaitForSingleObject(h, msecond) == WAIT_OBJECT_0;
	}

	~MSafeSemaphore()
	{
		CloseHandle(h);
	}

private:
	HANDLE h;
#else
public:
	MSafeSemaphore() : count(0)
	{
		pthread_cond_init(&cond, NULL);
	}
	~MSafeSemaphore()
	{
		pthread_cond_destroy(&cond);
	}
	void Signal() 
	{
		MLock l(cs);
		count++;
		pthread_cond_signal(&cond);
	}

	bool Wait()
	{ 
		MLock l(cs);
		while (count <= 0) {
			pthread_cond_wait(&cond, &cs.getMutex());
		}
		count--;
		return true;
	}

	bool Wait(unsigned int msecond)
	{ 
		MLock l(cs);

			timeval timev;
			timespec t;
			gettimeofday(&timev, NULL);

			t.tv_sec=timev.tv_sec+(msecond/1000);

			t.tv_nsec = timev.tv_usec*1000+(msecond%1000)*1000*1000;
			if (t.tv_nsec>=1000*1000*1000) {
				t.tv_nsec-=1000*1000*1000;
				t.tv_sec++;
			}
	
		while(count <= 0) {
			int ret = pthread_cond_timedwait(&cond, &cs.getMutex(), &t);
			if(ret != 0) {
				return false;
			}
		}
		count--;
		return true;
	}

private:
	pthread_cond_t cond;
	MMutex cs;
	int count;
#endif
public:
	MSafeSemaphore(const MSafeSemaphore&);
	MSafeSemaphore& operator=(const MSafeSemaphore&);
};

class MRWMutex
{
public:
	MRWMutex();
	~MRWMutex();

	void LockRead();
	void UnlockRead();
	void LockWrite();
	void UnlockWrite();

	////// Accessor functions
	int GetWaitingWriters();
	int GetWaitingReaders();
	int GetLockCount();

#ifdef WIN32
private:
	int WaitingWriters();
	int IncrementWriters();
	int DecrementWriters();

	int WaitingReaders();
	int IncrementReaders();
	int DecrementReaders();

private:
	int m_nLockCount;

	int m_nWaitingWriters;
	int m_nWaitingReaders;

	MSemaphore m_semReader;
	MSemaphore m_semWriter;
	MMutex m_mtxLockCount;
	//HANDLE m_hReaderEvent;
	//HANDLE m_hWriterEvent;
	//HANDLE m_hLockCountMutex;	

private:
	MRWMutex(const MRWMutex&);
	const MRWMutex& operator=(const MRWMutex&);

#else
	
private:
	pthread_rwlock_t fRWMutex;
#endif
};


#endif


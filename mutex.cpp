#include "mutex.h"

#include <stdlib.h>

#include <string.h>



#ifndef WIN32

	static pthread_mutexattr_t  *sMutexAttr=NULL;

	static void MutexAttrInit();

	static pthread_once_t sMutexAttrInit = PTHREAD_ONCE_INIT;

#endif



MMutex::MMutex()

{

#ifdef WIN32

    ::InitializeCriticalSection(&fMutex);

#else

    (void)pthread_once(&sMutexAttrInit, MutexAttrInit);

    (void)pthread_mutex_init(&fMutex, sMutexAttr);

#endif

}



#ifndef WIN32

void MutexAttrInit()

{

    sMutexAttr = (pthread_mutexattr_t*)malloc(sizeof(pthread_mutexattr_t));

    memset(sMutexAttr, 0, sizeof(pthread_mutexattr_t));

    pthread_mutexattr_init(sMutexAttr);

}

#endif



MMutex::~MMutex()

{

#ifdef WIN32

    ::DeleteCriticalSection(&fMutex);

#else

    pthread_mutex_destroy(&fMutex);

#endif

}



#ifdef WIN32

//////////////////////////////////////////////////////////////////////

// Construction/Destruction

//////////////////////////////////////////////////////////////////////



//---------------------------------------------------------------------

// MRWMutex - constructs a read-write lock object and events

//

// Exceptions   : NONE

// Return Codes : NONE 

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////

MRWMutex::MRWMutex() : m_nLockCount(0), m_nWaitingReaders(0), m_nWaitingWriters(0)

///////////////////////////////////////////////////////////////////////

{

}



//---------------------------------------------------------------------

// ~MRWMutex - destroys the read-write lock object and events

//

// Exceptions   : NONE

// Return Codes : NONE 

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////

MRWMutex::~MRWMutex()

///////////////////////////////////////////////////////////////////////

{

	// Make sure mutex is unlocked when object is destroyed

	m_mtxLockCount.Unlock();

}



//---------------------------------------------------------------------

// LockRead - allows multiple reading threads to lock a resource

// for shared access

//

// Exceptions   : NONE

// Return Codes : true (success), false (failure)

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////

void MRWMutex::LockRead()

///////////////////////////////////////////////////////////////////////

{

	WaitingReaders();



	if (m_nLockCount < 0)

	{

		// a reader thread must wait for the read event.. if a writer is holding lock

		m_semReader.Wait();

	}



	IncrementReaders();

}



//---------------------------------------------------------------------

// UnLockRead - allows reader threads to unlock a resource

//

// Exceptions   : NONE

// Return Codes : true (success), false (failure)

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////

void MRWMutex::UnlockRead()

///////////////////////////////////////////////////////////////////////

{

	DecrementReaders();



	if (!m_nWaitingReaders && !m_nLockCount)

	{

		m_semWriter.Signal();

	}

}



//---------------------------------------------------------------------

// LockWrite - allows single writer thread to lock a resource

// for exclusive access

//

// Exceptions   : NONE

// Return Codes : true (success), false (failure)

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////

void MRWMutex::LockWrite()

///////////////////////////////////////////////////////////////////////

{

	WaitingWriters();



	while (m_nLockCount)

	{

		m_semWriter.Wait();

	}



	IncrementWriters();

}



//---------------------------------------------------------------------

// UnlockWrite - allows single writing thread to unlock a resource

//

// Exceptions   : NONE

// Return Codes : true (success), false (failure)

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////

void MRWMutex::UnlockWrite()

///////////////////////////////////////////////////////////////////////

{

	DecrementWriters();



	// Readers are preferred in the following section....

	if (m_nWaitingWriters && !m_nWaitingReaders)

	{

		// when no readers are waiting... signal writers, if any

		m_semWriter.Signal();

	}

	else

	{

		while (m_nWaitingReaders > 0)

		{

			m_semReader.Signal();

		}

	}

}





//---------------------------------------------------------------------

// WaitingReader() - increments waiting reader count

//

// Exceptions   : NONE

// Return Codes : true (success), false (failure)

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////

int MRWMutex::WaitingReaders()

///////////////////////////////////////////////////////////////////////

{

	m_mtxLockCount.Lock();



	// One more thread is waiting on read event

	m_nWaitingReaders++;



	m_mtxLockCount.Unlock();



	return true;

}



//---------------------------------------------------------------------

// IncrementReaders() - decrements waiting reader count and increments

// lock count

//

// Exceptions   : NONE

// Return Codes : true (success), false (failure)

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////

int MRWMutex::IncrementReaders()

///////////////////////////////////////////////////////////////////////

{

	// InterlockedIncrement() could also be used, instead of this mutex

	m_mtxLockCount.Lock();



	// One more thread has obtained a read lock and is no longer waiting

	if (m_nLockCount >= 0)

	{

		m_nLockCount++;

		m_nWaitingReaders--;

	}



	m_mtxLockCount.Unlock();



	return true;

}



//---------------------------------------------------------------------

// DecrementReaders() - decrements lock count

//

// Exceptions   : NONE

// Return Codes : true (success), false (failure)

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////

int MRWMutex::DecrementReaders()

///////////////////////////////////////////////////////////////////////

{

	// InterlockedDecrement() could also be used, instead of this mutex

	m_mtxLockCount.Lock();



	if (m_nLockCount > 0)

	{

		// One less thread holding read lock

		m_nLockCount--;

	}



	m_mtxLockCount.Unlock();



	return true;

}



//---------------------------------------------------------------------

// WaitingWriter() - increments waiting writer count

//

// Exceptions   : NONE

// Return Codes : true (success), false (failure)

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////

int MRWMutex::WaitingWriters()

///////////////////////////////////////////////////////////////////////

{

	m_mtxLockCount.Lock();



	m_nWaitingWriters++;



	m_mtxLockCount.Unlock();



	return true;

}



//---------------------------------------------------------------------

// IncrementWriters() - decrements waiting writer count and pushes

// lock count below ground level

//

// Exceptions   : NONE

// Return Codes : true (success), false (failure)

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////

int MRWMutex::IncrementWriters()

///////////////////////////////////////////////////////////////////////

{

	m_mtxLockCount.Lock();



	// Only one thread can obtain a write lock .. so lock count goes underground

	// InterlockedDecrement() could also be used

	if (m_nLockCount == 0)

	{

		m_nLockCount = -1;

		m_nWaitingWriters--;

	}



	m_mtxLockCount.Unlock();



	return true;

}



//---------------------------------------------------------------------

// DecrementWriters() - pulls lock count to ground level

//

// Exceptions   : NONE

// Return Codes : true (success), false (failure)

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////

int MRWMutex::DecrementWriters()

///////////////////////////////////////////////////////////////////////

{

	m_mtxLockCount.Lock();



	if (m_nLockCount == -1)

	{

		m_nLockCount = 0;

	}



	m_mtxLockCount.Unlock();



	return true;

}



//---------------------------------------------------------------------

// GetLockCount() - returns current lock count

//

// Exceptions   : NONE

// Returns      : lock count (int)

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////

int MRWMutex::GetLockCount()

{

	int nCount = 0;



	m_mtxLockCount.Lock();



	nCount = m_nLockCount;



	m_mtxLockCount.Unlock();



	return nCount;

}





//---------------------------------------------------------------------

// GetWaitingReaders() - returns current no. of threads waiting for

// read access

//

// Exceptions   : NONE

// Returns      : waiting readers (int)

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////

int MRWMutex::GetWaitingReaders()

{

	int nCount = 0;



	m_mtxLockCount.Lock();



	nCount = m_nWaitingReaders;



	m_mtxLockCount.Unlock();



	return nCount;

}



//---------------------------------------------------------------------

// GetWaitingWriters() - returns current no. of threads waiting for

// write access

//

// Exceptions   : NONE

// Returns      : waiting writers (int)

//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////

int MRWMutex::GetWaitingWriters()

{

	int nCount = 0;



	m_mtxLockCount.Lock();



	nCount = m_nWaitingWriters;



	m_mtxLockCount.Unlock();



	return nCount;

}





#else

	static pthread_rwlockattr_t  *sRWMutexAttr=NULL;

	static void RWMutexAttrInit();

	static pthread_once_t sRWMutexAttrInit = PTHREAD_ONCE_INIT;



void RWMutexAttrInit()

{

    sRWMutexAttr = (pthread_rwlockattr_t*)malloc(sizeof(pthread_rwlockattr_t));

    memset(sRWMutexAttr, 0, sizeof(pthread_rwlockattr_t));

    pthread_rwlockattr_init(sRWMutexAttr);

}



MRWMutex::MRWMutex()

{

	(void)pthread_once(&sRWMutexAttrInit, RWMutexAttrInit);

    (void)pthread_rwlock_init(&fRWMutex, sRWMutexAttr);

}



MRWMutex::~MRWMutex() 

{

	pthread_rwlock_destroy(&fRWMutex);

}



void MRWMutex::LockRead()

{

	pthread_rwlock_rdlock(&fRWMutex);

}



void MRWMutex::LockWrite()

{

	pthread_rwlock_wrlock(&fRWMutex);

}



void MRWMutex::UnlockRead()

{

	pthread_rwlock_unlock(&fRWMutex);

}



void MRWMutex::UnlockWrite()

{

	pthread_rwlock_unlock(&fRWMutex);

}



int MRWMutex::GetWaitingWriters()

{

	return 0;

}

int MRWMutex::GetWaitingReaders()

{

	return 0;

}

int MRWMutex::GetLockCount()

{

	return 0;

}



#endif


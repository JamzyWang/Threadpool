#ifndef _MTHREAD_H_
#define _MTHREAD_H_

#ifdef WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

class MThread
{
public:
	MThread():
	  _pid(-1),
		  _brun(false),
		  _hthread(NULL)
	  {
	  }
	  ~MThread()
	  {
//		  Close();
	  }

	  bool Create(unsigned long (*start_address)(void*), void* p)
	  {
#ifdef WIN32
		  _brun = true;
		  //_pid = ::_beginthread((void(*)(void*))start_address, 0, p);
		  _hthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) start_address, p, 0, (unsigned long*)&_pid);
		  if (_hthread == NULL)
			  _brun = false;
		  //if (_pid == -1) //returns ¨C1L on an error
		  //    _brun = false;
#else
		  _brun = true;
		  pthread_t pid;
		  pthread_attr_t pattr;
		  pthread_attr_init(&pattr);
		  pthread_attr_setdetachstate(&pattr,PTHREAD_CREATE_DETACHED);

		  pthread_create(&pid, &pattr,(void*(*)(void *))start_address, p);
		  _pid= (int)pid;
		  pthread_attr_destroy(&pattr);
		  if (_pid == -1) //returns ¨C1L on an error
			  _brun = false;
#endif
		  return _brun;
	  }

	  void Terminate()
	  {
		  if (!_brun)
			  return;

		  _brun = false; //wish the thread would end itself
		  _pid = -1;
#ifdef WIN32
		  WaitForSingleObject(_hthread, 1000);

		  if(_hthread != NULL)
			  TerminateThread(_hthread, 0);

		  _hthread=NULL;
#endif
	  }

	  void Close()
	  {
#ifdef WIN32
		  if(_hthread != NULL)
			  CloseHandle(_hthread);

		  _hthread=NULL;
#endif
	  }

	  void WaitEnd()
	  {
		  if (!_brun)
			  return;

		  _brun = false; //wish the thread would end itself
		  _pid = -1;

#ifdef WIN32
		  WaitForSingleObject(_hthread, INFINITE);

		  //_hthread=NULL;
#else
		  //XXX
#endif
	  }

	  inline bool IsRun() { return _brun; }

	  static void Sleep(unsigned long ms)
	  {
#ifdef WIN32
		  ::Sleep(ms);
#else
		  struct timespec tp;
		  tp.tv_sec = (int) ms/1000;
		  tp.tv_nsec = (ms % 1000) * 1000000; // millisecond=>nanosecond
		  try
		  {
			nanosleep(&tp, &tp);
		  }catch(...){}
#endif
	  }

	  static void ExitThread()
	  {
#ifdef WIN32
		  _endthread();
#else
		  pthread_exit(NULL);
#endif
	  }

#ifdef WIN32
	  static DWORD        GetCurrentThreadID() { return ::GetCurrentThreadId(); }
#else    
	  static  pthread_t   GetCurrentThreadID() { return ::pthread_self(); }
#endif

	  void*	_data;
	  int     _pid;
	  bool    _brun;
	  void*  _hthread;
};

#endif



#include "thread.h"
#include "threadjob.h"

void  DefaultJobPool::push(ThreadJobPtr pjob){
	return jobs.push(pjob) ;
}
ThreadJobPtr DefaultJobPool::front(){
	return jobs.front() ;

};
void  DefaultJobPool::pop(){
	return jobs.pop();
}
size_t  DefaultJobPool::size(){
	return jobs.size();
}






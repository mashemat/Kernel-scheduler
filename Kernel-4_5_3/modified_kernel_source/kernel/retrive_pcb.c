#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include "sched/sched.h"

asmlinkage long sys_retrive_pcb(pid_t pid,int type){
	
	long retrive=-1;	
	struct task_struct *tsk;
	struct sched_entity *se;
	struct cfs_rq* cfs_rq;

	rcu_read_lock();
	tsk = find_task_by_vpid(pid);
	if (tsk){
		get_task_struct(tsk);
		se=&(tsk->se);
		if(se)
			cfs_rq=(se->cfs_rq);
	}
	rcu_read_unlock();
	
	if(type==1)
		retrive=se->co_1;		
		
	else if(type==2)
		//retrive=se->cache_miss_nr;
		retrive=se->co_2;
	else if(type==3)
		//retrive=cfs_rq->runnable_cachemiss_sum;
		retrive=se->co_3;
	else if(type==4)
		retrive=cfs_rq->runnable_cachemiss_avg;
	else if(type==5)
		retrive=se->cache_miss_rate;
	else
		retrive=cfs_rq->nr_running;
	
put_task_struct(tsk);
	
	return retrive;

}

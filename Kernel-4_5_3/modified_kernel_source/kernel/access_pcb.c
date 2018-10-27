#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include "sched/sched.h"

asmlinkage long sys_access_pcb(pid_t pid,unsigned long nr_cachemiss, unsigned long delta_time, unsigned long Nproc){
	/*'in the past' means 'from very beginning to last turn of calculating the cache rate miss'
	  'now' means ''in the past'+this turn of calculating the cache miss rate'
	   
	   for example, every 10ms, papi calculates the number of cache miss and call this system call to feed data.
           suppose that now is 100ms and data in the pcb has noty been updated, the procedure would be:
	    1. retrive the data in the past(at 90ms)
            2. get the data calculated by papi in the new 10ms(from 90ms to 100ms)
            3. use 1. and 2. to calculating exactly data at 100ms
	    4. update data in pcb
	*/
	struct task_struct *tsk;
	struct sched_entity *se;
	struct cfs_rq* cfs_rq;

	//unsigned long sum_p;//total cache miss of a process in the past
	//long avg_p;//cache miss rate of a process in the past
	//unsigned long nr;//total cache miss of a process from it was forked to now 
	long rate;//cache miss rate of a process at now
	
	//unsigned long sum_q;//total cache miss of all processes in queue in the past
	long avg_q;//cache miss rate of a queue in the past
	//unsigned long nr_q;//total cache miss of all processes in queue from very beginning to now 
	long rate_q;//cache miss rate of a queue at now
	
	rcu_read_lock();
	tsk = find_task_by_vpid(pid);
	if (tsk){
		get_task_struct(tsk);
		se=&(tsk->se);
		if(se)
			cfs_rq=(se->cfs_rq);
	}
	rcu_read_unlock();
	
	
	rate=nr_cachemiss/delta_time;
	
	if(cfs_rq){
		avg_q=cfs_rq->runnable_cachemiss_avg;
		if(rate-(se->cache_miss_rate)>=0)
			rate_q=(rate+avg_q*Nproc-(se->cache_miss_rate)+Nproc/2-1+(Nproc/2)%2)/Nproc;
		else
			rate_q=(rate+avg_q*Nproc-(se->cache_miss_rate)+Nproc/2+1-(Nproc/2)%2)/Nproc;
	}
	
	if(rate_q<1) rate_q=1;

	se->cache_miss_rate=rate;
	cfs_rq->runnable_cachemiss_avg=rate_q;
	
	put_task_struct(tsk);
	
	return 0;
}

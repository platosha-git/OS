struct proc 
{
	struct proc *p_forw; /* Doubly-linked run/sleep queue. */
	struct proc *p_back;
	struct proc *p_next; /* Linked list of active procs */
	struct proc **p_prev; /* and zombies. */

	
	/* substructures: */

	struct pcred *p_cred; /* Process owner's identity. */
	struct filedesc *p_fd; /* Ptr to open files structure. */
	struct pstats *p_stats; /* Accounting/statistics (PROC ONLY). */ 
	struct plimit *p_limit; /* Process limits. */
	struct vmspace *p_vmspace; /* Address space. */
	struct sigacts *p_sigacts; /* Signal actions, state (PROC ONLY). */

	#define p_ucred p_cred->pc_ucred
	#define p_rlimit p_limit->pl_rlimit

	int p_flag; /* P_* flags. */
	char p_stat; /* S* process status. */
	char p_pad1[3];
	pid_t p_pid; /* Process identifier. */
	
	struct proc *p_hash; /* Hashed based on p_pid for kill+exit+... */
	struct proc *p_pgrpnxt; /* Pointer to next process in process group. */
	struct proc *p_pptr; /* Pointer to process structure of parent. */
	struct proc *p_osptr; /* Pointer to older sibling processes. */

	/* The following fields are all zeroed upon creation in fork. */

	#define p_startzero p_ysptr

	struct proc *p_ysptr; /* Pointer to younger siblings. */
	struct proc *p_cptr; /* Pointer to youngest living child. */

	//Указатели поддерживают дерево
	//Теряет родителя - нарушается иерархия

	pid_t p_oppid; /* Save parent pid during ptrace. XXX */
	int p_dupfd; /* Sideways return value from fdopen. XXX */

	/* scheduling */

	u_int p_estcpu; /* Time averaged value of p_cpticks. */
	int p_cpticks; /* Ticks of cpu time. */
	fixpt_t p_pctcpu; /* %cpu for this process during p_swtime */
	void *p_wchan; /* Sleep address. */
	char *p_wmesg; /* Reason for sleep. */
	
	u_int p_swtime; /* Time swapped in or out. */
	u_int p_slptime; /* Time since last blocked. */

	struct itimerval p_realtimer; /* Alarm timer. */
	struct timeval p_rtime; /* Real time. */
	
	u_quad_t p_uticks; /* Statclock hits in user mode. */
	u_quad_t p_sticks; /* Statclock hits in system mode. */
	u_quad_t p_iticks; /* Statclock hits processing intr. */
	
	int p_traceflag; /* Kernel trace points. */
	struct vnode *p_tracep; /* Trace to vnode. */
	int p_siglist; /* Signals arrived but not delivered. */
	struct vnode *p_textvp; /* Vnode of executable. */
	char p_lock; /* Process lock (prevent swap) count. */
	char p_pad2[3]; /* alignment */

	
	/* End area that is zeroed on creation. */
	#define p_endzero p_startcopy

	
	/* The following fields are all copied upon creation in fork. */
	#define p_startcopy p_sigmask
	
	sigset_t p_sigmask; /* Current signal mask. */
	sigset_t p_sigignore; /* Signals being ignored. */
	sigset_t p_sigcatch; /* Signals being caught by user. */
	
	u_char p_priority; /* Process priority. */
	u_char p_usrpri; /* User-priority based on p_cpu and p_nice. */
	char p_nice; /* Process "nice" value. */
	char p_comm[MAXCOMLEN+1];

	struct pgrp *p_pgrp; /* Pointer to process group. */
	struct sysentvec *p_sysent; /* System call dispatch information. */
	struct rtprio p_rtprio; /* Realtime priority. */
	
	
	/* End area that is copied on creation. */
	#define p_endcopy p_addr


	struct user *p_addr; /* Kernel virtual addr of u-area (PROC ONLY). */
	struct mdproc p_md; /* Any machine-dependent fields. */
	u_short p_xstat; /* Exit status for wait; also stop signal. */
	u_short p_acflag; /* Accounting flags. */
	struct rusage *p_ru; /* Exit information. XXX */
};
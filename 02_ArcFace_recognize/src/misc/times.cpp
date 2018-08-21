#include "joinus.h"
#include <sys/times.h>
#include <sys/wait.h>

void	pr_times(clock_t, struct tms *, struct tms *);
void	do_cmd(char *);

//int
//main(int argc, char *argv[])
//{
//	int		i;
//
//	setbuf(stdout, NULL);
//	for (i = 1; i < argc; i++)
//		do_cmd(argv[i]);	/* once for each command-line arg */
//	exit(0);
//}



void pr_exit(int status)
{
	if (WIFEXITED(status))
		printf("normal termination, exit status = %d\n",
				WEXITSTATUS(status));
	else if (WIFSIGNALED(status))
		printf("abnormal termination, signal number = %d%s\n",
				WTERMSIG(status),
#ifdef	WCOREDUMP
				WCOREDUMP(status) ? " (core file generated)" : "");
#else
				"");
#endif
	else if (WIFSTOPPED(status))
		printf("child stopped, signal number = %d\n",
				WSTOPSIG(status));
}

void do_cmd(char *cmd)		/* execute and time the "cmd" */
{
	struct tms	tmsstart, tmsend;
	clock_t		start, end;
	int			status;

	printf("\ncommand: %s\n", cmd);

	if ((start = times(&tmsstart)) == -1)	/* starting values */
		err_sys("times error");

	if ((status = system(cmd)) < 0)			/* execute command */
		err_sys("system() error");

	if ((end = times(&tmsend)) == -1)		/* ending values */
		err_sys("times error");

	pr_times(end-start, &tmsstart, &tmsend);
	pr_exit(status);
}

void pr_times(clock_t real, struct tms *tmsstart, struct tms *tmsend)
{
	static long		clktck = 0;

	if (clktck == 0)	/* fetch clock ticks per second first time */
		if ((clktck = sysconf(_SC_CLK_TCK)) < 0)
			err_sys("sysconf error");

	printf("  real:  %7.2f\n", real / (double) clktck);
	printf("  user:  %7.2f\n",
	  (tmsend->tms_utime - tmsstart->tms_utime) / (double) clktck);
	printf("  sys:   %7.2f\n",
	  (tmsend->tms_stime - tmsstart->tms_stime) / (double) clktck);
	printf("  child user:  %7.2f\n",
	  (tmsend->tms_cutime - tmsstart->tms_cutime) / (double) clktck);
	printf("  child sys:   %7.2f\n",
	  (tmsend->tms_cstime - tmsstart->tms_cstime) / (double) clktck);
}
//
//#include <stdio.h>
//#include <stdlib.h> //malloc要用，没有的话，会有警告信息：隐式声明与内建函数'malloc'不兼容。不过警告信息不用管也没事
//
//#include <assert.h>
//#include <sys/time.h>
//
//int main()
//{
//float time_use=0;
//struct timeval start;
//struct timeval end;
////struct timezone tz; //后面有说明
//gettimeofday(&start,NULL); //gettimeofday(&start,&tz);结果一样
//printf("start.tv_sec:%d\n",start.tv_sec);
//printf("start.tv_usec:%d\n",start.tv_usec);
//
//sleep(3);
//gettimeofday(&end,NULL);
//printf("end.tv_sec:%d\n",end.tv_sec);
//printf("end.tv_usec:%d\n",end.tv_usec);
//time_use=(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);//微秒
//printf("time_use is %f\n",time_use);
//
////输出：time_use is 3001410.000000
//
////下面的采用指针的方式也可以，但是要注意指针类型若不分配内存的话，编译正确，但是运行结果会不对
//
//
//}

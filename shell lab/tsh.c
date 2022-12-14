/*
 * tsh - A tiny shell program with job control
 *
 * <Put your name and login ID here>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <memory.h>

/* Misc manifest constants */
#define MAXLINE 1024   /* max line size */
#define MAXARGS 128    /* max args on a command line */
#define MAXJOBS 16     /* max jobs at any point in time */
#define MAXJID 1 << 16 /* max job ID */

/* Job states job的状态 */
#define UNDEF 0 /* undefined 未定义*/
#define FG 1    /* running in foreground 前台*/
#define BG 2    /* running in background 后台*/
#define ST 3    /* stopped 停止*/

/*
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;   /* defined in libc */
char prompt[] = "tsh> "; /* command line prompt (DO NOT CHANGE) */
int verbose = 0;         /* if true, print additional output */
int nextjid = 1;         /* next job ID to allocate  下一个job的id*/
char sbuf[MAXLINE];      /* for composing sprintf messages */

struct job_t               // job结构体
{                          /* The job struct */
    pid_t pid;             /* job PID */
    int jid;               /* job ID [1, 2, ...] */
    int state;             /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE]; /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */

/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv);
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs);
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid);
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid);
int pid2jid(pid_t pid);
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine
 */
int main(int argc, char **argv)
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) 重定向输出文件，原本输出到stderror，现在全部输出到stdout*/
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF)
    {
        switch (c)
        {
        case 'h': /* print help message */
            usage();
            break;
        case 'v': /* emit additional diagnostic info */
            verbose = 1;
            break;
        case 'p':            /* don't print a prompt */
            emit_prompt = 0; /* handy for automatic testing */
            break;
        default:
            usage();
        }
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT, sigint_handler);   /* ctrl-c  捕捉2号信号，来自键盘的中断*/
    Signal(SIGTSTP, sigtstp_handler); /* ctrl-z  捕捉20号信号，来自终端的停止信号*/
    Signal(SIGCHLD, sigchld_handler); /* Terminated or stopped child  捕捉17号信号，一个子进程停止或者终止*/

    /* This one provides a clean way to kill the shell  一个干净的方式杀掉shell*/
    Signal(SIGQUIT, sigquit_handler);

    /* Initialize the job list 初始化job列表*/
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1)
    {
        /* Read command line */
        if (emit_prompt)
        {
            printf("%s", prompt); //打印"tsh>"
            fflush(stdout);
        }
        if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
            app_error("fgets error");
        if (feof(stdin)) // 判断输入文件结尾
        {                /* End of file (ctrl-d) */
            fflush(stdout);
            exit(0);
        }

        /* Evaluate the command line */
        eval(cmdline);
        fflush(stdout);
        fflush(stdout);
    }

    exit(0); /* control never reaches here */
}

/*
 * eval - Evaluate the command line that the user has just typed in
 *
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.
 */
void eval(char *cmdline)
{
    char *argv[MAXARGS]; // arguments array  参数数组
    pid_t pid;           // process id      进程id
    int bg;              // foreground or background ?   前台运行还是后台运行
    bg = parseline(cmdline, argv);

    // 输入为空行直接return
    if (argv[0] == NULL)
        return;

    // 不是内置命令
    if (!builtin_cmd(argv))
    {
        // 创建子进程执行命令
        pid = fork();
        if (pid == 0)
        {
            if (execve(argv[0], argv, environ) < 0)
            {
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }
        //后台进程执行
        if (bg)
        {
            addjob(jobs, pid, BG, cmdline); //后台进程加入jobs
            printf("[%d] (%d) %s", pid2jid(pid), pid, cmdline);
            // how to delete  TODO
            // deletejob(jobs, pid);
        }
        else
        {
            addjob(jobs, pid, FG, cmdline);
            // 父进程等待前台任务结束
            int status;
            if (waitpid(pid, &status, 0) < 0)
                unix_error("waitfg:waitpid error");
            else
                deletejob(jobs, pid);
        }
    }
    return;
}

/*
 * parseline - Parse the command line and build the argv array.
 *              解析命令行并建立参数数组
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.
 */
int parseline(const char *cmdline, char **argv)
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);         // 将cmdline拷贝到buf数组中
    buf[strlen(buf) - 1] = ' ';   /* replace trailing '\n' with space 将'\n'用' '代替 */
    while (*buf && (*buf == ' ')) /* ignore leading spaces 忽略多余的空格 */
        buf++;

    /* Build the argv list 建立参数列表*/
    argc = 0;
    if (*buf == '\'')
    {
        buf++;
        delim = strchr(buf, '\''); // 在strchr函数是在参数buf所指向的字符串中，寻找第一个出现的字符c, 返回找到字符的地址
    }
    else
    {
        delim = strchr(buf, ' ');
    }

    while (delim)
    {
        argv[argc++] = buf;           // 字符首地址放入到数组当中
        *delim = '\0';                // 最后一个设置为'\0'
        buf = delim + 1;              // 从下一个位置开始
        while (*buf && (*buf == ' ')) /* ignore spaces */
            buf++;

        if (*buf == '\'')
        {
            buf++;
            delim = strchr(buf, '\'');
        }
        else
        {
            delim = strchr(buf, ' ');
        }
    }
    argv[argc] = NULL; // 设置标志位，表示参数结束

    if (argc == 0) /* ignore blank line */
        return 1;  // 没有输入直接返回1

    /* should the job run in the background? 是否在后台运行*/
    if ((bg = (*argv[argc - 1] == '&')) != 0)
    {
        argv[--argc] = NULL;
    }
    return bg; //  返回1 表示在后台运行，返回0表示在前台运行
}

/*
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.
 *  判断是否是内置命令, 是内置命令返回1，否则返回0
 */
int builtin_cmd(char **argv)
{
    if ((strcmp(argv[0], "quit")) == 0) // quit command
        exit(0);
    if ((strcmp(argv[0], "jobs") == 0)) // jobs command
    {
        listjobs(jobs);
        return 1;
    }
    if (!(strcmp(argv[0], "bg"))) // bg <job> command 通过发送SIGCONT命令重新启动该job，并且在后台运行，参数job是一个pid或者jid
    {
        // TODO
        return 1;
    }
    if (strcmp(argv[0], "fg") == 0) // fg <job> command 通过发送SIGCONT命令重新启动该job，并且在前台运行，参数job是一个pid或者jid
    {
        // TODO
        return 1;
    }
    if ((strcmp(argv[0], "&") == 0)) // '&'是内置命令，但是忽略该命令
        return 1;
    return 0; /* not a builtin command */
}

/*
 * do_bgfg - Execute the builtin bg and fg commands  执行bg和fg命令
 */
void do_bgfg(char **argv)
{
    return;
}

/*
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid)
{
    return;
}

/*****************
 * Signal handlers
 *****************/

/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.
 */
void sigchld_handler(int sig)
{
    return;
}

/*
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.当在键盘上按ctrl+c, 内核会发送一个SIGINT信号
 *    捕捉它并且发送给前台进程
 */
void sigint_handler(int sig)
{
    pid_t pid = fgpid(jobs); // 在jobs中查找当前前台进程， 返回pid
    struct job_t *job = getjobpid(jobs, pid);
    // 干掉前台进程
    if (pid && job->state == FG)
    {
        printf("Job [%d] (%d) terminated by signal 2\n", pid2jid(pid), pid);
        clearjob(job); // 从任务列表中删除
    }
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.
    捕捉一个20号信号，发送给前台进程，停止进程
 */
void sigtstp_handler(int sig)
{
    // pid_t pid = fgpid(jobs); // 在jobs中查找当前前台进程， 返回pid
    // struct job_t *job = getjobpid(jobs, pid);
    // // 干掉前台进程
    // if (pid && job->state == FG)
    // {
    //     printf("Job [%d] (%d) terminated by signal 2\n", pid2jid(pid), pid);
    //     clearjob(job); // 从任务列表中删除
    // }

    return;
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct 删除一个job 参数为一个job指针 */
void clearjob(struct job_t *job)
{
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list 初始化job列表*/
void initjobs(struct job_t *jobs)
{
    int i;

    for (i = 0; i < MAXJOBS; i++) // 删除所有的job
        clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID 返回最大的jobID*/
int maxjid(struct job_t *jobs)
{
    int i, max = 0;

    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].jid > max)
            max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list 加入一个job到job列表*/
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline)
{
    int i;

    if (pid < 1)
        return 0;

    for (i = 0; i < MAXJOBS; i++)
    {
        if (jobs[i].pid == 0)
        {
            jobs[i].pid = pid;
            jobs[i].state = state;
            jobs[i].jid = nextjid++;
            if (nextjid > MAXJOBS)
                nextjid = 1;
            strcpy(jobs[i].cmdline, cmdline);
            if (verbose)
            {
                printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
        }
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list 删除一个PID等于pid的job*/
int deletejob(struct job_t *jobs, pid_t pid)
{
    int i;

    if (pid < 1)
        return 0;

    for (i = 0; i < MAXJOBS; i++)
    {
        if (jobs[i].pid == pid)
        {
            clearjob(&jobs[i]);
            nextjid = maxjid(jobs) + 1;
            return 1;
        }
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job 返回正在运行的前台job的pid，如果没有就返回0 */
pid_t fgpid(struct job_t *jobs)
{
    int i;

    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].state == FG)
            return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list 找到PID为pid的job，返回一个job结构体*/
struct job_t *getjobpid(struct job_t *jobs, pid_t pid)
{
    int i;

    if (pid < 1)
        return NULL;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].pid == pid)
            return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list  找到一个JID等于jid的job，返回一个job_t结构体 */
struct job_t *getjobjid(struct job_t *jobs, int jid)
{
    int i;

    if (jid < 1)
        return NULL;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].jid == jid)
            return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID 找到PID等于pid的job，返回该job的jid */
int pid2jid(pid_t pid)
{
    int i;

    if (pid < 1)
        return 0;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].pid == pid)
        {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list    打印job列表*/
void listjobs(struct job_t *jobs)
{
    int i;

    for (i = 0; i < MAXJOBS; i++)
    {
        if (jobs[i].pid != 0)
        {
            printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
            switch (jobs[i].state)
            {
            case BG:
                printf("Running ");
                break;
            case FG:
                printf("Foreground ");
                break;
            case ST:
                printf("Stopped ");
                break;
            default:
                printf("listjobs: Internal error: job[%d].state=%d ",
                       i, jobs[i].state);
            }
            printf("%s", jobs[i].cmdline);
        }
    }
}
/******************************
 * end job list helper routines
 ******************************/

/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void)
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
        unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig)
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}

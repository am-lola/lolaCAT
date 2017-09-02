#include <xdebug.h>
#include <xstdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <xstdio.h>
#include <wchar.h> //fwide
#include <spawn.h>

#ifdef QNX
#include <dlfcn.h>
#include <sys/neutrino.h>
#include <sys/procfs.h>
#include <sys/stat.h>
#if _NTO_VERSION>=640
#include <backtrace.h>
#endif
#endif


#ifdef LINUX
#include <execinfo.h>
#include <linux/limits.h>
#endif

#ifdef __APPLE__
#include <libproc.h>
#include <execinfo.h>
#include <sys/syslimits.h>
#endif


#undef DISABLE_BACKTRACE 

static void call_xatexit_fncs();

//////////////////////////////////////////////////
// assert functions
///////////////////////////////////////////////////
void xabort_fnc()
{
  static int xabort_called=0;
#if (defined QNX) && (_NTO_VERSION>640)
  //hold all threads except the this one
  ThreadCtl(_NTO_TCTL_THREADS_HOLD, 0);
#endif//QNX

  pdbg_ffl_ub("xabort\n");
#if (defined QNX)
  
  // use direct backtrace as gdb no longer installed on target qnx since qnx 6.6
  xdebug_direct_backtrace();
  
#else
  xdebug_backtrace();
    
#endif


  xabort_called++;

  if(xabort_called>1)
    /* perr_ffl("xabort_called multiple times (don't call xabort from xatexit function!)\n"); */
    perr_ffl("xabort called multiple times (don't call xabort from xatexit function!)\n");
  else
    call_xatexit_fncs();

  abort();
}

void xassert_fail(const char* expr, const char* file, const int line, const char* func)
{
  perr("In function %s, file %s, line %d: XASSERT(%s) failed\n",
       func, file, line, expr);
  
#ifdef QNX
       // use direct backtrace as gdb no longer installed on target qnx since qnx 6.6
  xdebug_direct_backtrace();
#else
       
  xdebug_backtrace();
#endif
  
  
  pdbg("calling abort to generate core file\n");
  abort();
}



//////////////////////////////////////////////////
// backtrace functions
///////////////////////////////////////////////////

//call gdb
void xdebug_backtrace()
{
  int   fd=-1;
  pid_t pid=0;
  int ival;

  char *NO_BT;
  char cmd[512];
  char progname[512]="ProgName";

  /* int fds[2]; */
  /* int fpindex; */
  /* FILE *fp=0; */
  char *argv[4];
  int status;
  //const char* amode="r";

#ifdef QNX
  struct dinfo_s
  {
  procfs_debuginfo    info;
  char                pathbuffer[PATH_MAX]; /* 1st byte is  info.path[0] */
  };

  char            buf[PATH_MAX + 1];
  struct dinfo_s  dinfo;
#endif//QNX
#if defined(LINUX) || defined(__APPLE__)
  int cmdlen=0;
#endif//
  char path[PATH_MAX+1];
  char cmdline[PATH_MAX+1];
  char *sh = "/bin/sh";  // default path...



#ifdef DISABLE_BACKTRACE
  pdbg_ffl_ub("backtrace disabled\n");
  return;
#endif
  NO_BT=getenv("AM2B_NO_BACKTRACE");
  if(NO_BT)
    {
     errno=0;
     ival= strtol(NO_BT, (char **) NULL, 10);
     if(errno == ERANGE)
     {
       perr_ffl(" : got AM2B_NO_BACKTRACE= %s: cannot convert to integer\n",NO_BT);
       return;
     }
     if(errno == EINVAL)
     {
       perr_ffl(": got AM2B_NO_BACKTRACE= %s: cannot convert to integer\n", NO_BT);
       return;
     }
    if(ival)
    {
      perr_ffl(" : got AM2B_NO_BACKTRACE= %s: no backtrace\n",NO_BT);
      return;
     } 
  //      else: ignore
    }

#ifdef QNX
  snprintf(buf, PATH_MAX+1,"/proc/%d/as", getpid());

  if((fd = open(buf, O_RDONLY|O_NONBLOCK)) == -1)
    {
      perr_ffl(" : error opening %s (%s)\n",
      buf,strerror(errno));
      return ;
     }

  status = devctl(fd, DCMD_PROC_MAPDEBUG_BASE, &dinfo,
  sizeof(dinfo), NULL);
  if(status != EOK)
  {
    perr_ffl(" : error getting process info %s (%s)\n",
    buf,strerror(errno));
    close(fd);
    return ;
   }

  close(fd);
  strncpy(progname,dinfo.info.path,512);
  fd=-1;
#endif//QNX

#if defined(LINUX)
  snprintf(path,PATH_MAX,"/proc/%d/cmdline",(int)getpid());
  fd=open(path,O_RDONLY);
  if(-1 == fd)
    {
    perr_ffl(" : error opening %s (%s)\n",
    path,strerror(errno));
    return;
   }
  cmdlen=read(fd,cmdline,PATH_MAX);
  if(cmdlen == PATH_MAX)
    pwrn_ffl("warning! cmdline too long!\n");
  if(cmdlen == -1 )
  {
    perr_ffl(": reading %s (%s)\n",
    path,strerror(errno));
    return ;
   }
  close(fd);
  strncpy(progname,cmdline,512);
#endif//LINUX
  
#if defined(__APPLE__)
  
  proc_name((int) getpid(), progname, 512);
  
#endif// OSX

  pdbg_ub("got process name %s\n", progname);
  
#ifdef LINUX
  snprintf(cmd,512,"gdb -q %s %d 2>/dev/null <<EOF\n"
    "set prompt (gdb)\n"
    "set solib-search-path ../lib:.\n"
    "thread apply all backtrace\n" //backtrace *all* threads!
    "detach\n"
    "quit\n"
    "EOF\n",
    progname,
    (int)getpid());
    
#endif
    
#ifdef __APPLE__
    snprintf(cmd,512,"lldb -n %s -p %d 2>/dev/null <<EOF\n"
      "bt all\n" //backtrace *all* threads!
      "detach\n"
      "quit\n"
      "EOF\n",
      progname,
      (int)getpid());
#endif

  /* pdbg("executing \"gdb -q %s %d\"\n", */
  /*     progname, */
  /*     (int)getpid()); */

  pdbg_ub("debug command= %s\n", cmd);
  pdbg_ub("================ start of gdb output ============= \n");

  argv[0] = "sh";
  argv[1] = "-c";
  argv[2] = (char *)cmd;
  argv[3] = 0;

  int myerrno=posix_spawnp(&pid,sh,0,0,argv,0);
  if(0 != myerrno)
    {
  status = myerrno;
  perr_ffl("error in posix_spawnp (%s)\n",strerror(myerrno));
  return ;
  }

  pdbg_ub(" (parent) waiting for spawned process to exit\n");
  if(waitpid(pid, &status, 0) == -1)
    {
  perr_ffl("error in waitpid\n");
  return ;
  }
  pdbg_ub("---------------- end of gdb output ------------- \n");
  return;
}

#if 1//direct backtrace (without invoking gdb)
 void xdebug_direct_backtrace()
 {
#if defined(LINUX) || defined(__APPLE__)
  void *trace[16];
  char **messages = (char **)NULL;
  int i, trace_size = 0;

  trace_size = backtrace(trace, 16);
  messages = backtrace_symbols(trace, trace_size);
  pdbg_ffl("printing execution path\n");
  if(0 == messages)
    {
  perr_ffl("got messages= 0x0\n");
  return ;
}
  for (i=0; i<trace_size; ++i)
    pdbg("[bt] %s\n", messages[i]);

#endif//LINUX

#ifdef QNX
#if _NTO_VERSION>640
  //hold all threads except the this one
  ThreadCtl(_NTO_TCTL_THREADS_HOLD, 0);

  pmsg_ffl("running backtrace\n");

  char out[1024];
  bt_addr_t pc[16];
  bt_accessor_t acc;
  bt_memmap_t memmap;
  Dl_info dli;
  int j;

  bt_init_accessor(&acc, BT_SELF);
  bt_load_memmap(&acc, &memmap);

  //for(i=0; i<10; i++)
  {
  int cnt=bt_get_backtrace(&acc, pc, sizeof(pc)/sizeof(bt_addr_t));
  bt_sprnf_addrs(&memmap, pc, cnt, "object file:%f, address(mem): %a, address(file): %l (%%I= %I)\n", out, sizeof(out), 0);
  puts(out);

  for(j=0;j<cnt;j++)
    {
  if(-1 == dladdr((void*)pc[j],&dli))
    {
  perr("dladdr: error= %s\n",
    dlerror());
  return ;
}
  if(dli.dli_fname)
    {
  pdbg("[bt - %d], object file= %s\n",
    j,
    dli.dli_fname
    );
}
  else
    {
  pdbg("[bt - %d], dli.dli_fname==NULL (application) sname= %s saddr= %p\n",
    j,dli.dli_sname,dli.dli_saddr);
}
}

}

  /* // */
  /* void* handle=dlsym(RTLD_DEFAULT,"_Z3food"); */
  /* printf("for foo got handle= %p\n", handle); */

  bt_unload_memmap(&memmap);
  bt_release_accessor(&acc);

  //continue threads
  ThreadCtl(_NTO_TCTL_THREADS_CONT, 0);
#else
  perr_ffl("NTO< 640: implement me\n");
#endif
#endif
}
#endif


 //////////////////////////////////////////////////
 //  default signal handlers
 ///////////////////////////////////////////////////
 static volatile int n_sigint=0;
 static volatile int stop_program_req=0;

 pthread_mutex_t sig_mtx=PTHREAD_MUTEX_INITIALIZER;

 static xdebug_sig_handler user_sigint_handler=0;
 static xdebug_sig_handler user_sigsegv_handler=0;
 static xdebug_sig_handler user_sigbus_handler=0;

 void xdebug_set_sigint_handler(xdebug_sig_handler h)
 {
   user_sigint_handler=h;
 }

 void xdebug_set_sigsegv_handler(xdebug_sig_handler h)
 {
   user_sigsegv_handler=h;
 }

 void xdebug_set_sigbus_handler(xdebug_sig_handler h)
 {
   user_sigbus_handler=h;
 }



 static void  sig_handler(int signo, siginfo_t* info, void* other)
 {
   switch(signo)
     {
     case SIGTERM:
       /* pdbg_ffl(": SIGTERM\n"); */
       pdbg_ffl_ub("SIGTERM\n");
       xabort();
       break;
     case SIGINT://CTRL-C
       pthread_mutex_lock(&sig_mtx);
       pdbg_ffl_ub("SIGINT\n");
       n_sigint++;
       stop_program_req=1;
       if(user_sigint_handler)
         {
           pdbg_ffl_ub("calling user callback %p\n", user_sigint_handler);
           /* perr_ffl(": calling user callback\n"); */
           (*user_sigint_handler)(signo,info,other);
         }

       if(n_sigint>3)//abort after three receiving SIGINT 3 times
         {
           perr_ffl("n_sigint>3, aborting\n" );
           pthread_mutex_unlock(&sig_mtx);
           xabort();
         }
       pthread_mutex_unlock(&sig_mtx);
       break;
     case SIGSEGV:
       pdbg_ffl_ub("SIGSEGV\n" );
       if(user_sigsegv_handler)
         {
           pdbg_ffl_ub("calling user callback %p\n" ,user_sigsegv_handler);
           (*user_sigsegv_handler)(signo,info,other);
         }
       xabort();
       break;
     case SIGBUS:
       pdbg_ffl_ub("SIGBUS\n");
       if(user_sigbus_handler)
         {
           pdbg_ffl_ub("calling user callback\n");
           (*user_sigbus_handler)(signo,info,other);
         }
       xabort();
       break;
     }

 }


 static void install_default_sighandlers()
 {
   struct sigaction act;
   sigset_t set;
   sigemptyset( &set );
   sigaddset(&set,SIGTERM);
   sigaddset(&set,SIGINT);
   sigaddset(&set,SIGSEGV);
   sigaddset(&set,SIGBUS);
   act.sa_flags = SA_SIGINFO;
   act.sa_mask = set;
   act.sa_handler = NULL;
   act.sa_sigaction=sig_handler;
   sigaction(SIGTERM, &act, NULL );
   sigaction(SIGINT,  &act, NULL );
   sigaction(SIGSEGV,  &act, NULL );
   sigaction(SIGBUS,  &act, NULL );
 }

 void xdebug_init()
 {
   install_default_sighandlers();
 }

 int xdebug_ctrl_c_pressed()
 {
   return (n_sigint>0);
 }

 //////////////////////////////////////////////////
 // atexit replacement
 ///////////////////////////////////////////////////
 pthread_mutex_t xatexit_mtx=PTHREAD_MUTEX_INITIALIZER;
 typedef void (*xatexit_fnc_t)(void);

#define N_XATEXIT 32
 static xatexit_fnc_t xatexit_fnc[N_XATEXIT];
 static int xatexit_n=0;


 int xatexit(void (*fnc)(void))
 {
   int i;
   pdbg_ffl_ub("\n");
   //initialize on first call
   pthread_mutex_lock(&xatexit_mtx);
   if(0 == xatexit_n)
     {
       for(i=0;i<N_XATEXIT;i++)
         xatexit_fnc[i]=0x0;
     }

   if(xatexit_n >= N_XATEXIT)
     {
       perr_ffl("can only register 32 functions\n");
       pthread_mutex_unlock(&xatexit_mtx);
       return -1;
     }
   //register
   if(0 == atexit(fnc))
     {
       xatexit_fnc[xatexit_n]=fnc;
       xatexit_n++;
       pthread_mutex_unlock(&xatexit_mtx);
       return 0;
     }
   else
     {
       pthread_mutex_unlock(&xatexit_mtx);
       perr_ffl("atexit failed\n");
       return -1;
     }
   perr_ffl("WTF\n");
   pthread_mutex_unlock(&xatexit_mtx);
   return 0;
 }

 void call_xatexit_fncs()
 {
   int i;
   pdbg_ffl_ub("\n");

   if(xatexit_n>0)
     {
       printf("\n==============  call_xatexit_fncs(): start, have %d functions registered ================\n",
              xatexit_n);
       for(i=0;i<xatexit_n;i++)
         {
           if(xatexit_fnc[i])
             {
               printf("  ----------- xatexit_fnc[%d] --------------   \n", i);
               xatexit_fnc[i]();
             }
         }
       printf("\n==============  call_xatexit_fncs(): end   ================\n");
     }
 }

 //////////////////////////////////////////////////
 // global variables
 ///////////////////////////////////////////////////
 volatile double xdebug_time=0;

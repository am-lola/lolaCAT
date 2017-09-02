#include "xtrace.h"

#ifdef QNX
#include <sys/trace.h>
#include <stdlib.h>
#include <xstdio.h>

static int do_trace=0;
//setup tracing mode and start gathering data
//tracelogger should be running:
//eg:  tracelogger -d1  -n 0  -f /dev/shmem/devr_hwl.kev
int trace_init()
{
  char *DO_TRACE= getenv("AM2B_DO_TRACE");
  if(DO_TRACE)
    {
      int val= strtol(DO_TRACE,(char **) NULL, 10);
      if(errno == ERANGE)
        {
          perr_ffl(" : got AM2B_DO_TRACE= %s: cannot convert to integer\n",DO_TRACE);
          return -1;
        }
      if(errno == EINVAL)
        {
          perr_ffl(": got AM2B_DO_TRACE= %s: cannot convert to integer\n", DO_TRACE);
          return -1;
        }
      do_trace=val;
      if(do_trace)
        pdbg(" : got AM2B_DO_TRACE= %s: enabling traceing\n",DO_TRACE);
      else
        pdbg(" : got AM2B_DO_TRACE= %s: disabling traceing\n",DO_TRACE);
    }

  if(do_trace)
    pdbg("init_trace(): do_trace= %d DO_TRACE=%s\n",
         do_trace,DO_TRACE);
  else
    pdbg("init_trace(): do_trace= %d DO_TRACE= 0x0\n",
         do_trace);

  static int call=0;

  if(0!=call)
    {
      perr_ffl("call= %d\n",call);
      return -1;
    }

  call++;
  if(do_trace)
    {
#define TRACE_EVENT(trace_event)                \
      if((int)((trace_event))==(-1))            \
        {                                       \
          perr_errno_ffl("TraceEvent failed");  \
          return -1;                            \
        }

      //mostly taken from the QNX documentation
      /*
       * Just in case, turn off all filters, since we
       * don't know their present state - go to the
       * known state of the filters.
       */
      TRACE_EVENT(TraceEvent(_NTO_TRACE_DELALLCLASSES));
      TRACE_EVENT(TraceEvent(_NTO_TRACE_CLRCLASSPID, _NTO_TRACE_KERCALL));
      TRACE_EVENT(TraceEvent(_NTO_TRACE_CLRCLASSTID, _NTO_TRACE_KERCALL));
      TRACE_EVENT(TraceEvent(_NTO_TRACE_CLRCLASSPID, _NTO_TRACE_THREAD));
      TRACE_EVENT(TraceEvent(_NTO_TRACE_CLRCLASSTID, _NTO_TRACE_THREAD));

      /*
       * Set fast emitting mode for all classes and
       * their events.
       */
      TRACE_EVENT(TraceEvent(_NTO_TRACE_SETALLCLASSESFAST));

      //Intercept all event classes
      TRACE_EVENT(TraceEvent(_NTO_TRACE_ADDALLCLASSES));

      //trace user events
      //TRACE_EVENT(TraceEvent(_NTO_TRACE_ADDCLASS,_NTO_TRACE_USER));

      /*
       * Start tracing process
       *
       * During the tracing process, the tracelogger(which
       * is being executed in a daemon mode) will log all events.
       * You can specify the number of iterations(i.e. the
       * number of kernel buffers logged) when you start tracelogger.
       */
      //TRACE_EVENT(TraceEvent(_NTO_TRACE_START));

      // /*
      //  * Insert four user-defined simple events and one string
      //  * event into the event stream. The user events have
      //  * arbitrary event IDs: 111, 222, 333, 444, and 555
      //  *(possible values are in the range 0...1023).
      //  * The user events with ID=(111, ..., 444) are simple events
      //  * that have two numbers attached:({1,11}, ..., {4,44}).
      //  * The user string event(ID 555) includes the string,
      //  * "Hello world".
      //  */
      // TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_INSERTSUSEREVENT, 111, 1, 11));
      // TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_INSERTSUSEREVENT, 222, 2, 22));
      // TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_INSERTSUSEREVENT, 333, 3, 33));
      // TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_INSERTSUSEREVENT, 444, 4, 44));
      // TRACE_EVENT(argv[0], TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT,555, "Hello world" ));

      /*
       * The main() of this execution flow returns.
       * However, the main() function of the tracelogger
       * will return after registering the specified number
       * of events.
       */
    }
  return 0;
}

int trace_start()
{
  if(do_trace)
    {
      pdbg("start_trace()\n");
      TRACE_EVENT(TraceEvent(_NTO_TRACE_START));
    }
  return 0;
}


int trace_stop()
{
  if(do_trace)
    {
      pdbg("stop_trace()\n");
      TRACE_EVENT(TraceEvent(_NTO_TRACE_FLUSHBUFFER));
      TRACE_EVENT(TraceEvent(_NTO_TRACE_STOP));
    }
  return 0;
}

int trace_evt(const char*pfx, const int event, const int line)
{
  char str[128];
  if(do_trace)
    {
      snprintf(str,128,"%s-L%d",pfx,line);
      if(-1 == trace_logf(_NTO_TRACE_USERFIRST+event,str))
        {
          perr_ffl("trace_logf\n");
          return -1;
        }
    }
  return 0;
}

int trace_evt_stamp(const char*pfx, const int event, const int line, const uint64_t stamp)
{
  char str[128];
  if(do_trace)
    {
      snprintf(str,128,"%s-L%d-S%llu",pfx,line,(long long unsigned)stamp);
      if(-1 == trace_logf(_NTO_TRACE_USERFIRST+event,str))
        {
          perr_ffl("trace_logf\n");
          return -1;
        }
    }
  return 0;
}

#else

int trace_init()
{
  return 0;
}

int trace_start()
{
  return 0;
}


int trace_stop()
{
  return 0;
}

int trace_evt(const char*pfx, const int event, const int line)
{
  return 0;
}

int trace_evt_stamp(const char*pfx, const int event, const int line, const uint64_t stamp)
{
  return 0;
}


#endif//QNX

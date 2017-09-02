//emacs -*-Mode: C++;-*-
#ifndef __XDEBUG_H__
#define __XDEBUG_H__

#include <signal.h>

#ifndef EOK
#define EOK 0
#endif

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
  

  //////////////////////////////////////////////////
  // assert and backtrace functions
  ///////////////////////////////////////////////////
  //!print backtrace for running program
  void xdebug_backtrace();


#if 1
  //!print backtrace for running program without calling gdb (linux and qnx)
  void xdebug_direct_backtrace();
#endif//0

#ifdef QNX
  void xassert_fail(const char* expr, const char* file, const int line, const char* func);
#else
  
  #ifndef CLANG_ANALYZER_NORETURN
    #ifdef __clang_analyzer__
        #define CLANG_ANALYZER_NORETURN __attribute__((analyzer_noreturn))
    #else
        #define CLANG_ANALYZER_NORETURN
    #endif
  #endif
  
  // Tell static analyzer this function with not return
    void xassert_fail(const char* expr, const char* file, const int line, const char* func) CLANG_ANALYZER_NORETURN;
#endif
  
  
  //!extended assert macro: print error message, backtrace + abort
#define XASSERT(x)                                              \
  ((x)                                                          \
   ?((void) 0)                                                  \
   : xassert_fail(#x, __FILE__, __LINE__, __PRETTY_FUNCTION__))

  //!call bt() + abort()

#define xabort()                                                        \
  perr_ffl(" calling xabort\n"); \
  xabort_fnc()


  void xabort_fnc();
  //////////////////////////////////////////////////
  // signal handlers
  ///////////////////////////////////////////////////
  //!initialize global debug functions [install default signal handlers]
  void xdebug_init();
  //!returns 1 if CTRL-C was pressed at least once
  int xdebug_ctrl_c_pressed();
  typedef void (*xdebug_sig_handler) (int, siginfo_t *, void *);
  void xdebug_set_sigint_handler(xdebug_sig_handler);
  void xdebug_set_sigsegv_handler(xdebug_sig_handler);
  void xdebug_set_sigbus_handler(xdebug_sig_handler);

  //////////////////////////////////////////////////
  // atexit replacement
  ///////////////////////////////////////////////////
  //!replacement for atexit: functions are registered with atexit,
  //but also called if xabort is called!
  int xatexit(void (*function)(void));


  //////////////////////////////////////////////////
  // program termination
  ///////////////////////////////////////////////////
  //!print debug message and return value x, if CTRL-C was pressed
#define CHECK_TERM(x)                           \
  if(xdebug_ctrl_c_pressed())                   \
    {                                           \
      pdbg_ffl_ub("CTRL-C pressed: exiting\n");    \
      return x;                                 \
    }

  //!same as CHECK_TERM, but return void
#define CHECK_TERM_VOID                         \
  if(xdebug_ctrl_c_pressed())                   \
    {                                           \
      pdbg_ffl_ub("CTRL-C pressed: exiting\n");    \
      return ;                                  \
    }

#define NOT_TERM  (!xdebug_ctrl_c_pressed())
#define IS_TERM  (xdebug_ctrl_c_pressed())



  //////////////////////////////////////////////////
  // global variables
  ///////////////////////////////////////////////////
  extern volatile double xdebug_time;
#ifdef __cplusplus
}
#endif//__cplusplus
#endif//__XSTDIO_H__

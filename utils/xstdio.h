//emacs -*-Mode: C++;-*-
/*!
  @file xstdio.h

  Functions and macros for formated output of error
  messages, warnings and debug messages.

  On QNX, the xprintf function is used to forward
  messages to a low-priority output thread.
  All messages are queued in a circular buffer.
  The maximum size for all output strings is 80bytes
  (standard terminal width)
  If messages are added too quickly, some messages will
  be lost.


  \todo: add severity levels/priorities

  Copyright (C) Thomas Buschmann, Institute of Applied Mechanics, TU-Muenchen
  All rights reserved.
  Contact: buschmann@amm.mw.tum.de

*/
#ifndef __XSTDIO_H__
#define __XSTDIO_H__
#include <ansi_console.h>
#include <errno.h>  //errno
#include <string.h> //strerror
#include <stdio.h>  //stdout

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus
  //!non-blocking output using low-priority IO thread
  void xprintf(const char *format, ...);

#ifdef QNX
  //use xprintf
# define msg_printf__ xprintf
#else//QNX
  //use printf
# define msg_printf__ printf
#endif//QNX


#define ERRMSG_COLOR
#ifdef ERRMSG_COLOR
# define PERR_BEGIN  ANSI_CODE(ANSI_FG_RED)
# define PWRN_BEGIN  ANSI_CODE(ANSI_FG_YELLOW)
# define PMSG_BEGIN
# define PDBG_BEGIN  ANSI_CODE(ANSI_FG_GREEN)
# define MSG_END  ANSI_NORMAL
#else
# define PERR_BEGIN
# define PWRN_BEGIN
# define PMSG_BEGIN
# define PDBG_BEGIN
# define MSG_END
#endif//ERRMSG_COLOR


//!length of lines in circular buffer
#define XSTDIO_LINE_SZ  256 //80
//!number of items in circular buffer
#define XSTDIO_N_LINES 5000

  const char* xstdio_tname();

  //!Display error message (allways directly output to stdout)
#define perr(format, ...)                                               \
  fprintf(stderr,PERR_BEGIN "(E) " format MSG_END, ##__VA_ARGS__)
  //  msg_printf__(PERR_BEGIN "(E) " format ERRMSG_END, ##__VA_ARGS__)
  //!Display warning message
#define pwrn(format, ...)                                       \
  msg_printf__(PWRN_BEGIN "(W) " format MSG_END, ##__VA_ARGS__)
  //!Display message
#define pmsg(format, ...)                                       \
  msg_printf__(PMSG_BEGIN "(M) " format MSG_END, ##__VA_ARGS__)
  //!display debug message, if NDEBUG isn't defined */
#define pdbg(format, ...)                                       \
  msg_printf__(PDBG_BEGIN "(D) " format MSG_END, ##__VA_ARGS__)

  //!perr message with leading file/function/line information
#define perr_ffl(x, ...)                                                \
  perr("[%s:%s:%d %s] " x,xstdio_tname(),__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)

  //!pwrn message with leading file/function/line information
#define pwrn_ffl(x, ...)                                                \
  pwrn("[%s:%s:%d %s] " x,xstdio_tname(),__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)

  //!pmsg message with leading file/function/line information
#define pmsg_ffl(x, ...)                                                \
  pmsg("[%s:%s:%d %s] " x,xstdio_tname(),__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)


  //!pdbg message with leading file/function/line information
#define pdbg_ffl(x, ...)                                                \
  pdbg("[%s:%s:%d %s] " x,xstdio_tname(),__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)


  //!perr_ffl with additional strerror(errno) output
#define perr_errno_ffl(x, ...)                  \
  perr("[%s:%s:%d %s] : errno= %d (%s) " x,         \
       xstdio_tname(),                            \
       __FILE__,__LINE__, __FUNCTION__,           \
       errno, strerror(errno)                     \
       ,##__VA_ARGS__)





  //!Display warning message (unbuffered)
#define pwrn_ub(format, ...)                                       \
  printf(PWRN_BEGIN "(W) " format MSG_END, ##__VA_ARGS__)
  //!Display message (unbuffered)
#define pmsg_ub(format, ...)                                       \
  printf(PMSG_BEGIN "(M) " format MSG_END, ##__VA_ARGS__)
  //!display debug message, if NDEBUG isn't defined  (unbuffered)
#define pdbg_ub(format, ...)                                        \
  printf(PDBG_BEGIN "(D) " format MSG_END, ##__VA_ARGS__)

  //!pwrn message with leading file/function/line information  (unbuffered)
#define pwrn_ffl_ub(x, ...)                                                \
  pwrn_ub("[%s:%s:%d %s] " x,xstdio_tname(),__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)

  //!pmsg message with leading file/function/line information  (unbuffered)
#define pmsg_ffl_ub(x, ...)                                                \
  pmsg_ub("[%s:%s:%d %s] " x,xstdio_tname(),__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)


  //!pdbg message with leading file/function/line information  (unbuffered)
#define pdbg_ffl_ub(x, ...)                                                \
  pdbg_ub("[%s:%s:%d %s] " x,xstdio_tname(),__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)


struct PLine
{
  //line number
  // uint64_t no;
  //line buffer
  char buf[XSTDIO_LINE_SZ];
};


#ifdef __cplusplus
}
#endif//__cplusplusc

#endif//__XSTDIO_H__

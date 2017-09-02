//emacs -*-Mode: C++;-*-
/*!
  @file xtrace.h

  utility functions for kernel tracing.

  set environment variable AM2B_DO_TRACE=1 to enable tracing.

  Copyright (C) Thomas Buschmann, Institute of Applied Mechanics, TU-Muenchen
  All rights reserved.
  Contact: buschmann@amm.mw.tum.de

*/
#ifndef __XTRACE_H__
#define __XTRACE_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
  //!initialize system tracing
  int trace_init();
  //!start system tracing
  int trace_start();
  //!stop tracing
  int trace_stop();

  /*!
    add user event (event string: *pfx-Lline)
    @param event: event number
    @param line : line number (__LINE__)
  */
  int trace_evt(const char*pfx, const int event, const int line);

  /*!
    add user event with timestamp (event string: *pfx-Lline-Sstamp)
    @param event: event number
    @param line : line number (__LINE__)
    @param stamp : time stamp
  */
  int trace_evt_stamp(const char*pfx, const int event, const int line, const uint64_t stamp);




#ifdef __cplusplus
}//extern C
#endif //__cplusplus

#endif//__XTRACE_H__

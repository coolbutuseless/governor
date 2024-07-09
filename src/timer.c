
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

#include "utils.h"

#define TIMER_MODE_CREATED 0
#define TIMER_MODE_CHECKED 1

typedef struct {
  double alarm;
  double interval;
  unsigned int mode;
} timer_struct;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Finalizer: Called via garbage collection by R itself
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void timer_finalizer(SEXP timer_) {
  timer_struct *timer = (timer_struct *)R_ExternalPtrAddr(timer_);
  free(timer);
  R_ClearExternalPtr(timer_);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Unpack an ExternalPointer to C struct pointer
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
timer_struct *unpack_ext_ptr_to_timer_struct(SEXP timer_) {
  
  if (!inherits(timer_, "timer")) {
    error("unpack_ext_ptr_to_timer_struct(): Not a 'timer_struct' object");
  }
  
  timer_struct *timer = TYPEOF(timer_) == EXTPTRSXP ? (timer_struct *)R_ExternalPtrAddr(timer_) : NULL;
  
  if (timer ==  NULL) {
    error("unpack_ext_ptr_to_timer_struct(): timer_struct pointer is invalid or NULL");
  }
  
  return timer;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Initialize a 'timer' sturctures
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP timer_init_(SEXP interval_, SEXP reset_mode_) {

  if (asReal(interval_) < 0) {
    error("timer_init_(): 'interval' cannot be negative");
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Alloc
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  timer_struct *timer = calloc(1, sizeof(timer_struct));
  if (timer == NULL) {
    error("timer_init_(): Could not allocate memory for 'timer_struct'");
  }
  
  if (strcmp(CHAR(STRING_ELT(reset_mode_, 0)), "checked")) {
    timer->mode = TIMER_MODE_CHECKED;
  } else {
    timer->mode = TIMER_MODE_CREATED;
  }
  
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup initial state
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  timer->interval = asReal(interval_); 
  
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  timer->alarm = ts_to_dbl(&ts) + timer->interval;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Wrap 'timer' as an ExternalPointer for R
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP timer_ = PROTECT(R_MakeExternalPtr(timer, R_NilValue, R_NilValue));
  R_RegisterCFinalizer(timer_, timer_finalizer);
  SET_CLASS(timer_, mkString("timer"));
  
  
  UNPROTECT(1);
  return timer_;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Linux
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// #include <time.h>
// struct timespec ts;
// timespec_get(&ts, TIME_UTC);
// struct timespec {
//   time_t   tv_sec;        /* seconds */
//   long     tv_nsec;       /* nanoseconds */
// };




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Use this wait within a for-loop or while-loop
//
// This function internally keeps track of the time since the last frame
// and adjusts the amount of 'sleep' in order to convert on a loop time
// that the user specified.
// If 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP timer_check_(SEXP timer_) {
  
  timer_struct *timer = unpack_ext_ptr_to_timer_struct(timer_);
  
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  double current_ts = ts_to_dbl(&ts);

  if (current_ts > timer->alarm) {
    if (timer->mode == TIMER_MODE_CHECKED) {
      timer->alarm = current_ts + timer->interval;
    } else {
      // Offset from when timer was created.  TIMER_MODE_CREATED
      timer->alarm += timer->interval;
    }
    return ScalarLogical(TRUE);
  }

  return ScalarLogical(FALSE);
}



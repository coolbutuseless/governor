
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

#include "utils.h"

typedef struct {
  double interval;     // The users specified interval (seconds)
  double alpha;        // Current learning rate
  double alpha_decay;  // Decay in learning rate
  double alpha_target; // Target learning rate in the long term
  double sleep_time;   // Amount of sleep required
  double prior_ts;     // timestamp on prior run
  double deficit;      // Deficit when we can't sleep
  int counter;         // Frame number
  bool valid;
} gov_struct;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Finalizer: Called via garbage collection by R itself
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void gov_finalizer(SEXP gov_) {
  gov_struct *gov = (gov_struct *)R_ExternalPtrAddr(gov_);
  free(gov);
  R_ClearExternalPtr(gov_);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Unpack an ExternalPointer to C struct pointer
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gov_struct *unpack_ext_ptr_to_gov_struct(SEXP gov_) {
  
  if (!inherits(gov_, "gov")) {
    error("unpack_ext_ptr_to_gov_struct(): Not a 'gov_struct' object");
  }
  
  gov_struct *gov = TYPEOF(gov_) == EXTPTRSXP ? (gov_struct *)R_ExternalPtrAddr(gov_) : NULL;
  
  if (gov ==  NULL) {
    error("unpack_ext_ptr_to_gov_struct(): gov_struct pointer is invalid or NULL");
  }
  
  return gov;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Initialize a 'gov' sturctures
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP gov_init_(SEXP interval_, SEXP alpha_, SEXP alpha_decay_, SEXP alpha_target_) {

  if (asReal(interval_) < 0) {
    error("gov_init_(): 'interval' cannot be negative");
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Alloc
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  gov_struct *gov = calloc(1, sizeof(gov_struct));
  if (gov == NULL) {
    error("init_gov_(): Could not allocate memory for 'gov_struct'");
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Setup initial state
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  gov->valid      = true;
  gov->interval   = asReal(interval_); // reference value
  gov->sleep_time = asReal(interval_); // Default sleep-time is just the interval
  gov->counter    = 0;                 // How many frames have we processed
  
  gov->alpha        = asReal(alpha_);         // Start high to respond quickly on startup. Then decrease
  gov->alpha_decay  = asReal(alpha_decay_);   // Start high to respond quickly on startup. Then decrease
  gov->alpha_target = asReal(alpha_target_);  // Start high to respond quickly on startup. Then decrease
  
  gov->deficit = 0; // frame deficit for consideration of frame skipping
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Wrap 'gov' as an ExternalPointer for R
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP gov_ = PROTECT(R_MakeExternalPtr(gov, R_NilValue, R_NilValue));
  R_RegisterCFinalizer(gov_, gov_finalizer);
  SET_CLASS(gov_, mkString("gov"));
  
  
  UNPROTECT(1);
  return gov_;
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
SEXP gov_wait_(SEXP gov_) {
  
  gov_struct *gov = unpack_ext_ptr_to_gov_struct(gov_);
  if (!gov->valid) return ScalarLogical(FALSE);
  
  struct timespec ts, rem, sleep;
  timespec_get(&ts, TIME_UTC);
  double current_ts = ts_to_dbl(&ts);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If this is first wait, then just wait for the sleeptime and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (gov->counter == 0) {
    gov->prior_ts = current_ts;
    dbl_to_ts(gov->sleep_time, &sleep);
    nanosleep(&sleep, &rem);
    gov->counter++;
    return ScalarLogical(FALSE);
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // How long is it since this function was called?
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  double delta = current_ts - gov->prior_ts;
  gov->prior_ts = current_ts;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Update sleep_time using an exponentially weighted moving average 
  // with 'learning rate'  = alpha
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  double new_sleep_time = gov->sleep_time - (delta - gov->interval);
  gov->sleep_time = (1 - gov->alpha) * gov->sleep_time + gov->alpha * new_sleep_time;
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If possible, sleep
  // otherwise add time overage to the 'deficit'
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (gov->sleep_time > 0) {
    dbl_to_ts(gov->sleep_time, &sleep);
    nanosleep(&sleep, &rem);
  } else {
    gov->deficit -= gov->sleep_time;
  }
  
  // Rprintf("%0.3f  %9.6f  %9.6f  : %9.6f  Skip: %i\n", gov->interval, delta, gov->sleep_time, gov->deficit, gov->deficit > gov->interval);

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Exponentially decay the learning rate
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (gov->alpha > gov->alpha_target) {
    gov->alpha *= gov->alpha_decay;
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // If we have enough deficit, we could advise the caller to skip a frame
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (gov->deficit > gov->interval) {
    gov->deficit -= gov->interval;
    return ScalarLogical(TRUE); // please skip the next frame if possible
  } else {
    return ScalarLogical(FALSE);
  }
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Disable a governor.  It will still be called, but rturn immediately.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP gov_disable_(SEXP gov_) {
  
  gov_struct *gov = unpack_ext_ptr_to_gov_struct(gov_);
  gov->valid = false;
  
  return R_NilValue;  
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP gov_enable_(SEXP gov_) {
  
  gov_struct *gov = unpack_ext_ptr_to_gov_struct(gov_);
  gov->valid = true;
  
  gov->counter = 0; // How many frames have we processed
  gov->deficit = 0; // frame deficit for consideration of frame skipping
  
  
  return R_NilValue;  
}

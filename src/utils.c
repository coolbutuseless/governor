
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

#include "utils.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Convert between double and timespec
//    e.g. 1.5 seconds = timespec(sec = 1, nsec = 500,000,000)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
double ts_to_dbl(struct timespec *ts) {
  return (double)ts->tv_sec + (double)ts->tv_nsec/1e9;
}

void dbl_to_ts(double time, struct timespec *ts) {
  ts->tv_sec  = (time_t)floor(time);
  ts->tv_nsec = (long)(1e9 * time - floor(time));
} 

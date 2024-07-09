
// #define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

extern SEXP gov_init_(SEXP interval_, SEXP alpha_, SEXP alpha_decay_, SEXP alpha_target_);
extern SEXP gov_wait_(SEXP gov_);
extern SEXP gov_disable_(SEXP gov_);
extern SEXP gov_enable_ (SEXP gov_);

extern SEXP timer_init_(SEXP interval_, SEXP reset_mode_);
extern SEXP timer_check_(SEXP timer_);
extern SEXP timer_disable_(SEXP timer_);
extern SEXP timer_enable_ (SEXP timer_);

static const R_CallMethodDef CEntries[] = {
  
  {"gov_init_"   , (DL_FUNC) &gov_init_   , 4},
  {"gov_wait_"   , (DL_FUNC) &gov_wait_   , 1},
  {"gov_disable_", (DL_FUNC) &gov_disable_, 1},
  {"gov_enable_" , (DL_FUNC) &gov_enable_ , 1},
  
  {"timer_init_"   , (DL_FUNC) &timer_init_   , 2},
  {"timer_check_"  , (DL_FUNC) &timer_check_  , 1},
  {"timer_disable_", (DL_FUNC) &timer_disable_, 1},
  {"timer_enable_" , (DL_FUNC) &timer_enable_ , 1},
  {NULL , NULL, 0}
};


void R_init_governor(DllInfo *info) {
  R_registerRoutines(
    info,      // DllInfo
    NULL,      // .C
    CEntries,  // .Call
    NULL,      // Fortran
    NULL       // External
  );
  R_useDynamicSymbols(info, FALSE);
}





// #define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

extern SEXP gov_init_(SEXP interval_, SEXP alpha_, SEXP alpha_decay_, SEXP alpha_target_);
extern SEXP gov_wait_(SEXP gov_);

static const R_CallMethodDef CEntries[] = {
  
  {"gov_init_", (DL_FUNC) &gov_init_, 4},
  {"gov_wait_", (DL_FUNC) &gov_wait_, 1},
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




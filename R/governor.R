

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Initialise a governor to control the speed of a for-loop
#' 
#' @param interval desired interval in seconds E.g. \code{interval = 1.5} sets the
#'        time-per-loop to 1.5 seconds.  E.g. \code{interval = 1/30} sets
#'        the loop to run at 30 times per second
#' @param alpha initial learning rate used to adjust wait time. Default: 0.4
#' @param alpha_decay rate at which alpha decays. Default: 0.95 i.e. 5% decrease
#'        each iteration
#' @param alpha_target the baseline alpha to reach when running. default: 0.05
#' @return gov object to be used with \code{gov_wait()}
#' @examples
#' # This loop should take approx 1 second
#' gov <- gov_init(1/30)
#' system.time({
#'   for (i in 1:30) {
#'     gov_wait(gov)
#'   }
#' })
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gov_init <- function(interval, alpha = 0.4, alpha_decay = 0.95, alpha_target = 0.05) {
  
  if (alpha <= 0 || alpha > 1) {
    stop("'alpha' must be in range (0, 1]")
  }
  
  if (alpha_decay < 0 || alpha_decay > 1) {
    stop("'alpha_decay' must be in range [0, 1]")
  }
  
  if (alpha_target <= 0 || alpha > alpha) {
    stop("'alpha_target' must be in range (0, alpha]")
  }
  
  if (.Platform$OS.type == 'unix') {
    .Call(gov_init_, interval, alpha, alpha_decay, alpha_target)
  } else {
    rgov_init(interval, alpha, alpha_decay, alpha_target)
  }
}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Wait an appropriate amount of time within a for-loop to match the desired
#' interval set in \code{gov}
#' 
#' @param gov object created by \code{gov_init()}
#' @return Logical value.  If \code{TRUE} then the governor is suggesting that
#'         the work for next loop be skipped if the loop interval is to 
#'         be maintained in the long term.
#' @examples
#' # This loop should take approx 1 second
#' gov <- gov_init(1/30)  # interval = 0.0333 seconds
#' system.time({
#'   for (i in 1:30) {
#'     Sys.sleep(0.01) # Work done in loop
#'     gov_wait(gov)   # Compensate to keep interval loop time
#'   }
#' })
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gov_wait <- function(gov) {
  if (.Platform$OS.type == 'unix') {
    invisible(.Call(gov_wait_, gov))
  } else {
    # This is windows, and we have to use 'rgov_init()' 
    gov()
  }
}



if (FALSE) {
  gov <- gov_init(1/10); gov_wait(gov); gov_wait(gov)
  # gov_wait(gov) |> bench::mark()

  gov <- gov_init(1/30); 
  skip <- FALSE
  system.time({
    for (i in 1:60) {
      if (!skip) {
        Sys.sleep(0.1)
      }
      skip <- gov_wait(gov)
    }
  })
  
}





























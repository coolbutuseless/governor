

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Create a timer object which will return \code{TRUE} when checked and the
#' given duration has elapsed.
#' 
#' The timer will automatically reset any time it is checked (via \code{check_timer()})
#' \emph{and} it returns \code{TRUE}.
#' 
#' @param duration timer duration in seconds
#' @param reset_mode one of 'checked' (default) or 'created' .  If 'checked', then
#'        when the timer is reset the next alarm will be set to \code{duration}
#'        seconds after \code{timer_check()} last returned \code{TRUE}.
#'        If 'created', then the time is reset to the next increment of \code{N * duration}
#'        after the timestamp when the timer was created
#' @return a 'timer' object to used with \code{timer_check()}
#' @examples
#' # Run two timers in a tight while loop
#' # The short timer should trigger every 0.1 seconds
#' # The long timer will trigger after 1 second
#' # Note that the timers will reset every time they trigger (after returning TRUE)
#' long_timer <- timer_init(1)
#' short_timer <- timer_init(0.1)
#' counter <- 0L
#' while(TRUE) {
#'   if (timer_check(long_timer)) {
#'     cat("\nLong  timer fired at count: ", counter, "\n")
#'     break;
#'   } 
#'   if (timer_check(short_timer)) {
#'     cat("Short timer fired at count: ", counter,  "\n")
#'   } 
#'   counter <- counter + 1L
#' }
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
timer_init <- function(duration, reset_mode = 'checked') {
  
  created <- Sys.time()
  target  <- created + duration
  count   <- 0L
  
  f <- function() {
    if (Sys.time() > target) {
      count <<- count + 1L
      if (reset_mode == 'checked') {
        target <<- Sys.time() + duration
      } else {
        target <<- created + count * duration
      }
      TRUE
    } else {
      FALSE
    }
  }
  
  
  f
}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Check the status of a timer
#'
#' @param timer timer object returned by \code{timer_init()}
#' @return logical indicating if timer was triggered. If \code{TRUE}, then 
#'         the internal state of the timer will be reset (so that it will 
#'         trigger again after another \code{duraction} has elapsed)
#' @examples
#' # Run two timers in a tight while loop
#' # The short timer should trigger every 0.1 seconds
#' # The long timer will trigger after 1 second
#' # Note that the timers will reset every time they trigger (after returning TRUE)
#' long_timer <- timer_init(1)
#' short_timer <- timer_init(0.1)
#' counter <- 0L
#' while(TRUE) {
#'   if (timer_check(long_timer)) {
#'     cat("\nLong  timer fired at count: ", counter, "\n")
#'     break;
#'   } 
#'   if (timer_check(short_timer)) {
#'     cat("Short timer fired at count: ", counter,  "\n")
#'   } 
#'   counter <- counter + 1L
#' }
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
timer_check <- function(timer) {
  timer()
}



if (FALSE) {
  long_timer <- timer_init(1)
  short_timer <- timer_init(0.1)
  counter <- 0L
  while(TRUE) {
    if (timer_check(long_timer)) {
      message("\nLong  timer fired at count: ", counter)
      break;
    } 
    if (timer_check(short_timer)) {
      message("Short timer fired at count: ", counter)
    } 
    counter <- counter + 1L
  }
}




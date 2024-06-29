
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Init a governor which only uses base R (useful for Windows)
#' 
#' Windows timing APIs are a bit complex and this is that governor that
#' will be used 
#' 
#' @inheritParams gov_init
#' @return a function
#' @examples
#' # Set the interval to 0.1 seconds per loop
#' rgov_wait <- rgov_init(interval = 1/10)
#' 
#' # The following loop should take about 10 * 0.1 = 1 second.
#' # rgov_wait() accounts for time spent executing the contents of the loop.
#' system.time({
#'   for (i in 1:10) {
#'     Sys.sleep(0.05)  # Work done in loop
#'     rgov_wait()      # Compensate to keep interval loop time
#'   }
#' })
#' 
#' @noRd
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
rgov_init <- function(interval, alpha = 0.4, alpha_decay = 0.95, alpha_target = 0.05) {
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Variables in the 'rgov_wait()' function environment
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  stime    <- interval
  counter  <- 1L
  prior_ts <- NA
  deficit  <- 0
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # Create the 'rgov_wait()' function
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  f <- function(...) {
  
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # Initi block
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (is.na(prior_ts)) {
      prior_ts <<- Sys.time()
      Sys.sleep(stime)
      return(invisible(FALSE))
    }
    
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # time since last call
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    now   <- Sys.time()
    delta <- as.numeric(now - prior_ts)
    prior_ts <<- now
    
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # Adjust the sleep tim
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    new_stime <- stime - (delta - interval)
    stime <<- (1 - alpha) * stime + alpha * new_stime
    
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # Wait for the required time
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (stime > 0) {
      Sys.sleep(stime)
    } else {
      deficit <<- deficit - stime
    }
    
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # Dynamically decay the 'alpha' value over the first 20 calls
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (alpha > alpha_target) {
      alpha <<- alpha * alpha_decay
    }
    counter <<- counter + 1L
    
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # rgov_wait() does not return anything
    #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (deficit > interval) {
      deficit <<- deficit - interval
      invisible(TRUE)
    } else {
      invisible(FALSE)
    }
  }
  
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  # 'rgov_init()' returns the 'rgov_wait()' governor
  #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  f
}


if (FALSE) {
  rgov_wait <- rgov_init(interval = 1/10, alpha = 0.9)
  
  system.time({
    for (i in 1:10) {
      Sys.sleep(0.08)
      rgov_wait()
    }
  })
  
  
  rgov_wait <- rgov_init(interval = 1/20, alpha = 0.9)
  
  system.time({
    skip <- FALSE
    for (i in 1:40) {
      message(skip)
      if (!skip) {
        Sys.sleep(0.06)
      } 
      skip <- rgov_wait()
    }
  })
  
  
}


<!-- README.md is generated from README.Rmd. Please edit that file -->

# governor

<!-- badges: start -->

![](https://img.shields.io/badge/cool-useless-green.svg)
[![R-CMD-check](https://github.com/coolbutuseless/governor-dev/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/coolbutuseless/governor-dev/actions/workflows/R-CMD-check.yaml)
<!-- badges: end -->

`{governor}` is a
[governor](https://en.wikipedia.org/wiki/Governor_(device)) (or *speed
limiter*) which limits the rate at which a *for*-loop or *while*-loop
will run.

The total execution speed is limited by inserting short pauses based
upon the time to run through the loop. The waiting time is adjusted
continuously to meet the target duration.

### What’s in the box

- `gov_init()`, `gov_wait()` for limiting the execution speed of a
  for-loop, while-loop or repeated function call.
- `timer_init()`, `timer_check()` for setting alarms such that
  `time_check()` will return `TRUE` after the given time has elapsed

## Installation

You can install from
[GitHub](https://github.com/coolbutuseless/governor) with:

``` r
# install.packages('remotes')
remotes::install_github('coolbutuseless/governor')
```

## Running a loop at a rate of 30 times/second

A common use for `{governor}` will be timing animation at, say, 30
frames per second.

``` r
library(governor)

# Run loop at 30fps if possible
gov <- gov_init(1/30); 

# Running the loop 30 times at 30 frames-per-second should take ~1 second
# The actual work in this loop only takes 0.3seconds (30 * 0.01)
# So `gov_wait()` will pause every loop to maintain the interval
system.time({
  for (i in 1:30) {
    Sys.sleep(0.01)  # Work done in loop
    gov_wait(gov)    # Compensate to keep interval loop time
  }
})
#>    user  system elapsed 
#>   0.012   0.000   1.040
```

## Skipping frames

When the actual work in the loop is fast, `{governor}` can compensate by
waiting a longer amount of time.

When the work in the loop is slow, then `{governor}` can advise that the
work for the next frame be skipped.

In this example, we want the loop to run at 30 frames per second
(i.e. an interval of 0.333 seconds), but the work itself takes 0.04
seconds. The return value of `gov_wait()` is a logical value indicating
whether it is recommended that the next frame is skipped in order to
achieve the desired loop interval.

In the output from this code, the `skip` variable is printed to show
that `gov_wait()` is advising that many frames should be skipped.

``` r
library(governor)

# Run loop at 30fps if possible
# Set a high learning rate so it will converge quickly
gov <- gov_init(1/30); 

# Running the loop 30 times at 30 frames-per-second should take ~1 second
# The actual work should take a total of 0.1 * 30 = 3 seconds!
system.time({
  skip <- FALSE
  for (i in 1:30) {
    if (!skip) {
      Sys.sleep(0.1)  # Work done in loop
    }
    skip <- gov_wait(gov)    # Compensate to keep interval loop time
    cat(skip, "\n")
  }
})
#> FALSE 
#> FALSE 
#> TRUE 
#> FALSE 
#> TRUE 
#> TRUE 
#> TRUE 
#> TRUE 
#> FALSE 
#> FALSE 
#> TRUE 
#> TRUE 
#> TRUE 
#> FALSE 
#> TRUE 
#> TRUE 
#> FALSE 
#> TRUE 
#> TRUE 
#> FALSE 
#> TRUE 
#> TRUE 
#> FALSE 
#> TRUE 
#> TRUE 
#> FALSE 
#> TRUE 
#> TRUE 
#> FALSE 
#> TRUE
#>    user  system elapsed 
#>   0.004   0.000   1.238
```

## Setting timers

Timers are like alarm clocks that will return `TRUE` when the given
duration has elapsed.

After returning `TRUE`, the timer will reset internally such that they
will trigger again after another period has elapsed.

``` r
long_timer  <- timer_init(1)
short_timer <- timer_init(0.1)
counter <- 0L
while(TRUE) {
  if (timer_check(long_timer)) {
    cat("\nLong  timer fired at count: ", counter, "\n")
    break;
  } 
  if (timer_check(short_timer)) {
    cat("Short timer fired at count: ", counter, "\n")
  } 
  counter <- counter + 1L
}
#> Short timer fired at count:  97000 
#> Short timer fired at count:  213149 
#> Short timer fired at count:  329055 
#> Short timer fired at count:  445058 
#> Short timer fired at count:  561283 
#> Short timer fired at count:  676199 
#> Short timer fired at count:  791301 
#> Short timer fired at count:  907952 
#> Short timer fired at count:  1014527 
#> 
#> Long  timer fired at count:  1130692
```

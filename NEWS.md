# governor 0.1.2.9001

* Fix 'Description' text to not start with the 'Title' text. CRAN notified me
  of this redundancy.
* Fix bug with dbl-to-timestamp conversion which was
  only evident when governor interval > 1 second

# governor 0.1.2  2024-07-11

* Fixes for CRAN
    * More descriptive DESCRIPTION

# governor 0.1.1  2024-07-09

* Windows now uses `nanosleep()`
* `timer` has now moved to C
* Option to enable/disable timers and governors

# governor 0.1.0  2024-06-29

* Initial release

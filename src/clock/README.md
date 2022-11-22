# The Clock

* Basic clock implementation
* Mainly for timeout management
* Has basic functionalities of a clock such as:
    * `check()` : checks elapsed time in `ms` since the reset of the clock.
    * `reset()` : resets the time of the clock to now.
    * `wait(int ms)` : waits for a certain amount of `ms`

This class is mainly used to detect timeouts in the ReplController, Client and Server classes.
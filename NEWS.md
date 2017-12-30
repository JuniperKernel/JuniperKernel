**If you are viewing this file on CRAN, please check latest news on GitHub [here](https://github.com/JuniperKernel/JuniperKernel/blob/master/NEWS.md).**

### Changes in v1.2.0.0 (in development as v1.1.0.0)

#### NEW FEATURES

1. `jclient`:
  * A minimal 'Jupyter' client is implemented to support testing of 'JuniperKernel'. This
    client is compiled with the full package and has an un-exported API. The documentation of
    this API is available via the usage in tests/doRunit.R and the related files.
  * Jupyter protocol tests are now made part of this package's CI.

#### BUG FIXES

* `shutdown_request` messages now cause a clean shutdown (with exit 0). Before this fix, a `shutdown_request` would trigger a `q("no")` from within R before the shell, ctrl, and stdin sockets could clean themslves up. This would cause the application to "hang" when it attempted to destroy the `zmq::context_t`, and hence the kernel would need to be forcefully shutdown.
* `is_complete_request` was missing an `else if` between the two `incomplete` checks resulting in an erroneous `invalid` state when a quote was missing.
* Fixed Github issue #8 "Completion suggests “N” and “A” when there’s nothing to show": `complete_request` now returns an empty list `list()` when no matches are found (was retunging `NA`).

#### NOTES

* The 'zeromq' and 'pbdZMQ' static sources have been removed with a dependency on the R package
  'pbdZMQ'. This is reflected in the DESCRIPTION file. Thank you to Wei-Chen Chen (@snoweye) from the
  pbdR core team for making this happen.


### v1.0.0.0 released to CRAN in Dec. 2017

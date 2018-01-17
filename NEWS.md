**If you are viewing this file on CRAN, please check latest news on GitHub [here](https://github.com/JuniperKernel/JuniperKernel/blob/master/NEWS.md).**

### Changes in v1.4.0.0 (in dev as v1.3.0.0)

### Changes in v1.2.2.0 (on CRAN 01/16/2018)

#### PATCHES

1. Point to pbdZMQ R package version 0.3-1. This enables external zeromq dependencies to be found
   included/linked correctly.

### Changes in v1.2.1.0 (on CRAN 01/05/2018)

#### PATCHES

1. Remove `subprocess` dependency as it is unavailable on Solaris.
2. Jupyter protocol tests are done in CI only.
3. configure pulls in the paths by hand from the installed pbdZMQ package.
4. include zeromq headers

### Changes in v1.2.0.0 (on CRAN 01/04/2018)

#### NEW FEATURES

1. `jclient`:
  * A minimal 'Jupyter' client is implemented to support testing of 'JuniperKernel'. This
    client is compiled with the full package and has an un-exported API. The documentation of
    this API is available via the usage in tests/doRunit.R and the related files.
  * Jupyter protocol tests are now made part of this package's CI.

#### BUG FIXES

* `shutdown_request` messages now cause a clean shutdown (with exit 0). Before this fix, a `shutdown_request` would trigger a `q("no")` from within R before the shell, ctrl, and stdin sockets could clean themslves up. This would cause the application to "hang" when it attempted to destroy the `zmq::context_t`, and hence the kernel would need to be forcefully shutdown.
* `is_complete_request` was missing an `else if` between the two `incomplete` checks resulting in an erroneous `invalid` state when a quote was missing.
* Fixed Github issue #8 "Completion suggests “N” and “A” when there’s nothing to show": `complete_request` now returns an empty list `list()` when no matches are found (was returning `NA`).

#### NOTES

* The 'zeromq' and 'pbdZMQ' static sources have been removed with a dependency on the R package
  'pbdZMQ'. This is reflected in the DESCRIPTION file. Thank you to Wei-Chen Chen (@snoweye) from the
  pbdR core team for making this happen.


### v1.0.0.0 released to CRAN on 12/19/2017

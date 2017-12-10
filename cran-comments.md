## Resubmission
This is a resubmission. In this version I have:

* Used single quotes for software names (e.g. 'Rcpp', 'Jupyter', 'Xeus')

* Expanded the Description field of DESCRIPTION to describe 'Jupyter' and 'Xeus' and provide links in the form <http://...> and <https://...>.

* Explained more clearly that this package provides interoperability with the 'Jupyter' ecosystem (and hopefully made clear that there will not be multiple packages).

* Removed redundant "R" in the package title.

* Added COPYRIGHTS file to inst/ and included all copyright holders into the Authors@R field.

* Renamed the arguments somewhat in the `installJuniper` function to clearly illustrate that the `prefix` field is for sepcifying custom paths. Additionally, the
  `user` argument has been renamed to illsutrate that it relates to the default 'Jupyter' installation and has a new default value of FALSE so that the user does
  not install into a user-local directory by default.

* Added dynamically linkable zeromq libs for linux distribution Ubuntu 14.04.5 LTS.

## Test environments
* local OS X install, R 3.4.3
* local Windows 10 install, R 3.4.3 and R 3.4.1
* ubuntu 14.04.5 LTS (on travis-ci), R 3.4.3
* Windows Server 2012 R2 x64 (build 9600) (on Appveyor), R 3.4.3
* win-builder (release, devel, oldrelease)

## R CMD check results
There were no ERRORs or WARNINGs.

For all builds there was the New submission NOTE:

```
* checking CRAN incoming feasibility ... NOTE
Maintainer: 'Spencer Aiello <spnrpa@gmail.com>'
 
New submission
```

For all Windows builds there was an additional NOTE:

```
* checking installed package size ... NOTE
  installed size is 17.9Mb
  sub-directories of 1Mb or more:
    include   1.5Mb
    libs     16.4Mb
```

The libs directory contain the zeromq and JuniperKernel dlls for both 32/64-bit architectures. The
windows ZeroMQ DLLs + headers are linked by JuniperKernel and so must be included. There is an R
package on CRAN (pbdZMQ) that bundles the zeromq headers and shared objects, but these cannot be
linked as-is.

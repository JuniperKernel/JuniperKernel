## JuniperKernel 1.2.0.0 Submission
In this version we have:
  * Added a NEWS.md
  * Removed 
  * Added an additional set of tests (included with the package in inst/runits)
  * Updated the COPYRIGHTS file and DESCRIPTION file to credit the xeus, xtl, and json
    dependency authors.
  * Removed the static zeromq sources 'inst/pbdZMQ-0.2-6.zip' and 'inst/zeromq-4.2.2.tar.gz'
  * Created a dependency on the 'pbdZMQ' package for ZeroMQ headers and dynamic libraries.
  * No longer compiling with flag -fPIC.
  * Made the configure script more portable

## Test environments
* local OS X install, R 3.4.3
* local Windows 10 install, R 3.4.3 and R.3.4.1
* ubuntu 14.04.5 LTS (on travis-ci), R 3.4.3
* Windows Server 2012 R2 x64 (build 9600) (on Appveyor), R 3.4.3
* win-builder (release, devel, oldrelease)

## R CMD check results
There were no ERRORs or WARNINGs.

On Linux builds I observed the NOTE:

```
* checking installed package size ... NOTE
  installed size is  5.5Mb
  sub-directories of 1Mb or more:
    include   1.1Mb
    libs      4.2Mb
```
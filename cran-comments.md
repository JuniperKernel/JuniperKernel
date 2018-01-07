## JuniperKernel 1.2.1.0 Submission

This submission patches version 1.2.0.0 with:
  * includes missing headers from Unix environments
  * removes the dependency on subprocess so that builds succeed on Solaris

## Test environments
* local OS X install, R 3.4.3
* local Windows 10 install, R 3.4.3 and R.3.4.1
* ubuntu 14.04.5 LTS (on travis-ci), R 3.4.3
* Windows Server 2012 R2 x64 (build 9600) (on Appveyor), R 3.4.3
* win-builder (release, devel, oldrelease)
* Oracle Solaris 10, x86, 32 bit, R-patched
* Debian Linux, R-devel, GCC

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
## JuniperKernel 1.2.3.0 Submission
In this version we have:
* This is a patch submission to update the 'pbdZMQ' dependency to the latest version.

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

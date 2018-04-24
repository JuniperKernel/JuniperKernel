## JuniperKernel 1.4.1.0 Submission
In this version we have:
* Made a patch to listen for Windows events

## Test environments
* local OS X install, R 3.4.3
* local Windows 10 install, R 3.4.3
* ubuntu 14.04.5 LTS (on travis-ci), R 3.5.0
* Windows Server 2012 R2 x64 (build 9600) (on Appveyor), R 3.4.4
* win-builder (release, devel)

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

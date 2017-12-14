## Resubmission 2
This is the second resubmission of 'JuniperKernel' 1.0.0.0. In this version I have:

* Removed the install-time dependency of the packages in the 'Suggests' section.

* Bundled the Rd man files with the source 'JuniperKernel' package.

* Removed all install-time downloads of binaries and headers.

* Bundled source files for 'ZeroMQ' and updated the copyright information in the COPYRIGHTS file.

* Reconfigured the build scripts for all platforms (Linux, Mac, and Windows) so that 'ZeroMQ' libraries are created
  at install time of the source package. Notably, this shrinks the Windows installed package size by 10MB to an
  observed size of:

```
* checking installed package size ... NOTE
  installed size is  7.7Mb
  sub-directories of 1Mb or more:
    include   1.5Mb
    libs      6.1Mb
```

* Fixed ISO C++ forbidden warnings by removing variable length arrays and compound literals. Compilation with the
  -Wall -pedantic flags were checked on a local Ubuntu 14.04, Travis CI Ubuntu 14.04, local Mac, local Windows, and
  AppVeyor Windows.

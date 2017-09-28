# <img src="./extras/juniper_logo.png" width=65>JuniperKernel

An [R](https://cran.r-project.org/) kernel for [Jupyter](https://jupyter.org) based on [Xeus](https://github.com/QuantStack/xeus) and built with [Rcpp](http://www.rcpp.org/).

## Building

##### Requirements

- R >= 3.4.0 
- R packages: BH,  Rcpp (>= 0.11.0), gdtools (>= 0.1.6), roxygen2, jsonlite, repr
- [Rtools34.exe](https://cran.r-project.org/bin/windows/Rtools/) for Windows

Other necessary bacon bits (fetched automatically--c.f. Makevars/Makevars.win):

- zeromq (4.4.2)
- xeus (0.6.0)
- xtl (0.2.3)

#### Building on Windows

Note that only the mingw64 compiler that is bundled with Rtools is supported. No other compilers are officially supported. Additionally this means that only 64-bit Windows is supported.

To build and install the Juniper kernel run the following from a `cmd` prompt:

```
cmd /c mk.bat
```

Alternatively, you can run the build from RStudio. 

You should see the following compilation output:

```
C:/Rtools/mingw_64/bin/g++  -std=gnu++11 -I"C:/PROGRA~1/R/R-34~1.1/include" -DNDEBUG -I../inst/include -I. -Wno-conversion-null -I"C:/Program Files/R/R-3.4.1/library/Rcpp/include" -I"C:/Program Files/R/R-3.4.1/library/gdtools/include" -I"C:/Program Files/R/R-3.4.1/library/BH/include"   -I"d:/Compiler/gcc-4.9.3/local330/include"     -O2 -Wall  -mtune=core2 -c RcppExports.cpp -o RcppExports.o
C:/Rtools/mingw_64/bin/g++  -std=gnu++11 -I"C:/PROGRA~1/R/R-34~1.1/include" -DNDEBUG -I../inst/include -I. -Wno-conversion-null -I"C:/Program Files/R/R-3.4.1/library/Rcpp/include" -I"C:/Program Files/R/R-3.4.1/library/gdtools/include" -I"C:/Program Files/R/R-3.4.1/library/BH/include"   -I"d:/Compiler/gcc-4.9.3/local330/include"     -O2 -Wall  -mtune=core2 -c juniper.cpp -o juniper.o
C:/Rtools/mingw_64/bin/g++ -shared -s -static-libgcc -o JuniperKernel.dll tmp.def RcppExports.o juniper.o -lzmq -Lx64 -Ld:/Compiler/gcc-4.9.3/local330/lib/x64 -Ld:/Compiler/gcc-4.9.3/local330/lib -LC:/PROGRA~1/R/R-34~1.1/bin/x64 -lR
installing to C:/Program Files/R/R-3.4.1/library/JuniperKernel/libs/x64
** R
** inst
** preparing package for lazy loading
No man pages found in package  'JuniperKernel' 
** help
*** installing help indices
** building package indices
** testing if installed package can be loaded
* DONE (JuniperKernel)
```

### Installation

In order to build 


#### Blog Post: https://spenai.org/bravepineapple/jupyter_kernel/


## Build Instructions

#### Windows

```
cmd /c mk.bat
```

#### OSX

```
$ make && make install
```


#### Juniper In Action

Juniper Screenshot:

![](./extras/jnote.png)


#### xwidgets demo:

xwidgets integration screenshot:
![](./extras/xwidgets_demo.png)

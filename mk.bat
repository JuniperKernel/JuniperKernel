:: clone xeus fork
git clone https://github.com/JuniperKernel/xeus.git
if %ERRORLEVEL% EQU 1 exit 1

C:\Rtools\bin\mv xeus src
if %ERRORLEVEL% EQU 1 exit 1

Rscript -e Rcpp::compileAttributes()
if %ERRORLEVEL% EQU 1 exit 1

Rscript -e roxygen2::roxygenize()
if %ERRORLEVEL% EQU 1 exit 1

::Rscript -e "install.packages('devtools',repo='https://cran.r-project.org')"
::if %ERRORLEVEL% EQU 1 exit 1

::Rscript -e "devtools::install_github('snoweye/pbdZMQ')"
::if %ERRORLEVEL% EQU 1 exit 1

Rscript -e "install.packages('subprocess',repo='https://cran.r-project.org')"
if %ERRORLEVEL% EQU 1 exit 1

R CMD build .
if %ERRORLEVEL% EQU 1 exit 1

mkdir build
C:\Rtools\bin\mv Juniper*gz build
cd build
R CMD INSTALL Juniper*gz

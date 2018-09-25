::quantstack bacon bits
:: xtl
set "XTL_VERSION=0.4.16"
set "XTL_TAR_FILE=%XTL_VERSION%.tar.gz"
set "XTL_URL=https://github.com/QuantStack/xtl/archive/%XTL_TAR_FILE%"

:: xeus
set "XEUS_VERSION=0.14.1"
set "XEUS_TAR_FILE=%XEUS_VERSION%.tar.gz"
set "XEUS_URL=https://github.com/QuantStack/xeus/archive/%XEUS_TAR_FILE%"

:: fetch headers and put them in inst\include
if not exist ".\inst\include" mkdir .\inst\include

:: fetch xtl headers
powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest %XTL_URL% -OutFile %XTL_TAR_FILE%"
C:\Rtools\bin\tar -xzf %XTL_TAR_FILE%
C:\Rtools\bin\mv xtl-%XTL_VERSION%\include\xtl .\inst\include
C:\Rtools\bin\rm -rf xtl-%XTL_VERSION% %XTL_TAR_FILE%


:: fetch xeus headers
powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest %XEUS_URL% -OutFile %XEUS_TAR_FILE%"
C:\Rtools\bin\tar -xzf %XEUS_TAR_FILE%
C:\Rtools\bin\mv xeus-%XEUS_VERSION%\include\xeus .\inst\include
C:\Rtools\bin\rm -rf xeus-%XEUS_VERSION% %XEUS_TAR_FILE%

Rscript -e Rcpp::compileAttributes()
if %ERRORLEVEL% EQU 1 exit 1

Rscript -e "install.packages('devtools',repo='https://cran.r-project.org')"
if %ERRORLEVEL% EQU 1 exit 1

:: install a version of roxygen2 that does not attempt to build the package...
Rscript -e "devtools::install_version('roxygen2', version='6.0.1', repo='https://cran.r-project.org')"
if %ERRORLEVEL% EQU 1 exit 1

Rscript -e roxygen2::roxygenize()
if %ERRORLEVEL% EQU 1 exit 1

Rscript -e "install.packages('subprocess',repo='https://cran.r-project.org')"
if %ERRORLEVEL% EQU 1 exit 1

R CMD build .
if %ERRORLEVEL% EQU 1 exit 1

mkdir build
C:\Rtools\bin\mv Juniper*gz build
cd build
R CMD INSTALL Juniper*gz
::R CMD INSTALL --no-multiarch Juniper*gz 

:: zmq bacon bits
set "ZMQ_VERSION=4.2.2"
set "ZMQ_HEADER_TAR_FILE=zmq-%ZMQ_VERSION%-headers.tar.gz"
set "ZMQ_HEADERS_URL=https://github.com/JuniperKernel/zmq/raw/master/headers/%ZMQ_HEADER_TAR_FILE%"
set "ZMQ_CPP_HEADER_URL=https://github.com/zeromq/cppzmq/raw/master/zmq.hpp"
set "ZMQ_CPPADDON_HEADER_URL=https://raw.githubusercontent.com/zeromq/cppzmq/master/zmq_addon.hpp"

::quantstack bacon bits
:: xtl
set "XTL_VERSION=0.3.1"
set "XTL_TAR_FILE=%XTL_VERSION%.tar.gz"
set "XTL_URL=https://github.com/QuantStack/xtl/archive/%XTL_TAR_FILE%"

:: xeus
set "XEUS_VERSION=0.7.0"
set "XEUS_TAR_FILE=%XEUS_VERSION%.tar.gz"
set "XEUS_URL=https://github.com/QuantStack/xeus/archive/%XEUS_TAR_FILE%"

:: fetch headers and put them in inst\include
if not exist ".\inst\include" mkdir .\inst\include

:: fetch zmq headers
powershell -Command "Invoke-WebRequest %ZMQ_CPP_HEADER_URL% -OutFile .\inst\include\zmq.hpp"
powershell -Command "Invoke-WebRequest %ZMQ_CPPADDON_HEADER_URL% -OutFile .\inst\include\zmq_addon.hpp"
powershell -Command "Invoke-WebRequest %ZMQ_HEADERS_URL% -OutFile %ZMQ_HEADER_TAR_FILE%"
C:\Rtools\bin\tar -xzf %ZMQ_HEADER_TAR_FILE%
C:\Rtools\bin\mv include/*h .\inst\include
C:\Rtools\bin\rm -rf %ZMQ_HEADER_TAR_FILE% include

:: fetch xtl headers
powershell -Command "Invoke-WebRequest %XTL_URL% -OutFile %XTL_TAR_FILE%"
C:\Rtools\bin\tar -xzf %XTL_TAR_FILE%
C:\Rtools\bin\mv xtl-%XTL_VERSION%\include\xtl .\inst\include
C:\Rtools\bin\rm -rf xtl-%XTL_VERSION% %XTL_TAR_FILE%


:: fetch xeus headers
powershell -Command "Invoke-WebRequest %XEUS_URL% -OutFile %XEUS_TAR_FILE%"
C:\Rtools\bin\tar -xzf %XEUS_TAR_FILE%
C:\Rtools\bin\mv xeus-%XEUS_VERSION%\include\xeus .\inst\include
C:\Rtools\bin\rm -rf xeus-%XEUS_VERSION% %XEUS_TAR_FILE%

Rscript -e Rcpp::compileAttributes()
if %ERRORLEVEL% EQU 1 exit 1

Rscript -e roxygen2::roxygenize()
if %ERRORLEVEL% EQU 1 exit 1

R CMD INSTALL --build .
if %ERRORLEVEL% EQU 1 exit 1

Rscript -e Rcpp::compileAttributes()
if %ERRORLEVEL% EQU 1 exit 1

Rscript -e roxygen2::roxygenize()
if %ERRORLEVEL% EQU 1 exit 1

mkdir build
R CMD build .
mv Juniper*tar.gz build

cd build && R CMD check --as-cran *tar.gz
if %ERRORLEVEL% EQU 1 exit 1

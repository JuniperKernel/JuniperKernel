make clean
if %ERRORLEVEL% EQU 1 exit 1

Rscript -e Rcpp::compileAttributes()
if %ERRORLEVEL% EQU 1 exit 1

Rscript -e roxygen2::roxygenize()
if %ERRORLEVEL% EQU 1 exit 1

mkdir build
R CMD build .
mv Juniper*tar.gz build

R CMD check --no-manual --no-build-vignettes --as-cran build\*tar.gz
if %ERRORLEVEL% EQU 1 exit 1

R -f scripts\r_cmd_check_validate.R
Rscript -e Rcpp::compileAttributes()
if %ERRORLEVEL% EQU 1 exit 1

Rscript -e roxygen2::roxygenize()
if %ERRORLEVEL% EQU 1 exit 1

R CMD INSTALL --no-multiarch --build .
if %ERRORLEVEL% EQU 1 exit 1

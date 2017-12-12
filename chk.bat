R CMD check --no-manual --no-build-vignettes --as-cran build\*tar.gz
if %ERRORLEVEL% EQU 1 exit 1

R -f scripts\r_cmd_check_validate.R

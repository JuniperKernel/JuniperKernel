#SHELL := /bin/bash
.DELETE_ON_ERROR:

# for printing variable values
# usage: make print-VARIABLE
#        > VARIABLE = value_of_variable
print-%  : ; @echo $*=$($*)

PKG_VERSION=3.0.0.1

R_BUILD_ARGS= --no-manual --no-build-vignettes
R_CHECK_ARGS= --no-manual --no-build-vignettes --as-cran

R_HOME=$(shell R RHOME)
R=$(R_HOME)/bin/R
RSCRIPT=$(R_HOME)/bin/Rscript

default: build/JuniperKernel_$(PKG_VERSION).tar.gz

build/JuniperKernel_$(PKG_VERSION).tar.gz: $(wildcard R/*R) $(wildcard src/*cpp) DESCRIPTION xeus
	@echo "building " $@ " because " $?
	@${RSCRIPT} -e 'Rcpp::compileAttributes()'
	@$(RSCRIPT) -e 'roxygen2::roxygenize()'
	@$(R) CMD build $(R_BUILD_ARGS) .
	@[ -d build ] || mkdir build
	@mv JuniperKernel_$(PKG_VERSION).tar.gz build/

#.PHONY: install
install: build/JuniperKernel_$(PKG_VERSION).tar.gz
	@(cd build && R CMD INSTALL JuniperKernel_$(PKG_VERSION).tar.gz)

xeus: src/xeus

src/xeus:
	@echo "Fetching xeus..."
	@(cd src && git clone https://github.com/JuniperKernel/xeus.git)

check: build/JuniperKernel_$(PKG_VERSION).tar.gz
	@echo "running R CMD check"
	@$(R) CMD check $(R_CHECK_ARGS) build/JuniperKernel_$(PKG_VERSION).tar.gz
	@$(R) -f scripts/r_cmd_check_validate.R

test: build/JuniperKernel_$(PKG_VERSION).tar.gz
	@echo "running R CMD check"
	@$(R) CMD INSTALL build/JuniperKernel_$(PKG_VERSION).tar.gz
	@${RSCRIPT} -e "install.packages('subprocess', repos='http://cran.rstudio.com')"
	@$(R) -f tests/doRunit.R

.PHONY: clean
clean:
	rm -rf build
	rm -rf man
	rm -rf JuniperKernel.Rcheck
	rm -rf R/RcppExports.R src/RcppExports.cpp
	rm -rf src/*o
	rm -rf *gz *zip
	rm -rf src/Makevars
	rm -rf src/*dll
	rm -rf inst/zmq
	#rm -rf inst/include/zmq*
	rm -rf inst/include/x*
	rm -rf src/*gz
	rm -rf src/._*
	rm -rf src/x64
	rm -rf inst/._*
	rm -rf *.ipynb*
	rm -rf inst/libs
	rm -rf src-*
	rm -rf src/xeus

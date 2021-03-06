.DELETE_ON_ERROR:

# for printing variable values
# usage: make print-VARIABLE
#        > VARIABLE = value_of_variable
print-%  : ; @echo $*=$($*)

PKG_VERSION=1.4.2.0

R_BUILD_ARGS= --no-manual --no-build-vignettes
R_CHECK_ARGS= --no-manual --no-build-vignettes --as-cran

R_HOME=$(shell R RHOME)
R=$(R_HOME)/bin/R
RSCRIPT=$(R_HOME)/bin/Rscript

#quantstack bacon bits
# xtl
XTL_VERSION = 0.4.16
XTL_TAR_FILE = ${XTL_VERSION}.tar.gz
XTL_URL = https://github.com/QuantStack/xtl/archive/${XTL_TAR_FILE}

# xeus
XEUS_VERSION = 0.14.1
XEUS_TAR_FILE = ${XEUS_VERSION}.tar.gz
XEUS_URL = https://github.com/QuantStack/xeus/archive/${XEUS_TAR_FILE}

default: build/JuniperKernel_$(PKG_VERSION).tar.gz

build/JuniperKernel_$(PKG_VERSION).tar.gz: $(wildcard R/*R) $(wildcard src/*cpp) DESCRIPTION headers
	@echo "building " $@ " because " $?
	@${RSCRIPT} -e 'Rcpp::compileAttributes()'
	@$(RSCRIPT) -e 'roxygen2::roxygenize()'
	@$(R) CMD build $(R_BUILD_ARGS) .
	@[ -d build ] || mkdir build
	@mv JuniperKernel_$(PKG_VERSION).tar.gz build/

#.PHONY: install
install: install_roxygen2_5 build/JuniperKernel_$(PKG_VERSION).tar.gz
	@(cd build && R CMD INSTALL JuniperKernel_$(PKG_VERSION).tar.gz)

install_roxygen2_5:
	@echo "Installing roxygen 6.0.1..."
	@$(RSCRIPT) -e "install.packages('devtools', repos='http://cran.rstudio.com')"
	@$(RSCRIPT) -e "devtools::install_version('roxygen2', version='6.0.1', repos='http://cran.rstudio.com')"

headers: ./inst/include/xtl ./inst/include/xeus

./inst/include/xtl:
	@echo "Fetching xtl headers..."
	@[ -d ./inst/include ] || mkdir -p ./inst/include
	@curl -skLO ${XTL_URL}
	@tar -xzf ${XTL_TAR_FILE}
	@mv xtl-${XTL_VERSION}/include/xtl ./inst/include
	@rm -rf xtl-${XTL_VERSION} ${XTL_TAR_FILE}

./inst/include/xeus:
	@echo "Fetching xeus headers..."
	@[ -d ./inst/include ] || mkdir -p ./inst/include
	@curl -skLO ${XEUS_URL}
	@tar -xzf ${XEUS_TAR_FILE}
	@mv xeus-${XEUS_VERSION}/include/xeus ./inst/include
	@rm -rf xeus-${XEUS_VERSION} ${XEUS_TAR_FILE}

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
	rm -rf *ipynb*
	rm -rf inst/libs
	rm -rf src-*

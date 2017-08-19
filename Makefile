SHELL := /bin/bash
.DELETE_ON_ERROR:

# for printing variable values
# usage: make print-VARIABLE
#        > VARIABLE = value_of_variable
print-%  : ; @echo $*=$($*)

PKG_VERSION=0.0.0.1

R_BUILD_ARGS= --no-manual
R_HOME=$(shell R RHOME)
R=$(R_HOME)/bin/R
RSCRIPT=$(R_HOME)/bin/Rscript

default: build/JuniperKernel_$(PKG_VERSION).tar.gz

build/JuniperKernel_$(PKG_VERSION).tar.gz: $(wildcard R/*R) DESCRIPTION NAMESPACE
	@echo "building " $@ " because " $?	
	@$(R) CMD build $(R_BUILD_ARGS) .
	@[ -d build ] || mkdir build
	@mv JuniperKernel_$(PKG_VERSION).tar.gz build/

.PHONY: clean
clean:
	rm -rf build

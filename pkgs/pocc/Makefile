.NOTPARALLEL:

PKG_LEVEL = ..
LEVEL = $(PKG_LEVEL)/..
include $(PKG_LEVEL)/Makefile.pkg.config

PkgBuildDir = $(HCL_PKG_BUILD)/pocc
PoCCBuildDir = $(PkgBuildDir)/pocc-devel
PoCCBin = $(PoCCBuildDir)/pocc-devel/bin

PoCCDistrib = https://svn.code.sf.net/p/pocc-devel/svn/pocc/trunk

### Rules
all:: build-pocc

#=== Build pocc ===#

build-pocc: $(PoCCBuildDir)/.built-pocc

$(PoCCBuildDir)/.built-pocc:
	@mkdir -p $(PkgBuildDir); \
	cd $(PkgBuildDir); \
	svn co $(PoCCDistrib) $(PoCCBuildDir); \
	cd $(PoCCBuildDir); \
	./install.sh devel
	@touch $@


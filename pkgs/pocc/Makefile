.NOTPARALLEL:

PKG_LEVEL = ..
LEVEL = $(PKG_LEVEL)/..
include $(PKG_LEVEL)/Makefile.pkg.config

PkgBuildDir = $(HCL_PKG_BUILD)/pocc
PoCCBuildDir = $(PkgBuildDir)/pocc-devel
PoCCBin = $(PoCCBuildDir)/pocc-devel/bin

PoCCDistrib = $(POCC_LINK)

### Rules
all:: build-pocc

#=== Build pocc ===#

build-pocc: $(PoCCBuildDir)/.built-pocc

$(PoCCBuildDir)/.built-pocc:
	@mkdir -p $(PkgBuildDir); \
	cd $(PkgBuildDir); \
	wget $(PoCCDistrib); \
	tar -xzf pocc.tar.gz;\ 
	mv pocc $(PoCCBuildDir)
	cd $(PoCCBuildDir); \
	./install.sh devel
	@touch $@


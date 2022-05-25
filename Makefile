UK_ROOT ?= $(PWD)/../../unikraft
UK_LIBS ?= $(PWD)/../../libs
LIBS := $(UK_LIBS)/libunwind:$(UK_LIBS)/compiler-rt:$(UK_LIBS)/libcxxabi:$(UK_LIBS)/libcxx:$(UK_LIBS)/newlib:$(UK_LIBS)/gcc

all:
	@$(MAKE) -C $(UK_ROOT) A=$(PWD) L=$(LIBS)

$(MAKECMDGOALS):
	@$(MAKE) -C $(UK_ROOT) A=$(PWD) L=$(LIBS) $(MAKECMDGOALS)

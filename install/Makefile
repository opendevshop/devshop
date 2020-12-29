
ENVSUBST_VARS=LOAD_SCRIPT_DEVSHOP_VERSION_SHA LOAD_SCRIPT_DEVSHOP_VERSION_REF

.PHONY: build
build: build/install.sh

build/install.sh: install.sh
	mkdir -p $(@D)
	LOAD_SCRIPT_DEVSHOP_VERSION_SHA='$(shell git rev-parse HEAD)' LOAD_SCRIPT_DEVSHOP_VERSION_REF='$(shell ../scripts/branch-or-tag)' envsubst '$(addprefix $$,$(ENVSUBST_VARS))' < $< > $@


.PHONY: clean
clean:
	$(RM) -r build/
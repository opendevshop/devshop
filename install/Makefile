
ENVSUBST_VARS=LOAD_SCRIPT_DEVSHOP_VERSION_SHA LOAD_SCRIPT_DEVSHOP_VERSION_REF

.PHONY: build
build: build/install-${LOAD_SCRIPT_DEVSHOP_VERSION_REF}.sh

build/install-${LOAD_SCRIPT_DEVSHOP_VERSION_REF}.sh: install.sh
	mkdir -p $(@D)
	LOAD_SCRIPT_DEVSHOP_VERSION_SHA='$(shell git rev-parse HEAD)' LOAD_SCRIPT_DEVSHOP_VERSION_REF='$(value BRANCH)' envsubst '$(addprefix $$,$(ENVSUBST_VARS))' < $< > $@


.PHONY: clean
clean:
	$(RM) -r build/
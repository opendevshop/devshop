
ENVSUBST_VARS=LOAD_SCRIPT_COMMIT_SHA

.PHONY: build
build: build/install.sh

build/install.sh: install.sh
	mkdir -p $(@D)
	LOAD_DEVSHOP_VERSION='$(shell git rev-parse HEAD)' envsubst '$(addprefix $$,$(ENVSUBST_VARS))' < $< > $@


.PHONY: clean
clean:
	$(RM) -r build/

ENVSUBST_VARS=LOAD_DEVSHOP_VERSION

.PHONY: build
build: index.html

index.html: install.sh
	mkdir -p $(@D)
	LOAD_DEVSHOP_VERSION='$(shell git rev-parse HEAD)' envsubst '$(addprefix $$,$(ENVSUBST_VARS))' < $< > $@


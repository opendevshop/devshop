
ENVSUBST_VARS=LOAD_DEVSHOP_SHA LOAD_DEVSHOP_REF LOAD_DEVSHOP_REMOTE

.PHONY: build
build: build/install.sh

build/install.sh: install.sh
	mkdir -p $(@D)
	LOAD_DEVSHOP_SHA='$(value GH_SHA)' LOAD_DEVSHOP_REF='$(value GH_REF)' LOAD_DEVSHOP_REMOTE='$(value GH_REMOTE)' envsubst '$(addprefix $$,$(ENVSUBST_VARS))' < $< > $@


.PHONY: clean
clean:
	$(RM) -r build/
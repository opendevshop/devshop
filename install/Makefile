
ENVSUBST_VARS=LOAD_DEVSHOP_SHA LOAD_DEVSHOP_REF LOAD_DEVSHOP_REMOTE

.PHONY: build
build: build/install.sh

build/install.sh: install.sh
	mkdir -p $(@D)
	LOAD_DEVSHOP_SHA='$(value COMMIT_REF)' LOAD_DEVSHOP_REF='$(value BRANCH)' LOAD_DEVSHOP_REMOTE='$(value REPOSITORY_URL)' envsubst '$(addprefix $$,$(ENVSUBST_VARS))' < $< > $@


.PHONY: clean
clean:
	$(RM) -r build/
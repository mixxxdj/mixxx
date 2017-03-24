SHELL := $(shell which zsh || which bash)

empty :=
space := $(empty) $(empty)

join-with = $(subst $(space),$1,$(strip $2))

device = $(call join-with,\ ,$(shell jq -r .controller.device packages/$(1)/package.json))
manufacturer = $(call join-with,\ ,$(shell jq -r .controller.manufacturer packages/$(1)/package.json))
mapping = $(buildDir)/$(call manufacturer,$(1))\ $(call device,$(1)).midi.xml
script = $(buildDir)/$(call manufacturer,$(1))-$(call device,$(1))-scripts.js

arch := $(shell uname)
package := ./package.json
buildDir := ./dist
version := $(shell jq -r .version package.json)

targets := $(shell jq -r '.controllers | join (" ")' package.json)

define targetScriptRules
$(call script,$(1)) : $(shell ls packages/**/*.js)
	BABEL_ENV=$(BABEL_ENV) ./scripts/compile-scripts.js $(1) "$$@"
endef

define targetMappingRules
$(call mapping,$(1)) : $(package) packages/$(1)/buttons.js packages/$(1)/template.xml.ejs
	./scripts/compile-mapping.js $(1) "$$@"
endef

define compileRule
compile : $(foreach target,$(1),$(call mapping,$(target)) $(call script,$(target)))
.DEFAULT : compile
.PHONY : compile
endef

define installRule
install : install_$(arch)

install_Darwin : $(foreach target,$(1),$(call mapping,$(target)) $(call script,$(target)))
	cp $(foreach target,$(1),$(call mapping,$(target)) $(call script,$(target))) $$(HOME)/Library/Application\ Support/Mixxx/controllers
.PHONY : install_Darwin

install_Linux : $(foreach target,$(1),$(call mapping,$(target)) $(call script,$(target)))
	cp $(foreach target,$(1),$(call mapping,$(target)) $(call script,$(target))) $$(HOME)/.mixxx/controllers
.PHONY : install_Linux

endef

define releaseRule
mixxx-launchpad-$(version) : $(foreach target,$(1),$(call mapping,$(target)) $(call script,$(target)))
	zip -9 $$@.zip $(foreach target,$(1),$(call mapping,$(target)) $(call script,$(target)))
endef

$(foreach target,$(targets),$(eval $(call targetScriptRules,$(target))))
$(foreach target,$(targets),$(eval $(call targetMappingRules,$(target))))
$(eval $(call compileRule,$(targets)))
$(eval $(call installRule,$(targets)))
$(eval $(call releaseRule,$(targets)))

release : mixxx-launchpad-$(version)
.PHONY : release

test :
	npm run lint
	npm run check
.PHONY : test

clean :
	rm -rf dist
.PHONY : clean

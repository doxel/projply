PREFIX?=/usr/local

SUBDIRS = bin

INSTALLDIRS = $(SUBDIRS:%=install-%)

.PHONY: build all install $(SUBDIRS) $(INSTALLDIRS)

all: build

build:
	docker build . -t projply

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

install: $(INSTALLDIRS)

$(INSTALLDIRS): 
	$(MAKE) -C $(@:install-%=%) install



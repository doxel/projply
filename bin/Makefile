PREFIX?=/usr/local

.PHONY: build all install

all: build

build:
	docker build . -t projply

install:
	install -D projply ${PREFIX}/bin

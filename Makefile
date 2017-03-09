VERSION ?= (git)

CFLAGS += -std=c99 -DVERSION='"$(VERSION)"'

all: bmi

clean:
	rm -f bmi bmi.o

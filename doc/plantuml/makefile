
SCRIPTS := $(notdir $(wildcard *.uml))

IMAGES := $(SCRIPTS:.uml=.png)

all: $(IMAGES)

%.png: %.uml
	plantuml -v $^

clean:
	rm -f *.png


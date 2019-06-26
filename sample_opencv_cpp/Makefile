OPENCV=`pkg-config --libs --cflags opencv`

SOURCE_FILES=$(wildcard *.cpp)
TARGETS=$(SOURCE_FILES:%.cpp=%.bin)
OBJECTS=$(SOURCE_FILES:%.cpp=%)

all: $(TARGETS)

%.bin: %.cpp
	g++ $(OPENCV) -o $@ $<

.PHONY: clean
clean:
	rm -f $(TARGETS)

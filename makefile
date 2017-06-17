CXX = gcc
CXXFLAGS = `pkg-config --cflags --libs gtk+-3.0` -Wall -g
SOURCES = main.c
#HEADERS = include.h
#$(SOURCES:.c=.h)
TARGET = imagepuzzle

$(TARGET): $(SOURCES)
	$(CXX) $(SOURCES) $(CXXFLAGS) -o $(TARGET)

.PHONY: clean

clean:
	@rm -f *.o $(TARGET) core

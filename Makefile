.PHONY: clean

CXX=g++
CXXFLAGS=-std=c++0x -g

TARGET=conan

SOURCES=src/main.cpp

OBJECTS=$(SOURCES:%.cpp=%.o)

%.o: %.pp
	$(CXX) -c $(CXXFLAGS) -o $(@) $(<)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(@) $(<) -ltwitcurl

clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)

TARGET = program
CXX = g++
CXXFLAGS = -std=c++20 -pthread
SRCS = PodsumowaniePodstawWatkow.cpp

all: $(TARGET)
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)
	
	clean:
	rm -f $(TARGET)
	echo "Cleaned up!"
run: $(TARGET)
	./$(TARGET)
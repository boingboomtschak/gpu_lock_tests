CXXFLAGS = -std=c++11

all: vk_lock_test

vk_lock_test: vk_lock_test.cpp easyvk.o lock_main.cinit
	$(CXX) $(CXXFLAGS) easyvk.o vk_lock_test.cpp -lvulkan -o vk_lock_test.run

easyvk.o: easyvk.cpp easyvk.h
	$(CXX) $(CXXFLAGS) -c easyvk.cpp

%.spv: %.cl
	clspv -cl-std=CL2.0 -inline-entry-points $< -o $@

%.cinit: %.cl
	clspv -cl-std=CL2.0 -inline-entry-points -output-format=c $< -o $@

clean:
	rm *.o
	rm *.run

all: vk_lock_test

vk_lock_test: vk_lock_test.cpp easyvk.o
	$(CXX) easyvk.o vk_lock_test.cpp -lvulkan -o vk_lock_test.run

easyvk.o: easyvk.cpp easyvk.h
	$(CXX) -c easyvk.cpp

clean:
	rm *.o
	rm *.run
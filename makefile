debug:
	mkdir -p build
	g++ -std=c++20 -fexceptions -g -DUNIT_TEST -I"." ./testing/*.cpp ./src/*.cpp -o ./build/main

release:
	mkdir -p build
	g++ -std=c++20 -I"." ./src/*.cpp -o ./build/release.o

clean:
	rm -rf ./build

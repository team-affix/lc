debug:
	mkdir -p build
	g++ -std=c++20 -fexceptions -g -DUNIT_TEST -I"." ./testing/*.cpp ./src/*.cpp -o ./build/main

release:
	mkdir -p build
	g++ -std=c++20 -I"." -c ./src/*.cpp -o ./build/release.o
	ar rcs ./build/liblc.a ./build/release.o

clean:
	rm -rf ./build

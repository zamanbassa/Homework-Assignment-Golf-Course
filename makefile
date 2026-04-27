files = shader.cpp

main: main.cpp
	g++ -std=c++11 -g shader.cpp main.cpp -lglfw -pthread -lGLEW -ldl -lGL -o main

clean:
	rm -f *.o main

run:
	./main

all:
	make clean
	make
	make run

rm *.o

g++ -g -c ../source/graph.cpp -o graph.o
g++ -g -c ../source/graphloader.cpp -o graphloader.o
g++ -g -c ../source/graphloader_driver.cpp -o graphloader_driver.o

g++ -g -o lg *.o


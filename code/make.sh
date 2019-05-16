#!/bin/bash
clear
g++ -std=c++11 init.cpp -o init.o
g++ -std=c++11 publisher.cpp -o publisher.o
g++ -std=c++11 mediator.cpp -o mediator.o
g++ -std=c++11 subscriber.cpp -o subscriber.o

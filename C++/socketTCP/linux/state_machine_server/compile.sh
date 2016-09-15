#!/bin/bash

g++ -o server main.cpp server.cpp save_pocket.cpp -lm -std=c++11 -pthread

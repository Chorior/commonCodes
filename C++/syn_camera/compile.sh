#!/bin/bash

g++ syn_camera.cpp -o syn_camera -pthread -std=c++11 `pkg-config --cflags --libs opencv`

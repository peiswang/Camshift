vpath %.h . 
vpath %.cpp . 

all:testCamshift 

testCamshift:testCamshift.cpp camshift.cpp camshift.h utils.h matrix.h
	g++ testCamshift.cpp camshift.cpp -o testCamshift `pkg-config --cflags --libs opencv`

.PHONY:clean
clean:
	-rm testCamshift

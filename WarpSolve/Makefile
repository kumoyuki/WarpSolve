CXX = gcc -std=gnu++11

LIB_SRCS := \
	sat3.cpp \
	satsolver.cpp \
	solution.cpp \
	warpsolve.cpp

LIB_OBJS := $(LIB_SRCS:.cpp=.o)


libsat.a : $(LIB_OBJS)
	ar -rcsv libsat.a $(LIB_OBJS)

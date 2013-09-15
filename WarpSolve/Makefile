CXX = gcc -std=gnu++11 -g

LIB_SRCS := \
	sat3.cpp \
	satsolver.cpp \
	solution.cpp \
	warpsolve.cpp

LIB_OBJS := $(LIB_SRCS:.cpp=.o)

MAIN_SRCS := \
	explicit.cpp \
	main.cpp \
	SExp.cpp

MAIN_OBJS := $(MAIN_SRCS:.cpp=.o)


libsat.a : $(LIB_OBJS)
	ar -rcsv $@ $(LIB_OBJS)


WarpSolve: $(MAIN_OBJS) libsat.a
	$(CXX) -o WarpSolve $(MAIN_SRCS) libsat.a
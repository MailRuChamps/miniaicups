CPPFLAGS=-std=c++17 -O3 -m64 -pipe -w
CPP=clang++-5.0

SRCS = $(shell find ${SOLUTION_CODE_PATH} -type f -name '*.cpp')

all: ${SRCS}
	${CPP} ${CPPFLAGS} -o ${COMPILED_FILE_PATH} ${SRCS}
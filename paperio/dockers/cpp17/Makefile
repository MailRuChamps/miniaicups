CXXFLAGS=-std=c++17 -O3 -m64 -pipe -w -pthread
CXX=g++-9

SRCS = $(shell find ${SOLUTION_CODE_PATH} -type f -name '*.cpp')

all: ${SRCS}
	${CXX} ${CXXFLAGS} -o ${COMPILED_FILE_PATH} ${SRCS}

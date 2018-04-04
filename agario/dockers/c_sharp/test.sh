#!bin/bash
CODE_DIR=$(echo "$(cd "../../examples/csharp"; pwd)")
BUILD_INTO_DIR=$(echo "$(cd "../../examples"; pwd)")

#Build container
docker build -t cups/csharp:latest ./

#Run compiler
timeout --foreground 5s docker run \
    -e MOUNT_POINT=/opt/client/csharpStrategy  \
    -e SOLUTION_CODE_PATH='/opt/client/'  \
    -v $CODE_DIR:/opt/client/code  \
    -v $BUILD_INTO_DIR:/opt/client/  \
    cups/csharp  \
    bash -c "eval \$COMPILATION_COMMAND"

#Run aoo
timeout --foreground 5s docker run -i  \
    -e MOUNT_POINT=/opt/client/csharpStrategy  \
    -e SOLUTION_CODE_PATH='/opt/client/'  \
    -v $CODE_DIR:/opt/client/code  \
    -v $BUILD_INTO_DIR:/opt/client/  \
    cups/csharp  \
    bash -c "eval \$RUN_COMMAND"  \
    < ./test.in.txt

RETVAL=$?

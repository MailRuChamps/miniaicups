#!bin/bash
CODE_DIR=$(echo "$(cd "../../examples/csharp"; pwd)")
BUILD_DIR=$(echo "$(cd "../../examples/"; pwd)")

docker build -t cups/csharp:latest ./
timeout 5s docker run -it -e MOUNT_POINT=/opt/client/csharpStrategy  \
    -e SOLUTION_CODE_PATH='/opt/client/'  \
    -v $CODE_DIR:/opt/client/code  \
    -v $BUILD_DIR:/opt/client/  \
    cups/csharp  \
    bash -c "eval \$RUN_COMMAND"  \
    
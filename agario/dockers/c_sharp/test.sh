#!bin/bash
docker build -t cups/csharp:latest ./
timeout 5s docker run -it -e MOUNT_POINT=/opt/client/csharpStrategy  \
    -e SOLUTION_CODE_PATH='/opt/client/'  \
    -v ../../examples/csharp:/opt/client/code  \
    -v ../../examples/build:/opt/client/  \
    cups/csharp  \
    bash -c "eval \$RUN_COMMAND"  \
    
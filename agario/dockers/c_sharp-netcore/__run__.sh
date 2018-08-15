#!/bin/bash

if [ -d /opt/client/run ]; then
    cd /opt/client/run && dotnet solution.dll
else
    mkdir -p /opt/client/run && \
    cd /opt/client/run && \
    unzip -q "$MOUNT_POINT" && \
    dotnet solution.dll
fi
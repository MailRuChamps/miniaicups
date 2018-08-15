#!/bin/bash

if [ -z "$(find . -maxdepth 1 -name '*.csproj' -printf 1 -quit)" ]; then 
    cp ../default/solution.csproj .
fi && \
HOME=/tmp DOTNET_SKIP_FIRST_TIME_EXPERIENCE=1 dotnet build -c Release --no-incremental -v:q -nologo -clp:"NoSummary;ErrorsOnly" -o build 2>&1 && \
cd build && \
zip -rq "$COMPILED_FILE_PATH" .

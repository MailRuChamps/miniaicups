#!/usr/bin/env bash
COP_PATH="$(pwd)/compile_output_processor.py"

if [ "$ZIPPED" = True ]; then
    yes | unzip -n $MOUNT_POINT -d $SOLUTION_CODE_PATH
else
    yes | cp $MOUNT_POINT $SOLUTION_CODE_PATH/$SOLUTION_CODE_ENTRYPOINT
fi

if [ "$COMPILE" = True ]; then
    STATUS="ok"
    MESSAGE="compilation done"
    FILE_PATH=$COMPILED_FILE_PATH

    ERRORS="$(eval $COMPILATION_COMMAND)"

    if [ $? -ne 0 ]; then
      STATUS="error"
      FILE_PATH=""
      MESSAGE="$ERRORS"
    fi
    echo "{\"status\": \"$STATUS\",\"message\": `echo "$MESSAGE" | python $COP_PATH`,\"path_to_compiled_file\": \"$FILE_PATH\"}" > $COMPILE_LOG_LOCATION

else
   ./client "$(eval echo $RUN_COMMAND)" $SOLUTION_CODE_PATH
fi

FROM stest.tech-mail.ru/aicups/paperio_base

RUN apt-get update -y && apt-get install -y build-essential curl &&  curl https://sh.rustup.rs -sSf | sh -s -- -y --default-toolchain stable
ENV SOLUTION_CODE_PATH=/opt/client/solution \
    SOLUTION_CODE_ENTRYPOINT=src/main.rs \
    COMPILED_FILE_PATH=/opt/client/solution/target/release/solution \
    COMPILATION_COMMAND="RUSTFLAGS=-Awarnings ~/.cargo/bin/cargo build --release --bin solution --manifest-path $SOLUTION_CODE_PATH/Cargo.toml --quiet 2>&1" \
    RUN_COMMAND="/lib64/ld-linux-x86-64.so.2 $MOUNT_POINT"

COPY Cargo.toml ./
RUN USER=user ~/.cargo/bin/cargo init --bin $SOLUTION_CODE_PATH &&\
        mv Cargo.toml $SOLUTION_CODE_PATH/Cargo.toml &&\
        ~/.cargo/bin/cargo build --release --manifest-path $SOLUTION_CODE_PATH/Cargo.toml && \
        rm -rf $SOLUTION_CODE_PATH/src/*

FROM ubuntu:25.10

# Update the package database
RUN apt clean
RUN apt update

# Install common tools
RUN apt install -y python3-minimal python3-pip pipx git less vim sudo cmake make g++-15 clang-format clangd-20

# Install additional tools and libraries
RUN apt install -y nodejs npm cargo

# Cleanup
RUN apt autoremove -y \
    && apt autoclean \
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /var/cache/apt/* \
    && rm -rf /tmp/*

# Update  gcc/g++ version to the latest ones
RUN ln -fs /usr/bin/g++-15 /usr/bin/g++
RUN ln -fs /usr/bin/gcc-15 /usr/bin/gcc

ARG USERNAME=dev
ARG USER_UID=1000
ARG USER_GID=1000

RUN printf "\n\n\n Received args:\n\tUSERNAME=$USERNAME\n\tUSER_UID=$USER_UID\n\tUSER_GID=$USER_GID\n"
RUN test -n "$USER_UID" && test -n "$USER_GID" || (printf "\n\n\n\tERROR: USER_UID (equal to '$USER_UID') or USER_GID (equal to '$USER_GID') is not set!\n\n\n" && exit 1)

# Delete Ubuntu user to avoid "UID/GID is already exists" problems
RUN set -e; \
    if id -u ubuntu >/dev/null 2>&1; then \
        echo "Delete 'ubuntu' user from the base image"; \
        userdel -r ubuntu 2>/dev/null || true; \
    fi; \
    if getent group ubuntu >/dev/null 2>&1; then \
        echo "Delete 'ubuntu' group from the base image"; \
        groupdel ubuntu 2>/dev/null || true; \
    fi;

# Create a non-root user 'dev' with host UID/GID
RUN set -e; \
    echo "Create group $USERNAME with GID=$USER_GID"; \
    groupadd --gid $USER_GID $USERNAME; \
    \
    echo "Create user $USERNAME with UID=$USER_UID and GID=$USER_GID"; \
    useradd -ms /bin/bash --uid $USER_UID --gid $USER_GID $USERNAME; \
    \
    # Configure NOPASSWD option
    echo "$USERNAME ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/$USERNAME; \
    chmod 0440 /etc/sudoers.d/$USERNAME; \
    \
    echo "User $USERNAME with UID=$USER_UID and GID=$USER_GID was successfully created"

# Установка rustup и последней стабильной версии Rust
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"

# RUN rustup update

RUN git clone https://github.com/tree-sitter/tree-sitter
WORKDIR tree-sitter
RUN ls -la
RUN make -j4
RUN sudo make install
RUN cargo install tree-sitter-cli --version 0.24.7 --root /usr/local --locked
ENV PATH="/usr/local/bin:/root/.cargo/bin:${PATH}"
WORKDIR ..
RUN git clone https://github.com/tree-sitter/tree-sitter-python
WORKDIR tree-sitter-python/
RUN ls -la
RUN tree-sitter init-config
RUN tree-sitter generate
RUN gcc -shared -o libtree-sitter-python.so -Isrc src/parser.c -fPIC
RUN sudo mkdir -p ~/.tree-sitter/bin
RUN sudo ln -s $(pwd)/libtree-sitter-python.so ~/.tree-sitter/bin/

WORKDIR ..
RUN echo '{"parser-directories":["/"]}' > /root/.config/tree-sitter/config.json

RUN chmod 777 /root/.config/tree-sitter/config.json
RUN chmod 755 /root
RUN chmod 755 /root/.config
RUN chmod 755 /root/.config/tree-sitter
RUN chmod 644 /root/.config/tree-sitter/config.json

# Switch to non-root user
USER $USERNAME

RUN ls /root/.config/tree-sitter/config.json 

RUN git config --global core.editor code

# Install conan through pipx
RUN pipx install conan
RUN pipx ensurepath

# Set the default command
CMD ["/bin/bash"]
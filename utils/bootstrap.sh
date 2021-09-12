#!/usr/bin/env bash
set -ex

LLVM_VERSION=13
if [ "$#" -eq 1 ]; then
    LLVM_VERSION=$1
fi

bootstrap_ubuntu_dependencies() {
    wget https://apt.kitware.com/kitware-archive.sh
    chmod +x kitware-archive.sh
    sudo ./kitware-archive.sh

    sudo apt -y install \
        bison \
        cmake \
        default-jdk \
        git \
        libutfcpp-dev \
        pkg-config \
        python3 \
        python3-pip \
        python3-setuptools \
        uuid-dev \
        zlib1g-dev

    wget https://apt.llvm.org/llvm.sh
    sed -i -E 's,Ubuntu_(.*),&\n    Pop_\1,g' llvm.sh
    chmod +x llvm.sh
    sudo ./llvm.sh $LLVM_VERSION

    sudo apt -y install \
        libllvm$LLVM_VERSION \
        llvm-$LLVM_VERSION \
        llvm-$LLVM_VERSION-dev \
        llvm-$LLVM_VERSION-doc \
        llvm-$LLVM_VERSION-examples \
        llvm-$LLVM_VERSION-runtime \
        llvm-$LLVM_VERSION-tools

    sudo apt -y install \
        clang-tools-$LLVM_VERSION \
        clang-$LLVM_VERSION-doc \
        libclang-common-$LLVM_VERSION-dev \
        libclang-$LLVM_VERSION-dev \
        libclang1-$LLVM_VERSION \
        clang-format-$LLVM_VERSION

    sudo apt -y install \
        libc++-$LLVM_VERSION-dev \
        libc++abi-$LLVM_VERSION-dev

    if [ $LLVM_VERSION -ge 13 ]; then
        sudo apt -y install libmlir-$LLVM_VERSION-dev mlir-$LLVM_VERSION-tools
    fi

    pip3 install --user lit
}


bootstrap_ubuntu_env() {
    echo "export PATH=/usr/lib/llvm-$LLVM_VERSION/bin:\$PATH" >> ~/.bashrc
}


bootstrap_ubuntu() {
    bootstrap_ubuntu_dependencies
    bootstrap_ubuntu_env
}


bootstrap_linux() {
    . /etc/os-release
    if [ $ID == ubuntu -o $ID == pop ]; then
        bootstrap_ubuntu
    else
        echoerr $ID is not supported.
        exit 1
    fi
}

bootstrap() {
    bootstrap_linux

    echo '------------------------------------------------------'
    echo '***************** bootstrap complete *****************'
    echo '------------------------------------------------------'
}

bootstrap

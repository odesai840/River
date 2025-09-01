#!/bin/bash

pushd ..
git submodule update --init --recursive

HEADER_FILE="Game/Source/MainBehavior.h"
CPP_FILE="Game/Source/MainBehavior.cpp"

if [ ! -f "$HEADER_FILE" ]; then
    echo "Generating MainBehavior.h stub..."
    cat > "$HEADER_FILE" << 'EOF'
#ifndef MAIN_BEHAVIOR_H
#define MAIN_BEHAVIOR_H

#include "GameInterface.h"

class MainBehavior : public RiverCore::GameInterface {
public:
    void OnStart() override;
    void OnUpdate(float deltaTime) override;
};

#endif
EOF
fi

if [ ! -f "$CPP_FILE" ]; then
    echo "Generating MainBehavior.cpp stub..."
    cat > "$CPP_FILE" << 'EOF'
#include "MainBehavior.h"

void MainBehavior::OnStart() {
    // Add your game initialization code here
}

void MainBehavior::OnUpdate(float deltaTime) {
    // Add your game update logic here
}
EOF
fi

cmake -S . -B Build
cmake --build Build
popd
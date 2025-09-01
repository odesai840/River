@echo off

pushd ..
git submodule update --init --recursive

set HEADER_FILE=Game\Source\MainBehavior.h
set CPP_FILE=Game\Source\MainBehavior.cpp

if not exist "%HEADER_FILE%" (
    echo Generating MainBehavior.h stub...
    (
        echo #ifndef MAIN_BEHAVIOR_H
        echo #define MAIN_BEHAVIOR_H
        echo.
        echo #include "GameInterface.h"
        echo.
        echo class MainBehavior : public RiverCore::GameInterface {
        echo public:
        echo     void OnStart^(^) override;
        echo     void OnUpdate^(float deltaTime^) override;
        echo };
        echo.
        echo #endif
    ) > "%HEADER_FILE%"
)

if not exist "%CPP_FILE%" (
    echo Generating MainBehavior.cpp stub...
    (
        echo #include "MainBehavior.h"
        echo.
        echo void MainBehavior::OnStart^(^) {
        echo     // Add your game initialization code here
        echo }
        echo.
        echo void MainBehavior::OnUpdate^(float deltaTime^) {
        echo     // Add your game update logic here
        echo }
    ) > "%CPP_FILE%"
)

cmake -S . -B Build
cmake --build Build
popd
pause
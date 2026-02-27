set BUILD_DIR=build

if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

cmake -S .. -B . -DCMAKE_PREFIX_PATH="C:\Qt\6.10.1\msvc2022_64\lib\cmake"

cmake --build . --target XTimerAPI --config Release
cmake --build . --target XTimer --config Debug

pause
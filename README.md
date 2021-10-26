# Timer
A powerful C++ timer, compiler is required to support C++14 or later standard.

## Concepts
### Not invade the target program API
I want to design a timer that does not "invade" the target program API, which means that I should not pass any new variables.

The straightforward design is that I use a global static timer to save and update all the information.
### Performance loss
I use the timer test the program performance, but at the release stage, maybe there is no need to do any timing.
I hope I have some options that may turn off the timer, if directly optimized out by compiler would be the best.

Based on such demand, I allow user use the macro `TIMER_USE_TIMER` to decide whether the timing codes would be compiled.
Notably, I should use `-O1` compiler instruction to totally optimize out the timing codes.
### API hiding
I want only expose APIs that are really used in practice and hide all the structure that may be experimental.
So I hide all the structure, e.g., `RelationTree`, and internal function, e.g., `ExistChecker`, into `Timer.cpp`.

Only abstract API exposed to users will be benefit for the steady, I hope users **never** try to use functions in `Timer.cpp`.
## How-To
### Compile
There are two approach to use the timer:
- Compile `Timer.cpp` together as a part of your project.
- Compile `Timer.cpp` as a shared library, link your code with the compiled library.
#### Compile as a part
Just modify the `CMakeLists.txt` every `add_library` or `add_executable`, add `Timer.cpp` in to the source list.
##### Example
Origin:
```cmake
# some codes...
add_definitions(-DTIMER_USE_TIMER=true)
add_library(XXX_lib 1.cpp 2.cpp ...)
add_executable(XXX_obj 1.cpp 2.cpp ...)
# some codes...
```
Modify to:
```cmake
# some codes...
add_definitions(-DTIMER_USE_TIMER=true)
add_library(XXX_lib Timer.cpp 1.cpp 2.cpp ...)
add_executable(XXX_obj Timer.cpp 1.cpp 2.cpp ...)
# some codes...
```
#### Compile as a library
Add instruction `add_library(Timer SHARED Timer.cpp)` to your `CMakeLists.txt` and link the object to `Timer`.
##### Example
Origin:
```cmake
# some codes...
add_library(XXX_lib 1.cpp 2.cpp ...)
add_executable(XXX_obj 1.cpp 2.cpp ...)
# some codes...
```
Modify to
```cmake
# some codes...
add_library(XXX_lib 1.cpp 2.cpp ...)
add_executable(XXX_obj 1.cpp 2.cpp ...)

add_definitions(-DTIMER_USE_TIMER=true)
add_library(Timer SHARED Timer.cpp)
target_link_libraries(XXX_lib Timer)
target_link_libraries(XXX_obj Timer)
# some codes...
```
### Use
Include `Timer.h` to your file (`#include "Timer.h"`).
Assume there are two files `main.cpp` and `utils.cpp`, `utils.cpp` is compiled as a shared library `utils`.
`main.cpp` use a function named `run` in `utils` and we compiled `main.cpp` as an executable `main`.
`CMakelist.txt` could be:
```cmake
cmake_minimum_required(VERSION 3.20)
project(main)

set(CMAKE_CXX_STANDARD 14)

add_library(utils SHARED utils.cpp)

add_executable(main main.cpp)
target_link_libraries(main utils)
```
`main.cpp` could be:
```c++
void run();

int main() {
    run();
}
```
`utils.cpp` could be:
```c++
#include <thread>
#include <chrono>

void sleep100ms() {
    std::this_thread::sleep_for(std::chrono::seconds{1});
}

void sleep200ms() {
    std::this_thread::sleep_for(std::chrono::seconds{1});
}

void run() {
    sleep100ms();
    sleep200ms();
}
```
Since we want to timing functions, we modify `main.cpp` and `utils.cpp` as following.

`main.cpp`:
```c++
#include "Timer.h"

void run();

int main() {
    Timer::StartRecording("main");
    run();
    Timer::StopRecording("main");
    Timer::ReportAll();
}
```
`utils.cpp`
```c++
#include <thread>
#include <chrono>
#include "Timer.h"

void sleep100ms() {
    Timer::StartRecording("run-sleep100ms", "run", Timer::us);
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    Timer::StopRecording("run-sleep100ms");
}

void sleep200ms() {
    Timer::StartRecording("run-sleep200ms", "run", Timer::us);
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    Timer::StopRecording("run-sleep200ms");
}

void run() {
    Timer::StartRecording("run", "main");
    sleep100ms();
    sleep200ms();
    Timer::StopRecording("run");
}
```
modify the `CMakelist.txt` as following:
```cmake
cmake_minimum_required(VERSION 3.20)
project(main)

set(CMAKE_CXX_STANDARD 14)

add_library(utils SHARED utils.cpp)

add_executable(main main.cpp)
target_link_libraries(main utils)

add_definitions(-DTIMER_USE_TIMER=true)
add_library(Timer SHARED Timer.cpp)
target_link_libraries(utils Timer)
target_link_libraries(main Timer)
```
The result would be:
```
Report {all} in the recorder (-1 means recorder not stopped):
|-main: 1 call(s), average 600ms
     |--run: 2 call(s), average 300ms, ratio 1
          |run-sleep200ms: 2 call(s), average 200251us, ratio 0.6675
          |run-sleep100ms: 2 call(s), average 100137us, ratio 0.3338
```
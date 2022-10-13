# Architecture of Evaluating Systems LAB1

The assignment text: [`task.pdf`](./task.pdf)

The solution source code: [`LAB1.cpp`](./LAB1.cpp)

The detailed report: [`report.pdf`](./report.pdf)

## Compiling and running
For these the CMake installed would be required.

### To run on local:
```shell
> cmake -S . -B ./build
> cmake --build ./build

> ./build/main(.exe)
```

### To run on virtual:
```shell
> docker build . -t LAB1_AES
> docker run -t LAB1_AES
```
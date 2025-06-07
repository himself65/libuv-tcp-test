# libuv TCP Server

A simple TCP echo server implemented using libuv.

## Prerequisites

- CMake (version 3.10 or higher)
- libuv development libraries
- C compiler (GCC or Clang)

### Installing Dependencies

On macOS:
```bash
brew install cmake libuv
```

On Ubuntu/Debian:
```bash
sudo apt-get install cmake libuv1-dev
```

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Running

```bash
./server
```

The server will start listening on port 7000. You can test it using netcat:

```bash
nc localhost 7000
```

Type any message and press Enter. The server will echo back your message. 
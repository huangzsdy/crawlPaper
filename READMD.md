# Scientific Paper Crawler

A high-performance academic preprint crawler written in Modern C++ that supports multi-threading, coroutines, and multi-processing concurrency models.

## Features

- **Multiple Sources**: Supports arXiv, bioRxiv, and ChemRxiv
- **Flexible Configuration**: TOML-based configuration system
- **Three Concurrency Models**: Threads, Coroutines (C++20), and Processes
- **Modern C++**: Uses C++20 features including coroutines, concepts, and ranges
- **Extensible Architecture**: Plugin-based parser system
- **Data Export**: JSON, CSV, and XML output formats

## Build Requirements

- C++20 compatible compiler (GCC 11+, Clang 12+, MSVC 2019+)
- CMake 3.15+
- libcurl
- POSIX threads

## Build Instructions
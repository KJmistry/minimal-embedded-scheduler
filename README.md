# Minimal Embedded Scheduler

A lightweight, modular C library for scheduling and executing periodic tasks in embedded or real-time systems.
Designed for simplicity, portability, and ease of integration.

## Features

- Register, deregister, and execute periodic tasks with configurable intervals (10ms, 20ms, 100ms, 1s, 1min).
- Efficient tick-based scheduling suitable for resource-constrained environments.
- Utility functions for time management and logging.
- Example application demonstrating usage.
- Dynamic sleep calculation for efficient main loop timing.
- **TaskTimer utility** for single-shot and periodic timers with callback support.

## Directory Structure

```
src/         # Core scheduler and common utilities
src/common/  # Shared type definitions, macros, and time utilities
example/     # Sample application using the scheduler and TaskTimer
build/       # Build artifacts (created by CMake/Make)
bin/         # Compiled example binaries
install/     # Installed library and binaries (after 'make install')
```

## Getting Started

### Prerequisites

- GCC or compatible C compiler
- CMake (version 3.10 or higher recommended)
- Make

### Build Instructions

```sh
mkdir -p build
cd build
cmake ..
make
make install
```

- The static library `libscheduler.a` will be in `install/lib/`.
- The example binary `sample_example.bin` will be in `bin/`.

### Usage

#### Scheduler

- **Initialize the Scheduler:**
  ```c
  cU32_t maxPermittedDelayMs = 1000;
  Scheduler_Init(TIMER_TICK_RESOLUTION_IN_MS, &maxPermittedDelayMs);
  ```
- **Register a Task:**
  ```c
  Scheduler_RegisterTask(my_callback, TIME_INTERVAL_100MS);
  ```
- **Main Loop Example:**
  ```c
  while (running) {
      Scheduler_UpdateTick();
      Scheduler_ExecuteTasksReadyToRun();
      Utils_SleepNanoSec(Scheduler_GetDynamicSleep());
  }
  ```

#### TaskTimer

- **Initialize TaskTimer:**
  ```c
  TaskTimer_Init();
  ```
- **Start a single-shot timer:**
  ```c
  TaskTimer_t myTimer;
  TaskTimer_Start(&myTimer, 5000, my_callback, my_user_data); // 5000ms
  ```
- **Stop a timer:**
  ```c
  TaskTimer_Stop(&myTimer);
  ```
- **Register TaskTimer_Tick with the scheduler for periodic timer updates:**
  ```c
  Scheduler_RegisterTask(TaskTimer_Tick, TIME_INTERVAL_100MS);
  ```

- See `example/sample_main.c` for a complete usage demonstration.

### Logging

- Use macros (`DPRINT`, `IPRINT`, `WPRINT`, `EPRINT`) for colored, timestamped logs.

### Custom Types

- All code uses types from `src/common/common_stddef.h` (e.g., `cU8_t`, `cBool`, `cStatus_e`).

## Contributing

Contributions are welcome! Please submit issues and pull requests for bug fixes, enhancements, or new features.

## Authors

- Kshitij Mistry

---

For questions or support, open an issue on GitHub.

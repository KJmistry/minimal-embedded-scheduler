# Minimal Embedded Scheduler

A lightweight, modular C library for scheduling and executing periodic tasks in embedded or real-time systems.
Designed for simplicity, portability, and ease of integration.

## Features

- Register, deregister, and execute periodic tasks with configurable intervals (10ms, 20ms, 100ms, 1s, 1min).
- Efficient tick-based scheduling suitable for resource-constrained environments.
- Custom type definitions for portability.
- Utility functions for time management and logging.
- Example application demonstrating usage.
- Dynamic sleep calculation for efficient main loop timing.

## Directory Structure

```
src/         # Core scheduler and common utilities
src/common/  # Shared type definitions, macros, and time utilities
example/     # Sample application using the scheduler
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

1. **Clone the repository:**
   ```sh
   git clone <repo-url>
   cd minimal-embedded-scheduler
   ```

2. **Build with CMake:**
   ```sh
   mkdir -p build
   cd build
   cmake ..
   make
   make install
   ```

   - The static library `libscheduler.a` will be in `install/lib/`.
   - The example binary `sample_example.bin` will be in `bin/`.

3. **Run the Example:**
   ```sh
   ./bin/sample_example.bin
   ```

### Usage

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
- See `example/sample_main.c` for a complete usage demonstration.

### API Reference

- `Scheduler_Init(timerResolutionMs, pMaxPermittedDelayMs)`
  Initialize the scheduler with a tick resolution in milliseconds and set the maximum permitted delay.

- `Scheduler_RegisterTask(callback, interval)`
  Register a periodic task.

- `Scheduler_DeregisterTask(callback)`
  Remove a task from the scheduler.

- `Scheduler_UpdateTick()`
  Update tick counters and mark tasks ready to run.

- `Scheduler_GetDynamicSleep()`
  Returns the recommended sleep time in nanoseconds for the main loop.

- `Scheduler_ExecuteTasksReadyToRun()`
  Execute all tasks marked as ready.

- Utility functions for time management are in `src/common/common_utils.h`.

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

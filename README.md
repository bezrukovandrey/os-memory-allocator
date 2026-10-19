# OS Memory Allocator

This project implements a custom memory allocator in C as part of an operating systems lab. It explores low-level memory management concepts such as allocation strategies, virtual memory mapping, and allocator testing.

## Overview

The repository contains two versions of the work:

- `lab2/` — the base implementation
- `lab2_advanced/` — the advanced version
- `os_lab2_report.pdf` — project report

The allocator is built around a custom implementation of memory allocation logic, with support for different allocation policies and a test-driven validation workflow.

## Features

- Custom memory allocation logic in C
- Support for multiple placement policies:
  - First Fit
  - Best Fit
  - Worst Fit
  - Next Fit
- Integration with `mmap` via wrapper code
- Interactive allocator shell for testing
- Simulation and trace-based test comparisons
- Makefile-based build and validation workflow

## Project Structure

- `lab2/src/` — main source code and build files
- `lab2/src/tests/` — input scenarios used for validation
- `lab2/src/Makefile` — build/test rules
- `lab2/src/Makefile.config` — allocator configuration
- `lab2_advanced/src/` — advanced variant of the allocator
- `README_*.md` — supplemental documentation

## Build

From the project source directory:

```bash
cd lab2/src
make -B mem_shell

# Producer-Consumer System Analysis

**Authors:**
Andrae Muys (original code),
Ioannis Michalainas

This assignment investigates the performance of a multi-threaded producer-consumer system where *p* producer threads generate computational tasks and *q* consumer threads process them. The average task waiting time is measured to evaluate the impact of consumer count on performance and to identify optimal thread configurations.

## Tools Used
- **Compilation**: `gcc`
- **Build System**: GNU `make`
- **Profiling Script**: `python` with:
    - `matplotlib` (plotting)
    - `numpy` (statistics)
- **System Monitoring**: `htop`, `mpstat` (Linux)
- **Debugging**: `valgrind` (memory leak checks)
- **Synchronization**: POSIX threads (`pthread.h`)

## How to Run

**Run the script**:

   ```bash
   python test/profile.py
   ```
   This will:
   - Compile the program
   - Generate results in test/results/
       - PNG plots for all producer configurations
       - Raw metrics in results.txt

## Appendix
For full experimental details and analysis, see the complete report: `docs/report.pdf`
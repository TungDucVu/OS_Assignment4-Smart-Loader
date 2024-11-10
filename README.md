# OS_Assignment4

Extension of the simple loader of assigment 1


- **Memory Allocation**:
  - Do not use `mmap` memory upfront for any segment, including the PT_LOAD segment with the entrypoint.
  
- **Execution & Page Faults**:
  - Run `_start` by typecasting the entrypoint address directly. This will cause a segmentation fault due to unallocated memory.
  - Treat segmentation faults as page faults, as they happen due to unallocated (but valid) memory addresses.
  
- **Fault Handling**:
  - Use `mmap` to allocate memory and load segments on-demand (lazily) during execution without terminating the program.
  - Ensure total memory allocated by `mmap` is in 4KB page-sized multiples.

- **Dynamic Segment Loading**:
  - Allocate memory page-by-page for each segment rather than all at once.
  - Ensure virtual memory within each segment is contiguous; physical memory may not be.
  - Multiple page faults will occur for segments larger than 4KB (e.g., allocate 4KB initially, then allocate additional pages as needed).

- **Implementation**:
  - Implement as either a static or shared library.
  - Use the Makefile from assignment-1 to compile the loader and test cases.
  
- **Test Cases**:
  - Use `fib.c` (from assignment-1) and `sum.c` as test cases.

- **Reporting**:
  - After execution, report:
    1. Total page faults
    2. Total page allocations
    3. Total internal fragmentation in KB

Contribution: Group 64
### Tung Duc Vu - 2023558: **Fault Handling & Memory Allocation**
- Implement fault handling logic to:
  - Detect segmentation faults and treat them as page faults for unallocated memory.
  - Use `mmap` to allocate pages dynamically as faults occur.
- Ensure page allocations are done in 4KB page-sized chunks, handling intra-segment contiguous virtual memory.
- Track total page faults, page allocations, and internal fragmentation, storing these metrics for the final report.

### Turbold Amarbat - 2023559: **Segment Loading, Execution & Reporting**
- Implement lazy segment loading:
  - Load the contents of each segment only when the page fault occurs.
  - Ensure that segments load page-by-page for larger sections.
- Integrate the `_start` execution logic, typecasting the entrypoint and managing program flow continuation after faults are handled.
- Implement and format the final report, showing total page faults, page allocations, and internal fragmentation.

- **Testing**:  `fib.c` and `sum.c`.
- **Documentation**: 
- **Debugging and Optimization**: 

To run the Simple Smart Loader:
- type in command: `make`
- run the program: `./loader fib` or `./loader sum`
- report will be printed out after execution.

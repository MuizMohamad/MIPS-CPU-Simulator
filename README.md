# MIPS-CPU-Simulator

A simulator for MIPS CPU architecture consists of two part,
a processor simulator without cache, and also a processor simulator with cache.

# Compilation

gcc -o mipssim mipssim.c memory_hierarchy.c -std=gnu99 -lm

# Invoking the simulator

The following are examples of invoking the simulator with valid command-line parameters.  
  
`./mipssim 0 memfile-simple.txt regfile.txt` 
  
Where mipssim is the name of the executablele and 0 indicates cache is disabled.
Additionally, memfile-simple.txt is the name of the memory file and regfile.txt is
the name of the register state file. Both memory and register state files are located in
the same directory as mipssim.
Note: in this assignment cache is always disabled, and you must pass 0 as the first
argument after the executable file.


The following are examples of invoking the simulator with valid command-line parameters.  
  
`./mipssim 1024 1 memfile-simple.txt regfile.txt`  
  
Where mipssim is the name of the executable file. 1024 indicates cache is enabled and
set to a size of 1024 bytes. 1 configures the cache type as direct mapped (2 is fully
associative and 3 is 2-way set associative). Additionally, memfile-simple.txt is the
name of the memory file and regfile.txt is the name of the register state file. Both
memory and register state files are located in the same directory as mipssim.



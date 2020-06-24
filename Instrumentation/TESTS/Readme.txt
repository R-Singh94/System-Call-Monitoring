PIN version 3.13
This Folder holds all the parts of the Warm-Up Section
bblcount.cpp -> Counts number of Basic Blocks
malloctrace.cpp -> Provides the number of calls to malloc and total memory allocated
callcount.cpp -> Counts the number of direct and indirect control flow transfers

BUILD:
    make

    -- The binaries will be generated and stored in bin/ . This directory will be in the same folder as Warmup--

OUTPUT:
    All outputs will be stored in the log folder in the same directory
    Example: bblcount.log, callcount.log, malloctrace.log

RUN:
    I have placed the PIN folder in this directory. I found it useful to alias the PIN executable to avoid braching to find the PIN executable.
        -- alias pin='../Pin/pin-3.13/pin'

    To run a program:

    bblcount.cpp
        pin -t bin/bblcount.so -- ls -l /usr/bin/
        pin -t bin/bblcount.so -- gedit
        pin -t bin/bblcount.so -- wget https://www.cs.stonybrook.edu
    The program is able to count basic blocks for threaded applications like gedit. But fails to perform the operation on multi-process programs like firefox.
    The output lists the BBL count for each thread and the total count.
    
    malloctrace.cpp
        pin -t bin/malloctrace.so -- ls -l /usr/bin/
        pin -t bin/malloctrace.so -- gedit
        pin -t bin/malloctrace.so -- wget https://www.cs.stonybrook.edu
    This program also works for threaded applications. But fails for multi-process applications.

    callcount.cpp
        pin -t bin/callcount.so -- ls -l /usr/bin/
        pin -t bin/callcount.so -- gedit
        pin -t bin/callcount.so -- wget https://www.cs.stonybrook.edu 
    This program also works for threaded applications. But fails for multi-process applications.
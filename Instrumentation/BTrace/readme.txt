PIN version 3.13
The program produces the results as strace.

BUILD:
    make
    make all

TESTS:
    ../pin-3.13/pin -t bin/btrace.so -- ls -l /usr/bin/
    ../pin-3.13/pin -t bin/btrace.so -- wget https://www.cs.stonybrook.edu
    ../pin-3.13/pin -t bin/btrace.so -- gedit
    ../pin-3.13/pin -t bin/btrace.so -- vim

RESULTS:
    The results will be available a log file [LOCATION: log/btrace.log]
    -Most of the common system calls for files, memeory and network calls are documented
    -Remaining of the calls will be posted in a general format specifying the system call number and arguments

LIMITATIONS:
    - The program has a bug that does not print the output to the console all the time. It does print it when running certain applications like gedit. I have not figured out the problem.
      I have used popen to write using cat. Unfortuantely it does not work all the time.
    - Does not work with firefox, libreoffice

CALLS DOCUEMTED:
Open, Close, Read, Write, MMap, Brk, Access, Socketcall, Socket, Connect, FStat, LStat, Stat, 
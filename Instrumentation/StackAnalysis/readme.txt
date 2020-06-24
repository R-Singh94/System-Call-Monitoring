PIN version 3.13
The program assess the stack usage

BUILD:
    make
    make all

TEST:
    ../pin-3.13/pin -t bin/stacktrace.so -- ls -l /usr/bin/
    ../pin-3.13/pin -t bin/stacktrace.so -- libreoffice
    ../pin-3.13/pin -t bin/stacktrace.so -- wget https://www.cs.stonybrook.edu
    ../pin-3.13/pin -t bin/stacktrace.so -- gedit

RESULTS:
    I have only implemented the portion that checks the stack usage. I was not able to work on preventing ROP attacks or detect stack pivoting.
    The application measures the stack usage for multi-threaded applications as well

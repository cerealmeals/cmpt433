# gdb-multiarch -q -x ~/cmpt433/private/as2/remoteDebug.gdb ~/cmpt433/public/light_sampler

# Connect to the remote target
target remote 192.168.6.2:2001

# Set a breakpoint at main
b main

# Optionally, start the program
continue
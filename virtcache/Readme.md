# Virtual Cache
Provides a virtual cache.

## Implemented Caches
- Directly-Mapped

# Building the project
`make lib` to build the library
`make prog` to build the programs

## Building with debug symbols
`make ENABLE_DEBUG=1`

## Cleanup
`make clean`

# Remarks
Note: If the library is rebuilt, you will notice that running processes are using a different version of the library than newly started processes.

# Mime
A C++ library for MIME. 
This is fork of the original library at (https://github.com/WindowsNT/mime)

## Build example with gcc
```
g++ -Werror -Wall -g main.cpp -o mime
```

## What is different

- Removed Win32 specific calls.
- Removed Windows build files. The library is just the header.
- Removed AdES
- Removed Decoding (too many Win32 calls) and not needed for the project at hand

# Caesar

An IDE for C/C++ based on libclang and GtkSourceView.

At the moment, Caesar is meant to help writing code and nothing else.
No project management, no compilation and no debugging.
Please do all these things by writing proper build files, invoking the command line and using and standalone debugger.

# Build

## Dependencies

* A compiler capable of C++11
* CMake
* libclang
* libllvm
* gtkmm3
* gtksourceviewmm3
* PkgConfig

## Building

Standard cmake procedure:

```
cmake -B build .
cmake --build build
```

or, for older versions of CMake:

```
mkdir build
cd build
cmake ..
make
```

# Usage

To use Caesar with your existing project, make sure your project exports a `compile_commands.json`
(this is the only thing that I know of that libclang accepts as input).
For a CMake project, you can achieve that by enabling the flag `CMAKE_EXPORT_COMPILE_COMMANDS`.
Caesar itself is set up to export the commands, so you can use it as a first try.

In the "new project" tab, select the folder of your project (used to search for files) and the build directory
(used to find the build configuration).


# Caveats

The IDE is under development and contains some rough edges.
Many things that are supposed to work automatically at a later stage still require manual interaction, like
requesting code completions and updating the directory tree.
Also, the IDE does not notice when an open file changed on disk and might unknowingly overwrite it when you save.
Unexpected exceptions (which should only occur on wrong usage) will cause a crash instead of a notification dialog.

Bottom line: Use at your own risk.



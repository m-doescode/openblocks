# Building on Linux

For pre-requisite packages, check [deps.txt](./deps.txt)

If your distribution does not provide ReactPhysics3D, you will have to build (and install) it yourself prior to this step

Use the following to generate the build files:

    cmake -Bbuild .

Then build the project using:

    cmake --build build

# Building on Windows

The project will be built using VCPKG and MSVC

## Pre-requisites

* Visual Studio 17.13.3 or higher, with MSVC v143/14.42.34438.0 or higher
* Qt 6.8.3 or higher, with MSVC toolchain
* CMake
* Git (for cloning the repo, optional)
* QScintilla already built (see [docs/qscintilla.md](./docs/qscintilla.md)) *\*likely temporary\**

To start, clone the repository:

    git clone https://git.maelstrom.dev/maelstrom/openblocks

Then, launch the *x64 Native Tools Command Prompt for VS 2022*, and cd into the directory that you cloned the project into

Once in the directory, add Qt to the `CMAKE_PREFIX_PATH` variable (for instance, if you install Qt to `C:\Qt\6.8.3`):

    set CMAKE_PREFIX_PATH=C:\Qt\6.8.3\msvc2022_64

Now, generate the build files with cmake via the vcpkg preset:

    cmake -Bbuild . --preset vcpkg

Then, finally, build in release mode\*:

    cmake --build build --config Release

The compiled binaries should then be placed in `./build/bin/` and should be ready for redistribution without any further work.

If any of the compilation steps fail, or the binaries fail to execute, please create an issue so that this can be corrected.

\* Release mode is necessary as debug mode copies DLLs that are not linked to the output binary

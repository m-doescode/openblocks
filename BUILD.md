# Building on Linux

You will need to install Qt beforehand. All other packages will automatically be obtained through CPM.

ccache is highly recommended.

Use the following to generate the build files:

    cmake -Bbuild .

> [!NOTE]  
> Add -DCMAKE_BUILD_TYPE=Release to produce a release build

Then build the project using:

    cmake --build build

The compiled binaries should then be located in `./build/bin/` and should be ready for redistribution without any further work.

If any of the compilation steps fail, or the binaries fail to execute, please create an issue so that this can be corrected.

# Building on Windows

The process is very similar on Windows

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

Now, generate the build files:

    cmake -Bbuild .

Then, finally build:

    cmake --build build

> [!NOTE]
> To build in release mode, add -DCMAKE_BUILD_TYPE=Release to the configure (first) command,
> and add --config Release to the build (second) command

The compiled binaries should then be located in `./build/bin/[Debug|Release]` and should be ready for redistribution without any further work.

If any of the compilation steps fail, or the binaries fail to execute, please create an issue so that this can be corrected.
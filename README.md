# Geomeculus

**Geomeculus** is a program for doing geometric algebra and **geome**tric cal**culus**.
It is implemented in C. The program is designed to be used as a library as well as a standalone command-line tool.

## Getting started
You can use the `scripts/build.py` script to set up the development environment and build Geomeculus. This project provides a meta build system that automatically detects the code dependencies, generates the [Ninja](https://ninja-build.org) build scripts, and builds the project. The meta build system should work for most modern Linux systems.

To install the build tools, run the following command (`python3` is required) if a supported package manager, `apt`, `dnf`, `pacman`, or `zypper`, is available on your system:
```shell
python3 scripts/build.py --install-build-tools
```

If the build script cannot locate a supported package manager, it will print the required packages that are missing and you need to install them manually. The following packages are required:
- python3 (for running the build script)
- git (for cloning the source code)
- C and C++ compiler (GCC, Clang, or Intel® C/C++ compiler)
- [Ninja](https://ninja-build.org) Build system

To build Geomeculus (release version), simply run:
```shell
python3 scripts/build.py --build
```

To build the debug version, run:
```shell
python3 scripts/build.py --build --debug
```

You can also build the release version and the debug version at the same time:
```shell
python3 scripts/build.py --build --release --debug
```

The supported compilers include Clang, GCC, and Intel® C compiler (icx). You can specify the compiler by using "--compiler" option, with the following choices: "clang", "gcc", and "icx". For example, to use clang:
```shell
python3 scripts/build.py --build --release --compiler clang
```
The default compiler is gcc. To use the icx compiler, you need to install the [Intel® oneAPI Base Toolkit](https://www.intel.com/content/www/us/en/developer/tools/oneapi/base-toolkit-download.html) and configure the local environment variables using the `setvars.sh` script, which can be found in the installation directory, for example:
```shell
source /opt/intel/oneapi/setvars.sh
```

Then, you can build the system using the icx compiler:
```shell
python3 scripts/build.py --build --release --compiler icx
```

### Sanity tests
After building Geomeculus, you can run the sanity tests:
```shell
python3 scripts/build.py --sanity-test --release
```

You can run the sanity tests for both the release version and the debug version if you have built both versions:
```shell
python3 scripts/build.py --sanity-test --release --debug
```

### Running Geomeculus Scripts
A geomeculus script (.gmc) can be run in multiple ways. The simplest way is to run `geomeculus` executable with the script file as the argument. For example:
```shell
./build/release/bin/geomeculus samples/playground.gmc
```

Alternatively, we can use standard input redirection to run the script:
```shell
./build/release/bin/geomeculus < samples/playground.gmc
```

We can also use piping to run the script:
```shell
cat samples/playground.gmc | ./build/release/bin/geomeculus
```

We can import a script using `--import` and enter the interactive mode:
```shell
./build/release/bin/geomeculus --import samples/playground.gmc
```
The named expressions defined in the script following `--import` will be imported into the current interactive session.

We can run the script in the interactive mode as well. First, run the `geomeculus` executable without any arguments:
```shell
./build/release/bin/geomeculus
```

Then, we can type the `exec` commands in the interactive mode.
```shell
exec samples/playground.gmc
```

When we use the `exec` command in the interactive mode, geomeculus will execute the script and then return to the interactive mode. We can use the `exec` command to "import" the named expressions defined in the script into the current interactive session.

## License

Licensed under the [Apache 2.0](LICENSE) license.

## Contributing

### Bug reports

The memory management summary will be printed at the end of the program execution. Any status codes is not "OK", please file a bug report.

### Code contributions

Please submit a pull request.

## Disclaimer

This project shares the same [lead developer](https://www.binshao.info/) as [LightAIMD](https://github.com/microsoft/LightAIMD), so you may notice similarities in code style and design patterns. For details on source code files reused from the LightAIMD repository, please refer to the [NOTICE](NOTICE.md) file.

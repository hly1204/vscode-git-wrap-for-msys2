# VSCode-Git-Wrap-For-Msys2

See [#134146](https://github.com/microsoft/vscode/pull/134146).

Since VSCode consistently looking for an **executable** on Windows, old workarounds cannot work, so I make a wrapped executable for Msys2 Git.

## Usage

Compile in [UCRT64](https://www.msys2.org/docs/environments/) and put the executable somewhere, in my case, the related setting for VSCode should be

```json
"git.path": "C:\\msys64\\home\\hly\\vscode-git-wrap-for-msys2.exe"
```

## Build

Install all build dependencies first.

```shell
pacman -S mingw-w64-ucrt-x86_64-toolchain \
          mingw-w64-ucrt-x86_64-cmake \
          mingw-w64-ucrt-x86_64-ninja
```

### CMake

```shell
cmake -B build \
      -D CMAKE_BUILD_TYPE=Release \
      -G Ninja
cd build
ninja
```

### GCC or CLANG

```shell
cc main.c -O2 -o vscode-git-wrap-for-msys2 -fPIC
strip -p vscode-git-wrap-for-msys2.exe
```

or

```shell
clang main.c -O2 -o vscode-git-wrap-for-msys2 -fPIC
strip -p vscode-git-wrap-for-msys2.exe
```

If you change your installation directory of msys2 other than `C:\msys64`, you will need to change the definition in `main.c` which is

```c
#define USR_BIN_PATH TEXT("<your-path-to-msys64>\\usr\\bin")
```

Hopefully this would work.

## License

[Unlicense](https://unlicense.org/) or [CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/legalcode.en) for the [`main.c`](./main.c) file.

[BSD-3-Clause](https://opensource.org/license/BSD-3-clause) for the [`.clang-format`](./.clang-format) file which I copied from the Qt project.

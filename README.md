
# innoextract - A tool to unpack installers created by Inno Setup

[Inno Setup](http://www.jrsoftware.org/isinfo.php) is a tool to create installers for Microsoft Windows applications. innoextract allows to extract such installers under non-Windows systems without running the actual installer using wine. innoextract currently supports installers created by Inno Setup 1.2.10 to 5.5.5.

In addition to standard Inno Setup installers, innoextract also supports some modified Inno Setup variants including Martijn Laan's My Inno Setup Extensions 3.0.6.1 as well as GOG.com's Inno Setup-based game installers.

innoextract is available under the ZLIB license - see the LICENSE file.

See the website for [Linux packages](http://constexpr.org/innoextract/#packages).

## Contact

[Website](http://constexpr.org/innoextract/)

Author: [Daniel Scharrer](http://constexpr.org/)

## Dependencies

* **[Boost](http://www.boost.org/) 1.37** or newer
* **liblzma** from [xz-utils](http://tukaani.org/xz/) *(optional)*
* **iconv** (*optional*, either as part of the system libc, as is the case with [glibc](http://www.gnu.org/software/libc/) and [uClibc](http://www.uclibc.org/), or as a separate [libiconv](http://www.gnu.org/software/libiconv/))

For Boost you will need the headers as well as the `iostreams`, `filesystem`, `date_time`, `system` and `program_options` libraries. Older Boost version may work but are not actively supported. The boost `iostreams` library needs to be build with zlib and bzip2 support.

While innoextract can be built without liblzma by manually setting `-DUSE_LZMA=OFF`, it is highly recommended and you won't be able to extract most installers created by newer Inno Setup versions without it.

To build innoextract you will also need **[CMake](http://cmake.org/) 2.8** and a working C++ compiler, as well as the development headers for liblzma and boost.

See the Website for [operating system-specific instructions](http://constexpr.org/innoextract/install).

## Compile and install

To compile innoextract, run:

    $ mkdir -p build && cd build
    $ cmake ..
    $ make

To install the binaries system-wide, run as root:

    # make install

Build options:

| Option                   | Default   | Description |
|:------------------------ |:---------:|:----------- |
| `USE_ARC4`               | `ON`      | Build ARC4 decryption support.
| `USE_LZMA`               | `ON`      | Use `liblzma`.
| `WITH_CONV`              | *not set* | The charset conversion library to use. Valid values are `iconv`, `win32` and `builtin`^1. If not set, a library appropriate for the target platform will be chosen.
| `CMAKE_BUILD_TYPE`       | `Release` | Set to `Debug` to enable debug output.
| `DEBUG`                  | `OFF`^2   | Enable debug output and runtime checks.
| `DEBUG_EXTRA`            | `OFF`     | Expensive debug options.
| `SET_WARNING_FLAGS`      | `ON`      | Adjust compiler warning flags. This should not affect the produced binaries but is useful to catch potential problems.
| `SET_OPTIMIZATION_FLAGS` | `ON`      | Adjust compiler optimization flags. For non-debug builds the only thing this does is instruct the linker to only link against libraries that are actually needed.
| `CXX_STD_VERSION`        | `2017`    | Maximum C++ standard version to enable.
| `USE_DYNAMIC_UTIMENSAT`  | `OFF`     | Dynamically load utimensat(2) if not available at compile time
| `USE_STATIC_LIBS`        | `OFF`^3   | Turns on static linking for all libraries, including `-static-libgcc` and `-static-libstdc++`. You can also use the individual options below:
| `LZMA_USE_STATIC_LIBS`   | `OFF`^4   | Statically link `liblzma`.
| `Boost_USE_STATIC_LIBS`  | `OFF`^4   | Statically link Boost. See also `FindBoost.cmake`
| `ZLIB_USE_STATIC_LIBS`   | `OFF`^4   | Statically link `libz`. (used via Boost)
| `BZip2_USE_STATIC_LIBS`  | `OFF`^4   | Statically link `libbz2`. (used via Boost)
| `iconv_USE_STATIC_LIBS`  | `OFF`^4   | Statically link `libiconv`.
| `STRICT_USE`             | `OFF`     | Abort if there are missing optional dependencies
1. The builtin charset conversion only supports Windows-1252 and UTF-16LE. This is normally enough for filenames, but custom message strings (which can be included in filenames) may use arbitrary encodings.
2. Enabled automatically if `CMAKE_BUILD_TYPE` is set to `Debug`.
3. Under Windows, the default is `ON`.
4. Default is `ON` if `USE_STATIC_LIBS` is enabled.

Install options:

| Option                      | Default              | Description |
|:--------------------------- |:--------------------:|:----------- |
| `CMAKE_INSTALL_PREFIX`      | `/usr/local`         | Where to install innoextract.
| `CMAKE_INSTALL_BINDIR`      | `bin`                | Location for binaries (relative to prefix).
| `CMAKE_INSTALL_DATAROOTDIR` | `share`              | Location for data files (relative to prefix).
| `CMAKE_INSTALL_MANDIR`      | `${DATAROOTDIR}/man` | Location for man pages (relative to prefix).

Set options by passing `-D<option>=<value>` to cmake.

## Run

To extract a setup file to the current directory run:

    $ innoextract <file>

A list of available options can be retrieved using

    $ innoextract --help

Documentation is also available as a man page:

    $ man 1 innoextract

## Limitations

* There is no support for extracting individual components and limited support for filtering by name.

* Included scripts and checks are not executed.

* The mapping from Inno Setup variables like the application directory to subdirectories is hard-coded.

* Names for data slice/disk files in multi-file installers must follow the standard naming scheme.

A perhaps more complete, but Windows-only, tool to extract Inno Setup files is [innounp](http://innounp.sourceforge.net/).

Extracting Windows installer executables created by programs other than Inno Setup is out of the scope of this project. Some of these can be unpacked by the following programs:

* [cabextract](http://www.cabextract.org.uk/)

* [unshield](https://github.com/twogood/unshield)

## Disclaimer

This project is in no way associated with Inno Setup or [www.jrsoftware.org](http://www.jrsoftware.org/).

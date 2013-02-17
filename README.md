
# innoextract - A tool to unpack installers created by Inno Setup

[Inno Setup](http://www.jrsoftware.org/isinfo.php) is a tool to create installers for Microsoft Windows applications. innoextract allows to extract such installers under non-Windows systems without running the actual installer using wine. innoextract currently supports installers created by Inno Setup 1.2.10 to 5.5.3.

innoextract is available under the ZLIB license - see the LICENSE file.

See the website for [Linux packages](http://constexpr.org/innoextract/#packages).

## Contact

[Website](http://constexpr.org/innoextract/)

Author: [Daniel Scharrer](http://constexpr.org/)

## Dependencies

* **[Boost](http://www.boost.org/) 1.37** or newer
* **liblzma** from [xz-utils](http://tukaani.org/xz/) *(optional)*
* **iconv** (either as part of the system libc, as is the case with [glibc](http://www.gnu.org/software/libc/) and [uClibc](http://www.uclibc.org/), or as a separate [libiconv](http://www.gnu.org/software/libiconv/))

For Boost you will need the headers as well as the `iostreams`, `filesystem`, `date_time`, `system` and `program_options` libraries. Older Boost version may work but are not actively supported. The boost `iostreams` library needs to be build with zlib and bzip2 support.

While the liblzma dependency is optional, it is highly recommended and you won't be able to extract most installers created by newer Inno Setup versions without it.

To build innoextract you will also need **[CMake](http://cmake.org/) 2.8** and a working C++ compiler, as well as the development headers for liblzma and boost.

The website might have more [specific instructions for your Linux distribution](http://constexpr.org/innoextract/install).

## Compile and install

To compile innoextract, run:

    $ mkdir -p build && cd build && cmake ..
    $ make

To install the binaries system-wide, run as root:

    # make install

Build options:

| Option                   | Default      | Description |
|:------------------------ |:------------:|:----------- |
| `USE_LZMA`               | `ON`         | Use `liblzma` if available.
| `CMAKE_BUILD_TYPE`       | `Release`    | Set to `Debug` to enable debug output.
| `SET_WARNING_FLAGS`      | `ON`         | Adjust compiler warning flags. This should not affect the produced binaries but is useful to catch potential problems.
| `SET_OPTIMIZATION_FLAGS` | `ON`         | Adjust compiler optimization flags. For non-debug builds the only thing this does is instruct the linker to only link against libraries that are actually needed.
| `USE_CXX11`              | `ON`         | Try to compile in C++11 mode if available.
| `DEBUG_EXTRA`            | `OFF`        | Expensive debug options.
| `USE_STATIC_LIBS`        | `OFF`        | Turns on static linking for all libraries, including `-static-libgcc` and `-static-libstdc++`. You can also use the individual options below:
| `LZMA_USE_STATIC_LIBS`   | `OFF`        | Statically link `liblzma`.
| `Boost_USE_STATIC_LIBS`  | `OFF`        | Statically link Boost. See also `FindBoost.cmake`
| `ZLIB_USE_STATIC_LIBS`   | `OFF`        | Statically link `libz`. (used via Boost)
| `BZip2_USE_STATIC_LIBS`  | `OFF`        | Statically link `libbz2`. (used via Boost)
| `iconv_USE_STATIC_LIBS`  | `OFF`        | Statically link `libiconv`.

Install options:

| Option                      | Default              | Description |
|:--------------------------- |:--------------------:|:----------- |
| `CMAKE_INSTALL_PREFIX`      | `/usr/local`         | Where to install innoextract.
| `CMAKE_INSTALL_BINDIR`      | `bin`                | Location for binaries (relative to prefix).
| `CMAKE_INSTALL_DATAROOTDIR` | `share/man`          | Location for data files (relative to prefix).
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

* innoextract currently only supports extracting all the data. There is no support for extracting individual files or components and limited support for extracting language-specific files.

* Included scripts and checks are not executed.

* Data is always extracted to the current directory and the mapping from Inno Setup variables like the application directory to subdirectories is hard-coded.

* innoextract does not check if an installer includes multiple files with the same name and will continually overwrite the destination file when extracting.

* Names for data files in multi-file installers must follow the standard naming scheme.

* Encrypted installers are not supported.

A perhaps more complete, but Windows-only, tool to extract Inno Setup files is [innounp](http://innounp.sourceforge.net/).

Extracting Windows installer executables created by programs other than Inno Setup is out of the scope of this project. Some of these can be unpacked by the following programs:

* [cabextract](http://www.cabextract.org.uk/)

* [unshield](http://www.synce.org/oldwiki/index.php/Unshield)

## Disclaimer

This project is in no way associated with Inno Setup or [www.jrsoftware.org](http://www.jrsoftware.org/).

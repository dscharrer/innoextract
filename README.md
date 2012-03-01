
# Inno Extract - tool to extract installers created by Inno Setup

[Inno Setup](http://www.jrsoftware.org/isinfo.php) is a tool to create installers for Microsoft Windows applications. Inno Extracts allows to extract such installers under non-windows systems without running the actual installer using wine. Inno Extract currently supports installers created by Inno Setup 1.2.10 to 5.4.3.

Inno Extract is available under the ZLIB license - see the LICENSE file.

## Contact

[GitHub project](https://github.com/dscharrer/InnoExtract)

Author: [Daniel Scharrer](mailto:Daniel Scharrer <daniel@constexpr.org>)

## Dependencies

* **Boost 1.37** or newer
* **liblzma** from [xz-utils](http://tukaani.org/xz/) *(optional)*

For Boost you will need the headers as well as the `iostreams`, `filesystem`, `date_time`, `system` and `program_options` libraries. Older Boost version may work but are not actively supported. The boost `iostreams` library needs to be build with zlib and bzip2 support.

While the liblzma dependency is optional, it is highly recommended and you won't be able to extract most installers created by newer Inno Setup versions without it.

## Compile and install

To compile Inno Extract, run:

    $ mkdir -p build && cd build && cmake ..
    $ make

To install the binaries system-wide, run as root:

    # make install

Build options:

* `USE_LZMA` (default=ON): Use *liblzma* if available.
* `CMAKE_BUILD_TYPE` (default=Release): Set to `Debug` to enable debug output.
* `CMAKE_INSTALL_PREFIX` (default: `/usr/local` on UNIX): Where to install Inno Extract.
* `DEBUG_EXTRA` (default=OFF): Expensive debug options
* `MAN_DIR` (default: `share/man`): Install location for man pages (relative to prefix).

Enable by passing `-D<option>=1` to cmake, disable using `-D<option>=0`

## Run

To extract a setup file run:

    $ innoextract <file>

A list of available options can be retrieved using

    $ innoextract --help

Documentation is also available as a man page:

    $ man 1 innoextract

## Limitations

* Inno Extract currently only supports extracting all the data. There is no support for extracting individual files, components or languages.

* Included scripts and checks are not executed.

* Data is always extracted to the current directory and the mapping from Inno Setup variables like the application directory to subdirectories is hard-coded.

* Inno Extract does not check if an installer includes multiple files with the same name and will continually overwrite the destination file when extracting.

A perhaps more complete, but windows-only, tool to extract Inno Setup files is [innounp](http://innounp.sourceforge.net/).

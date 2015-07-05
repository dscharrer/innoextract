Name:           innoextract
Version:        1.4
Release:        4%{?dist}
License:        zlib
Summary:        Inno Setup installers extractor
Url:            http://constexpr.org/innoextract/
Source:         http://constexpr.org/innoextract/files/%{name}-%{version}/%{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  boost-devel
BuildRequires:  xz-devel

%description
Inno Setup is a tool to create installers for Microsoft Windows
applications. innoextract allows to extract such installers under
non-windows systems without running the actual installer using wine.

%prep
%setup -q

%build
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DCMAKE_CXX_FLAGS="$RPM_OPT_FLAGS" -DMAN_DIR=%{_mandir}
make %{?_smp_mflags}

%install
%make_install

%files
%license LICENSE
%doc README.md CHANGELOG VERSION
%{_mandir}/man1/innoextract.1*
%{_bindir}/innoextract

%changelog
* Sun Jul 5 2015 Mosaab Alzoubi <moceap@hotmail.com> - 1.4-5
- Porting to Fedora #2 try
- Rewrite the spec to be compatible with Fedora packaging guidlines #2

* Sat Jul 4 2015 Mosaab Alzoubi <moceap@hotmail.com> - 1.4-4
- Porting to Fedora #1 try
- Rewrite the spec to be compatible with Fedora packaging guidlines #1

* Mon Mar 11 2013 Daniel Scharrer <daniel@constexpr.org> - 1.4-3
- Bump version to 1.4 (new upstream release):
- Fixed issues with the progress bar in sandbox environments
- Fixed extracting very large installers with 32-bit innoextract builds
- Improved handling
- The --list command-line option can now combined with --test or --extract
- The --version command-line option can now be modified with --quiet
  or --silent
- Added support for preserving timestamps of extracted files
  (enabled by default)
- Added a --timestamps (-T) command-line options to control or disable
  file timestamps
- Added an --output-dir (-d) command-line option to control where files
  are extracted
- Various bug fixes and tweaks

* Tue Jul 03 2012 Daniel Scharrer <daniel@constexpr.org> -1.3-1
- bump version to 1.3:
- Respect --quiet and --silent for multi-file installers
- Compile in C++11 mode if supported
- Warn about unsupported setup data versions
- Add support for Inno Setup 5.5.0 installers

* Sun Mar 25 2012 Daniel Scharrer <daniel@constexpr.org> - 1.2-1
- created package

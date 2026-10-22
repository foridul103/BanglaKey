Name:           banglakey
Version:        0.1.0
Release:        1%{?dist}
Summary:        Native Bengali input method for Linux (Bijoy & Phonetic)

License:        GPL-3.0-or-later
URL:            https://github.com/banglakey/banglakey-linux
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc-c++ >= 11
BuildRequires:  cmake >= 3.25
BuildRequires:  ninja-build
BuildRequires:  extra-cmake-modules
BuildRequires:  fcitx5-devel >= 5.1.0
BuildRequires:  qt6-qtbase-devel >= 6.5
BuildRequires:  qt6-qtdeclarative-devel >= 6.5
BuildRequires:  gtest-devel

Requires:       fcitx5
Requires:       qt6-qtdeclarative

%description
BanglaKey is a native, high-performance Bengali input system for Linux.
It integrates both Bijoy-compatible layout typing and natural phonetic
typing into a single input method engine under Fcitx5, alongside a modern
Qt 6 / QML settings center tailored for KDE Plasma 6 on Wayland and X11.

%package devel
Summary:        Development headers and libraries for BanglaKey engine
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
The %{name}-devel package contains libraries and header files for
developing applications that use the BanglaKey typing engine.

%prep
%autosetup -p1

%build
%cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBANGLAKEY_BUILD_ENGINE=ON \
    -DBANGLAKEY_BUILD_FCITX5=ON \
    -DBANGLAKEY_BUILD_DESKTOP=ON \
    -DBANGLAKEY_BUILD_TESTS=ON \
    -DBANGLAKEY_INSTALL=ON
%cmake_build

%check
%ctest

%install
%cmake_install

%files
%license LICENSE
%doc README.md docs/ARCHITECTURE.md
%{_bindir}/banglakey-settings
%{_libdir}/fcitx5/banglakey-fcitx5.so
%{_datadir}/fcitx5/addon/banglakey.conf
%{_datadir}/fcitx5/inputmethod/banglakey-addon.conf

%files devel
%{_includedir}/banglakey/
%{_libdir}/libbanglakey_engine.a

%changelog
* Wed Oct 15 2026 Foredul Haque <foredulhaque@example.com> - 0.1.0-1
- Initial Phase 1 foundation package release for Fedora 44

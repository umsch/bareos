# Determine Windows BITS (32/64) from name (mingw32-.../ming64-...)
#%define WINDOWS_BITS %(echo %name | grep 64 >/dev/null 2>&1 && echo "64" || echo "32")



# flavors:
#   If name contains debug, enable debug during build.
#   If name contains prevista, build for windows < vista.
%define flavors postvista postvista-debug
%define dirs_with_unittests lib findlib
%define bareos_configs bareos-dir.d/ bareos-fd.d/ bareos-sd.d/ tray-monitor.d/ bconsole.conf

Name:           winbareos
Version:        0.0.0
Release:        0
Summary:        Bareos build for Windows
License:        LGPLv2+
Group:          Development/Libraries
URL:            http://bareos.org
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch:      noarch
#!BuildIgnore: post-build-checks
Source0:        bareos-%{version}.tar.bz2

%define addonsdir /bareos-addons/
BuildRequires:  bareos-addons

BuildRequires:  mingw32-filesystem
BuildRequires:  mingw32-cross-gcc
BuildRequires:  mingw32-cross-gcc-c++
BuildRequires:  mingw32-cross-binutils
BuildRequires:  mingw32-cross-pkg-config
BuildRequires:  mingw32-libqt4
BuildRequires:  mingw32-libqt4-devel
BuildRequires:  mingw32-libqt4-filesystem
BuildRequires:  mingw32-libwinpthread1
BuildRequires:  mingw32-winpthreads-devel
BuildRequires:  mingw32-libopenssl-devel
BuildRequires:  mingw32-libopenssl
BuildRequires:  mingw32-openssl
BuildRequires:  mingw32-libdb-devel
BuildRequires:  mingw32-libgcc
BuildRequires:  mingw32-libtool
BuildRequires:  mingw32-libgcrypt20
BuildRequires:  mingw32-cross-nsis
BuildRequires:  mingw32-gcc
BuildRequires:  mingw32-gcc-c++
BuildRequires:  mingw32-zlib-devel
BuildRequires:  mingw32-zlib
BuildRequires:  mingw32-libpng16-16
BuildRequires:  mingw32-libstdc++
BuildRequires:  mingw32-readline
BuildRequires:  mingw32-readline-devel
BuildRequires:  mingw32-lzo
BuildRequires:  mingw32-lzo-devel
BuildRequires:  mingw32-libfastlz
BuildRequires:  mingw32-libfastlz-devel
BuildRequires:  mingw32-libsqlite3-0
BuildRequires:  mingw32-libsqlite-devel
BuildRequires:  mingw32-gtest-devel
BuildRequires:  mingw32-libgtest0
BuildRequires:  mingw32-libjansson
BuildRequires:  mingw32-libjansson-devel


BuildRequires:  mingw64-filesystem
BuildRequires:  mingw64-cross-gcc
BuildRequires:  mingw64-cross-gcc-c++
BuildRequires:  mingw64-cross-binutils
BuildRequires:  mingw64-cross-pkg-config
BuildRequires:  mingw64-libqt4
BuildRequires:  mingw64-libqt4-devel
BuildRequires:  mingw64-libqt4-filesystem
BuildRequires:  mingw64-libwinpthread1
BuildRequires:  mingw64-winpthreads-devel
BuildRequires:  mingw64-libopenssl-devel
BuildRequires:  mingw64-libopenssl
BuildRequires:  mingw64-openssl
BuildRequires:  mingw64-libdb-devel
BuildRequires:  mingw64-libgcc
BuildRequires:  mingw64-libtool
BuildRequires:  mingw64-libgcrypt20
BuildRequires:  mingw64-cross-nsis
BuildRequires:  mingw64-gcc
BuildRequires:  mingw64-gcc-c++
BuildRequires:  mingw64-zlib-devel
BuildRequires:  mingw64-zlib
BuildRequires:  mingw64-libpng16-16
BuildRequires:  mingw64-libstdc++
BuildRequires:  mingw64-readline
BuildRequires:  mingw64-readline-devel
BuildRequires:  mingw64-lzo
BuildRequires:  mingw64-lzo-devel
BuildRequires:  mingw64-libfastlz
BuildRequires:  mingw64-libfastlz-devel
BuildRequires:  mingw64-libsqlite3-0
BuildRequires:  mingw64-libsqlite-devel
BuildRequires:  mingw64-gtest-devel
BuildRequires:  mingw64-libgtest0
BuildRequires:  mingw64-libjansson
BuildRequires:  mingw64-libjansson-devel

BuildRequires:  bc
BuildRequires:  less
BuildRequires:  procps
BuildRequires:  sed
BuildRequires:  vim
BuildRequires:  cmake

%description
Base package for Bareos Windows build.

%package postvista-32
Summary:        bareos
%description postvista-32
Bareos for Windows versions >= Windows Vista

%package postvista-64
Summary:        bareos
%description postvista-64
Bareos for Windows versions >= Windows Vista


%package postvista-debug-32
Summary:        bareos
%description postvista-debug-32
Bareos Debug for Windows versions >= Windows Vista

%package postvista-debug-64
Summary:        bareos
%description postvista-debug-64
Bareos Debug for Windows versions >= Windows Vista

%package debugsrc
Summary: bareos debug sources
%description debugsrc
Bareos debug sources for Windows


%prep
%setup -q -n bareos-%{version}

pushd core
# unpack addons
for i in `ls %addonsdir`; do
   tar xvf %addonsdir/$i
done

popd

for flavor in %flavors; do
  for WINDOWS_BITS in 32 64; do
     mkdir $flavor-$WINDOWS_BITS
  done
done

%build

for flavor in %flavors; do

   WINDOWS_VERSION=$(echo $flavor | grep postvista >/dev/null && echo 0x600 || echo 0x500)

   CMAKE_PARAMS="-Dsqlite3=yes \
      -Dpostgresql=yes \
      -Dtraymonitor=yes \
      -DWINDOWS_VERSION=${WINDOWS_VERSION} \
      -Ddb_password=@db_password@ \
      -Ddb_port=@db_port@ \
      -Ddb_user=@db_user@ \
      -Ddir_password=@dir_password@ \
      -Dfd_password=@fd_password@ \
      -Dsd_password=@sd_password@ \
      -Dmon_dir_password=@mon_dir_password@ \
      -Dmon_fd_password=@mon_fd_password@ \
      -Dmon_sd_password=@mon_sd_password@ \
      -Dsd_password=@sd_password@ \
      ../core"

   export WINDOWS_BITS=64
%define __strip %{_mingw64_strip}
%define __objdump %{_mingw64_objdump}
%define _use_internal_dependency_generator 0
#define __find_requires %%{_mingw64_findrequires}
%define __find_provides %{_mingw64_findprovides}
#define __os_install_post #{_mingw64_debug_install_post} \
#                          #{_mingw64_install_post}
%define bindir %{_mingw64_bindir}
   pushd $flavor-$WINDOWS_BITS
   %{_mingw64_cmake_qt4} ${CMAKE_PARAMS} -DWINDOWS_BITS=$WINDOWS_BITS
   make %{?jobs:-j%jobs} DESTDIR=%{buildroot}/${flavor}-$WINDOWS_BITS install
   popd

   export WINDOWS_BITS=32

%define __strip %{_mingw32_strip}
%define __objdump %{_mingw32_objdump}
%define _use_internal_dependency_generator 0
#define __find_requires %%{_mingw32_findrequires}
%define __find_provides %{_mingw32_findprovides}
#define __os_install_post #{_mingw32_debug_install_post} \
#                          #{_mingw32_install_post}
%define bindir %{_mingw32_bindir}
   pushd $flavor-$WINDOWS_BITS
   %{_mingw32_cmake_qt4} ${CMAKE_PARAMS} -DWINDOWS_BITS=$WINDOWS_BITS
   make %{?jobs:-j%jobs} DESTDIR=%{buildroot}/${flavor}-$WINDOWS_BITS install
   popd



done


%install
cp  -av ../bareos-%{version} $RPM_BUILD_ROOT/

%clean
rm -rf $RPM_BUILD_ROOT

%files


%files postvista-32
%defattr(-,root,root)
/winbareos-%version-postvista-32-bit-r*.exe

%files postvista-64
%defattr(-,root,root)
/winbareos-%version-postvista-64-bit-r*.exe

%files postvista-debug-32
/winbareos-%version-postvista-debug-32-bit-r*.exe

%files postvista-debug-64
/winbareos-%version-postvista-debug-64-bit-r*.exe


%files debugsrc
%defattr(-,root,root)
/bareos-*

%changelog

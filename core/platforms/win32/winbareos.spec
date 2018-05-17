# flavors:
#   If name contains debug, enable debug during build.
#   If name contains prevista, build for windows < vista.
#define flavors postvista postvista-debug

%define flavors postvista postvista-debug
%define bits 32 64


%define dirs_with_unittests lib findlib
%define bareos_configs bareos-dir.d/ bareos-fd.d/ bareos-sd.d/ tray-monitor.d/ bconsole.conf

%define SIGNCERT %{_builddir}/ia.p12
%define SIGNPWFILE %{_builddir}/signpassword


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


Source1:         winbareos.nsi
Source2:         clientdialog.ini
Source3:         directordialog.ini
Source4:         storagedialog.ini
Source6:         bareos.ico
Source9:         databasedialog.ini



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

BuildRequires:  opsi-utils
%define opsidest /var/lib/opsi/repository



BuildRequires:  bareos-addons
BuildRequires:  winbareos-nssm
BuildRequires:  winbareos-php

BuildRequires:  mingw32-sed
BuildRequires:  mingw64-sed

BuildRequires:  mingw32-sqlite
BuildRequires:  mingw64-sqlite

BuildRequires:  osslsigncode
BuildRequires:  obs-name-resolution-settings


%define NSISDLLS KillProcWMI.dll AccessControl.dll LogEx.dll

%description
Base package for Bareos Windows build.

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
   mkdir -p $flavor-$WINDOWS_BITS-build
   pushd $flavor-$WINDOWS_BITS-build
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
   mkdir -p $flavor-$WINDOWS_BITS-build
   pushd $flavor-$WINDOWS_BITS-build
   %{_mingw32_cmake_qt4} ${CMAKE_PARAMS} -DWINDOWS_BITS=$WINDOWS_BITS
   make %{?jobs:-j%jobs} DESTDIR=%{buildroot}/${flavor}-$WINDOWS_BITS install
   popd


done


for flavor in %{flavors}; do

   cd %{_builddir}/bareos-%{version}/core

   WIN_DEBUG=$(echo $flavor | grep debug >/dev/null && echo yes || echo no)

   mkdir -p $RPM_BUILD_ROOT/$flavor/nsisplugins
   for dll in %NSISDLLS; do
      cp $dll $RPM_BUILD_ROOT/$flavor/nsisplugins
   done

   for BITS in 32 64; do
      mkdir -p $RPM_BUILD_ROOT/$flavor/release${BITS}
   done

   DESCRIPTION="Bareos - Backup Archiving Recovery Open Sourced"

   #cd %{buildroot}

   for BITS in 32 64; do
      for file in `find %{buildroot}/${flavor}-$BITS -name '*.exe'` `find %{buildroot}/${flavor}-$BITS -name '*.dll'` ; do
         basename=`basename $file`
         dest=$RPM_BUILD_ROOT/$flavor/release${BITS}/$basename
         cp $file $dest
      done
   done

   pushd    $RPM_BUILD_ROOT/$flavor/release${BITS}

      #libcmocka.dll \
   for file in \
      libcrypto-*.dll \
      libfastlz.dll \
      libgcc_s_*-1.dll \
      libhistory6.dll \
      libjansson-4.dll \
      liblzo2-2.dll \
      libpng*.dll \
      libreadline6.dll \
      libssl-*.dll \
      libstdc++-6.dll \
      libsqlite3-0.dll \
      libtermcap-0.dll \
      openssl.exe \
      libwinpthread-1.dll \
      QtCore4.dll \
      QtGui4.dll \
      sed.exe \
      sqlite3.exe \
      zlib1.dll \
   ; do
      cp %{_mingw32_bindir}/$file $RPM_BUILD_ROOT/$flavor/release32
      cp %{_mingw64_bindir}/$file $RPM_BUILD_ROOT/$flavor/release64
   done
   popd

   for BITS in 32 64; do
      if [  "${BITS}" = "64" ]; then
         MINGWDIR=x86_64-w64-mingw32
         else
         MINGWDIR=i686-w64-mingw32
      fi

      # run this in subshell in background
      (
      mkdir -p $RPM_BUILD_ROOT/$flavor/release${BITS}
      pushd    $RPM_BUILD_ROOT/$flavor/release${BITS}

      echo "The installer may contain the following software:" >> %_sourcedir/LICENSE
      echo "" >> %_sourcedir/LICENSE

      # nssm
      cp -a /usr/lib/windows/nssm/win${BITS}/nssm.exe .
      echo "" >> %_sourcedir/LICENSE
      echo "NSSM - the Non-Sucking Service Manager: https://nssm.cc/" >> %_sourcedir/LICENSE
      echo "##### LICENSE FILE OF NSSM START #####" >> %_sourcedir/LICENSE
      cat /usr/lib/windows/nssm/README.txt >> %_sourcedir/LICENSE
      echo "##### LICENSE FILE OF NSSM END #####" >> %_sourcedir/LICENSE
      echo "" >> %_sourcedir/LICENSE


      # bareos-webui
      #mkdir bareos-webui
      cp -a %{_builddir}/bareos-%{version}/webui webui  # copy bareos-webui

      #cp -a %{_builddir}/bareos-%{version}/webui/install .

      #mkdir -p tests
      #cp -a %{_builddir}/bareos-%{version}/webui/tests/selenium tests

      pwd
      echo "" >> %_sourcedir/LICENSE
      echo "##### LICENSE FILE OF BAREOS_WEBUI START #####" >> %_sourcedir/LICENSE
      cat  %{_builddir}/bareos-%{version}/webui/LICENSE >> %_sourcedir/LICENSE # append bareos-webui license file to LICENSE
      echo "##### LICENSE FILE OF BAREOS_WEBUI END #####" >> %_sourcedir/LICENSE
      echo "" >> %_sourcedir/LICENSE


      # php
      cp -a /usr/lib/windows/php/ .
      #cp php/php.ini .
      echo "" >> %_sourcedir/LICENSE
      echo "PHP: http://php.net/" >> %_sourcedir/LICENSE
      echo "##### LICENSE FILE OF PHP START #####" >> %_sourcedir/LICENSE
      cat php/license.txt >> %_sourcedir/LICENSE
      echo "##### LICENSE FILE OF PHP END #####" >> %_sourcedir/LICENSE
      echo "" >> %_sourcedir/LICENSE

      pwd
      popd

      # copy the sql ddls over
      cp -a  %{_builddir}/bareos-%{version}/core/src/cats/ddl $RPM_BUILD_ROOT/$flavor/release${BITS}

      # copy the sources over if we create debug package
      cp -a %{_builddir}/bareos-%{version}/core/src $RPM_BUILD_ROOT/$flavor/release${BITS}/bareos-%{version}

      cp -r %{buildroot}/${flavor}-$BITS/usr/${MINGWDIR}/sys-root/mingw/etc/bareos $RPM_BUILD_ROOT/$flavor/release${BITS}/config

      cp -r %{_builddir}/bareos-%{version}/core/platforms/win32/bareos-config-deploy.bat $RPM_BUILD_ROOT/$flavor/release${BITS}

      cp -r %{_builddir}/bareos-%{version}/core/platforms/win32/fillup.sed $RPM_BUILD_ROOT/$flavor/release${BITS}/config

      mkdir $RPM_BUILD_ROOT/$flavor/release${BITS}/Plugins
      cp -rv %{_builddir}/bareos-%{version}/core/src/plugins/*/*.py $RPM_BUILD_ROOT/$flavor/release${BITS}/Plugins

      cp %SOURCE1 %SOURCE2 %SOURCE3 %SOURCE4 %SOURCE6 %SOURCE9 \
               %_sourcedir/LICENSE $RPM_BUILD_ROOT/$flavor/release${BITS}
      cd $RPM_BUILD_ROOT/$flavor/release${BITS}

      makensis -DVERSION=%version -DPRODUCT_VERSION=%version-%release -DBIT_WIDTH=${BITS} \
               -DWIN_DEBUG=${WIN_DEBUG} $RPM_BUILD_ROOT/$flavor/release${BITS}/winbareos.nsi | sed "s/^/${flavor}-${BITS}BIT-DEBUG-${WIN_DEBUG}: /g"
      ) &
      #subshell end
   done
done

# wait for subshells to complete
wait






%install
cp  -a ../bareos-%{version} $RPM_BUILD_ROOT/


#for flavor in %{flavors}; do
#   mkdir -p $RPM_BUILD_ROOT%{_mingw32_bindir}
#   mkdir -p $RPM_BUILD_ROOT%{_mingw64_bindir}
#
#   FLAVOR=`echo "%name" | sed 's/winbareos-nsi-//g'`
#   DESCRIPTION="Bareos installer version %version"
#   URL="http://www.bareos.com"
#
#   for BITS in 32 64; do
#      cp $RPM_BUILD_ROOT/$flavor/release${BITS}/Bareos*.exe \
#           $RPM_BUILD_ROOT/winbareos-%version-$flavor-${BITS}-bit-r%release-unsigned.exe
#
#      osslsigncode  sign \
#                    -pkcs12 %SIGNCERT \
#                    -readpass %SIGNPWFILE \
#                    -n "${DESCRIPTION}" \
#                    -i http://www.bareos.com/ \
#                    -t http://timestamp.comodoca.com/authenticode \
#                    -in  $RPM_BUILD_ROOT/winbareos-%version-$flavor-${BITS}-bit-r%release-unsigned.exe \
#                    -out $RPM_BUILD_ROOT/winbareos-%version-$flavor-${BITS}-bit-r%release.exe
#
#      osslsigncode verify -in $RPM_BUILD_ROOT/winbareos-%version-$flavor-${BITS}-bit-r%release.exe
#
#      rm $RPM_BUILD_ROOT/winbareos-%version-$flavor-${BITS}-bit-r%release-unsigned.exe
#
#      rm -R $RPM_BUILD_ROOT/$flavor/release${BITS}
#
#   done
#
#   rm -R $RPM_BUILD_ROOT/$flavor/nsisplugins
#done

mkdir -p $RPM_BUILD_ROOT%{_mingw32_bindir}
mkdir -p $RPM_BUILD_ROOT%{_mingw64_bindir}


for flavor in %{flavors}; do

   FLAVOR=`echo "%name" | sed 's/winbareos-nsi-//g'`
   DESCRIPTION="Bareos installer version %version"
   URL="http://www.bareos.com"

   for BITS in 32 64; do
      cp $RPM_BUILD_ROOT/$flavor/release${BITS}/Bareos*.exe \
           $RPM_BUILD_ROOT/winbareos-%version-$flavor-${BITS}-bit-r%release-unsigned.exe

      mv  $RPM_BUILD_ROOT/winbareos-%version-$flavor-${BITS}-bit-r%release-unsigned.exe \
          $RPM_BUILD_ROOT/winbareos-%version-$flavor-${BITS}-bit-r%release.exe

      rm -R $RPM_BUILD_ROOT/$flavor/release${BITS}

   done

   rm -R $RPM_BUILD_ROOT/$flavor/nsisplugins
done



#opsi

cd core/platforms

# OPSI ProductVersion is at most 32 characters long
VERSION32C=$(sed -r -e 's/(.{1,32}).*/\1/' -e 's/\.*$//' <<< %{version})

# set version and release for OPSI
sed -i -e "s/^version: \$PackageVersion/version: %{release}/i" \
       -e "s/^version: \$ProductVersion/version: $VERSION32C/i" opsi/OPSI/control
WINBAREOS32=`ls -1 $RPM_BUILD_ROOT/winbareos*-postvista-32-bit-*.exe`
WINBAREOS64=`ls -1 $RPM_BUILD_ROOT/winbareos*-postvista-64-bit-*.exe`
if [ -r "$WINBAREOS32" ] && [ -r "$WINBAREOS64" ]; then
    mkdir -p opsi/CLIENT_DATA/data
    cp -a $WINBAREOS32 $WINBAREOS64 opsi/CLIENT_DATA/data

    WINBAREOS32b='data\\'`basename $WINBAREOS32`
    WINBAREOS64b='data\\'`basename $WINBAREOS64`
    sed -i -e's/^Set $ProductExe32$ .*/Set $ProductExe32$ = "'$WINBAREOS32b'"/' \
           -e's/^Set $ProductExe64$ .*/Set $ProductExe64$ = "'$WINBAREOS64b'"/' opsi/CLIENT_DATA/setup3264.ins

    opsi-makeproductfile -m -z opsi
fi
test -r winbareos*.opsi

mkdir -p $RPM_BUILD_ROOT%{opsidest}
cp -a winbareos*.opsi* $RPM_BUILD_ROOT%{opsidest}




# cleanup

rm -R $RPM_BUILD_ROOT/bareos-%{version}
rm -R $RPM_BUILD_ROOT/post*

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root)
/winbareos-*.exe
%{opsidest}/winbareos*.opsi*

%changelog

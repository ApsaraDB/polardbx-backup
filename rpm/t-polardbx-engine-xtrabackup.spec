#####################################
Name:          t-polardbx-engine-xtrabackup
Version:       8.0.32
Release:       %(echo $RELEASE)%{?dist}
Summary:       XtraBackup online backup for MySQL / InnoDB

Group:         Applications/Databases
License:       GPLv2
URL:           http://gitlab.alibaba-inc.com/GalaxyEngine/percona-xtrabackup


BuildRequires: cmake >= 3.8.2
BuildRequires: libaio-devel, libgcrypt-devel, ncurses-devel, readline-devel
BuildRequires: zlib-devel, libev-devel, libcurl-devel, libstdc++-static
BuildRequires: devtoolset-7-gcc
BuildRequires: devtoolset-7-gcc-c++
BuildRequires: devtoolset-7-binutils
BuildRequires: bison, libudev-devel, python-sphinx, procps-ng-devel

%global __python %{__python3}

Requires:      rsync
BuildRoot:     %{_tmppath}/%{name}-%{version}-%{release}-root
Packager:      jianwei.zhao@alibaba-inc.com
AutoReqProv:   no
Prefix:        /u01/polardbx_engine_xtrabackup80

%define _mandir /usr/share/man
# do not strip binary files, just compress man doc
%define __os_install_post /usr/lib/rpm/brp-compress
%define _unpackaged_files_terminate_build 0

%description
Percona XtraBackup is OpenSource online (non-blockable) backup solution for InnoDB and XtraDB engines

%build
cd $OLDPWD/../

CC=/opt/rh/devtoolset-7/root/usr/bin/gcc
CXX=/opt/rh/devtoolset-7/root/usr/bin/g++
CFLAGS="-O3 -g -fexceptions -static-libgcc -static-libstdc++ -fno-omit-frame-pointer -fno-strict-aliasing"
CXXFLAGS="-O3 -g -fexceptions -static-libgcc -static-libstdc++ -fno-omit-frame-pointer -fno-strict-aliasing"
export CC CXX CFLAGS CXXFLAGS

cat extra/boost/boost_1_77_0.tar.bz2.*  > extra/boost/boost_1_77_0.tar.bz2

cmake -DBUILD_CONFIG=xtrabackup_release \
      -DCMAKE_BUILD_TYPE="RelWithDebInfo" \
      -DCMAKE_INSTALL_PREFIX=%{prefix} \
      -DINSTALL_MANDIR=%{_mandir} \
      -DWITH_BOOST="./extra/boost/boost_1_77_0.tar.bz2" \
      -DINSTALL_PLUGINDIR="lib/plugin" \
      -DFORCE_INSOURCE_BUILD=1 .

%install
cd $OLDPWD/../
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT  -j `cat /proc/cpuinfo | grep processor| wc -l` || make install DESTDIR=$RPM_BUILD_ROOT  -j `cat /proc/cpuinfo | grep processor| wc -l` 
rm -rf $RPM_BUILD_ROOT/%{_libdir}/libmysqlservices.a
rm -rf $RPM_BUILD_ROOT/usr/lib/libmysqlservices.a
rm -rf $RPM_BUILD_ROOT/usr/docs/INFO_SRC
rm -rf $RPM_BUILD_ROOT/%{_mandir}/man8
rm -rf $RPM_BUILD_ROOT/%{_mandir}/man1/c*
rm -rf $RPM_BUILD_ROOT/%{_mandir}/man1/m*
rm -rf $RPM_BUILD_ROOT/%{_mandir}/man1/i*
rm -rf $RPM_BUILD_ROOT/%{_mandir}/man1/l*
rm -rf $RPM_BUILD_ROOT/%{_mandir}/man1/p*
rm -rf $RPM_BUILD_ROOT/%{_mandir}/man1/z*
rm -rf $RPM_BUILD_ROOT/%{prefix}/xtrabackup-test

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{prefix}/bin/xtrabackup
%{prefix}/bin/xbstream
%{prefix}/bin/xbcrypt
%{prefix}/bin/xbcloud
%{prefix}/bin/xbcloud_osenv
%{prefix}/lib/plugin/keyring_file.so
%{prefix}/lib/plugin/keyring_rds.so
%{prefix}/bin/mysqlbinlogtailor


%changelog
* Fri Aug 31 2018 Evgeniy Patlan <evgeniy.patlan@percona.com>
- Packaging for 8.0
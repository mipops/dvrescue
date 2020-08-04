# Copyright (c) 2019-2020 Info@MediaArea.net
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.

# norootforbuild

%global dvrescue_version		0.20.06
%global libmediainfo_version	19.09
%global libzen_version			0.4.37

Name:			dvrescue
Version:		%dvrescue_version
Release:		1
Summary:		Data migration from DV tapes into digital files suitable for long-term preservation
Group:			Productivity/Multimedia/Other
License:		BSD-3-Clause
URL:			https://MediaArea.net/DVRescue
Packager:		Jerome Martinez <Info@MediaArea.net>
Source0:		dvrescue_%{version}.tar.gz
Prefix:			%{_prefix}
BuildRoot:		%{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: 	gcc-c++
BuildRequires:	pkgconfig
BuildRequires:  automake
BuildRequires:  autoconf
BuildRequires:  libtool
BuildRequires:	libmediainfo-devel >= %libmediainfo_version
BuildRequires:	libzen-devel >= %libzen_version
BuildRequires:	zlib-devel
Requires:		xmlstarlet

%if 0%{?rhel_version} >= 800 || 0%{?centos_version} >= 800
BuildRequires:  gdb
%endif

%if 0%{?mageia}
%ifarch x86_64
BuildRequires:  lib64openssl-devel
%else
BuildRequires:  libopenssl-devel
%endif
%endif

%description
Data migration from DV tapes into digital files suitable for long-term preservation

%prep
%setup -q -n dvrescue
%__chmod 644 *.txt *.md LICENSE

%build
export CFLAGS="-g $RPM_OPT_FLAGS"
export CXXFLAGS="-g $RPM_OPT_FLAGS"

# build CLI
pushd Project/GNU/CLI
	%__chmod +x autogen
	./autogen
	%if 0%{?mageia} >= 6
		%configure --disable-dependency-tracking
	%else
		%configure
	%endif

	%__make %{?jobs:-j%{jobs}}
popd

%install
pushd Project/GNU/CLI
	%__make install DESTDIR=%{buildroot}
popd

%clean
[ -d "%{buildroot}" -a "%{buildroot}" != "" ] && %__rm -rf "%{buildroot}"

%files
%defattr(-,root,root,-)
%doc LICENSE
%doc History.txt
%{_bindir}/*

%changelog
* Tue Jan 01 2019 Jerome Martinez <Info@MediaArea.net> - 0.20.06-0
- See History.txt for more info and real dates

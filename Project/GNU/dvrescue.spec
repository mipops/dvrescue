# Copyright (c) 2019-2020 Info@MediaArea.net
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.

# norootforbuild

%global dvrescue_version		0.21.11
%global libmediainfo_version	21.09
%global libzen_version			0.4.39

Name:			dvrescue
Version:		%dvrescue_version
Release:		1
Summary:		Convert DV tapes into digital files suitable for long-term preservation
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

%if 0%{?fedora_version} >= 33
%global build_gui 1
%else
%global build_gui 0
%endif

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

%if 0%{?build_gui}
%package gui
Summary:	Convert DV tapes into digital files suitable for long-term preservation (GUI)
Group:		Productivity/Multimedia/Other

BuildRequires:  nasm
BuildRequires:  libXv-devel
%if 0%{?fedora_version} || 0%{?centos} >= 7
BuildRequires:  pkgconfig(Qt5)
BuildRequires:  pkgconfig(Qt5QuickControls2)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5XmlPatterns)
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  pkgconfig(Qt5Qwt6)
BuildRequires:  pkgconfig(alsa)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(libxslt)
Requires:       xmlstarlet

BuildRequires:  libusbx
%endif

%if 0%{?mageia}
%ifarch x86_64
BuildRequires:  lib64qt5base5-devel
BuildRequires:  lib64qt5quicktemplates2-devel
BuildRequires:  lib64qt5quicktemplates2_5
BuildRequires:  lib64qt5quickcontrols2-devel
BuildRequires:  lib64qt5quickcontrols2_5
BuildRequires:  lib64qt5quickwidgets-devel
BuildRequires:  lib64qt5multimedia-devel
BuildRequires:  lib64qt5svg-devel
BuildRequires:  lib64qt5xmlpatterns-devel
BuildRequires:  lib64qt5xmlpatterns5
BuildRequires:  lib64qwt-qt5-devel
BuildRequires:  lib64qwt-qt5_6
%else
BuildRequires:  libqt5base5-devel
BuildRequires:  libqt5quicktemplates2-devel
BuildRequires:  libqt5quicktemplates2_5
BuildRequires:  libqt5quickcontrols2-devel
BuildRequires:  libqt5quickcontrols2_5
BuildRequires:  libqt5quickwidgets-devel
BuildRequires:  libqt5multimedia-devel
BuildRequires:  libqt5svg-devel
BuildRequires:  libqt5xmlpatterns-devel
BuildRequires:  libqt5xmlpatterns5
BuildRequires:  libqwt-qt5-devel
BuildRequires:  libqwt-qt5_6
%endif
%endif

%if 0%{?suse_version} >= 1200
BuildRequires:  libqt5-qtbase-devel
BuildRequires:  libqt5-qtsvg-devel
BuildRequires:  libqt5-qtxmlpatterns-devel
BuildRequires:  libqt5-qtmultimedia-devel
BuildRequires:  libQt5QuickControls2-devel
BuildRequires:  qwt6-devel
%endif
%endif

%description
Data migration from DV tapes into digital files suitable for long-term preservation

%if 0%{?build_gui}
%description gui
Data migration from DV tapes into digital files suitable for long-term preservation (Graphical User Interface)
%endif

%prep
%setup -q -n dvrescue
%__chmod 644 dvrescue/*.txt dvrescue/*.md dvrescue/LICENSE.txt

%build
export CFLAGS="-g $RPM_OPT_FLAGS"
export CXXFLAGS="-g $RPM_OPT_FLAGS"

# build CLI
pushd dvrescue/Project/GNU/CLI
	%__chmod +x autogen
	./autogen
	%if 0%{?mageia} >= 6
		%configure --disable-dependency-tracking
	%else
		%configure
	%endif

	%__make %{?jobs:-j%{jobs}}
popd

# now build GUI
%if 0%{?build_gui}
pushd ffmpeg
	./configure --enable-gpl --disable-autodetect --enable-alsa --disable-doc --disable-debug --enable-pic --enable-static --enable-lto --disable-shared --prefix=`pwd`
	%__make %{?jobs:-j%{jobs}} install
popd
pushd ZenLib/Project/GNU/Library
	./configure --enable-static --disable-shared
	%__make %{?jobs:-j%{jobs}}
popd
pushd MediaInfoLib/Project/GNU/Library
	./configure --enable-static --disable-shared --enable-staticlibs
	%__make %{?jobs:-j%{jobs}}
popd
pushd MediaInfo/Project/GNU/CLI
	./configure --enable-staticlibs
	%__make %{?jobs:-j%{jobs}}
popd
#pushd xmlstarlet
#	./configure
#	%__make %{?jobs:-j%{jobs}}
#popd
mkdir -p dvrescue/Source/GUI/dvrescue/build
pushd dvrescue/Source/GUI/dvrescue/build
	export USE_SYSTEM=true
	qmake-qt5 ..
	%__make %{?jobs:-j%{jobs}}
popd
%endif

%install
pushd dvrescue/Project/GNU/CLI
	%__make install DESTDIR=%{buildroot}
popd

%if 0%{?build_gui}
pushd dvrescue/Source/GUI/dvrescue/build
	%__install -D -m 755 dvrescue/dvrescue %{buildroot}%{_bindir}/dvrescue-gui
popd
%__install -D -m 755 ffmpeg/ffmpeg %{buildroot}%{_libdir}/dvrescue/bin/ffmpeg
#%__install -D -m 755 xmlstarlet/xml %{buildroot}%{_libdir}/dvrescue/bin/ffmpeg/xmlstarlet
%__install -D -m 755 dvrescue/Project/GNU/CLI/dvrescue %{buildroot}%{_libdir}/dvrescue/bin/dvrescue
%__install -D -m 755 MediaInfo/Project/GNU/CLI/mediainfo %{buildroot}%{_libdir}/dvrescue/bin/mediainfo
%__install -D -m 644 dvrescue/Source/GUI/dvrescue/dvrescue/icons/icon.png %{buildroot}%{_datadir}/pixmaps/dvrescue.png
%__install -D -m 644 dvrescue/Project/GNU/GUI/dvrescue-gui.desktop %{buildroot}/%{_datadir}/applications/dvrescue-gui.desktop
%__install -D -m 644 dvrescue/Project/GNU/GUI/dvrescue-gui.metainfo.xml %{buildroot}%{_datadir}/metainfo/dvrrescue-gui.metainfo.xml
%endif

%clean
[ -d "%{buildroot}" -a "%{buildroot}" != "" ] && %__rm -rf "%{buildroot}"

%files
%defattr(-,root,root,-)
%doc dvrescue/LICENSE.txt
%doc dvrescue/History.txt
%{_bindir}/dvrescue
%{_bindir}/dvloupe
%{_bindir}/dvmap
%{_bindir}/dvpackager
%{_bindir}/dvplay
%{_bindir}/dvsampler

%if 0%{?build_gui}
%files gui
%defattr(-,root,root,-)
%doc dvrescue/LICENSE.txt
%doc dvrescue/History.txt
%{_bindir}/dvrescue-gui
%{_libdir}/dvrescue/bin/ffmpeg
#%{_libdir}/dvrescue/bin/xmlstarlet
%{_libdir}/dvrescue/bin/dvrescue
%{_libdir}/dvrescue/bin/mediainfo
%{_datadir}/pixmaps/*.png
%{_datadir}/applications/*.desktop
%{_datadir}/metainfo/*.xml
%endif

%changelog
* Tue Jan 01 2019 Jerome Martinez <Info@MediaArea.net> - 0.21.11-0
- See History.txt for more info and real dates

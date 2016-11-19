#
# spec file for package capi4hylafax
#
# Copyright (c) 2014 SUSE LINUX Products GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#


Name:           capi4hylafax
Version:        01.03.00
Release:        342.2_vb
Summary:        Adds a faxcapi modem for hylafax
License:        GPL-2.0+
Group:          Hardware/Fax
Url:            http://www.avm.de

Source:         ftp://ftp.avm.de/tools/capi4hylafax.linux/%name-%version.tar.gz
Patch0:         capi4hylafax-suse.diff
Patch1:         capi4hylafax-secfix.diff
Patch2:         capi4hylafax-secfix2.diff
Patch3:         capi4hylafax-configure.diff
Patch4:         capi4hylafax-gcc48.diff
Patch5:         capi4hylafax-01.03.00-fix-bashisms.patch
Patch6:         capi4hylafax-constcharptr.diff
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
BuildRequires:  capi4linux
BuildRequires:  capi4linux-devel
BuildRequires:  gcc-c++
BuildRequires:  ghostscript-library
BuildRequires:  libpng-devel
BuildRequires:  libtiff-devel
BuildRequires:  pwdutils
Requires:       a2ps
Requires:       gs_lib
Requires:       hylafax
Requires:       hylafax-client
Requires:       smtp_daemon
Requires:       tiff
Requires(pre):  sh-utils fileutils /usr/sbin/useradd /usr/sbin/usermod

%description
capi4hylafax adds a faxcapi modem to the hylafax environment. It allows
you to send and receive FAX documents with CAPI 2.0 fax controllers via
a hylafax server.

%prep
%setup -q
%patch0 -p1
%patch1 -p1
%patch2 -p1
%patch3
%patch4 -p1
%patch5 -p1
%patch6 -p1
find ../ -name .cvsignore -delete

%build
%configure \
		--with-hylafax-spooldir=/var/spool/hylafax \
 		--enable-debug 
make %{?_smp_mflags}

%install
install -d "%buildroot/var/spool/hylafax"
install -d "%buildroot/usr/lib"
install -d "%buildroot/%_libdir"
install -d "%buildroot/%_sbindir"
install -d "%buildroot/%_sysconfdir"
make install BIN="%buildroot/%_bindir" \
		SPOOL="%buildroot/var/spool/hylafax" \
		LIBEXEC="%buildroot/usr/lib/fax" \
		SBIN="%buildroot/%_sbindir" \
		LIBDIR="%buildroot/%_libdir" \
		DESTDIR="%buildroot"
install -m755 faxaddmodem.capi "%buildroot/%_sbindir/"
install -m755 setupconffile "%buildroot/%_sbindir/faxaddmodem.capi_dia"
cp -p config.faxCAPI "%buildroot/%_sysconfdir/config.faxCAPI.sample"
mkdir -p "%buildroot/%_defaultdocdir/capi4hylafax"
cp -p AUTHORS LIESMICH.html README.html fritz_pic.tif COPYING Readme_src \
	LIESMICH.SuSE README.SuSE \
	ChangeLog sample_AVMC4_config.faxCAPI sample_faxrcvd \
	config.faxCAPI \
	"%buildroot/%_defaultdocdir/capi4hylafax"

%pre
/usr/sbin/useradd -r -o -g uucp -u 33 -s /bin/bash -c "Facsimile agent" -d /var/spool/hylafax fax 2> /dev/null || :
/usr/sbin/usermod -g uucp -G dialout fax 2> /dev/null || :
test -f /var/spool/hylafax/etc/config.faxCAPI -a ! -f /etc/config.faxCAPI && \
    cp -a /var/spool/hylafax/etc/config.faxCAPI /etc/config.faxCAPI || :

%post
#!/bin/sh
cd etc
FILES=`grep -l "FaxReceiveUser:" config.fax*`
test -z "$FILES" && exit
CFG=""
for f in $FILES ; do
    case $f in
	*~);;
	*orig);;
	*save);;
	*new);;
	*sample);;
	*)  FRG=`grep "FaxReceiveGroup:" $f`
	    if [ -z "$FRG" ]; then
		CFG="${CFG} $f"
	    fi
	    ;;
    esac
done
test -z "$CFG" && exit
for f in $CFG ; do
    mv ${f} ${f}.orig
    cat ${f}.orig | while read line ; do
	case $line in
	    FaxReceiveUser:*)
		echo "FaxReceiveUser:         fax" >> ${f}
		echo "FaxReceiveGroup:        dialout" >> ${f}
		;;
	    *)  echo "$line" >> ${f}
		;;
	esac
    done
done

%files
%defattr(-, root, root)
%doc %{_defaultdocdir}/capi4hylafax
%_bindir/c2faxrecv
%_bindir/c2faxsend
%_sbindir/faxaddmodem.capi
%_sbindir/faxaddmodem.capi_dia
%_sysconfdir/config.faxCAPI.sample

%changelog
* Sat Dec 27 2014 Led <ledest@gmail.com>
- fix bashism in setupconffile (faxaddmodem.capi_dia) script
- update patches:
  + capi4hylafax-suse.diff
* Mon Dec  8 2014 jengelh@inai.de
- Use source URL, use rpm macros.
* Mon Nov 17 2014 Led <ledest@gmail.com>
- Add capi4hylafax-01.03.00-fix-bashisms.patch to fix bashisms in
  setupconffile (faxaddmodem.capi_dia) script
* Sat Oct 26 2013 p.drouand@gmail.com
- Remove %%insserv_prereq %%fillup_prereq PreReq tag
  package provide neither sysconfig file or sysvinit script
* Tue Jun 25 2013 pgajdos@suse.com
- fix build with gcc 4.8
  * capi4hylafax-gcc48.diff
* Wed Dec 21 2011 jengelh@medozas.de
- Remove redundant/unwanted tags/section (cf. specfile guidelines)
* Tue Dec 20 2011 coolo@suse.com
- remove call to suse_update_config (very old work around)
* Sat Sep 17 2011 jengelh@medozas.de
- Remove redundant tags/sections from specfile
- Use %%_smp_mflags for parallel build
* Sun Sep 26 2010 ro@suse.de
- fix grep syntax in configure for new grep(1)
* Wed Aug 26 2009 mls@suse.de
- make patch0 usage consistent
* Tue Jul 31 2007 kkeil@suse.de
- first standalone source

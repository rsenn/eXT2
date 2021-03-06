#
# "$Id: Makefile 4756 2006-01-15 18:36:16Z mike $"
#
# Top-level makefile for the Fast Light Tool Kit (FLTK).
#
# Copyright 1998-2006 by Bill Spitzak and others.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.
#
# Please report all bugs and problems on the following page:
#
#      http://www.fltk.org/str.php
#

include makeinclude

DIRS	=	$(IMAGEDIRS) src

all: makeinclude
	for dir in $(DIRS); do\
		echo "=== making $$dir ===";\
		(cd $$dir; $(MAKE) $(MFLAGS)) || break;\
	done

install: makeinclude
	-mkdir -p $(DESTDIR)$(bindir)
	$(RM) $(DESTDIR)$(bindir)/fltk-config
	-cp fltk-config $(DESTDIR)$(bindir)
	-chmod 755 $(DESTDIR)$(bindir)/fltk-config
	for dir in FL $(DIRS); do\
		echo "=== installing $$dir ===";\
		(cd $$dir; $(MAKE) $(MFLAGS) install) || break;\
	done

install-desktop: makeinclude

uninstall: makeinclude
	$(RM) $(DESTDIR)$(bindir)/fltk-config
	for dir in FL $(DIRS); do\
		echo "=== uninstalling $$dir ===";\
		(cd $$dir; $(MAKE) $(MFLAGS) uninstall) || break;\
	done

uninstall-desktop: makeinclude

depend: makeinclude
	for dir in $(DIRS); do\
		echo "=== making dependencies in $$dir ===";\
		(cd $$dir; $(MAKE) $(MFLAGS) depend) || break;\
	done

clean:
	-$(RM) core *.o
	for dir in $(DIRS); do\
		echo "=== cleaning $$dir ===";\
		(cd $$dir; $(MAKE) $(MFLAGS) clean) || break;\
	done

distclean: clean
	$(RM) config.*
	$(RM) fltk-config fltk.list makeinclude
	$(RM) FL/Makefile

makeinclude: configure configh.in makeinclude.in
	if test -f config.status; then \
		./config.status --recheck; \
		./config.status; \
	else \
		./configure; \
	fi
	touch config.h
	chmod +x fltk-config

configure: configure.in
	autoconf

portable-dist:
	epm -v -s fltk.xpm fltk

native-dist:
	epm -v -f native fltk


#
# End of "$Id: Makefile 4756 2006-01-15 18:36:16Z mike $".
#

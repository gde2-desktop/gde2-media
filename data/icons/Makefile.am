NULL =

SUBDIRS = 		\
	scalable 	\
	16x16 		\
	22x22 		\
	24x24 		\
	32x32 		\
	$(NULL)

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/gde2

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

-include $(top_srcdir)/git.mk

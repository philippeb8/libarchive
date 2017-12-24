#!/bin/bash
CC=/usr/local/bin/fcldc.sh CFLAGS="-isystem libarchive/system/" ./configure -C --disable-bsdtar --disable-bsdcpio --disable-bsdcat

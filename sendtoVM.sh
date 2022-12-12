#!/bin/sh
scp -i ../../../../keys/id_rsa -r util.h util.c urlparser.h urlparser.c sws.c structures.h readdirs.c readdirs.h logger.h logger.c handler.h handler.c getuserdir.h getuserdir.c cgi.h flags.h cgi.c Makefile sid@192.168.1.175:/home/sid/latest_sws

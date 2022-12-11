#!/bin/sh
scp -i ../../../../keys/id_rsa -r Makefile cgi.c cgi.h flags.h getuserdir.c getuserdir.h handler.c handler.h readdirs.c readdirs.h structures.h sws.c util.c util.h sid@192.168.1.175:/home/sid/sws_command_after_git

#!/bin/sh
scp -i ../../../../keys/id_rsa -r Makefile cgi.c cgi.h flags.h getuserdir.c getuserdir.h handler.c handler.h readdirs.c readdirs.h structures.h sws.c util.c util.h sid@10.156.141.77:/home/sid/sws_command_after_git

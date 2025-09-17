@echo off
chcp 1251 > log
del log

set CHARSTER="-finput-charset=utf-8 -fexec-charset=utf-8"

set EXE=main.exe 
set CPP=main.cpp 
set INCLUDE="/home/varya/Desktop/ITbusko/2-curse/mario/lib/PDCurses"
set BINARY="/home/varya/Desktop/ITbusko/2-curse/mario/lib/PDCurses/x11 -libpdcurses"

if exist %EXE% del %EXE%

g++ "%CHARSET%" "%INCLUDE%" %CPP% -o %EXE% "%BINARY%"

%EXE%

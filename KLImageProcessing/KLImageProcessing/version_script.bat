@echo off
set /a y=%date:~3,1%*365
set /a m=1%date:~5,2%-100
set /a d=1%date:~8,2%-100
set /a v=y+m+d
echo #define VER_BUILD %v%>"version.h" 
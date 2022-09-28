@echo off
if not exist build_num.rec echo "0">>build_num.rec
set /P build_num=<build_num.rec
set /a build_num = %build_num%+1
echo %build_num% >build_num.rec
echo #define VER_BUILD %build_num% >Version.h
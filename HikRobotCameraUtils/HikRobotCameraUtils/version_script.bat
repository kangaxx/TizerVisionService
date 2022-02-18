@echo off
echo #define VER_BUILD %date:~3,1%%date:~5,2%%date:~8,2%.%time:~0,2%>"Version.h"
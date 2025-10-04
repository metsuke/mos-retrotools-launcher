@echo off
echo Acorn BBC build ....

rem echo dir: %cd%
rem echo param1: %1
rem pause

copy %1.agd ".\Suite BBC\scripts\"
cd "Suite BBC"
call build %~n1 r
cd ..

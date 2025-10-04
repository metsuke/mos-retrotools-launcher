@echo off
echo Timex TC2048 build ....
copy %1.agd ".\Suite TC2048\AGDsource"
cd "Suite TC2048"
call build %~n1
cd ..

@echo off
echo TC2048 export ....
copy %1.agd ".\Suite TC2048\AGDsource"
cd "Suite TC2048"
call export %~n1 %~n2
cd ..

@echo off
echo ZX Spectrum export ....
copy %1.agd ".\Suite ZX\AGDsource"
cd "Suite ZX"
call export %~n1 %~n2
cd ..

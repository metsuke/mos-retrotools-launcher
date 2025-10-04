@echo off
echo ZX Spectrum build ....
copy %1.agd ".\Suite ZX\AGDsource"
cd "Suite ZX"
call build %~n1
cd ..

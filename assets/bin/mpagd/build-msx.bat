@echo off
echo MSX build ....
copy %1.agd ".\Suite MSX\AGDsources"
cd "Suite MSX"
call build %~n1
cd ..

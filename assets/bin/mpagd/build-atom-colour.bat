@echo off
cls
echo Acorn Atom Colour build ....
echo.
copy %1.agd ".\Suite Atom\AGDsources\" >nul
cd "Suite Atom"
call build %~n1 g
cd ..

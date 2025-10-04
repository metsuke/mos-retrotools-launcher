@echo off
cls
echo VZ200 Colour build ....
echo.
copy %1.agd ".\Suite VZ200\AGDsources\"
cd "Suite VZ200"
call build %~n1 m c
cd ..

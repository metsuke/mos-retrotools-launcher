@echo off
echo VZ200 build ....
copy %1.agd ".\Suite VZ200\AGDsources\"
cd "Suite VZ200"
call build %~n1 7 i
cd ..

@echo off
echo Enterprise build ....
copy %1.agd ".\Suite EP\AGDsources\"
cd "Suite EP"
call build %~n1
cd ..

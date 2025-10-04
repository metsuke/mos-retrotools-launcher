@echo off
echo Acorn Atom build ....
copy %1.agd ".\Suite Atom\AGDsources\"
cd "Suite Atom"
call build %~n1
cd ..

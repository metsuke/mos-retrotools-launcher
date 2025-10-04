@echo off
echo CPC build ....
copy %1.agd ".\Suite CPC\AGDsource"
cd "Suite CPC"
call build %~n1
cd ..

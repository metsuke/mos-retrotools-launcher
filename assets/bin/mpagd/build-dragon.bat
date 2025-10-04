@echo off
echo Dragon build ....
copy "%1%2.agd" ".\Suite Dragon\AGDsource\"
cd "Suite Dragon"
call btd %~n1
cd ..

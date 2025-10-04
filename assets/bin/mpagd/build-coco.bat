@echo off
echo Coco build ....
copy %1.agd ".\Suite Dragon\"
cd "Suite Dragon"
call btc %~n1
cd ..

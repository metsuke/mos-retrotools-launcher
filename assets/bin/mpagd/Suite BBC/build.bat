@echo off

rem Compile AGD file
 copy scripts\%1.agd agd
 if errorlevel 1 goto error
 cd AGD
 agd %1 %2 %3 %4 %5 %6

 copy %1.inc ..\cc65\ >nul
 copy game.cfg ..\cc65\ >nul
 del %1.*
 del game.cfg

rem Assemble file
 cd ..\cc65
 call make %1 %2 %3 %4 %5 %6 %7 %8 %9

 copy %1.bin ..\Output
 del %1.bin
 cd ..

rem run emulator
 call run.bat %1
 goto end

:error
 echo %1.agd not found .....

:end

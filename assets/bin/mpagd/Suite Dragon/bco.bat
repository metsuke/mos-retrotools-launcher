@echo off
echo Going to COMPILE for PMODE3
 set flagB=0
 if "%2"=="b" @set flagB=1
 if "%3"=="b" @set flagB=1
 if "%4"=="b" @set flagB=1
 if "%5"=="b" @set flagB=1
 if "%6"=="b" @set flagB=1
 if "%7"=="b" @set flagB=1

 set flagR=0
 if "%2"=="r" @set flagR=2
 if "%3"=="r" @set flagR=2
 if "%4"=="r" @set flagR=2
 if "%5"=="r" @set flagR=2
 if "%6"=="r" @set flagR=2
 if "%7"=="r" @set flagR=2

@set /a "flags=%flagB%+%flagR%"

rem Compile AGD file
 copy AGDsource\%1.agd agd
 if errorlevel 1 goto error
 cd AGD
 AGD %1 %flags%

rem prepare compile phase
 copy %1.inc ..\ASM6809\
 copy game.cfg ..\ASM6809\
 del %1.*

rem Assemble file
 cd ..\ASM6809
 call make %1 %2 %3 %4 %5 %6 g
goto end

:error
 echo %1.agd not found .....
goto endbad

:end
echo Going to DELETE virtual files
pause
del XRoar\%1.DSK
del XRoar\%1.VDK
del ImgTool\%1.DSK
del DragDOS\%1.VDK

:endbad

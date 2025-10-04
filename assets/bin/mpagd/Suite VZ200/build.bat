@echo off
 cls

rem Compile AGD file
 copy AGDsources\%1.agd agd >nul
 if errorlevel 1 goto error
 cd AGD
 CompilerVZ200 %1 %2 %3 %4 %5
 copy %1.asm "..\sjasm\" >nul
 del %1.asm
 del %1.agd
 echo.

rem Assemble game
 echo Assembling %1.agd ....
 cd "..\sjasm"
 copy /b header.txt+%1.asm agdcode.asm >nul 
 sjasm.exe agdcode.asm agdcode.vz
 copy agdcode.vz "..\VZem" >nul
 del %1.asm
rem del agdcode.*
 echo.

rem Calculate RAM usage
rem   Code  : $7B00-$F3FF (30976)
rem   Stack : $F400-$F4FF (  256)
rem   MAP   : $F500-$F7FF (  768)
 set file=agdcode.vz
 set /a maxbytesize = 30976
 FOR /F "usebackq" %%A IN ('%file%') DO set /a size=%%~zA
 set /a psize = size - 24
 set /a free = maxbytesize - psize
 echo.Memory:
 echo   CODE : max %maxbytesize% bytes, used %psize% bytes, free %free% bytes
 echo.
 if %free% GEQ 0 (
  rem Start emulator
   echo Starting emulator ....
   cd "..\VZem"
   VZem.exe -f agdcode.vz
  rem del agdcode.vz
   echo Ready
 ) else (
  echo ERROR .... Program too big.
 )
   cd ..
 goto endbad

:error
 echo %1.agd not found .....

:endbad

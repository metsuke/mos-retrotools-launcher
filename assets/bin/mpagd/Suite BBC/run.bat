@echo off

REM Locate your BeebAsm & b-em exes here

 set BEEBASM=beebasm\beebasm.exe
 set BEM=b-em\b-em.exe

REM Use BeebAsm to make a BBC disc image

 IF NOT EXIST Output/%1.bin goto error
 echo PUTFILE "Output/%1.bin", "AGDGAME", $1200, $1200 > disc.asm
 %BEEBASM% -i disc.asm -do Discs\%1.ssd -boot AGDGAME
 del disc.asm

REM Fire up b-em with the disc image

 %BEM% Discs\%1.ssd
 goto end

:error
 
 echo Output\%1.bin not found.

:end

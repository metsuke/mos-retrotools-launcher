@echo off

rem Compile AGD file
 copy AGDsource\%1.agd agd
 cd AGD
 CompilerTmx %1 -s -y
 copy %1.asm ..\sjasmplus\
 rem del %1.*

rem Assemble game
 cd ..\sjasmplus
 sjasmplus %1.asm --lst=list.txt
 bin2tap %1.bin -a 32000 -b -c 31999 -r 32000
 copy %1.tap ..\
 del %1.tap
 del %1.asm

rem Start emulator
 cd ..\speccy
 speccy -tc2048 -autorun ..\%1.tap
 del ..\%1.tap
 cd ..

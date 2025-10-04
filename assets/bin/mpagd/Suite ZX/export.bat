@echo off

rem Compile AGD file
 copy AGDsource\%1.agd agd
 cd AGD
 CompilerZX %1 -s -y
 copy %1.asm ..\sjasmplus\
 copy ..\..\user.asm ..\sjasmplus\
 del %1.*
 del ..\..\user.asm

rem Assemble game
 cd ..\sjasmplus
 sjasmplus %1.asm --lst=list.txt
 if %2==0 bin2tap %1.bin -a 24832
 if %2==1 bin2tap %1.bin -a 24832 -b -c 24831 -r 24832
 copy %1.tap ..\..\
 del %1.tap
 del %1.asm
 cd ..

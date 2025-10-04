@echo off

rem Compile AGD file
 copy AGDsource\%1.agd agd
 cd AGD
 CompilerZX %1
 copy %1.asm ..\sjasmplus\
 copy ..\..\user.asm ..\sjasmplus\
 del %1.*

rem Assemble game
 cd ..\sjasmplus
 copy leader.txt+%1.asm+trailer.txt agdcode.asm
 sjasmplus.exe agdcode.asm --lst=list.txt
 copy test.tap ..\speccy
 del %1.asm
 del user.asm
 del agdcode.asm
 del test.tap

rem Start emulator
 cd ..\speccy
 speccy -128 test.tap
 del test.tap
 cd ..

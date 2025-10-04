@echo off
cd SCREENS
for %%A in (*.scr) do (
   if %%~zA == 6912 (
      ..\Cutter\cutter %%~nA
   ) else (
      copy %%A  %%~nA.bin
   )

   rem create binaries for CoCo-Dragon
    copy ..\_Headers\CHeader.bin /B + %%~nA.bin /B + ..\_Headers\CFooter.bin /B %%~nAINTROC.BIN /B
    copy ..\_Headers\DHeader.bin /B + %%~nA.bin /B  %%~nAINTROD.BIN /B
    del %%~nA.bin

   rem copy virtual files to Utils folders
    move ..\..\_OK\%%~nA\%%~nA.DSK ..\ImgTool
    move ..\..\_OK\%%~nA\%%~nA.VDK ..\DragDOS\

   rem copy images to DragonDos/ImgTool folders
    move %%~nAINTROC.BIN ..\ImgTool\INTRO4.BIN
    move %%~nAINTROD.BIN ..\DragDOS\INTRO4.BIN

   rem create/add to CoCo DSK
    cd ..\ImgTool
    call pack %%~nA
    del INTRO4.BIN
    move %%~nA.DSK ..\..\_OK\%%~nA\

   rem create/add to Dragon VDK
    cd ..\DragDOS
    call pack %%~nA
    del INTRO4.BIN
    move %%~nA.VDK ..\..\_OK\%%~nA\

   rem back to the images directory
    cd ..\SCREENS
    echo ENDED %%~nA
    pause
)
cd..
:end

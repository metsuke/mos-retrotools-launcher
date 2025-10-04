@echo off
cd SCREENS
for %%A in (*.scr) do (
   if %%~zA == 6912 (
      ..\Cutter\cutter %%~nA
   ) else (
      copy %%A  %%~nA.bin
   )
   copy ..\_Headers\CHeader.bin /B + %%~nA.bin /B + ..\_Headers\CFooter.bin /B %%~nAINTROC.BIN /B
   copy ..\_Headers\DHeader.bin /B + %%~nA.bin /B  %%~nAINTROD.BIN /B
   del %%~nA.bin
)
cd..

:end

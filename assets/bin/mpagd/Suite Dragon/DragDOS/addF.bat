for %%a in (*.BIN) do (
   Dragondos WRITE %1.VDK %%a
)
Dragondos DIR %1.VDK >listing.txt

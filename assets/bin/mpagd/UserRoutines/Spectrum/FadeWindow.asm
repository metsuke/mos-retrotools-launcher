
; Attribute fade effect for play area.
; Taken from original AGD code.

fade   ld hl,(dispx)       ; get current coordinates.
       push hl             ; store them.
       ld hl,(wintop)      ; get coordinates of window.
       ld (dispx),hl       ; put into dispx for calculation.
       call gaadd          ; get attribute address.
       ld b,8              ; frames over which to fade.
fade4  push bc             ; store frame counter.
       push hl             ; store attribute address.
       ld bc,(winhgt)      ; window height and width.
       ld e,32             ; width of Spectrum screen.

fade0  push bc             ; store width.
       push hl             ; store window address.

fade1  ld a,(hl)           ; fetch byte.
       and 56              ; paper.
       sub 8               ; fade the paper.
       jr nc,fade2         ; not hit black yet.
       xor a               ; restore to black.
fade2  ld d,a              ; copy to d.
       ld a,(hl)           ; restore result.
       and 7               ; only want ink now.
       jr z,fade3          ; it's already black.
       dec a               ; fade ink.
fade3  or d                ; merge in paper.
       ld (hl),a           ; write new value.
       inc hl              ; next screen address.
       djnz fade1          ; repeat for remaining columns.

       pop hl              ; retrieve address of start of last line.
       ld d,0              ; zero in d so that de is distance to next line.
       add hl,de           ; point to next line.
       pop bc              ; retrieve width.
       dec c               ; one less row.
       jr nz,fade0         ; repeat for all rows.

       call vsync          ; pause/frame synch.
       call vsync          ; pause/frame synch.
       pop hl              ; restore attribute start address.
       pop bc              ; retrieve width.
       djnz fade4          ; repeat for all frames.

       pop hl              ; retrieve old coordinates from stack.
       ld (dispx),hl       ; restore line and column.
       ret

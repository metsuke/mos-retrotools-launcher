
; Reset variables A to Z to zero.
; 14 bytes used.

       ld hl,vara          ; first variable.
       ld (hl),0           ; reset it.
       ld de,varb          ; next variable in list.
       ld bc,varz-vara     ; number of copies to make.
       ldir                ; copy to remaining variables.
       ret

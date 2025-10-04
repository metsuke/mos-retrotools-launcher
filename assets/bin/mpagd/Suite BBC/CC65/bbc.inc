;----------------------------------------------------------------------
; BBC Platform Specific functions
;----------------------------------------------------------------------

;--------------------------------------------------------
; Keys
;
; Out: joyval=x65FUDLR (bit cleared if key pressed)
;             ||||||||
;             |||||||+> Right    KEY 0  - X
;             ||||||+-> Left     KEY 1  - Z
;             |||||+--> Down     KEY 2  - .
;             ||||+---> Up       KEY 3  - ;
;             |||+----> Fire1    KEY 4  - SPC
;             ||+-----> Fire2    KEY 5  - Q
;             |+------> Fire3    KEY 6  - P
;             +-------> Not used
;
;                       Option1  KEY 7  - 1
;                       Option2  KEY 8  - 2
;                       Option3  KEY 9  - 3
;                       Option4  KEY 10 - 4
;--------------------------------------------------------

;              X   Z   .   ;  SPC  Q   P
;keys:   .byte $42,$61,$68,$48,$62,$10,$37       ; Keys defined by game designer.
;        .byte $30,$31,$11,$12                   ; menu options.

jkeys:  .byte $42,$61,$68,$48,$62,$10,$37       ; Keys defined by game designer.
        .byte $30,$31,$11,$12                   ; menu options.

;----------------------------------------------------
; Wait for keypress.
;----------------------------------------------------

prskey:
	txa
	pha
prsloop:
	jsr vsync

	lda #$79
	ldx #$01
	jsr OSBYTE

	cpx #$ff
	beq prsloop		; wait until key pressed
	txa
	jsr joykey
	pla
	tax
	rts

bbcinit:
	sei
	lda EVENTV
	sta old_eventv
	lda EVENTV+1
	sta old_eventv+1

	lda #<event_handler
	sta EVENTV
	lda #>event_handler
	sta EVENTV+1
	cli
	
	; Enable VSYNC event.
	lda #14
	ldx #4
	jmp OSBYTE

bbckill:
	sei
	lda old_eventv
	sta EVENTV
	lda old_eventv+1
	sta EVENTV+1
	cli
	
	; Disable VSYNC event.
	lda #13
	ldx #4
	jmp OSBYTE


old_eventv:
	.res 2

event_handler:
	php
	cmp #4
	bne not_vsync

	inc vsync_count

not_vsync:
	plp
	jmp (old_eventv)

bbcsync:
	lda vsync_count
	cmp #2
	bcc bbcsync
	lda #0
	sta vsync_count
	rts

;--------------------------------------------------------
; Keyboard test routine.
;
; Input:
;  A = key to read, high nibble=row and low nibble=col
;
; Output:
;  carry clr = key pressed
;  carry set = key not pressed
;--------------------------------------------------------

ktest:
	sta z80_a		; save key
	tya
	pha

; _BEEB keyboard read
	lda z80_a
	eor #$80		; _BEEB just look for this key
	tax

	lda #$79
	jsr OSBYTE

	txa
	bmi pressed 	; _BEEB X is negative if key is pressed

	sec			; key not pressed
	pla
	tay
	rts

pressed:
 lda z80_a
 sta $67ff
	clc			; key pressed
	pla
	tay
	rts

;---------------------------------------------------------------
; Getkey in column,row format
;
; Output:
;  A = high nibble=row and low nibble=column key in matrix
;---------------------------------------------------------------

kget:
	stx xtmp
kget1:
	lda #$79			; _BEEB read keyboard with OSBYTE &79
	ldx #$01
	jsr OSBYTE

	cpx #$ff
	beq kget1

	txa
	pha
kget2:
	lda #$79			; wait for released key
	ldx #$01
	jsr OSBYTE

	cpx #$ff
	bne kget2

	pla
	ldx xtmp
	rts

;----------------------------------------------------
; AtoMMC joystick controls.
; _BEEB TODO
;----------------------------------------------------

joyinit:
joysin:
	rts


;----------------------------------------------------
; Delay routine 1/50 sec
;
; Wait 1/60 sec = 16666 usec
; Wait 208 x 16 =  3328 usec
;                 19994 usec
; rts           =     6 usec
; Total         = 20000 usec
;----------------------------------------------------

delay:
	sta xtmp
del_loop:
	lda #19
	jsr OSBYTE		; wait for vsync _BEEB

	ldy #208		; wait 208 x 16 = 3328 usec
delay1:
	adc ($80,x)		;	 6 usec
	adc ($80,x)		;	 6 usec
	dey			;	 2 usec
	bne delay1		;	 2 usec
	nop			; 2 usec
	nop			; 2 usec
				; tot: 20000 usec
	dec xtmp
	bne del_loop
	rts

;-------------------------------------------------------------
; Screen synchronisation.
;
;  - read joystick/keyboard
;  - handle sound
;  - sync framerate with clock
;  - handle shrapnel every even frame
;-------------------------------------------------------------

vsync:
	pha
	tya
	pha
	txa
	pha
	jsr joykey		; read joystick/keyboard.
vsync1:
	lda #19
	jsr OSBYTE		; _BEEB vsync

	lda clock
	and #1
	bne:+
	jsr proshr		; handle shrapnel every even frame
:
	lda sndtyp
	beq sndskip
; _BEEB TODO SOUND
;sndloop:
;	lda SpeakerBit		; handle sound
;	ldy sndtyp
;sndwait:
;	dey
;	bne sndwait
;	eor #4
;	sta SpeakerBit
;	dec sndtyp
;	bne sndloop
sndskip: 
	pla
	tax
	pla
	tay
	pla
	rts

sndtyp:	.byte 0

;----------------------------------------------------------------------
; BBC video hardware fns
;----------------------------------------------------------------------

screeninit:
	ldx #13
crtcloop:
	stx $FE00
	lda crtc_regs_high,x
	sta $FE01
	dex
	bpl crtcloop

    ; Set ULA
    lda #$88            ; MODE 4
    sta $248            ; Tell the OS or it will mess with ULA settings at vsync
    sta $FE20

; fall through to palette

setpal:
	ldx #15
palloop:
	lda ula_pal,x
	sta $fe21
	dex
	bpl palloop
	rts

crtc_regs_high:
	.byte 63				; R0  horizontal total
	.byte 32				; R1  horizontal displayed
	.byte 49				; R2  horizontal position
	.byte $24				; R3  sync width 40 = &28
	.byte 38				; R4  vertical total
	.byte 0					; R5  vertical total adjust
	.byte 24				; R6  vertical displayed
	.byte 35				; R7  vertical position; 35=top of screen
	.byte $00				; R8  interlace
	.byte 7					; R9  scanlines per row
	.byte 32				; R10 cursor start
	.byte 8					; R11 cursor end
	.byte >(ScreenAddr/8)	; R12 screen start address, high
	.byte <(ScreenAddr/8)	; R13 screen start address, low

ula_pal:
	.byte $00 + PAL_black
	.byte $10 + PAL_black
	.byte $20 + PAL_black
	.byte $30 + PAL_black
	.byte $40 + PAL_black
	.byte $50 + PAL_black
	.byte $60 + PAL_black
	.byte $70 + PAL_black
	.byte $80 + PAL_white
	.byte $90 + PAL_white
	.byte $a0 + PAL_white
	.byte $b0 + PAL_white
	.byte $c0 + PAL_white
	.byte $d0 + PAL_white
	.byte $e0 + PAL_white
	.byte $f0 + PAL_white

;----------------------------------------------------
; Draw sprite
;----------------------------------------------------
.if oflag
sprite:
	stx xtmp		; Save X-reg
	jsr scadd 		; get screen address in scraddr.

	lda dispx 		; x position.
	and #7 			; position straddling cells.
	sta z80_b		; store in b register.

	lda z80_l		; store sprite graphic address.
	sta sprit1+1
	sta sprit2+1
	lda z80_h
	sta sprit1+2
	sta sprit2+2

	ldx #0			; pixel height.
	ldy #0
sprit1:
.if swrflag
	lda objdta + data_address - data_start,x		; fetch first byte.
.else
	lda objdta,x		; fetch first byte.
.endif
	sta spr
	inx
sprit2:
.if swrflag
	lda objdta + data_address - data_start,x
.else
	lda objdta,x
.endif
	sta spr+1

	lda #0
	sta spr+2
	jsr sprit7		; shift sprite

	lda spr			; fetch graphic.
	ldy #0			; _BEEB
	eor (scraddr),y		; merge with screen image.
	sta (scraddr),y		; write to screen.

	lda spr+1		; fetch graphic.
	ldy #8			; _BEEB
	eor (scraddr),y		; merge with screen image.
	sta (scraddr),y		; write to screen.

	lda spr+2		; fetch graphic.
	ldy #16			; _BEEB
	eor (scraddr),y		; merge with screen image.
	sta (scraddr),y		; write to screen.

	jsr nline

	inx			; next source byte.
	cpx #32
	bne sprit1		; repeat

	ldx xtmp		; retreive X-reg
	rts

;spr:	.byte 0,0,0
.endif
;------------------------------------------------------------------
; This routine returns a screen address for (dispx, dispy) in scraddr.
;------------------------------------------------------------------

scadd:
	ldx dispy
	lda dispx
	and #$f8
	clc
	adc SCADTB_lb,x
	sta scraddr
	lda SCADTB_hb,x
	adc #0
	sta scraddr+1
	rts

;-----------------------------------------------------------------
; These are the sprite routines.
; sspria = single sprite, old (ix).
; ssprib = single sprite, new (ix+5).
; sspric = both sprites, old (ix) and new (ix+5).
;-----------------------------------------------------------------

sspria:
	jsr gsprad		; get old sprite address.
sspri2:
.if rflag
	lda z80_e				; 3c
	sta dline_spraddr1+1	; 4c
	sta dline_spraddr2+1	; 4c
	lda z80_d				; 3c
	sta dline_spraddr1+2	; 4c
	sta dline_spraddr2+2	; 4c

	ldx sprshft
	lda shift_table,x
	sta dline_shift1+1
	sta dline_shift2+1
	lda shift_table+1,x
	sta dline_shift1+2
	sta dline_shift2+2

	ldx #0			; vertical lines.
sspri0:
	jsr dline		; draw a line.
  .if bflag
	cpx #48
  .else
	cpx #32
  .endif
	bne sspri0		; repeat 16 times x 2 bytes
	rts
.else
  .if bflag
	lda #24
  .else
	lda #16			; vertical lines.
  .endif
	sta acnt
sspri0:
	jsr dline		; draw a line.
	dec acnt
	bne sspri0		; repeat 16 times
	rts

acnt:	.byte 0
.endif

;-----------------------------------------------------------------

ssprib:
	jsr gspran 		; get new sprite address.
	jmp sspri2

;-----------------------------------------------------------------

sspric:
.if rflag
	jsr gsprad 		; get old sprite address.

	lda z80_e				; 3c
	sta ddline_spraddr3+1	; 4c
	sta ddline_spraddr4+1	; 4c

	lda z80_d				; 3c
	sta ddline_spraddr3+2	; 4c
	sta ddline_spraddr4+2	; 4c

	ldx sprshft
	lda shift_table,x
	sta ddline_shift3+1
	sta ddline_shift4+1
	lda shift_table+1,x
	sta ddline_shift3+2
	sta ddline_shift4+2

	jsr exx  		; store addresses.
	jsr gspran 		; get new sprite addresses.

	lda z80_e				; 3c
	sta ddline_spraddr1+1	; 4c
	sta ddline_spraddr2+1	; 4c

	lda z80_d				; 3c
	sta ddline_spraddr1+2	; 4c
	sta ddline_spraddr2+2	; 4c

	ldx sprshft
	lda shift_table,x
	sta ddline_shift1+1
	sta ddline_shift2+1
	lda shift_table+1,x
	sta ddline_shift1+2
	sta ddline_shift2+2

	ldx #0
lloop:
	jsr ddline 		; draw a line.
  .if bflag
	cpx #48
  .else
	cpx #32
  .endif
	bne lloop
	rts

shift_table:
.word shift0
.word shift2
.word shift4
.word shift6

;-------------------------------------------------------------
; Drop through.
; Line drawn, now work out next target address.
;
; Input:
;  B  = right mask
;  C  = left mask
;  DE = spriteaddress
;  scraddr = screen address
;-------------------------------------------------------------

dline:

; first screen byte

dline_spraddr1:
	ldy $ffff,x 		; graphic data.
dline_shift1:
	lda $ffff,y
	sta sprtmp
	and z80_c 			; mask away what's not needed.

	ldy #0
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; bung it in.

; carry to next screen byte

	lda sprtmp			; fetch data.
	and z80_b 			; mask away unwanted
	sta sprtmp

; middle screen byte

	inx
dline_spraddr2:
	ldy $ffff,x 		; second bit of data.
dline_shift2:
	lda $ffff,y
	sta sprtmp2
	and z80_c 			; mask away what's not needed.
	ora sprtmp

	ldy #8
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; bung it in.

; carry to last screen byte

	lda sprtmp2
	and z80_b 			; mask away unwanted

	ldy #16
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; bung it in.

	inx
.else
	jsr gsprad 		; get old sprite address.
	jsr exx  		; store addresses.
	jsr gspran 		; get new sprite addresses.

.if bflag
	lda #23
.else
	lda #15
.endif
	sta ccnt
lloop:
	jsr dline 		; draw a line.
	jsr exx  		; restore old addresses.
	jsr dline 		; delete a line.
	jsr exx  		; flip to new sprite addresses.
	dec ccnt
	bne lloop

	jsr dline		; draw a line.
	jsr exx 		; restore old addresses.

;-------------------------------------------------------------
; Drop through.
; Line drawn, now work out next target address.
;
; Input:
;  B  = right mask
;  C  = left mask
;  DE = spriteaddress
;  scraddr = screen address
;-------------------------------------------------------------

dline:
	ldy #0
	lda (z80_de),y 		; graphic data.
	pha
	and z80_c 		; mask away what's not needed.
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; bung it in.

; _BEEB screen arrangement
;	clc
;	lda scraddr
;	adc #16
;	sta scraddr
;	bcc :+
;	inc scraddr+1
;:
	pla
	and z80_b 		; mask away unwanted bits.
	ldy #16
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; bung it in.

	inc z80_e 		; next graphic.
	bne :+
	inc z80_d
:
; _BEEB screen arrangement
;	sec
;	lda scraddr 		; one character cell to the left.
;	sbc #8
;	sta scraddr
;	lda scraddr+1
;	sbc #0
;	sta scraddr+1

	ldy #0
	lda (z80_de),y 		; second bit of data.
	ldy #8
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; bung it in.

	inc z80_e 		; point to next line of data.
	bne :+
	inc z80_d
:
; _BEEB screen arrangement
;	sec
;	lda scraddr 		; another char left.
;	sbc #8
;	sta scraddr
;	lda scraddr+1
;	sbc #0
;	sta scraddr+1
.endif

;----------------------------------------------------------------------
; Line drawn, now work out next target address.
;----------------------------------------------------------------------

; _BEEB screen arrangement

nline:
	lda scraddr 		; get low byte of address.
	and #7				
	cmp #7				; is this last line of row?
	beq beeb_next_row

	; within same row
	inc scraddr			; new low byte of address.
;	bne :+
;	inc scraddr+1		; new high byte of address.
;:
	rts

beeb_next_row:
	clc
	lda scraddr
	adc #<(ScreenRowBytes-7)
	sta scraddr			; new low byte of address.
	lda scraddr+1
	adc #>(ScreenRowBytes-7)
	sta scraddr+1		; new high byte of address.
	rts

.if rflag
;-------------------------------------------------------------
; Drop through.
; Line drawn, now work out next target address.
;
; Input:
;  B  = right mask
;  C  = left mask
;  DE = spriteaddress
;  scraddr = screen address
;-------------------------------------------------------------

ddline:

; NEW SPRITE

; first screen byte

ddline_spraddr1:
	ldy $ffff,x 		; graphic data.
ddline_shift1:
	lda $ffff,y
	sta sprtmp
	and z80_c 			; mask away what's not needed.

	ldy #0
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; bung it in.

; carry to next screen byte

	lda sprtmp			; fetch data.
	and z80_b 			; mask away unwanted
	sta sprtmp

; middle screen byte

	inx
ddline_spraddr2:
	ldy $ffff,x 		; second bit of data.
ddline_shift2:
	lda $ffff,y
	sta sprtmp2
	and z80_c 			; mask away what's not needed.
	ora sprtmp

	ldy #8
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; bung it in.

; carry to last screen byte

	lda sprtmp2
	and z80_b 			; mask away unwanted

	ldy #16
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; bung it in.

; _BEEB next row

	lda scraddr 		; get low byte of address.
	and #7				
	cmp #7				; is this last line of row?
	beq :+

	; within same row can't increment page
	inc scraddr			; new low byte of address.
	bne ddline2

:
	clc
	lda scraddr
	adc #<(ScreenRowBytes-7)
	sta scraddr			; new low byte of address.
	lda scraddr+1
	adc #>(ScreenRowBytes-7)
	sta scraddr+1		; new high byte of address.

; OLD SPRITE

ddline2:

; still first row but different data!

	dex

; first screen byte

ddline_spraddr3:
	ldy $ffff,x			; graphic data.
ddline_shift3:
	lda $ffff,y
	sta sprtmp
	and z80_cp 			; mask away what's not needed.

	ldy #0
	eor (z80_hlp),y 	; XOR with what's there.
	sta (z80_hlp),y 	; bung it in.

; carry to next screen byte

	lda sprtmp			; fetch data.
	and z80_bp 			; mask away unwanted
	sta sprtmp

; middle screen byte

	inx
ddline_spraddr4:
	ldy $ffff,x 		; second bit of data.
ddline_shift4:
	lda $ffff,y
	sta sprtmp2
	and z80_cp 			; mask away what's not needed.
	ora sprtmp

	ldy #8
	eor (z80_hlp),y 	; XOR with what's there.
	sta (z80_hlp),y 	; bung it in.

; last screen byte

	lda sprtmp2			; fetch data.
	and z80_bp 			; mask away unwanted

	ldy #16
	eor (z80_hlp),y 	; XOR with what's there.
	sta (z80_hlp),y 	; bung it in.

; next row of sprite data

	inx

; _BEEB screen arrangement

	lda z80_lp 			; get low byte of address.
	and #7				
	cmp #7				; is this last line of row?
	beq :+

	; within same row can't increment page
	inc z80_lp			; new low byte of address.
	rts

:
	clc
	lda z80_lp
	adc #<(ScreenRowBytes-7)
	sta z80_lp			; new low byte of address.
	lda z80_lp+1
	adc #>(ScreenRowBytes-7)
	sta z80_lp+1		; new high byte of address.
	rts
.endif

;----------------------------------------------------
; Display character in A at dispx,dispy.
;
; Input:
;  A 	   = character to print
;----------------------------------------------------

pchar:
	sta fntaddr
	lda #0
	sta fntaddr+1
	asl fntaddr  		; multiply char by 8.
	rol fntaddr+1
	asl fntaddr
	rol fntaddr+1
	asl fntaddr
	rol fntaddr+1

	lda fntaddr
	clc
	adc #<(FONT-256)
	sta fntaddr		; that's the low byte.
	lda fntaddr+1
	adc #>(FONT-256)
	sta fntaddr+1		; add displacement.
pchark:
	jsr gprad		; get screen address.
	ldx #7			; lines to write.
pchar0:
	ldy #0
	lda (fntaddr),y 	; get image byte.
	ldy scrtab,x		; Get rowoffset
.if iflag
	eor #TxtInvert		; Invert
.endif
	sta (scraddr),y 	; copy to screen.
	inc fntaddr		; next image byte.
	bne :+
	inc fntaddr+1
:
	dex			; next screen row down.
	bpl pchar0		; repeat.
	rts

scrtab:		.byte $07,$06,$05,$04,$03,$02,$01,$00
;scrtab:	.byte $e0,$c0,$a0,$80,$60,$40,$20,$00

.if rflag
;----------------------------------------------------
; Shift tables
;----------------------------------------------------

.align 256
shift0:
.repeat 256, i
	.byte i
.endrep

shift2:
.repeat 256, i
	.byte (i >> 2) | (i << 6) & $c0
.endrep

shift4:
.repeat 256, i
	.byte (i >> 4) | ((i << 4) & $f0)
.endrep

shift6:
.repeat 256, i
	.byte (i >> 6) | ((i << 2) & $fc)
.endrep
.endif

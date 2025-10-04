; Game engine code --------------------------------------------------------------

; Arcade Game Designer.
; (C) 2008 Jonathan Cauldwell.
; ZX Spectrum Next Engine v0.1.

; Conditional compilation flags
; Code is installed if flag is set
; Flags are set in commandline assembly or by the compiler

; Flags set by AGD compiler
;	mflag  			; MENU + INV
;	pflag 			; particle engine
;	sflag 			; scrollytext
;	dflag 			; digging mode
;	cflag			; collectable blocks
;	oflag			; objects
;	lflag			; ladders
;
; Flags set manually
;	aflag			; adventure mode
;	bflag			; big sprites (16x24)
;	fflag			; floppy version (skip #0axx)
;	gflag			; graphic colour mode
;	hflag			; hidden sprite mode
;	iflag			; invert mode

.if mflag 
	.out "- MEN/INV enabled"  
.endif
.if pflag 
	.out "- Particles enabled" 
.endif
.if sflag 
	.out "- Scrolling enabled" 
.endif
.if dflag
	.out "- Digging enabled" 
.endif
.if cflag
	.out "- Collectable blocks enabled" 
.endif
.if oflag
	.out "- Objects enabled" 
.endif
.if lflag
	.out "- Ladders enabled" 
.endif
.if aflag
	.out "- Adventure mode enabled" 
.endif
.if bflag
	.out "- Big Sprites (16x24) enabled" 
.endif
.if gflag
	.out "- Colourmode enabled" 
.endif
.if hflag
	.out "- Hidden sprites/foregroundblocks enabled" 
.endif
.if iflag
	.out "- Invert mode enabled" 
.endif
.if rflag
	.out "- RAM saving sprites enabled" 
.endif
.if xflag
	.out "- Metablocks 16x6" 
.endif

; Global definitions ------------------------------------------------------------

.if swrflag
	FONT = font + data_address - data_start				; Font address
.else
	FONT = font 
.endif

; Block characteristics.

	PLATFM	= 1		; platform.
	WALL	= PLATFM + 1	; solid wall.
	LADDER	= WALL + 1	; ladder.
	FODDER	= LADDER + 1	; fodder block.
	DEADLY	= FODDER + 1	; deadly block.
	CUSTOM	= DEADLY + 1	; custom block.
	WATER	= CUSTOM + 1	; water block.
        COLECT	= WATER + 1	; collectable block.
        NUMTYP	= COLECT + 1	; number of types.

; Sprites.

.if bflag 
	SPR_HGT	= 24		; Sprite height
	SPR_WID = 16		; Sprite width
	NUMSPR	= 8		; number of sprites.
.else				; 16x16 sprites
	SPR_HGT = 16		; Sprite height
	SPR_WID = 16		; Sprite width
	NUMSPR	= 12		; number of sprites.
.endif
	TABSIZ = 17			; size of each entry.
	SPRBUF = NUMSPR * TABSIZ; size of entire table.
	NMESIZ = 4			; bytes stored in nmetab for each sprite.

; Sprite table variable offsets.

	var_Type = 0		; sprite type
	var_Image = 1		; sprite time number
	var_Frame = 2		; sprite frame
	var_Y = 3			; sprite y coordinate
	var_X = 4			; sprite X coordinate

	var_newType = 5		; sprite new type
	var_newImage = 6	; sprite new image number
	var_newFrame = 7	; sprite new frame
	var_newY = 8		; sprite new y coordinate
	var_newX = 9		; sprite new x coordinate
	
	var_Direction = 10	; sprite direction
	var_Param1 = 11		; sprite parameter 1
	var_Param2 = 12		; sprite parameter 2

	var_jumpLo = 13		; sprite jump ptr low
	var_jumpHi = 14		; sprite jump ptr high
	var_dataLo = 15		; sprite data ptr low
	var_dataHi = 16		; sprite data ptr high

; Particle engine.

	NUMSHR = 55			; pieces of shrapnel.
	SHRSIZ = 6			; bytes per particle.

.if iflag
	TxtInvert   = $ff	; Invert byte for character printing
	ScrFillByte = $ff	; Screen fill byte for CLS
.else
	TxtInvert   = $00	; Invert byte for character printing
	ScrFillByte = $00	; Screen fill byte for CLS
.endif

	ASCII_NEWLINE = 13

;===============================================================
; Game starts here
;===============================================================

;--------------------------------------------------------------
; If a font is required...
;--------------------------------------------------------------

	jsr game	 		; start the game.
	rts

; Don't change the order of these four.  
; Menu routine relies on winlft following wintop.

wintop:	.byte WINDOWTOP		; top of window.
winlft:	.byte WINDOWLFT		; left edge.
winhgt:	.byte WINDOWHGT		; window height.
winwid:	.byte WINDOWWID		; window width.
numob:	.byte NUMOBJ		; number of objects in game.

; Pixel versions of wintop, winlft, winhgt, winwid.

wntopx:	.byte (8 * WINDOWTOP)
wnlftx:	.byte (8 * WINDOWLFT)
wnbotx:	.byte ((WINDOWTOP * 8) + (WINDOWHGT * 8) - 16)
wnrgtx:	.byte ((WINDOWLFT * 8) + (WINDOWWID * 8) - 16)-2

; Make sure pointers are arranged in the same order as the data itself.

.if swrflag
frmptr:	.word frmlst + data_address - data_start        ; sprite frames.
.else
frmptr:	.word frmlst        ; sprite frames.
.endif

; Assorted game routines which can go in contended memory.

;--------------------------------------------------------------
; Modify for inventory.
; called by the INV command
;
; Input:
;  X   = message nr with objects seperated with ,
;
; Output:
;  OPT = selected line nr of INV menu
;--------------------------------------------------------------

.if mflag
minve:
.if xflag
	lda #WINDOWHGT
	asl a
	sta winhgt
	lda #WINDOWWID
	asl a
	sta winwid
.endif
	lda #<(invdis)		; routine address.
	sta mod0+1		; set up menu routine.
	sta mod2+1		; set up count routine.
	lda #>(invdis)
	sta mod0+2
	sta mod2+2
	lda #<(fopt)		; find option from available objects.
	sta mod1+1		; set up routine.
	lda #>(fopt)
	sta mod1+1+1
	jmp dbox		; do menu routine.

;--------------------------------------------------------------
; Modify for menu.
; called by the MENU command
;
; Input:
;  X   = message nr with menu items seperated with ,
;
; Output:
;  OPT = selected line nr of MENU menu
;--------------------------------------------------------------

mmenu:
	lda #<(always)		; routine address.
	sta mod0+1		; set up routine.
	sta mod2+1		; set up count routine.
	lda #>(always)
	sta mod0+2
	sta mod2+2

	lda #<(fstd)		; standard option selection.
	sta mod1+1		; set up routine.
	lda #>(fstd)
	sta mod1+2

; Drop through into box routine.

;--------------------------------------------------------------
; Work out size of box for message or menu.
;--------------------------------------------------------------

dbox:
.if swrflag
	lda #<(msgdat + data_address - data_start)		; pointer to messages.
	sta z80_l
	lda #>(msgdat + data_address - data_start)
	sta z80_h
.else
	lda #<msgdat		; pointer to messages.
	sta z80_l
	lda #>msgdat
	sta z80_h
.endif
	
	jsr getwrd		; get message number.

	lda z80_h		; store pointer to message.
	sta TmpAddr
	lda z80_l
	sta TmpAddr+1

	lda #1			; height.
	sta z80_d
	lda #0			; start at object zero.
	sta combyt		; store number of object in combyt.
	sta z80_e		; maximum width.
dbox5:
	lda #0			; this line"s width.
	sta z80_b
mod2:
	jsr always		; item in player"s possession?
	cmp #255
	bne dbox6		; not in inventory, skip this line.
	inc z80_d		; add to tally.
dbox6:
	ldy #0			; get character.
	lda (z80_hl),y
	sta z80_a
	inc z80_l		; next character.
	bne :+
	inc z80_h
:
	lda z80_a		; reached end of line?
	cmp #','
	beq dbox3		; yes.
	cmp #ASCII_NEWLINE
	beq dbox3		; yes.
	inc z80_b		; add to this line"s width.
	lda z80_a
	bmi dbox4		; end of message? yes, end count.
	jmp dbox6		; repeat until we find the end.
dbox3:
	lda z80_e		; maximum line width.
	cmp z80_b		; have we exceeded longest so far?
	bpl dbox5		; no, carry on looking.
	lda z80_b		; make this the widest so far.
	sta z80_e
	jmp dbox5		; keep looking.
dbox4:
	lda z80_e		; maximum line width.
	cmp z80_b		; have we exceeded longest so far?
	bpl dbox8		; no, carry on looking.
	lda z80_b		; final line is the longest so far.
	sta z80_e
dbox8:
	dec z80_d		; decrement items found.
	bne :+			; total was zero.
	lda #255
	sta varopt
	jmp dbox15		
:
	lda z80_e		; longest line.
	bne :+			; was it zero?
	jmp dbox15		; total was zero.
:
	sta bwid		; set up size.
	lda z80_d
	sta blen

;--------------------------------------------------------------
; That's set up our box size.
;--------------------------------------------------------------

	lda winhgt		; window height in characters.
	sec
	sbc z80_d		; subtract height of box.
	lsr a			; divide by 2.
	clc
	adc wintop		; add top edge of window.
	sta btop		; set up box top.

	lda winwid		; window width in characters.
	sec	
	sbc z80_e		; subtract box width.
	lsr a			; divide by 2.
	clc
	adc winlft		; add left edge of window.
	sta blft		; box left.

	lda #<(FONT-256)		; font.
	sta grbase		; set up for text display.
	lda #>(FONT-256)
	sta grbase+1

	lda TmpAddr+1		; restore message pointer.
	sta z80_l
	lda TmpAddr
	sta z80_h

	lda btop		; box top.
	sta dispy		; set display coordinate.
	lda #0			; start at object zero.
	sta combyt		; store number of object in combyt.
dbox2:
	lda combyt		; get object number.
	sta z80_a
mod0:
	jsr always		; check inventory for display.
	cmp #255
	beq :+			
	jmp dbox13		; not in inventory, skip this line.
:
	lda blft		; box left.
	sta dispx		; set left display position.
	lda bwid		; box width.
	sta z80_b		; store width.
dbox0:
	ldy #0			; get character.
	lda (z80_hl),y
	cmp #','		; end of line?
	beq dbox1		; yes, next one.
	cmp #ASCII_NEWLINE			; end of line?
	beq dbox1		; yes, next one.

	cmp #141			; end of line?
	bne :+
	dec bwid
	jmp dbox7		; yes, next one.
:
	dec z80_b		; one less to display.
	and #127		; remove terminator.

	jsr pchr		; display on screen.

	ldy #0
	lda (z80_hl),y		; get character.
	sta z80_a
	inc z80_l		; next character.
	bne :+
	inc z80_h
:
	lda z80_a
	cmp #128		; end of message?
	bmi :+
	jmp dbox7		; yes, job done.
:
	lda z80_b		; chars remaining.
	beq :+			; are any left?
	jmp dbox0		; yes, continue.
:
;---------------------------------------------------
; Reached limit of characters per line.
;---------------------------------------------------

dbox9:
	ldy #0
	lda (z80_hl),y		; get character.
	inc z80_l		; next one.
	bne :+
	inc z80_h
:
	cmp #','		; another line?
	beq dbox10		; yes, do next line.
	cmp #ASCII_NEWLINE			; another line?
	beq dbox10		; yes, do next line.
	cmp #128		; end of message?
	bcs :+
	jmp dbox11		; yes, finish message.
:
	jmp dbox9

;---------------------------------------------------
; Fill box to end of line.
;---------------------------------------------------

dboxf:
	lda #32			; space character.
	jsr pchr		; display character.
	dec z80_b
	beq :+
	jmp dboxf		; repeat for remaining chars on line.
:
	rts
dbox1:
	inc z80_l		; skip character.
	bne :+
	inc z80_h
:
	jsr dboxf		; fill box out to right side.
dbox10:
	inc dispy		; y coordinate down a line next position.
	jmp dbox2		; next line.
dbox7:
	lda z80_b		; chars remaining.
	bne :+			; are any left?
	jmp dbox11		; no, nothing to draw.
:
	jsr dboxf		; fill message to line.

;------------------------------------------------------
; Drawn the box menu, now select option.
;------------------------------------------------------

dbox11:
	lda btop		; box top.
	sta dispy		; set bar position.
dbox14:
	jsr joykey		; get controls.
	cmp #$7f		; anything pressed?
	bne dbox14		; yes, debounce it.
	jsr dbar		; draw bar.
dbox12:
	jsr joykey		; get controls.
	cmp #$7f		; anything pressed?
	beq dbox12		; no, nothing.
	and #16			; fire button pressed?
	bne :+
mod1:
	jmp fstd		; yes, job done.
:
	jsr dbar		; delete bar.

	lda joyval		; joystick reading.
	and #8			; going up?
	beq dboxu		; yes, go up.

	ldx dispy		; vertical position of bar.
	inx			; look down.
	txa
	sec
	sbc btop		; find distance from top.
	cmp blen		; top of box.
	bne :+
	jmp dbox14		; yes, go no further.
:
	inc dispy		; move bar.
	jmp dbox14		; continue.
dboxu:
	lda dispy		; vertical position of bar.
	cmp btop		; are we at the top?
	bne :+
	jmp dbox14		; yes, go no further.
:
	dec dispy		; move bar.
	jmp dbox14		; continue.
fstd:
	lda dispy		; bar position.
	sec
	sbc btop		; find selected option.
	sta varopt		; store the option.
	jmp redraw		; redraw the screen.

;------------------------------------------------------
; Option not available.  Skip this line.
;------------------------------------------------------

dbox13:
	ldy #0
	lda (z80_hl),y		; get character.
	inc z80_l		; next one.

	bne :+
	inc z80_h
:
	cmp #','		; another line?
	bne :+
	jmp dbox2		; yes, do next line.
:
	cmp #ASCII_NEWLINE			; another line?
	bne :+
	jmp dbox2		; yes, do next line.
:

	bpl :+			; end of message?
	jmp dbox11		; yes, finish message.
:
	jmp dbox13
dbox15:
.if xflag
	lda #WINDOWWID
	sta winwid
	lda #WINDOWHGT
	sta winhgt
.endif
	lda TmpAddr		; pop message pointer from the stack.
	sta z80_h
	lda TmpAddr+1
	sta z80_l
	rts

;------------------------------------------------------
; Invert bar
;------------------------------------------------------

dbar:
	lda blft		; box left.
	sta dispx		; set display coordinate.
	jsr gprad		; get printing address.

	lda bwid		; box width.
	sta z80_c		; loop counter in c.
	lda z80_h		
	sta z80_d		; store screen address high byte.
dbar1:
	ldx #7			; pixel height in b.
dbar0:
	ldy scrtab,x
	lda (scraddr),y		; get screen byte.
	eor #255		; reverse all bits.
	sta (scraddr),y		; write back to screen.
	dex			; next line down.
	bpl dbar0		; draw rest of character.

	lda scraddr
	adc #8
	sta scraddr
	bcc :+
	inc scraddr+1
:
	dec z80_c		; decrement character counter.
	bne dbar1		; repeat for whole line.
	rts

;------------------------------------------------------
; Point to object
;
; Input:
;  -
;
; Output:
;  A = object number, A=255 if already in possession
;------------------------------------------------------

invdis:
	lda z80_l		; store message text pointer.
	pha
	lda z80_h
	pha
	lda combyt		; object number.
	inc combyt		; ready for next one.
	jsr gotob		; check if we have object.
	tay
	pla
	sta z80_h
	pla
	sta z80_l
	tya
	rts

;------------------------------------------------------
; Find option selected.
;
; Input:
;  -
;
; Output:
;  OPT = selected object
;------------------------------------------------------

fopt:
	lda dispy
	sec
	sbc btop		; find selected option.
	sta tmp+2		; option selected in b register.
	inc tmp+2

	lda #0			; set to first item.
	sta combyt		; object number.
fopt0:
	jsr fobj		; find next object in inventory.
	dec tmp+2
	bne fopt0		; repeat for relevant steps down the list.

	lda combyt		; get option.
	sta varopt		; store the option.
	dec varopt		; one less, due to where we increment combyt.
	jmp redraw		; redraw the screen.
fobj:
	ldy combyt		; object number.
	inc combyt		; ready for next item.
	tya
	jsr gotob		; do we have this item?
	cmp #255
	bne :+
	rts
:
	jmp fobj		; yes, it's on the list.
.endif

;----------------------------------------------------
; Clear sprite table.
;
; sprtab[0] - sprtab[SPRBUF-1] = 255
;----------------------------------------------------

xspr:
	lda #255		; clear byte.
	ldx #0			; length of table.
xspr0:
	sta sprtab,x		; sprite table.
	inx			; move to next byte.
	cpx #SPRBUF
	bne xspr0		; repeat for rest of table.
	rts

;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; Sound, NOT IMPLEMENTED!!!
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;
;silenc:
;	jsr silen1 		; silence channel 1.
;	jsr silen2 		; silence channel 2.
;	jsr silen3 		; silence channel 3.
;	jmp plsnd 		; play all channels to switch them off.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

;-------------------------------------------------------------
; Initialise all objects.
;
; Reset current room,y,x to start room,y,x for all objects
;-------------------------------------------------------------

.if oflag
iniob:
.if swrflag
	lda #<(objdta + data_address - data_start) 		; objects table.
	sta z80_x
	lda #>(objdta + data_address - data_start)
	sta z80_i
.else
	lda #<objdta		; objects table.
	sta z80_x
	lda #>objdta
	sta z80_i
.endif

	ldx numob 		; number of objects in the game.
iniob0:
	ldy #35
	lda (z80_ix),y 		; start screen.
	ldy #32
	sta (z80_ix),y 		; set start screen.

	ldy #36
	lda (z80_ix),y 		; find start y.
	ldy #33
	sta (z80_ix),y 		; set start y.

	ldy #37
	lda (z80_ix),y 		; get initial x.
	ldy #34
	sta (z80_ix),y 		; set x coord.

	clc 			; point to next object.
	lda z80_x
	adc #38			; distance between objects.
	sta z80_x
	bcc :+
	inc z80_i
:
	dex 			; repeat.
	bne iniob0

	rts
.endif

;-----------------------------------------------
; Redraw the screen.
;
; Remove old copy of all sprites for redraw.
;-----------------------------------------------

redraw:
.if xflag
	lda #WINDOWWID
	sta winwid
	lda #WINDOWHGT
	sta winhgt
.endif
	lda z80_i 		; place sprite pointer on stack.
	pha
	lda z80_x
	pha

	jsr droom		; show screen layout.
.if oflag
	jsr shwob		; draw objects.
.endif
numsp0:
	lda #NUMSPR		; sprites to draw.
	sta tmp

	lda #<sprtab		; sprite table.
	sta z80_x
	lda #>sprtab
	sta z80_i
redrw0:
	ldy #0
	lda (z80_ix),y		; old sprite type.
	cmp #255		; is it enabled?
	beq redrw1 		; no, find next one.

	ldy #var_Y
	lda (z80_ix),y 		; sprite y.
	cmp #177		; beyond maximum?
	bcs redrw1		; yes, nothing to draw.

	jsr sspria		; show single sprite.

redrw1:
	clc			; next sprite.
	lda z80_x
	adc #TABSIZ		; distance to next odd/even entry.
	sta z80_x
	bcc :+
	inc z80_i
:
	dec tmp			; repeat for remaining sprites.
	bne redrw0

rpblc1:
;	jsr dshrp		; redraw shrapnel.

.if aflag
	jsr rbloc		; draw blocks for this screen
.endif
	pla			; retrieve sprite pointer.
	sta z80_x
	pla
	sta z80_i

	rts

;----------------------------------------------------------------------
; Clear screen routine.
;
; Fill screenmem $8000-$97ff with ScrFillByte
;----------------------------------------------------------------------

cls:
	lda #>ScreenAddr		; screen address.
	sta clsloop+2
	lda #ScrFillByte
	ldy #0
clsloop:
	sta ScreenAddr,y
	iny
	bne clsloop
	inc clsloop+2
	ldx clsloop+2
	cpx #>(ScreenAddr+ScreenSize)		; _BEEB
	bne clsloop
	rts

;----------------------------------------------------------------------
; FODDER check
;----------------------------------------------------------------------

.if pflag .or dflag
fdchk:
	cmp #FODDER 		; is it fodder?
	beq :+
	rts 			; no.
:
	lda #0			; wipe fodder in MAP
	ldy #0
	sta (bufaddr),y 	; rewrite block type.

	lda dispx		; x=x/8
	pha
	lsr a
	lsr a
	lsr a
	sta dispx

	lda dispy		; y=y/8
	pha
	lsr a
	lsr a
	lsr a
	sta dispy

	lda #0 			; block to write.
	jsr pattr 		; write block.

	pla
	sta dispy
	pla
	sta dispx
	rts
.endif

;----------------------------------------------------
; Scrolly text and puzzle variables.
;----------------------------------------------------

.if sflag
txtbit:	.byte 128		; bit to write.
txtwid:	.byte 16		; width of ticker message.
.if swrflag
txtpos:	.word (msgdat + data_address - data_start)
txtini:	.word (msgdat + data_address - data_start)
.else
txtpos:	.word msgdat
txtini:	.word msgdat
.endif
txtscr:	.word ScreenAddr
.endif

;----------------------------------------------------
; Specialist routines.
; Process shrapnel.
;----------------------------------------------------
proshr:
.if pflag
	lda #<SHRAPN		; table.
	sta z80_x
	lda #>SHRAPN
	sta z80_i

	lda #NUMSHR		; shrapnel pieces to process.
	sta shrctr
prosh0:
	ldy #0
	lda (z80_ix),y		; on/off marker.
	asl a
proshx:
	bcs :+
	jsr prosh1 		; on, so process it.
:
	clc
	lda z80_x
	adc #SHRSIZ
	sta z80_x
	bcc :+
	inc z80_i
:
	dec shrctr		; round again.
	bne prosh0
.endif
.if sflag
	jsr scrly
.endif
	rts

.if pflag
;----------------------------------------------------
; Proces shrapnel piece
;----------------------------------------------------

prosh1:
	jsr plot 		; delete the pixel.

	lda #<shrptr		; shrapnel routine pointers.
	sta z80_l
	lda #>shrptr
	sta z80_h

	ldy #0
	lda (z80_ix),y		; restore shrapnel type.
	jsr prosh2 		; run the routine.
	jsr chkxy		; check x and y are good before we redisplay.

	lda #<SHRSIZ 		; distance to next.
	sta z80_e
	lda #>SHRSIZ
	sta z80_d
	rts

;----------------------------------------------------
; Run the routine
;----------------------------------------------------

prosh2:
	asl a 			; 2 bytes per address.
	tay
	lda shrptr,y
	sta z80_l
	lda shrptr+1,y 		; fetch high byte from table.
	sta z80_h
	jmp (z80_hl) 		; jump to routine.

;----------------------------------------------------
; Paricle routine table
;----------------------------------------------------

shrptr:	.word laser		; laser.
	.word trail		; vapour trail.
	.word shrap		; shrapnel from explosion.
	.word dotl		; horizontal starfield left.
	.word dotr		; horizontal starfield right.
	.word dotu		; vertical starfield up.
	.word dotd		; vertical starfield down.
	.word ptcusr		; user particle.

;----------------------------------------------------
; Explosion shrapnel.
;----------------------------------------------------

shrap:
	ldy #1
	lda (z80_ix),y 		; get the angle.
	clc
	adc #<shrsin		; shrapnel sine table.
	sta z80_l
	lda #>shrsin
	adc #0
	sta z80_h

	ldy #0
	lda (z80_hl),y 		; fetch value from table.
	sta z80_e
	inc z80_l 		; next byte of table.
	bne :+
	inc z80_h
:
	ldy #0
	lda (z80_hl),y		; fetch value from table.
	sta z80_d
	inc z80_l		; next byte of table.
	bne :+
	inc z80_h
:
	ldy #0
	lda (z80_hl),y 		; fetch value from table.
	sta z80_c
	inc z80_l 		; next byte of table.
	bne :+
	inc z80_h
:
	ldy #0
	lda (z80_hl),y 		; fetch value from table.
	sta z80_b

	ldy #2
	lda (z80_ix),y 		; x coordinate in hl.
	clc
	adc z80_e		; add sine lb
	sta (z80_ix),y		; store new coordinate lb.
	ldy #3
	lda (z80_ix),y
	adc z80_d		; add sine hb
	sta (z80_ix),y		; store new coordinate hb.

	ldy #4
	lda (z80_ix),y	 	; y coordinate in hl.
	clc
	adc z80_c		; add cosine lb
	sta (z80_ix),y		; store new coordinate lb.
	ldy #5
	lda (z80_ix),y
	adc z80_b		; add cosine lb
	sta (z80_ix),y		; store new coordinate hb.

	rts

;----------------------------------------------------
; Move dots
;----------------------------------------------------

dotl:
	ldy #5
	lda (z80_ix),y
	sec
	sbc #1		 	; move left.
	sta (z80_ix),y
	rts
dotr:
	ldy #5
	lda (z80_ix),y
	clc
	adc #1		 	; move left.
	sta (z80_ix),y
	rts
dotu:
	ldy #3
	lda (z80_ix),y
	sec
	sbc #1		 	; move up.
	sta (z80_ix),y
	rts
dotd:
	ldy #3
	lda (z80_ix),y
	clc
	adc #1			; move down.
	sta (z80_ix),y
	rts

;----------------------------------------------------
; Check if coordinates are ok before redrawing at new position.
;
; left:   X>L		X=L	Ok
; right:  R+15>X	X=R	Ok
; top:    Y>T		Y=T	Ok
; bottom: B+15>Y	Y=B	Ok
;----------------------------------------------------

chkxy:

; top:    Y>T		Y=T	Ok

	ldy #3
	lda (z80_ix),y	 	; fetch shrapnel coordinate.
	cmp wntopx		; window top.
	bcs :+			; compare with top window limit.
	jmp kilshr		; out of window, kill shrapnel.
:
; left:   X>L		X=L	Ok

	ldy #5
	lda (z80_ix),y	 	; fetch shrapnel coordinate.
	cmp wnlftx		; left edge.
	bcs :+			; compare with left window limit.
	jmp kilshr		; out of window, kill shrapnel.
:
; bottom: B+15>Y	Y=B	Ok

	lda wnbotx		; point to bottom.
	clc
	adc #15
	ldy #3
	cmp (z80_ix),y	 	; fetch shrapnel coordinate.
	bcs :+			; compare with shrapnel x coordinate.
	jmp kilshr		; off screen, kill shrapnel..
:
; right:  R+15>X	X=R	Ok

	lda wnrgtx		; point to right edge.
	clc
	adc #15
	ldy #5
	cmp (z80_ix),y	 	; fetch shrapnel coordinate.
	bcs :+			; compare with window limit.
	jmp kilshr		; off screen, kill shrapnel.
:

;----------------------------------------------------
; Drop through.
; Display shrapnel.
;----------------------------------------------------

plot:
	ldy #3
	lda (z80_ix),y		; y integer.
	sta dispy	 	; workspace coordinates.
	ldy #5
	lda (z80_ix),y	 	; x integer.
	sta dispx 		; workspace coordinates.

	ldy #0
	lda (z80_ix),y 		; type.
	bne :+			; is it a laser?
	jmp plot1 		; yes, draw laser instead.
:
plot0:
	lda dispx		; which pixel within byte do we
	and #7			; want to set first?
	tay
	lda dots,y 		; table of small pixel positions.
	sta z80_e 		; get value.

	jsr scadd 		; screen address.
	ldy #0
	lda (scraddr),y		; see what's already there.
	eor z80_e
	sta (scraddr),y 	; put back on screen.
	rts

plot1:
	jsr scadd 		; screen address.
	ldy #0
	lda (scraddr),y 	; fetch byte there.
	eor #255 		; toggle all bits.
	sta (scraddr),y 	; new byte.
	rts

;----------------------------------------------------
; Switch off shrapnel
;----------------------------------------------------

kilshr:
	lda #128
	ldy #0
	sta (z80_ix),y	; switch off shrapnel.
	rts

;----------------------------------------------------
; Sine/cosine table
;----------------------------------------------------

shrsin:	.word 0,1024,391,946,724,724,946,391
	.word 1024,0,946,65144,724,64811,391,64589
	.word 0,64512,65144,64589,64811,64811,64589,65144
	.word 64512,0,64589,391,64811,724,65144,946

;----------------------------------------------------
; Create trail
;----------------------------------------------------

trail:
	ldy #1
	lda (z80_ix),y 	; time remaining.
	sec
	sbc #1
	sta (z80_ix),y
	bne :+
	jmp trailk		; time to switch it off.
:
	jsr qrand		; get a random number.
	lsr a 			; x or y axis?
	bcc :+
	jmp trailv		; use y.
:
; Trail horizontal

	lsr a 			; which direction?
	bcc :+
	jmp traill		; go left.
:
; Trail right

	ldy #5
	lda (z80_ix),y
	clc
	adc #1	 		; go right.
	sta (z80_ix),y
	rts

; Trail left

traill:
	ldy #5
	lda (z80_ix),y
	sec
	sbc #1 			; go left.
	sta (z80_ix),y
	rts

; Trail vertical

trailv:
	lsr a		 	; which direction?
	bcc :+
	jmp trailu		; go up.
:
; Trail down

	ldy #3
	lda (z80_ix),y
	clc
	adc #1 			; go down.
	sta (z80_ix),y
	rts

; Trail up

trailu:
	ldy #3
	lda (z80_ix),y
	sec
	sbc #1 			; go up.
	sta (z80_ix),y
	rts

; Kill trail

trailk:
	lda #200		; set off-screen to kill vapour trail.
	ldy #3
	sta (z80_ix),y
	rts

;----------------------------------------------------
; Create laser beam
;----------------------------------------------------

laser:
	ldy #1
	lda (z80_ix),y 		; direction.
	ror a 			; left or right?
	bcs :+
	jmp laserl		; move left.
:
; Laser right

	lda #8			; distance to travel.
	sta z80_b
	jmp laserm		; move laser.

; Laser left

laserl:
	lda #248		; distance to travel.
	sta z80_b
laserm:
	ldy #5
	lda (z80_ix),y		; x position.
	clc
	adc z80_b		; add distance.
	sta (z80_ix),y		; set new x coordinate.

; Test new block.

	sta dispx 		; set x for block collision detection purposes.
	ldy #3
	lda (z80_ix),y 		; get y.
	sta dispy		; set coordinate for collision test.
	jsr tstbl 		; get block type there.
	cmp #WALL		; is it solid?
	bne :+
	jmp trailk		; yes, it cannot pass.
:
.if pflag .or dflag
        cmp #FODDER             ; is it fodder?
        bne :+
        jsr fdchk               ; remove fodder block.
        jmp trailk              ; destroy laser.
:
.endif
	rts

;----------------------------------------------------
; Dots mask
;----------------------------------------------------

dots:	.byte 128,64,32,16,8,4,2,1


;----------------------------------------------------
; Plot, preserving de.
;----------------------------------------------------

plotde:
	lda z80_d 		; put de on stack.
	pha
	lda z80_e
	pha

	jsr plot 		; plot pixel.

	pla			; restore de from stack.
	sta z80_e
	pla
	sta z80_d

	rts

;----------------------------------------------------
; Shoot a laser.
;----------------------------------------------------

shoot:
	sta z80_c		; store direction in c register.
	ldy #8
	lda (z80_ix),y 		; y coordinate.
	clc
shoot1:
	adc #(SPR_HGT/2-1)	; down 7 pixels.
	sta z80_l 		; puty y coordinate in l.

	ldy #9
	lda (z80_ix),y 		; x coordinate in h.
	sta z80_h

	lda z80_i		; store pointer to sprite.
	pha
	lda z80_x
	pha

	jsr fpslot 		; find particle slot.
	bcs :+
	jmp vapou2		; failed, restore ix.
:
	lda #0
	ldy #0
	sta (z80_ix),y 		; set up a laser.

	lda z80_c
	ldy #1
	sta (z80_ix),y 		; set the direction.

	lda z80_l
	ldy #3
	sta (z80_ix),y		; set y coordinate.

	ror z80_c		; check direction we want.
	bcc :+
	jmp shootr		; shoot right.
:
	lda z80_h		; X position.
shoot0:
	and #248		; align on character boundary.
	ldy #5
	sta (z80_ix),y		; set x coordinate.
	jmp vapou0 		; draw first image.
shootr:
	lda z80_h		; x position.
	clc
	adc #15			; look right.
	jmp shoot0		; align and continue.

;----------------------------------------------------
; Create a bit of vapour trail.
;----------------------------------------------------

vapour:
	lda z80_i		; store pointer to sprite.
	pha
	lda z80_x
	pha

	ldy #8
	lda (z80_ix),y 		; y coordinate.
	clc
vapou3:
	adc #(SPR_HGT/2-1)	; mid-point of sprite.
	sta z80_l

	ldy #9
	lda (z80_ix),y 		; x coordinate.
	adc #7
	sta z80_h

	jsr fpslot 		; find particle slot.
	bcc :+
	jmp vapou1		; no, we can use it.
:
vapou2:
	pla
	sta z80_x
	pla
	sta z80_i
	rts
vapou1:
	lda z80_l
	ldy #3
	sta (z80_ix),y		; set up y.

	lda z80_h
	ldy #5
	sta (z80_ix),y 		; set up x coordinate.

	jsr qrand		; get quick random number.
	and #15			; random time.
	clc
	adc #15			; minimum time on screen.
	ldy #1
	sta (z80_ix),y		; set time on screen.

	lda #1
	ldy #0
	sta (z80_ix),y		; define particle as vapour trail.
vapou0:
	jsr chkxy		; plot first position.
	jmp vapou2

;----------------------------------------------------
; Create a user particle.
;----------------------------------------------------

ptusr:
	sta z80_f		; store timer.

	ldy #8
	lda (z80_ix),y 		; y coordinate.
	clc
	adc #7			; mid-point of sprite.
	sta z80_l

	ldy #9
	lda (z80_ix),y 		; x coordinate.
	clc
	adc #7			; mid-point of sprite.
	sta z80_h

	jsr fpslot 		; find particle slot.
	bcs ptusr1
	rts 			; out of slots, can't generate anything.
ptusr1:
	lda z80_l
	ldy #3
	sta (z80_ix),y 		; set up y.

	lda z80_h
	ldy #5
	sta (z80_ix),y		; set up x coordinate.

	lda z80_f 		; restore timer.
	ldy #1
	sta (z80_ix),y		; set time on screen.

	lda #7
	ldy #0
	sta (z80_ix),y		; define particle as user particle.

	jmp chkxy		; plot first position.

;----------------------------------------------------
; Create a vertical or horizontal star.
;----------------------------------------------------

star:
	lda z80_i		; store pointer to sprite.
	pha
	lda z80_x
	pha

	jsr fpslot 		; find particle slot.
	bcs star7		; found one we can use.
star0:
	pla 			; restore sprite pointer.
	sta z80_x
	pla
	sta z80_i
	rts 			; out of slots, can't generate anything.
star7:
	lda z80_c		; direction.
	and #3 			; is it left?
	bne :+
	jmp star1 		; yes, it's left.
:
	cmp #1 			; is it right?
	bne :+
	jmp star2 		; yes, it's right.
:
	cmp #2 			; is it up?
	bne :+
	jmp star3 		; yes, it's up.
:
	ldy wntopx 		; get edge of screen.
	iny			; down one pixel.
	tya
star8:
	ldy #3
	sta (z80_ix),y 		; set y coord.
	jsr qrand 		; get quick random number.
star9:
	ldy #5
	sta (z80_ix),y		; set x position.

	lda z80_c		; direction.
	and #3			; zero to three.
	clc
	adc #3			; 3 to 6 for starfield.
	ldy #0
	sta (z80_ix),y		; define particle as star.
	jsr chkxy		; plot first position.
	jmp star0
star1:
	jsr qrand		; get quick random number.
	ldy #3
	sta (z80_ix),y 		; set y coord.

	lda wnrgtx 		; get edge of screen.
	clc
	adc #15			; add width of sprite minus 1.
	jmp star9
star2:
	jsr qrand 		; get quick random number.
	ldy #3
	sta (z80_ix),y		; set y coord.

	lda wnlftx		; get edge of screen.
	jmp star9
star3:
	lda wnbotx 		; get edge of screen.
	clc
	adc #15 		; height of sprite minus one pixel.
	jmp star8

;----------------------------------------------------
; Find particle slot for lasers or vapour trail.
; can't use alternate accumulator.
;----------------------------------------------------

fpslot:
	lda #<SHRAPN 		; shrapnel table.
	sta z80_x
	lda #>SHRAPN
	sta z80_i

	lda #NUMSHR		; number of pieces in table.
	sta z80_b
fpslt0:
	ldy #0
	lda (z80_ix),y		; get type.
	asl a  			; is this slot in use?
	bcc :+
	rts			; no, we can use it.
:
	clc			; point to more shrapnel.
	lda z80_x
	adc #SHRSIZ
	sta z80_x
	bcc :+
	inc z80_i
:
	dec z80_b		; repeat for all shrapnel.
	bne fpslt0

	clc
	rts 			; out of slots, can't generate anything.

;----------------------------------------------------
; Create an explosion at sprite position.
;----------------------------------------------------

explod:
	sta z80_c 		; particles to create.

	lda z80_i 		; store pointer to sprite.
	pha
	lda z80_x
	pha

	ldy #8
	lda (z80_ix),y 		; y coordinate.
	sta z80_l
	ldy #9
	lda (z80_ix),y		; x coordinate.
	sta z80_h

	lda #<SHRAPN		; shrapnel table.
	sta z80_x
	lda #>SHRAPN
	sta z80_i

	lda #NUMSHR		; number of pieces in table.
	sta explcnt
expld0:
	ldy #0
	lda (z80_ix),y		; get type.
	asl a 			; is this slot in use?
	bcs expld1		; no, we can use it.
expld2:
	clc
	lda z80_x
	adc #SHRSIZ
	sta z80_x
	bcc :+
	inc z80_i
:
	dec explcnt		; repeat for all shrapnel.
	bne expld0
expld3:
	pla			; restore sprite pointer.
	sta z80_x
	pla
	sta z80_i
	rts 			; out of slots, can't generate any more.

expld1:
	lda z80_c		; shrapnel counter.
	and #15			; 0 to 15.
	clc			; add to x.
	adc z80_l
	ldy #3
	sta (z80_ix),y		; y coord.

	lda seed3 		; crap random number.
	and #15			; 0 to 15.
	clc 			; add to y.
	adc z80_h
	ldy #5
	sta (z80_ix),y		; x coord.

	lda #2
	ldy #0
	sta (z80_ix),y		; switch it on.

	jsr chkxy		; plot first position.
	jsr qrand		; quick random angle.
	and #60 		; keep within range.
	ldy #1
	sta (z80_ix),y		; angle.

	dec z80_c		; one less piece of shrapnel to generate.
	bne expld2 		; back to main explosion loop.
	jmp expld3 		; restore sprite pointer and exit.

;----------------------------------------------------
; Quick random
;----------------------------------------------------

qrand:
	jsr random		; r register.
	eor seed3		; combine with seed.
	sta seed3 		; new seed.
	rts

;----------------------------------------------------
; Display all shrapnel.
;----------------------------------------------------

dshrp:
	lda #<plotde		; display routine.
	sta proshx+1
	lda #>plotde
	sta proshx+2
	jsr proshr		; process shrapnel.

	lda #<prosh1		; processing routine.
	sta proshx+1
	lda #>prosh1
	sta proshx+2
	rts

;------------------------------------------------------
; Particle engine.
;
; Init particle data for 55 particles in SHRAPN table.
; Every particle has 6 bytes.
;
; global:	-
; local:	x,y,hl
; calls:	-
;------------------------------------------------------

inishr:
	lda #<SHRAPN 		; table.
	sta z80_l
	lda #>SHRAPN
	sta z80_h

	ldy #0
	ldx #NUMSHR		; shrapnel pieces to process.
inish0:
	lda #255 		; kill the shrapnel.
	sta (z80_hl),y

	clc 			; point there.
	lda z80_l
	adc #SHRSIZ		; distance to next.
	sta z80_l
	bcc :+
	inc z80_h
:
	dex
	bne inish0 		; round again.
	rts

;------------------------------------------------------
; Check for collision between laser and sprite.
;------------------------------------------------------

lcol:
	lda #<SHRAPN		; shrapnel table.
	sta z80_l
	lda #>SHRAPN
	sta z80_h

	lda #NUMSHR		; number of pieces in table.
	sta z80_b
lcol0:
	ldy #0
	lda (z80_hl),y 		; get type.
	beq lcol1		; yes, check collision.
lcol3:
	clc			; point to more shrapnel.
	lda z80_l
	adc #SHRSIZ
	sta z80_l
	bcc :+
	inc z80_h
:
	dec z80_b		; repeat for all shrapnel.
	bne lcol0
	clc
	rts 			; no collision, carry not set.
lcol1:
	ldy #3
	lda (z80_hl),y		; get y.
	sec
	ldy #8
	sbc (z80_ix),y		; subtract sprite y.
lcolh:
	cmp #SPR_HGT 		; within range?
	bcc :+
	jmp lcol2		; no, missed.
:
	ldy #5
	lda (z80_hl),y 		; get x.
	sec
	ldy #9
	sbc (z80_ix),y 		; subtract sprite y.
	cmp #16			; within range?
	bcs :+
	jmp lcol4 		; yes, collision occurred.
:
lcol2:
	jmp lcol3
lcol4:
	sec
	rts 			; return with carry set for collision.
.endif

;------------------------------------------------------
; Main game engine code starts here.
; After initialisation, mloop is the main loop
;------------------------------------------------------

game:

; Set up screen address table.

setsat:
	lda #<ScreenAddr		; start of screen.
	sta scraddr
	lda #>ScreenAddr
	sta scraddr+1

	ldy #0			; vertical lines on screen.
setsa0:
	lda scraddr
	sta SCADTB_lb,y		; write low byte.
	lda scraddr+1
	sta SCADTB_hb,y		; write high byte.
	jsr nline		; next line down.
	iny			; next position in table.
	bne setsa0

; Init graphics mode

	jsr screeninit

; Init AtoMMC joystick
	jsr joyinit		; AtoMMC joystick on PORT B

rpblc2:
.if pflag
	jsr inishr 		; initialise particle engine.
.endif
evintr:
	jsr evnt12 		; call intro/menu event.

	lda #WALL 		; write default property.
	ldx #0
clrmap:
	sta MAP,x 		; block properties.
	sta MAP+256,x
	sta MAP+512,x
	inx			; next byte.
	bne clrmap
.if oflag
	jsr iniob 		; initialise objects.
.endif
	lda #0			; put zero in accumulator.
	sta gamwon		; reset game won flag.

	jsr inisc 		; init the score.
mapst:
	lda stmap 		; start position on map.
	sta roomtb		; set up position in table, if there is one.

inipbl:
.if aflag
.if swrflag
	lda #<(eop + data_address - data_start)		; reset blockpointer
	sta pbptr
	lda #>(eop + data_address - data_start)
	sta pbptr+1
.else
	lda #<eop		; reset blockpointer
	sta pbptr
	lda #>eop
	sta pbptr+1
.endif
.endif
	jsr initsc 		; set up first screen.

	lda #<ssprit 		; default to spare sprite in table.
	sta z80_x
	lda #>ssprit
	sta z80_i
evini:
	jsr evnt13 		; initialisation.

; Two restarts.
; First restart - clear all sprites and initialise everything.

rstrt:
	jsr rsevt 		; restart events.
	jsr xspr 		; clear sprite table.
	jsr sprlst 		; fetch pointer to screen sprites.
	jsr ispr 		; initialise sprite table.

	jmp rstrt0

; Second restart - clear all but player, and don't initialise him.

rstrtn:
	jsr rsevt		; restart events.
	jsr nspr 		; clear all non-player sprites.
	jsr sprlst 		; fetch pointer to screen sprites.
	jsr kspr 		; initialise sprite table, no more players.

; Set up the player and/or enemy sprites.

rstrt0:
	lda #0 			; zero in accumulator.
	sta nexlev 		; reset next level flag.
	sta restfl 		; reset restart flag.
	sta deadf 		; reset dead flag.
	jsr droom 		; show screen layout.
rpblc0:
.if pflag
	jsr inishr 		; initialise particle engine.
.endif
.if aflag
	jsr rbloc		; draw blocks for this screen
.endif
.if oflag
	jsr shwob		; draw objects.
.endif

	lda #<sprtab 		; address of sprite table, even sprites.
	sta z80_x
	lda #>sprtab
	sta z80_i
	jsr dspr 		; display sprites.

	lda #<(sprtab+TABSIZ) 	; address of first odd sprite.
	sta z80_x
	lda #>(sprtab+TABSIZ)
	sta z80_i
	jsr dspr 		; display sprites.
mloop:
	jsr vsync 		; synchronise with display.

	lda #<sprtab 		; address of sprite table, even sprites.
	sta z80_x
	lda #>sprtab
	sta z80_i
	jsr dspr 		; display even sprites.

;	jsr plsnd 		; play sounds.
	jsr vsync 		; synchronise with display.

	lda #<(sprtab+TABSIZ) 	; address of first odd sprite.
	sta z80_x
	lda #>(sprtab+TABSIZ)
	sta z80_i
	jsr dspr 		; display odd sprites.

	lda #<(ssprit) 		; point to spare sprite for spawning purposes.
	sta z80_x
	lda #>(ssprit)
	sta z80_i
evlp1:
	jsr evnt10 		; called once per main loop.
	jsr pspr 		; process sprites.

; Main loop events.

	lda #<ssprit 		; point to spare sprite for spawning purposes.
	sta z80_x
	lda #>ssprit
	sta z80_i
evlp2:
	jsr evnt11 		; called once per main loop.
bsortx:
	jsr bsort 		; sort sprites.

	lda nexlev		; finished level flag.
	bne newlev		; is set, go to next level.
	lda gamwon		; finished game flag.
	bne evwon		; is set, finish the game.
	lda restfl 		; finished level flag.
	cmp #1			; has it been set?
	bne :+
	jmp rstrt		; yes, go to next level.
:
	cmp #2			; has it been set?
	bne :+
	jmp rstrtn		; yes, go to next level.
:
	lda deadf 		; dead flag.
	bne pdead		; yes, player dead.

; back to start of main loop.

	inc frmno
	inc clock
	jmp mloop		; switched to a jmp mloop during test mode.

;----------------------------------------------------------
; Read blocks from list and update screen accordingly.
;----------------------------------------------------------

.if aflag
rbloc:
.if swrflag
	lda #<(eop + data_address - data_start)		; reset blockpointer
	sta pbbuf
	lda #>(eop + data_address - data_start)
	sta pbbuf+1
.else
	lda #<eop		; reset blockpointer
	sta pbbuf
	lda #>eop
	sta pbbuf+1
.endif

rbloc2:
	lda pbbuf			; check for last block
	cmp pbptr
	bne rbloc1
	lda pbbuf+1
	cmp pbptr+1
	bne rbloc1
	rts
rbloc1:
	ldy #0
	lda (pbbuf),y		; check if block for this scno
	cmp scno
	bne rbloc0		; if not, skip
	iny
	lda (pbbuf),y		; get y
	sta dispy
	iny
	lda (pbbuf),y		; get x
	sta dispx
	iny
	lda (pbbuf),y		; get blocknr
	jsr pattr2		; draw block
rbloc0:
	clc			; point to next block
	lda pbbuf
	adc #4
	sta pbbuf
	bcc rbloc2
	inc pbbuf+1
	jmp rbloc2
.endif

;----------------------------------------------------------
; New level
;----------------------------------------------------------

newlev:
	lda scno 			; current screen.
	clc
	adc #1				; next screen.
.if swrflag
	cmp numsc + data_address - data_start			; total number of screens.
.else
	cmp numsc 			; total number of screens.
.endif
	bcs evwon			; yes, game finished.
	sta scno			; set new level number.
	jmp rstrt			; restart, clearing all aliens.

evwon:
	jsr evnt18		 	; game completed.
	jmp tidyup			; tidy up and return to BASIC/calling routine.

;----------------------------------------------------------
; Player dead.
;----------------------------------------------------------

pdead:
	lda #0				; zeroise accumulator.
	sta deadf			; reset dead flag.
evdie:
	jsr evnt16 			; death subroutine.
	lda numlif			; number of lives.
	beq :+
	jmp rstrt 			; restart game.
:
evfail:
	jsr evnt17 			; failure event.
	jmp game			; restart game

;----------------------------------------------------------
; Tidy things up
;----------------------------------------------------------

tidyup:
	ldy #0				; digits to check.
tidyu2:
	lda score,y 			; get score digit.
	cmp hiscor 			; are we larger than high score digit?
	bcc tidyu0			; high score is bigger.
	bne tidyu1			; score is greater, record new high score.
	iny				; next digit of high score.
	cpy #6
	bne tidyu2			; repeat for all digits
tidyu0:
	lda #<score			; return pointing to score.
	sta z80_c
	lda #>score
	sta z80_b
	rts
tidyu1:
	ldy #5
tidyu3:
	lda score,y			; score.
	sta hiscor,y			; high score.
	dey
	bpl tidyu3 			; copy score to high score.
evnewh:
	jsr evnt19			; new high score event.
	jmp tidyu0			; tidy up.

;--------------------------------------------------
; Restart event.
;--------------------------------------------------

rsevt:
	lda #<ssprit 			; default to spare element in table.
	sta z80_x
	lda #>ssprit
	sta z80_i
evrs:
	jmp evnt14	 		; call restart event.

;------------------------------------------------------------------
; Copy number passed in a to string position bc, right-justified.
;
; Input:
;  A  = number
;  BC = string address
;
; Output:
;  BC = string with number
;-----------------------------------------------------------------

num2ch:
	sta z80_d		; Save number

	lda #0
	sta flag
numdg3:
	ldx #100		; hundreds column.
	stx z80_e
	jsr numdg		; show digit.
numdg2:
	ldx #10			; tens column.
	stx z80_e
	jsr numdg		; show digit.

	inc flag
	ldx #1			; units column.
	stx z80_e
numdg:
	lda #48			; clear digit.
	sta z80_a
numdg1:
	lda z80_d
	cmp z80_e
	bcc numdg0		; nothing to show.
	sec
	lda z80_d
	sbc z80_e		; subtract from column.
	sta z80_d
	inc z80_a		; increment digit.
	inc flag
	jmp numdg1		; repeat until column is zero.
numdg0:
	ldy #0
	lda z80_a
	sta (z80_bc),y		; write digit to buffer.
	lda flag
	beq :+
	inc z80_c		; next buffer position.
	bne :+
	inc z80_b
:
	rts
num2dd:
	sta z80_d		; Save number

	lda #1
	sta flag

	jmp numdg2
num2td:
	sta z80_d		; Save number

	lda #1
	sta flag
	jmp numdg3

;---------------------------------------------------------
; Reset score to "000000"
;---------------------------------------------------------

inisc:
	lda #'0'
	ldx #5			; digits to initialise.
inisc0:
	sta score,x 		; write zero digit.
	dex			; next column.
	bpl inisc0		; repeat for all digits.

	rts

;-----------------------------------------------------
; Multiply h by d and return in hl.
;
; Input:
;  H = first number
;  D = second number
;
; Output:
;  HL = result H x D
;-----------------------------------------------------

imul:
	lda z80_d		; HL = H * D
	sta z80_e
	lda z80_h
	sta z80_c		; make c first multiplier.
imul0:
	lda #0			; zeroise total.
	sta z80_l
	sta z80_h

	lda z80_h
	sta z80_d		; zeroise high byte.

	lda #8			; repeat 8 times.
	sta z80_b
imul1:
	lsr z80_c		; rotate rightmost bit into carry.
	bcc imul2		; wasn't set.
	clc			; bit was set, so add de.
	lda z80_l
	adc z80_e
	sta z80_l
	lda z80_h
	adc z80_d
	sta z80_h
	clc 			; reset carry.
imul2:
	asl z80_e 		; shift de 1 bit left.
	rol z80_d
	dec z80_b
	bne imul1		; repeat 8 times.

	rts

;-----------------------------------------------
; Divide d by e and return in d, remainder in a.
;
; Input:
;  D = first number
;  E = second number
;
; Output:
;  D = result D/E
;  A = remainder
;-----------------------------------------------

idiv:
	lda #0
	ldy #8		 	; bits to shift.
	asl z80_d
idiv0:
	rol a 			; multiply d by 2.
	cmp z80_e 		; test if e is smaller.
	bcc idiv1		; e is greater, no division this time.
	sbc z80_e		; subtract it.
idiv1:
	rol z80_d		; rotate into d.
	dey
	bne idiv0		; repeat
	rts

;---------------------------------------------------
; Play AY sound effect
;---------------------------------------------------

plsnd:
	rts

;---------------------------------------------------
; Objects handling.
; 32 bytes for image
; 3 for room, y and x
; 3 for starting room, y and x.
; 254 = disabled.
; 255 = object in player"s pockets.
;---------------------------------------------------

;---------------------------------------------------
; Show items present.
;---------------------------------------------------

.if oflag
shwob:
.if swrflag
	lda #<(objdta + data_address - data_start) 			; objects table.
	sta z80_l
	lda #>(objdta + data_address - data_start)
	sta z80_h
.else
	lda #<objdta			; objects table.
	sta z80_l
	lda #>objdta
	sta z80_h
.endif
	lda numob 			; number of objects in the game.
	sta sprcnt
shwob0:
	ldy #32 			; distance to room number.
	lda (z80_hl),y 			; same as an item?
	cmp scno 			; current location.
	bne :+
	jsr dobj 			; yes, display object.
:
	clc
	lda z80_l
	adc #38 			; distance to next item.
	sta z80_l
	lda z80_h
	adc #0
	sta z80_h	 		; point to it.
	dec sprcnt
	bne shwob0 			; repeat for others.
	rts

;---------------------------------------------------
; Display object.
; hl must point to object's start address.
;
; Input:
;  HL = object address
;---------------------------------------------------

dobj:
	ldy #33
	lda (z80_hl),y 			; point to y.
	sta dispy
	iny
	lda (z80_hl),y 			; point to x.
	sta dispx
dobj1:
	jmp sprite 			; draw this sprite.

;--------------------------------------
; Remove an object.
;
; Input:
;  A = object number
;--------------------------------------

remob:
	cmp numob			; number of objects in game.
	bcc :+				; are we checking past the end?
	rts				; yes, can't get non-existent item.
:
	pha				; remember object.
	jsr getob			; pick it up if we haven't already got it.
	pla				; retrieve object number.
	jsr gotob			; get its address.
	lda #254
	ldy #32
	sta (z80_hl),y			; remove it.
	rts

;---------------------------------------------------
; Pick up object number held in the accumulator.
;
; Input:
;  A = object number
;---------------------------------------------------

getob:
	cmp numob 		; number of objects in game.
	bcc :+			; are we checking past the end?
	rts			; yes, can't get non-existent item.
:
	jsr gotob 		; check if we already have it.
	cmp #255
	bne :+
	rts			; we already do.
:
	ldy #32
	lda (z80_hl),y		; is it on this screen?
	cmp scno 		; current screen.
	bne getob0		; not on screen, so nothing to delete.

	lda #255
	sta (z80_hl),y		; pick it up.
	iny 			; point to y coord.
getob1:
	ldy #33
	lda (z80_hl),y		; y coord.
	sta dispy
	ldy #34
	lda (z80_hl),y 		; x coord.
	sta dispx
	jmp dobj1 		; delete object sprite.
getob0:
	lda #255
	sta (z80_hl),y 		; pick it up.
	rts
.endif

;-----------------------------------------------------------------
; Got object check.
; Call with object in accumulator, returns zero set if in pockets.
;
; Input:
;  A = object number
;-----------------------------------------------------------------
.if oflag .or mflag
gotob:
	cmp numob 		; number of objects in game.
	bcc :+ 			; are we checking past the end?
	jmp gotob0 		; yes, we can't have a non-existent object.
:
	jsr findob		; find the object.
gotob1:
	rts

gotob0:
	lda #254 		; missing.
	jmp gotob1

findob:
	pha			; save object number
.if swrflag
	lda #<(objdta + data_address - data_start) 		; objects.
	sta z80_l
	lda #>(objdta + data_address - data_start)
	sta z80_h
.else
	lda #<objdta 		; objects.
	sta z80_l
	lda #>objdta
	sta z80_h
.endif
	pla			; retreive object number
	beq fndob1 		; is it zero? yes, skip loop.
	tax 			; loop counter
fndob2:
	clc
	lda z80_l
	adc #38 		; size of each object.
	sta z80_l
	bcc :+
	inc z80_h
:
	dex 			; repeat until we find address.
	bne fndob2
fndob1:
	ldy #32			; distance to room it's in.
	lda (z80_hl),y		; fetch status.
	rts
.endif

;---------------------------------------------
; Drop object number at (dispx, dispy).
;
; Input:
;  A = object number
;---------------------------------------------

.if oflag
drpob:
	cmp numob 		; are we checking past the end?
	bcc :+
	rts			; yes, can't drop non-existent item.
:
	jsr gotob		; make sure object is in inventory.
	cmp scno		; already on this screen?
	bne :+
	rts			; yes, nothing to do.
:
	ldy #32
	lda scno
	sta (z80_hl),y		; bring onto screen.
	lda dispy		; sprite y coordinate.
	iny 
	sta (z80_hl),y		; point to object y.
	lda dispx 		; sprite x coordinate.
	iny
	sta (z80_hl),y 		; point to object x
	jmp dobj		; draw the object sprite.

;-----------------------------------------------
; Seek objects at sprite position.
;
; Output:
;  A = object number, if not found A=255
;-----------------------------------------------

skobj:
.if swrflag
	lda #<(objdta + data_address - data_start) 		; pointer to objects.
	sta z80_l
	lda #>(objdta + data_address - data_start)
	sta z80_h
.else
	lda #<objdta 		; pointer to objects.
	sta z80_l
	lda #>objdta
	sta z80_h
.endif

	lda numob 		; number of objects in game.
	sta z80_b 		; set up the loop counter.
skobj0:
	lda scno		; current room number.
	ldy #32
	cmp (z80_hl),y		; is object in here?
	bne :+
	jsr skobj1		; yes, check coordinates.
:
	clc			; point to next object in table.
	lda z80_l
	adc #38			; size of each object.
	sta z80_l
	bcc :+
	inc z80_h
:
	dec z80_b
	bne skobj0		; repeat for all objects.

	lda #255		; end of list and nothing found, return 255.
	rts

skobj1:
	ldy #33			; point to y coordinate.
	lda (z80_hl),y		; point to y coordinate.
	sec
	ldy #var_newY
	sbc (z80_ix),y 		; subtract sprite y.
	clc
	adc #15			; add sprite height minus one.
	cmp #31			; within range?
	bcs skobj2		; no, ignore object.

	ldy #34			; point to x coordinate now.
	lda (z80_hl),y 		; get coordinate.
	sec
	ldy #var_newX
	sbc (z80_ix),y 		; subtract the sprite x.
	clc			; add sprite width minus one.
	adc #15
	cmp #31			; within range?
	bcs skobj2		; no, ignore object.

	pla			; remove return address from stack.
	pla

	lda numob 		; objects in game.
	sec
	sbc z80_b		; subtract loop counter.
skobj2:
	rts			; accumulator now points to object.
.endif

;---------------------------------------------------------------------
; Spawn a new sprite.
;---------------------------------------------------------------------

spawn:
	lda #<sprtab		; sprite table.
	sta z80_l
	lda #>sprtab
	sta z80_h
numsp1:
	lda #NUMSPR		; number of sprites.
	sta spcnt
spaw0:
	ldy #var_Type
	lda (z80_hl),y		; get sprite type.
	cmp #255		; is it an unused slot?
	beq spaw1 		; yes, we can use this one.

	clc 			; point to next sprite in table.
	lda z80_l
	adc #TABSIZ		; size of each entry.
	sta z80_l
	bcc :+
	inc z80_h
:
	dec spcnt		; one less iteration.
	bne spaw0		; keep going until we find a slot.

; Didn't find one but drop through and set up a dummy sprite instead.

spaw1:
	lda z80_i		; address of original sprite.
	pha
	lda z80_x
	pha

	lda z80_l		; store spawned sprite address.
	sta spptr
	lda z80_h
	sta spptr+1

	lda z80_c
	ldy #var_Type
	sta (z80_hl),y 		; set the type.
	ldy #var_newType
	sta (z80_hl),y		; copy

	lda z80_b
	ldy #var_Image
	sta (z80_hl),y		; set the image.
	ldy #var_newImage
	sta (z80_hl),y		; copy

	lda #0 				; frame zero.
	ldy #var_Frame
	sta (z80_hl),y		; set frame.
	ldy #var_newFrame
	sta (z80_hl),y		; copy

	ldy #8
	lda (z80_ix),y 		; x coordinate.
	ldy #3
	sta (z80_hl),y		; set sprite coordinate.
	ldy #8
	sta (z80_hl),y		; copy

	ldy #9
	lda (z80_ix),y 		; y coordinate.
	ldy #4
	sta (z80_hl),y		; set sprite coordinate.
	ldy #9
	sta (z80_hl),y		; copy

	ldy #10				; direction of original.
	lda (z80_ix),y
	sta (z80_hl),y		; direction

	lda #0
	ldy #var_jumpLo
	sta (z80_hl),y		; reset parameter.
	iny
	sta (z80_hl),y		; reset parameter.
	iny
	sta (z80_hl),y		; reset parameter.
	iny
	sta (z80_hl),y		; reset parameter.
rtssp:
	lda spptr			; address of new sprite.
	sta z80_x
	lda spptr+1
	sta z80_i
evis1:
	jsr evnt09 			; call sprite initialisation event.

	lda spptr 			; address of new sprite.
	sta z80_x
	lda spptr+1
	sta z80_i

	; _BEEB clipping code copied from CPC Engine - MISSING?!
	ldy #var_Y
	lda (z80_hl), y		; old x coord
	cmp #SpriteMaxY     ; beyond maximum?
	bcs :+				; yes, don't draw it.

	jsr sspria 			; display the new sprite.
:
	pla					; address of original sprite.
	sta z80_x
	pla
	sta z80_i

	rts

score:	.byte "000000"		; player"s score.
hiscor:	.byte "000000"		; high score.
bonus:	.byte "000000"		; bonus.
grbase:	.word ScreenAddr	; graphics base address.

;----------------------------------------------------
; Check y-pos
;----------------------------------------------------

checkx:
	lda dispy		; y position.
	cmp #24			; off screen?
	bcs :+
	rts			; no, it's okay.
:
	pla			; remove return address from stack.
	sta z80_l
	pla
	sta z80_h
	rts

;-----------------------------------------------
; Displays the current high score.
;-----------------------------------------------

dhisc:
	lda #<hiscor 		; high score text.
	sta dscor3+1
	sta bscor3+1
	lda #>hiscor
	sta dscor3+2
	sta bscor3+2
	jmp dscor1		; check in printable range then show 6 digits.

;------------------------------------------------------
; Displays the current score.
;------------------------------------------------------

dscor:
	lda #<score		; score text.
	sta dscor3+1
	sta bscor3+1
	lda #>score
	sta dscor3+2
	sta bscor3+2
dscor1:
	sty tmpscr
	jsr preprt		; set up font and print position.
	jsr checkx		; make sure we're in a printable range.

;	lda #6			; digits to display.
;	sta z80_b
	lda prtmod		; get print mode.
	beq :+			; standard size text?
	jmp bscor0		; no, show double-height.
:
dscor0:
;	ldy #0
;	lda (z80_hl),y 		; fetch character.
	ldy tmpscr
dscor3:
	lda score,y
	jsr pchar 		; display character.
	inc dispx		; move along x coordinate

;	inc z80_l		; next score column.
;	bne :+
;	inc z80_h
;:
	inc tmpscr
	dec z80_b
	bne dscor0 		; repeat for all digits.
dscor2:
	lda dispx 		; set up display coordinates.
	sta charx
	lda dispy
	sta chary
	rts

tmpscr:	.byte 0
;------------------------------------------------------
; Displays the current score in double-height characters.
;
; Input:
;  B  = digit number
;  HL = score string
;------------------------------------------------------

bscor0:
;	ldy #0

;	lda (z80_hl),y 		; fetch character.
	ldy tmp
bscor3:
	lda score,y
	jsr bchar 		; display big char.

;	inc z80_l 		; next score column.
;	bne :+
;	inc z80_h
;:
	inc tmp
	dec z80_b
	beq :+
	jmp bscor0 		; repeat for all digits.
:
	jmp dscor2 		; tidy up line and column variables.

;-----------------------------------------------------
; Adds number in the hl pair to the score.
;-----------------------------------------------------

addsc:
	lda #<(score+1) 	; ten thousands column.
	sta z80_e
	lda #>(score+1)
	sta z80_d
	lda #<10000		; amount to add each time.
	sta z80_c
	lda #>10000
	sta z80_b
	jsr incsc		; add to score.

	inc z80_e		; thousands column.
	bne :+
	inc z80_d
:
	lda #<1000		; amount to add each time.
	sta z80_c
	lda #>1000
	sta z80_b
	jsr incsc 		; add to score.

	inc z80_e		; hundreds column.
	bne :+
	inc z80_d
:
	lda #<100		; amount to add each time.
	sta z80_c
	lda #>100
	sta z80_b
	jsr incsc		; add to score.

	inc z80_e 		; tens column.
	bne :+
	inc z80_d
:
	lda #<10		; amount to add each time.
	sta z80_c
	lda #>10
	sta z80_b
	jsr incsc 		; add to score.

	inc z80_e		; units column.
	bne :+
	inc z80_d
:
	lda #<1			; units.
	sta z80_c
	lda #>1
	sta z80_b
incsc:
	lda z80_h		; store amount to add.
	pha
	lda z80_l
	pha

	sec			; subtract from amount to add.
	lda z80_l
	sbc z80_c
	sta z80_l
	lda z80_h
	sbc z80_b
	sta z80_h
	bcc incsc0		; too much, restore value.

	pla			; delete the previous amount from the stack.
	pla

	lda z80_d 		; store column position.
	pha
	lda z80_e
	pha
	jsr incsc2		; do the increment.

	pla			; restore column.
	sta z80_e
	pla
	sta z80_d
	jmp incsc		; repeat until all added.

incsc0:
	pla			; restore previous value.
	sta z80_l
	pla
	sta z80_h
	rts
incsc2:
	ldy #0
	lda (z80_de),y 		; get amount.
	clc
	adc #1			; add one to column.
	sta (z80_de),y		; write new column total.
	cmp #'9'+1		; gone beyond range of digits?
	bcs :+
	rts			; no, carry on.
:
	lda #'0'		; make it zero.
	sta (z80_de),y		; write new column total.
	dec z80_e		; back one column.
	bne :+
	dec z80_d
:
	jmp incsc2

;------------------------------------
; Add bonus to score and reset bonus
;------------------------------------

addbo:
	ldx #5			; last digit.
	clc
addbo0:
	lda score,x		; get score.
	adc bonus,x		; add bonus.
	sec			; 0 to 18.
	sbc #48
	pha
	lda #'0'
	sta bonus,x
	pla
	cmp #58
	bcc addbo1
	sec
	sbc #10
addbo1:
	sta score,x		; zeroise bonus.
	dex			; next digit.
	bpl addbo0		; repeat for all 6 digits.
	rts

;------------------------------------
; Swap score and bonus.
;------------------------------------

swpsb:
	ldx #5			; digits to add.
swpsb0:
	lda score,x 		; get score digits.
	pha			; save digit
	lda bonus,x 		; get bonus digits.
	sta score,x		; switch score-bonus
	pla
	sta bonus,x
	dex 			; repeat for all 6 digits.
	bpl swpsb0
	rts

;----------------------------------------------------
; Get print address.
;----------------------------------------------------

gprad:
	tya
	pha

	lda dispx 		; x coordinate.
	sta scraddr
	lda #0
	sta scraddr+1
	asl scraddr  	; multiply char by 8
	rol scraddr+1
	asl scraddr
	rol scraddr+1
	asl scraddr
	rol scraddr+1

	lda dispy		; y coordinate.
	asl a
	asl a
	asl a			; multiply char by 8
	tay

	clc
	lda scraddr
	adc SCADTB_lb,y
	sta scraddr
	lda scraddr+1
	adc SCADTB_hb,y
	sta scraddr+1

	lda prtmod		; Double line if bigtxt
	beq noskip
	lda dispx
	and #$e0
	beq noskip
	inc scraddr+1
noskip:

	pla
	tay
	rts
;--------------------------------------------------------------
; Get property buffer address of char at (dispx, dispy) in hl.
; 
; Output:
;  buffaddr = bufaddr = MAP + dispy*32 + dispx
;--------------------------------------------------------------

pradd:
	lda dispy 		; y coordinate.
	sta bufaddr
	lda #0
	sta bufaddr+1
	asl bufaddr  		; multiply char by 32
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	clc			; add address of MAP graphics.
	lda bufaddr
	adc dispx
	adc #<MAP
	sta bufaddr
	lda bufaddr+1
	adc #>MAP
	sta bufaddr+1
	rts

;.if kflag
;gaadd:
;	lda dispy 		; y coordinate.
;	sta bufaddr
;	lda #0
;	sta bufaddr+1
;	asl bufaddr  		; multiply char by 32
;	rol bufaddr+1
;	asl bufaddr
;	rol bufaddr+1
;	asl bufaddr
;	rol bufaddr+1
;	asl bufaddr
;	rol bufaddr+1
;	asl bufaddr
;	rol bufaddr+1
;	clc			; add address of MAP graphics.
;	lda bufaddr
;	adc dispx
;	adc #<att_table
;	sta bufaddr
;	lda bufaddr+1
;	adc #>att_table
;	sta bufaddr+1
;	rts
;.endif

;----------------------------------------------
; Print attributes, properties and pixels.
;
; Input:
;  A	= tile number
;----------------------------------------------

pbpattr:			; entry PUTBLOCK command
	ldy charx
	sty dispx
	ldy chary
	sty dispy
pattr:
.if aflag
	pha
	jsr wbloc		; save blockinfo
	pla
.endif

pattr2:
	sta z80_b		; store cell in b register for now.
	tax
.if swrflag
	lda bprop + data_address - data_start,x 		; block properties.
.else
	lda bprop,x 		; block properties.
.endif
	sta z80_c
	cmp #COLECT
	bne :+
	lda z80_b
	sta colpatt
:
	jsr pradd 		; get property buffer address.
	lda z80_c
	ldy #0
	sta (bufaddr),y 	; write property.
	lda z80_b 		; restore cell.
	jmp panp
; Print attributes, no properties.

;colpatt:	.byte 0

panp:
	sta z80_e		; displacement in e.
	lda #0
	sta z80_d		; no high byte.
	asl z80_e  		; multiply char by 8.
	rol z80_d
	asl z80_e
	rol z80_d
	asl z80_e
	rol z80_d
	clc
	lda z80_e
.if swrflag
	adc #<(chgfx + data_address - data_start) 		; address of graphics.
	sta tileaddr
	lda z80_d
	adc #>(chgfx + data_address - data_start)
.else
	adc #<chgfx 		; address of graphics.
	sta tileaddr
	lda z80_d
	adc #>chgfx
.endif
	sta tileaddr+1
	jsr gprad 		; get screen address.
	ldx #7			; number of pixel rows to write.
panp0:
	ldy #0
	lda (tileaddr),y 	; get image byte.
.if iflag
	eor #$ff		; Invert
.endif
	ldy scrtab,x
	sta (scraddr),y 	; copy to screen.
	inc tileaddr 		; next image byte.
	bne :+
	inc tileaddr+1
:
	dex	 		; repeat for 8 pixel rows.
	bpl panp0
	inc dispx 		; move along one.
	inc charx
	rts

;----------------------------------------------
; Write block
;----------------------------------------------

.if aflag
wbloc:
	ldy #3
	sta (pbptr),y		; store block number
	dey
	lda dispx
	sta (pbptr),y		; write x position of block.
	dey
	lda dispy
	sta (pbptr),y		; write y position of block.
	dey
	lda scno
	sta (pbptr),y		; write screen.
	clc			; point to next free location
	lda pbptr
	adc #4
	sta pbptr
	bcc :+
	inc pbptr+1
:
	rts
.endif

;.if kflag
;panp:
;.endif

;----------------------------------------------
; Print character pixels, no more.
;
; Input:
;  A	= character to print
;----------------------------------------------

pchr:
	jsr pchar 		; show character in accumulator.
	inc dispx		; move along one.
	rts

;----------------------------------------------------
; Shifter sprite routine for objects.
;----------------------------------------------------

.if oflag
sprit7:
	lda z80_b
	beq sprit0
	sta z80_a
sprit3:
	lsr spr			; shift into position.
	ror spr+1
	ror spr+2
	dec z80_a		; one less iteration.
	bne sprit3
sprit0:
	rts 			; now apply to screen.
.endif

;-----------------------------------------------------------
; Get room address.
;-----------------------------------------------------------

groom:
	ldx scno 		; screen number.
	ldy #0
groomx:
.if swrflag
	lda #<(scdat + data_address - data_start) 		; pointer to screens.
	sta z80_l
	lda #>(scdat + data_address - data_start)
	sta z80_h
.else
	lda #<scdat 		; pointer to screens.
	sta z80_l
	lda #>scdat
	sta z80_h
.endif
groom1:
	cpx #0			; is it the first one?
	beq groom0 		; no more screens to skip.

	clc
	lda z80_l
.if swrflag
	adc scdat + data_address - data_start,y 		; low byte of screen size.
	sta z80_l
	iny			; point to high byte.
	lda z80_h
	adc scdat+ data_address - data_start,y 		; high byte of screen size.
	sta z80_h
.else
	adc scdat,y 		; low byte of screen size.
	sta z80_l
	iny			; point to high byte.
	lda z80_h
	adc scdat,y 		; high byte of screen size.
	sta z80_h
.endif
	iny			; next address.

	dex 			; one less iteration.
	jmp groom1 		; loop until we reach the end.
groom0:
.if swrflag
	lda numsc + data_address - data_start		; add displacement.
.else
	lda numsc		; add displacement.
.endif
	asl a
	clc			; add double displacement to address.
	adc z80_l
	sta z80_l
	lda z80_h
	adc #0
	sta z80_h
	rts

;-----------------------------------------------------------
; Draw present room.
;-----------------------------------------------------------

droom:
	lda wintop 		; window top.
	sta dispy		; set cursor y position.
droom2:
	jsr groom 		; get address of current room.
	lda #0	 		; zero in accumulator.
	sta comcnt 		; reset compression counter.
	lda winhgt 		; height of window.
	sta rrow		; set row counter
droom0:
	lda winlft 		; window left edge.
	sta dispx 		; set cursor x position.
	lda winwid 		; width of window.
	sta rcol		; set column counter
droom1:
	jsr flbyt 		; decompress next byte on the fly.
.if xflag
	jsr drwmeta
.else
	jsr pattr2 		; show attributes and block.
.endif
	dec rcol		; one less column.
	bne droom1 		; repeat for entire line.
	inc dispy		; move down one line.
.if xflag
	inc dispy		; move down one line.
.endif
	dec rrow 		; one less row.
	bne droom0 		; repeat for all rows.
	rts

;----------------------------------------------
; Decompress bytes on-the-fly.
;----------------------------------------------

flbyt:
	lda comcnt 		; compression counter.
	bne flbyt1		; any more to decompress?  yes.

	ldy #0
	lda (z80_hl),y 		; fetch next byte.
	inc z80_l 		; point to next cell.
	bne :+
	inc z80_h
:
	cmp #255 		; is this byte a control code?
	beq :+
	rts 			; no, this byte is uncompressed.
:
	lda (z80_hl),y 		; fetch byte type.
	sta combyt 		; set up the type.
	inc z80_l 		; point to quantity.
	bne :+
	inc z80_h
:
	lda (z80_hl),y 		; get quantity.
	inc z80_l 		; point to next byte.
	bne :+
	inc z80_h
:
flbyt1:
	sta comcnt 		; store new quantity.
	dec comcnt		; one less.
	lda combyt 		; byte to expand.
	rts

; ------------------------------------------------------------------------------------------------------------------------------------------
; Drawing a MetaBlock (4 tiles 8x8 => 16x16)
; param in regA tells the block number to use, if 0 use 0,0,0,0  else use N,N+2 || N+1,N+3
; ------------------------------------------------------------------------------------------------------------------------------------------
.if xflag
drwmeta:
	sta tmpblk	
	jsr pattr2

	inc dispy
	dec dispx
	ldx tmpblk
	beq :+
	inx
:
	txa
	jsr pattr2

	ldx tmpblk
	beq :+
	inx
	inx
	inx
:
	txa
	jsr pattr2

	dec dispy
	dec dispx
	ldx tmpblk
	beq :+
	inx
	inx
:
	txa
	jsr pattr2
	rts

tmpblk:	.byte 0

.endif

;------------------------------------------
; Ladder down check.
;
; Input:
;  IX = sprite pointer
;------------------------------------------

.if lflag
laddd:
	ldy #9
	lda (z80_ix),y		; x coordinate.
	sta dispx

	ldy #8
	lda (z80_ix),y		; y coordinate.
	and #254		; make it even.
	sta (z80_ix),y 		; reset it.
	clc 			; look down 16 pixels.
numsp5:
	adc #SPR_HGT
	sta dispy		; coords in dispx,dispy.
	jmp laddv

;------------------------------------------
; Ladder up check.
;
; Input:
;  IX = sprite pointer
;
; Output:
;  A  = 0 is ok, A <>0 is not ok
;------------------------------------------

laddu:
	ldy #9
	lda (z80_ix),y		; x coordinate.
	sta dispx

	ldy #8
	lda (z80_ix),y		; y coordinate.
	and #254 		; make it even.
	sta (z80_ix),y		; reset it.
	clc 			; look 2 pixels above feet.
numsp6:
	adc #SPR_HGT-2
	sta dispy		; coords in dispx,dispy.
laddv:
	jsr tstbl 		; get map address.
	jsr ldchk 		; standard ladder check.
	beq :+
	rts 			; no way through.
:
	inc bufaddr 		; look right one cell.
	bne :+
	inc bufaddr+1
:
	jsr ldchk 		; do the check.
	beq :+
	rts 			; impassable.
:
	lda dispx 		; y coordinate.
	and #7 			; position straddling block cells.
	bne :+
	rts 			; no more checks needed.
:
	inc bufaddr 		; look to third cell.
	bne :+
	inc bufaddr+1
:
	jsr ldchk 		; do the check.
	rts  			; return with zero flag set accordingly.
.endif

;---------------------------------------------------------
; Can go up check.
;
; Input:
;  IX = sprite pointer
;
; Output:
;  A  = 0 is ok, A <>0 is not ok
;---------------------------------------------------------

cangu:
	ldy #9
	lda (z80_ix),y		; x coordinate.
	sta dispx
	ldy #8
	lda (z80_ix),y 		; y coordinate.
	sec
	sbc #2
	sta dispy		; coords in dispx,dispy.
	jsr tstbl 		; get map address.
	jsr lrchk 		; standard left/right check.
	beq :+
	rts			; no way through.
:
	inc bufaddr		; look right one cell.
	bne :+
	inc bufaddr+1
:
	jsr lrchk 		; do the check.
	beq :+
	rts			; impassable.
:
	lda dispx		; x coordinate.
	and #7			; position straddling block cells.
	bne :+
	rts			; no more checks needed.
:
	inc bufaddr		; look to third cell.
	bne :+
	inc bufaddr+1
:
	jsr lrchk		; do the check.
	rts 			; return with zero flag set accordingly.

;---------------------------------------------------------
; Can go down check.
;
; Input:
;  IX = sprite pointer
;
; Output:
;  A  = 0 is ok, A <>0 is not ok
;---------------------------------------------------------

cangd:
	ldy #9
	lda (z80_ix),y 		; x coordinate.
	sta dispx
	ldy #8
	lda (z80_ix),y		; y coordinate.
	clc
numsp3:
	adc #SPR_HGT 		; look down 16 pixels.
	sta dispy		; coords in dispx,dispy.
	jsr tstbl 		; get map address.
	jsr plchk 		; block, platform check.
	beq :+
	rts			; no way through.
:
	inc bufaddr		; look right one cell.
	bne :+
	inc bufaddr+1
:
	jsr plchk		; block, platform check.
	beq :+
	rts			; impassable.
:
	lda dispx		; x coordinate.
	and #7			; position straddling block cells.
	bne :+
	rts			; no more checks needed.
:
	inc bufaddr		; look to third cell.
	bne :+
	inc bufaddr+1
:
	jsr plchk		; block, platform check.
	rts			; return with zero flag set accordingly.

;---------------------------------------------------------
; Can go left check.
;
; Input:
;  IX = sprite pointer
;
; Output:
;  A  = 0 is ok, A <>0 is not ok
;---------------------------------------------------------

cangl:
	ldy #8
	lda (z80_ix),y 		; y coordinate.
	sta dispy
	ldy #9
	lda (z80_ix),y 		; x coordinate.
	sec
	sbc #2			; look left 2 pixels.
	sta dispx		; coords in dispx,dispy.
	jmp cangh		; test if we can go there.

;---------------------------------------------------------
; Can go right check.
;
; Input:
;  IX = sprite pointer
;
; Output:
;  A  = 0 is ok, A <>0 is not ok
;---------------------------------------------------------

cangr:
	ldy #8
	lda (z80_ix),y		; y coordinate.
	sta dispy
	ldy #9
	lda (z80_ix),y		; x coordinate.
	clc
	adc #16			; look right 16 pixels.
	sta dispx		; coords in dispx,dispy.
cangh:
cangh2:
	lda #(SPR_HGT/8+1)	; default rows to write.
	sta z80_b
	lda dispy		; y position.
	and #7			; does x straddle cells?
	bne cangh0		; yes, loop counter is good.
	dec z80_b		; one less row to write.
cangh0:
	jsr tstbl		; get map address.
cangh1:
	jsr lrchk		; standard left/right check.
	beq :+
	rts			; no way through.
:
	pha
	clc
	lda bufaddr
	adc #32			; look down.
	sta bufaddr
	bcc :+
	inc bufaddr+1
:
	pla

	dec z80_b
	bne cangh1
	rts

;-------------------------------------
; Check left/right movement is okay.
;
; Input:
;  bufaddr = MAP + x/8 + y/8*32
;
; Output:
;  A  = 0 is ok, A <>0 is not ok
;-------------------------------------

lrchk:
	ldy #0
	lda (bufaddr),y		; fetch map cell.
	cmp #WALL 		; is it passable?
	beq lrchkx		; no.

	cmp #FODDER		; fodder has to be dug.
	beq lrchkx		; not passable.
	lda #0
	rts

;--------------------------------------------------------------
; Called by mmenu
;--------------------------------------------------------------

always:
	lda #255		; report it as okay.
	rts

lrchkx:
	lda #1 			; reset all bits.
	rts


;--------------------------------------------------------------
; Check platform or solid item is not in way.
;
; Input:
;  bufaddr = MAP + x/8 + y/8*32
;
; Output:
;  A  = 0 is ok, A <>0 is not ok
;--------------------------------------------------------------

plchk:
	ldy #0
	lda (bufaddr),y 	; fetch map cell.
	cmp #WALL 		; is it passable?
	beq lrchkx		; no.
.if pflag .or dflag
	cmp #FODDER		; fodder has to be dug.
	beq lrchkx		; not passable.
.endif
	cmp #PLATFM		; platform is solid.
	beq plchkx		; not passable.
.if lflag
	cmp #LADDER		; is it a ladder?
	beq lrchkx		; on ladder, deny movement.
.endif
plchk0:
	lda #0			; report as ok
	rts
plchkx:
	lda dispy		; x coordinate.
	and #7			; position straddling blocks.
	beq lrchkx		; on platform, deny movement.
	jmp plchk0

;--------------------------------------------------------------
; Check ladder is available.
;
; Input:
;  bufaddr = MAP + x/8 + y/8*32
;
; Output:
;  A  = 0 is ok, A <>0 is not ok
;--------------------------------------------------------------

.if lflag
ldchk:
	ldy #0
	lda (bufaddr),y 	; fetch cell.
	cmp #LADDER 		; is it a ladder?
	beq ldchk1
	lda #1
	rts  			; return with zero flag set accordingly.
ldchk1:
	lda #0
	rts
.endif

;--------------------------------------------------------------
; Get collectables.
;--------------------------------------------------------------

.if cflag
getcol:
        lda #COLECT             ; collectable blocks.
        sta z80_b
        jsr tded                ; test for collectable blocks.
        cmp z80_b               ; did we find one?
        beq :+
        rts                     ; none were found, job done.
:
        jsr gtblk               ; get block.
        jsr evnt20              ; collected block event.
        jmp getcol              ; repeat until none left.

; Get collectable block.

gtblk:
	ldy #0
	lda (bufaddr),y
	sta z80_a
        lda #0
        sta (bufaddr),y		; make it empty now.
       
	lda bufaddr		; set dispx
	and #31
	sta dispx

	lda bufaddr+1		; Set dispy
	sec
	sbc #>MAP
	sta bufaddr+1
	asl bufaddr
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	lda bufaddr+1
	sta dispy
 
	lda colpatt		; get blocknr
	sta z80_e		; displacement in e.
	lda #0
	sta z80_d		; no high byte.
	asl z80_e  		; multiply char by 8.
	rol z80_d
	asl z80_e
	rol z80_d
	asl z80_e
	rol z80_d
	clc
	lda z80_e
.if swrflag
	adc #<(chgfx + data_address - data_start) 		; address of graphics.
	sta tileaddr
	lda z80_d
	adc #>(chgfx + data_address - data_start)
.else
	adc #<chgfx 		; address of graphics.
	sta tileaddr
	lda z80_d
	adc #>chgfx
.endif
	sta tileaddr+1
	jsr gprad 		; get screen address.
	ldx #7			; number of pixel rows to write.
gtblk0:
	ldy #0
	lda (tileaddr),y 	; get image byte.
	ldy scrtab,x
	eor (scraddr),y 	; XOR tile on screen
	sta (scraddr),y 	; copy to screen.
	inc tileaddr 		; next image byte.
	bne :+
	inc tileaddr+1
:
	dex	 		; repeat for 8 pixel rows.
	bpl gtblk0
	rts
.endif

;--------------------------------------------------------------
; Touched deadly block check.
; returns with DEADLY (must be non-zero) in accumulator if true.
;
; Input:
;  IX = sprite address
;
; Output:
;  A  = 0 is ok, A=5 is not ok
;--------------------------------------------------------------

tded:
	ldy #8
	lda (z80_ix),y 		; y coordinate.
	sta dispy
	iny
	lda (z80_ix),y 		; x coordinate.
	sta dispx		; coords in dispx,dispy.
	jsr tstbl		; get map address.
	pha
	lda #31			; default distance to next line down.
	sta z80_e
	pla
	cmp z80_b		; is this the required block?
	bne :+
	rts			; yes.
:
	inc bufaddr 		; next cell.
	bne :+
	inc bufaddr+1
:
	ldy #0
	lda (bufaddr),y		; fetch type.
	cmp z80_b 		; is this deadly/custom?
	bne :+
	rts			; yes.
:
	lda dispx		; horizontal position.
	sta z80_c 		; store column in c register.
	and #7			; is it straddling cells?
	bne :+
	jmp tded0		; no.
:
	inc bufaddr 		; last cell.
	bne :+
	inc bufaddr+1
:
	ldy #0
	lda (bufaddr),y 	; fetch type.
	cmp z80_b		; is this the block?
	bne :+
	rts			; yes.
:
	dec z80_e		; one less cell to next row down.
tded0:
	clc 			; point to next row.
	lda bufaddr
	adc z80_e
	sta bufaddr
	bcc :+
	inc bufaddr+1
:
	ldy #0
	lda (bufaddr),y		; fetch left cell block.
	cmp z80_b		; is this fatal?
	bne :+
	rts			; yes.
:
	inc bufaddr 		; next cell.
	bne :+
	inc bufaddr+1
:
	ldy #0
	lda (bufaddr),y 	; fetch type.
	cmp z80_b		; is this fatal?
	bne :+
	rts			; yes.
:
	lda z80_c		; horizontal position.
	and #7			; is it straddling cells?
	bne :+
	jmp tded1 		; no.
:
	inc bufaddr		; last cell.
	bne :+
	inc bufaddr+1
:
	ldy #0
	lda (bufaddr),y		; fetch type.
	cmp z80_b		; is this fatal?
	bne :+
	rts			; yes.
:
tded1:
	lda dispy		; vertical position.
	and #7 			; is it straddling cells?
	bne :+
	rts			; no, job done.
:
	clc			; point to next row.
	lda bufaddr
	adc z80_e
	sta bufaddr
	bcc :+
	inc bufaddr+1
:
	ldy #0
	lda (bufaddr),y 	; fetch left cell block.
	cmp z80_b		; is this fatal?
	bne :+
	rts			; yes.
:
	inc bufaddr		; next cell.
	bne :+
	inc bufaddr+1
:
	ldy #0
	lda (bufaddr),y 	; fetch type.
	cmp z80_b		; is this fatal?
	bne :+
	rts			; yes.
:
	lda z80_c		; horizontal position.
	and #7			; is it straddling cells?
	bne :+
	rts			; no.
:
	inc bufaddr		; last cell.
	bne :+
	inc bufaddr+1
:
	ldy #0
	lda (bufaddr),y		; fetch final type.
	rts 			; return with final type in accumulator.

;---------------------------------------------------
; Fetch block type at (dispx, dispy).
;
; Output:
;  A = block type
;---------------------------------------------------

tstbl:
	lda dispy 		; fetch y coord.
	lsr a			; bufaddr = y/8
	lsr a
	lsr a
;	sta chary

	sta bufaddr
	lda #0
	sta bufaddr+1

	asl bufaddr  		; bufaddr = y/8 * 32
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1

	lda dispx		; x/8
	lsr a
	lsr a
	lsr a
;	sta charx

	clc			; bufaddr = MAP + x/8 + y/8*32
	adc bufaddr
	adc #<MAP
	sta bufaddr
	lda bufaddr+1
	adc #>MAP
	sta bufaddr+1

	ldy #0
	lda (bufaddr),y 	; fetch byte there.
	rts

;-------------------------------------------------------------------
; Jump - if we can.
; Requires initial speed to be set up in accumulator prior to call.
;
; Input:
;  IX = sprite address
;-------------------------------------------------------------------

jump:
	ldy #var_jumpLo
	lda (z80_ix),y		; jump table low.
	ldy #var_jumpHi
	ora (z80_ix),y		; jump table high.
	beq :+
	rts			; already in the air.
:
.if swrflag
	lda #>(jtab + data_address - data_start)
	ldy #var_jumpHi
	sta (z80_ix),y		; set jump high.
	lda #<(jtab + data_address - data_start)		; jump table start.
.else
	lda #>jtab
	ldy #var_jumpHi
	sta (z80_ix),y		; set jump high.
	lda #<jtab		; jump table start.
.endif
	ldy #var_jumpLo
	sta (z80_ix),y		; set jump low.
	rts

; Jump table.
; _BEEB - jump table must reside entirely in a page otherwise grav fn breaks
;.align 32
;jtab:
;	.byte 248,250,252
;	.byte 254,254,255
;	.byte 255,255,0,0
;	.byte 0,1,1,1,2,2
;	.byte 4,6,8,8,8,99

;------------------------------------------------
; Random numbers code.
; Pseudo-random number generator, 8-bit.
;
; Output:
;  RND = random number
;------------------------------------------------

random:
	lda seed
	beq doEor
	asl a
	beq noEor
	bcc noEor
doEor:	eor #$1d
noEor:	sta seed
	rts

	lda seed		; get last random number.
	asl a
	asl a
	clc
	adc seed
	clc
	adc #$45
	sta seed		; store new seed.
	sta varrnd		; return number in variable.
	rts

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
;        .byte $30,$31,$11,$12                   ; menu options.;
;
;jkeys:  .byte $36,$37,$68,$48,$62,$10,$37       ; Keys defined by game designer.
;        .byte $30,$31,$11,$12                   ; menu options.

;-------------------------------------------------------
; Joystick and keyboard reading routines.
;
; contrl = 0, Keyboard
;          1, JoyKeyb
;          2, JoyMMC
;-------------------------------------------------------

joykey:
	lda contrl 		; control flag.
	cmp #1
	bne :+
	jmp joyjoy 		; read keyboard joystick
:
	cmp #2
	bne :+
	jmp joysin 		; read MMC joystick.
:
; Keyboard controls.

	lda #0		 	; zero reading.
	sta z80_e

	ldy #6	 		; address of last key.
joyke0:
.if swrflag
	lda keys + data_address - data_start,y 		; get key from table.
.else
	lda keys,y 		; get key from table.
.endif
	jsr ktest		; being pressed?
	rol z80_e 		; rotate into reading.

	dey		 	; next key.
	bpl joyke0 		; repeat for all keys.
	jmp joyjo1 		; store the value.

; Keyboard joystick controls.

joyjoy:
	lda #0		 	; zero reading.
	sta z80_e

	ldy #6	 		; address of last key.
joyjo3:
	lda jkeys,y 		; get key from table.
	jsr ktest		; being pressed?
	rol z80_e 		; rotate into reading.

	dey		 	; next key.
	bpl joyjo3 		; repeat for all keys.
joyjo1:
	lda z80_e 		; copy e register to accumulator.
joyjo2:
	sta joyval		; remember value.
	rts

;---------------------------------------------------------------
; Display message.
;
; Input:
;  A = message number
;---------------------------------------------------------------

dmsg:
	tax
.if swrflag
	lda #<(msgdat + data_address - data_start)		; pointer to messages.
	sta z80_l
	lda #>(msgdat + data_address - data_start)
	sta z80_h
.else
	lda #<msgdat		; pointer to messages.
	sta z80_l
	lda #>msgdat
	sta z80_h
.endif
	jsr getwrd		; get message number.
dmsg3:
	jsr preprt		; pre-printing stuff.
	jsr checkx		; make sure we"re in a printable range.
	lda prtmod		; print mode.
	bne bmsg1		; no, double-height text.
dmsg0:
	lda z80_h		; store string pointer.
	pha
	lda z80_l
	pha

	ldy #0
	lda (z80_hl),y		; fetch byte to display.
	and #127		; remove any end marker.
	cmp #ASCII_NEWLINE
	beq dmsg1
	jsr pchar		; display character.
	jsr nexpos 		; display position.
	bne dmsg2		; not on a new line.
	jsr nexlin		; next line down.
dmsg2:
	pla			; retrieve string pointer
	sta z80_l
	pla
	sta z80_h

	ldy #0
	lda (z80_hl),y		; fetch last character.
	asl a  			; was it the end?
	bcc :+
	jmp dscor2		; yes, job done.
:
	inc z80_l		; next character to display.
	bne :+
	inc z80_h
:
	jmp dmsg0
dmsg1:
	inc dispy
	lda dispy
	cmp #24
	bcc dmsg4
	lda #0
	sta dispy
dmsg4:
	lda #0
	sta dispx
	jmp dmsg2


;----------------------------------------------------------
; Display message in big text.
;
; Input:
;  HL = string pointer
;----------------------------------------------------------

bmsg1:
	ldy #0
	lda (z80_hl),y 		; get character to display.
	and #127		; only want 7 bits.
	cmp #ASCII_NEWLINE
	beq bmsg2
	jsr bchar 		; display big char.
bmsg3:
	ldy #0
	lda (z80_hl),y 		; look at last character.
	pha
	inc z80_l 		; next character in list.
	bne :+
	inc z80_h
:
	pla
	asl a  			; was terminator flag set?
	bcc bmsg1		; no, keep going.

	rts
bmsg2:
	lda #0
	sta dispx
	inc dispy
	inc dispy
	lda dispy
	cmp #23
	bcc bmsg3
	lda #0
	sta dispy
	jmp bmsg3

;----------------------------------------------------------
; Big character display.
;
; Input:
;  A = character
;----------------------------------------------------------

bchar:
	sta z80_e		; save char in lb
	lda #0
	sta z80_d		; reset hb

	asl z80_e 		; multiply char by 8.
	rol z80_d
	asl z80_e
	rol z80_d
	asl z80_e
	rol z80_d		; de = a*8

	clc			; de = #<(FONT-256) + a*8
	lda z80_e
	adc #<(FONT-256) 		; address of font.
	sta z80_e
	lda z80_d
	adc #>(FONT-256)
	sta z80_d

	jsr gprad 		; get screen address.

	ldx #0			; height of character in font.
bchar0:
	ldy #0
	lda (z80_de),y 		; get a bit of the font.

.if iflag
	eor #$ff
.endif

	sta (scraddr),y
	pha
	jsr nline 		; next line down.
	pla
	sta (scraddr),y
	jsr nline 		; next line down.

	clc
	inc z80_e 		; next line of font.
	bne :+
	inc z80_d
:
	inx
	cpx #8
	bne bchar0
	
	jsr nexpos		; display position.
	bne bchar2 		; not on a new line.
bchar3:
	inc dispy
	jsr nexlin 		; next line check.
bchar2:
	jmp dscor2		; tidy up line and column variables.


;-------------------------------------------------
; Display a character.
;
; Input:
;  A = character
;-------------------------------------------------

achar:
	sta z80_b 		; copy to b.
	jsr preprt 		; get ready to print.
	lda z80_b		; character in accumulator.
	ldx prtmod 		; print mode.
	beq :+
	jmp bchar 		; no, double-height text.
:
	jsr pchar 		; display character.
	jsr nexpos 		; display position.
	beq bchar3		; next line down.
	jmp bchar2 		; tidy up.


;-------------------------------------------------
; Get next print column position.
;-------------------------------------------------

nexpos:
	inc dispx		; move along one position.
	lda dispx 		; get coordinate.
;	and #31
	rts 			; return with status in zero flag.

;-------------------------------------------------
; Get next print line position.
;-------------------------------------------------

nexlin:
	inc dispy 		; newline.
	lda dispy		; vertical position.
	cmp #24			; past screen edge?
	bcs :+
	rts			; no, still okay.
:
	lda #0			; restart at top.
	sta dispy
	rts

;--------------------------------------------------------
; Pre-print preliminaries.
;--------------------------------------------------------

preprt:
	lda #<(FONT-256)		; font pointer.
	sta grbase		; set up graphics base.
	lda #>(FONT-256)
	sta grbase+1
prescr:
	lda charx 		; display coordinates.
	sta dispx		; set up general coordinates.
	lda chary
	sta dispy
	rts

;--------------------------------------------------------------
; Get messagenr x in hl
;
; Input:
;  HL = pointer to message list
;  X  = message number.
;--------------------------------------------------------------

getwrd:
	cpx #0
	bne:+ 			; first word in list?
	rts 			; yep, don't search.
:
	ldy #0
getwd0:
	lda (z80_hl),y
	pha
	inc z80_l
	bne :+
	inc z80_h
:
	pla
	cmp #128		; found end?
	bmi getwd0		; no, carry on.
	dex			; until we have right number.
	bne getwd0
	rts

;-----------------------------------------------------------
; Bubble sort.
;-----------------------------------------------------------

bsort:
	lda #NUMSPR - 1		; sprites to swap.
	sta qscnt

	lda #<sprtab 		; sprite table.
	sta z80_x
	lda #>sprtab
	sta z80_i
bsort0:
	ldy #0
	lda (z80_ix),y 		; first sprite type.
	cmp #255 		; is it switched off?
	beq swemp		; yes, may need to switch another in here.

	ldy #TABSIZ
	lda (z80_ix),y 		; check next slot exists.
	cmp #255 		; is it enabled?
	beq bsort2 		; no, nothing to swap.

	ldy #TABSIZ+3
	lda (z80_ix),y 		; fetch next sprite's coordinate.
	ldy #3
	cmp (z80_ix),y 		; compare with this x coordinate.
	bcc bsort1		; next sprite is higher - may need to switch.
bsort2:
	clc
	lda z80_x
	adc #TABSIZ 		; distance to next odd/even entry.
	sta z80_x
	bcc :+
	inc z80_i
:
	dec qscnt 
	bne bsort0		; repeat for remaining sprites.
	rts

bsort1:
	ldy #TABSIZ
	lda (z80_ix),y		; sprite on/off flag.
	cmp #255		; is it enabled?
	beq bsort2		; no, nothing to swap.
	jsr swspr		; swap positions.
	jmp bsort2
swemp:
	ldy #TABSIZ
	lda (z80_ix),y		; next table entry.
	cmp #255		; is that one on?
	beq bsort2		; no, nothing to swap.
	jsr swspr		; swap positions.
	jmp bsort2

; Swap sprites.

swspr:
	lda z80_x		; table address 
	sta z80_e		; copy to de pair.
	sta z80_l		; copy to hl pair.
	lda z80_i
	sta z80_h
	sta z80_d

	clc
	lda z80_l
	adc #TABSIZ		; distance to second entry.
	sta z80_l
	bcc :+
	inc z80_h
:
	lda #TABSIZ		; bytes to swap.
	sta z80_b
	ldy #0
swspr0:
	lda (z80_hl),y		; fetch second byte.
	pha
	lda (z80_de),y 		; fetch first byte.
	sta (z80_hl),y 		; copy to second.
	pla
	sta (z80_de),y 		; copy to first sprite entry.

	inc z80_e 		; next byte.
	bne :+	
	inc z80_d
:
	inc z80_l 		; next byte.
	bne :+
	inc z80_h
:
	dec z80_b
	bne swspr0 		; swap all bytes in table entry.
	rts

;----------------------------------------------------
; Process sprites.
;----------------------------------------------------

pspr:
	lda #NUMSPR		; sprites to process.
	sta sprptr

	lda #<sprtab 		; sprite table.
	sta z80_x
	lda #>sprtab
	sta z80_i
pspr1:
	ldy #0
	lda (z80_ix),y		; fetch sprite type.
	cmp #9 			; within range of sprite types?
	bcs :+
	jsr pspr2 		; yes, process this one.
:
	clc
	lda z80_x
	adc #TABSIZ 		; distance to next odd/even entry.
	sta z80_x
	bcc :+
	inc z80_i		; next sprite.
:
	dec sprptr 		; repeat for remaining sprites.
	bne pspr1
	rts

pspr2:
	lda z80_x 		; store original sprite pointer.
	sta ogptr
	lda z80_i
	sta ogptr+1
	jsr pspr3		; do the routine.
rtorg:
	lda ogptr 		; restore original pointer to sprite.
	sta z80_x
	lda ogptr+1
	sta z80_i
rtorg0:
	rts

pspr3:
	lda #<evtyp0		; sprite type events list.
	sta z80_l
	lda #>evtyp0
	sta z80_h
pspr4:
	lda (z80_ix),y
	asl a			; double accumulator.
	clc
	adc z80_l
	sta z80_l
	bcc :+
	inc z80_h
:
	lda (z80_hl),y
	sta z80_e 		; copy to de.
	pha

	inc z80_l 		; next byte of address.
	bne :+
	inc z80_h
:
	lda (z80_hl),y 		; address high.
	sta z80_d

	pha	 		; swap address into hl.
	lda z80_h
	sta z80_d
	pla
	sta z80_h
	pla
	sta z80_l
	lda z80_l
	sta z80_e
	
	jmp (z80_hl) 		; go there.

; Address of each sprite type's routine.

evtyp0:	.word evnt00
evtyp1:	.word evnt01
evtyp2:	.word evnt02
evtyp3:	.word evnt03
evtyp4:	.word evnt04
evtyp5:	.word evnt05
evtyp6:	.word evnt06
evtyp7:	.word evnt07
evtyp8:	.word evnt08

;--------------------------------------------------------------
; Display sprites.
;
; Input:
;  IX = sprite table
;--------------------------------------------------------------

dspr:
	lda #(NUMSPR/2)		; number of sprites to display.
	sta sprcnt

dspr0:
	ldy #var_Type
	lda (z80_ix),y 		; get sprite type.
	cmp #255 			; is it enabled?
	bne dspr1 			; yes, it needs deleting.
dspr5:
	ldy #var_newType
	lda (z80_ix),y 		; new type.
	cmp #255			; is it enabled?
	beq dspr2
	jmp dspr3 			; yes, it needs drawing.
dspr2:
	ldy #var_newType
	lda (z80_ix),y 		; copy new type.
	ldy #var_Type
	sta (z80_ix),y
	ldy #var_newImage
	lda (z80_ix),y 		; copy new image number.
	ldy #var_Image
	sta (z80_ix),y
	ldy #var_newFrame
	lda (z80_ix),y 		; copy new frame.
	ldy #var_Frame
	sta (z80_ix),y
	ldy #var_newY
	lda (z80_ix),y 		; copy new y.
	ldy #var_Y
	sta (z80_ix),y
	ldy #var_newX
	lda (z80_ix),y 		; copy new x.
	ldy #var_X
	sta (z80_ix),y

	clc
	lda z80_x
	adc #(TABSIZ*2)		; distance to next odd/even entry.
	sta z80_x
	lda z80_i
	adc #0
	sta z80_i 			; next sprite.
	dec sprcnt
	bne dspr0			; repeat for remaining sprites.
	rts

dspr1:
	; _BEEB clipping code copied from CPC Engine
	ldy #var_Y
	lda (z80_ix), y		; old x coord
	cmp #SpriteMaxY     ; beyond maximum?
	bcs dspr5			; yes, don't delete it.

	ldy #var_newType
	lda (z80_ix),y 		; type of new sprite.
	cmp #255			; is this enabled?
	bne dspr4 			; yes, display both.

dspr6:
	jsr sspria 			; show single sprite.
	jmp dspr2

; Displaying two sprites.  Don't bother redrawing if nothing has changed.

dspr4:
	; _BEEB clipping code copied from CPC Engine
	ldy #var_newY
	lda (z80_ix), y		; old x coord
	cmp #SpriteMaxY     ; beyond maximum?
	bcs dspr6			; yes, don't display it.

	ldy #var_X
	lda (z80_ix),y		; old x.
	ldy #var_newX
	cmp (z80_ix),y 		; compare with new value.
	bne dspr7 			; they differ, need to redraw.

	ldy #var_Y
	lda (z80_ix),y		; old y.
	ldy #var_newY
	cmp (z80_ix),y 		; compare against new value.
	bne dspr7			; they differ, need to redraw.

	ldy #var_Frame
	lda (z80_ix),y 		; old frame.
	ldy #var_newFrame
	cmp (z80_ix),y 		; compare against new value.
	bne dspr7 			; they differ, need to redraw.

	ldy #var_Image
	lda (z80_ix),y 		; old image.
	ldy #var_newImage
	cmp (z80_ix),y 		; compare against new value.
	bne dspr7 			; they differ, need to redraw.
	jmp dspr2			; everything is the same, don't redraw.
dspr7:
	jsr sspric 			; delete old sprite, draw new one simultaneously.
	jmp dspr2
dspr3:
	; _BEEB clipping code copied from CPC Engine
	ldy #var_newY
	lda (z80_ix), y		; old x coord
	cmp #SpriteMaxY     ; beyond maximum?
	bcc :+				; no, continue
	jmp dspr2			; yes, don't display it.
:
	jsr ssprib 			; show single sprite.
	jmp dspr2

;-----------------------------------------
; Get sprite address calculations.
; gspran = new sprite, gsprad = old sprite.
;
; Input:
;  IX = sprite address
;-----------------------------------------

gspran:
	ldy #var_newY
	lda (z80_ix),y 		; new y coordinate.
	sta dispy
	ldy #var_newX
	lda (z80_ix),y 		; new x coordinate.
	sta dispx
	ldy #var_newImage
	lda (z80_ix),y 		; new sprite image.
	jsr gfrm		; fetch start frame for this sprite.

	ldy #0
	lda (z80_hl),y 		; frame in accumulator.
	ldy #var_newFrame
	clc
	adc (z80_ix),y 		; new add frame number.
	jmp gspra0

;-----------------------------------------
; Calculate old sprite address
;
; Input:
;  IX = sprite address
;
; Output:
;  B  = right byte mask
;  C  = left byte mask
;  DE = spriteframe address
;  scraddr = screenaddress(dispx,dispy)
;-----------------------------------------

gsprad:
	ldy #var_Y
	lda (z80_ix),y		; y coordinate.
	sta dispy
	ldy #var_X
	lda (z80_ix),y		; x coordinate.
	sta dispx
	ldy #var_Image
	lda (z80_ix),y 		; sprite image.
	jsr gfrm 		; fetch start frame for this sprite.

	ldy #0
	lda (z80_hl),y 		; frame in accumulator.
	ldy #var_Frame
	clc
	adc (z80_ix),y 		; add frame number.

gspra0:
.if rflag
	sta z80_e		; multiply by 32.
	lda #0
	sta z80_d

	asl z80_e
	rol z80_d
	asl z80_e
	rol z80_d
	asl z80_e
	rol z80_d
	asl z80_e
	rol z80_d
.if bflag
	lda z80_e		; multiply by 48
	sta tmp1
	lda z80_d
	sta tmp2
.endif
	asl z80_e
	rol z80_d
.if bflag
	clc
	lda z80_e
	adc tmp1
	sta z80_e
	lda z80_d
	adc tmp2
	sta z80_d
.endif
.else
	lsr a	  		; multiply by 128.
	sta z80_d 		; store in d.
	sta tmp1
	lda #0
	ror a
	sta z80_e 		; got low byte.
	sta tmp2
.if bflag
	lsr tmp1		; multiply by 192
	ror tmp2
	clc
	lda tmp2
	adc z80_e
	sta z80_e
	lda tmp1
	adc z80_d
	sta z80_d
.endif
.endif
	clc 			; address of play sprites.
	lda z80_e
.if swrflag
	adc #<(sprgfx + data_address - data_start)
	sta z80_e
	lda z80_d
	adc #>(sprgfx + data_address - data_start)
	sta z80_d
.else
	adc #<sprgfx 
	sta z80_e
	lda z80_d
	adc #>sprgfx
	sta z80_d
.endif

	lda dispx 		; y coordinate.
	and #6 			; position within byte boundary.
	tax	 		; low byte of table displacement.
.if rflag
	stx sprshft
.else
	asl a	  		; multiply by 32.
	asl a  			; already a multiple
	asl a  			; of 2, so just 4
.if bflag
	sta tmp1
	asl a  			; shifts needed.
	clc
	adc tmp1
.else
	asl a  			; shifts needed.
.endif
	clc 			; add to sprite address.
	adc z80_e
	sta z80_e
	bcc :+
	inc z80_d
:
.endif
	lda spmask,x		 ; pointer to mask table.
	sta z80_c 		; left mask.
	lda spmask+1,x
	sta z80_b 		; right mask.
; Drop into screen address routine.
	jmp scadd

spmask:	.byte 255,0,63,192,15,240,3,252


;-----------------------------------------------------------
; Animates a sprite.
;
; Input:
;  IX = sprite address
;  HL = last sprite address
;-----------------------------------------------------------

animsp:
	and frmno
	beq :+
	rts
:
	ldy #var_newImage
	lda (z80_ix),y		; sprite image
	jsr gfrm		; get frame data.

	inc z80_l		; point to frames.
	bne :+
	inc z80_h
:
	ldy #var_newFrame
	lda (z80_ix),y		; sprite frame.
	clc
	adc #1			; next one along.
	ldy #0
	cmp (z80_hl),y		; reached the last frame?
	bcc anims0		; no, not yet.
	lda #0			; start at first frame.
anims0:
	ldy #var_newFrame
	sta (z80_ix),y		; new frame.
	rts

;--------------------------------------------------------------
; Animate back
;
; Input:
;  IX = sprite address
;  HL = last sprite address
;--------------------------------------------------------------

animbk:
	and frmno
	beq :+
	rts
:
	ldy #var_newImage
	lda (z80_ix),y		; sprite image.
	jsr gfrm		; get frame data.

	inc z80_l 		; point to frames.
	bne :+
	inc z80_h
:
	ldy #var_newFrame
	lda (z80_ix),y 		; sprite frame.
	beq :+
	jmp rtanb0 		; yes, start at end.
:
	ldy #0
	lda (z80_hl),y 		; last sprite.
rtanb0:
	sec
	sbc #1			; next one along.
	jmp anims0		; set new frame.

;--------------------------------------------------------------
; Check for collision with other sprite, strict enforcement.
;
; Input:
;  b		= sprite to test for
;  ix		= current sprite pointer
;
; global:	b
; local:	x,y,hl,de,skptr
; calls:	-
;--------------------------------------------------------------

sktyp:
	lda #<sprtab				; sprite table.
	sta z80_l
	lda #>sprtab
	sta z80_h
numsp2:
	lda #NUMSPR				; number of sprites.
	sta sktptr
sktyp0:
	lda z80_l 				; store pointer to sprite.
	sta skptr
	lda z80_h
	sta skptr+1

	ldy #0
	lda (z80_hl),y 				; get sprite type.
	cmp z80_b				; is it the type we seek?
	beq coltyp				; yes, we can use this one.
sktyp1:
	clc
	lda skptr				; retrieve sprite pointer.
	adc #TABSIZ				; size of each entry.
	sta z80_l
	lda skptr+1
	adc #0
	sta z80_h
	dec sktptr					; one less iteration.
	bne sktyp0				; keep going until we find a slot.
	lda #0					; default to ROM address - no sprite.
	sta z80_l
	sta z80_h
	sta skptr				; store pointer to sprite.
	sta skptr+1

	clc					; don't return with zero flag set.
	rts 					; didn't find one.

coltyp:
	ldy #0
	lda (z80_ix),y				; current sprite type.
	cmp z80_b				; seeking sprite of same type?
	beq colty1				; yes, need to check we're not detecting ourselves.
colty0:
	ldy #9					; distance to x position in table.
	lda (z80_hl),y				; fetch x coordinate.
	sta z80_e
	dey
	lda (z80_hl),y				; fetch y coordinate.
	sta z80_d

; Drop into collision detection.

colc16:
	ldy #9
	lda (z80_ix),y			 	; x coord.
	sec					; subtract x.
	sbc z80_e
	bcs  colc1a 				; result is positive.
	eor #$ff				; make negative positive.
	clc
	adc #1
colc1a:
	cmp #16 				; within x range?
	bcs sktyp1				; no - they"ve missed.
	sta z80_c				; store difference.

	ldy #8
	lda (z80_ix),y				; y coord.
	sec
	sbc z80_d				; subtract y.
	bcs colc1b				; result is positive.
	eor #$ff				; make negative positive.
	clc
	adc #1
colc1b:
	cmp #16					; within y range?
	bcs sktyp1 				; no - they've missed.
	clc					; add x difference.
	adc z80_c
	cmp #26					; only 5 corner pixels touching?
	bcs :+
	sec
	rts 					; carry set if there's a collision.
:
	jmp sktyp1				; try next sprite in table.
colty1:
	lda z80_x  				; compare the two.
	cmp z80_l
	bne end_col
	lda z80_i
	cmp z80_h
	bne end_col
	jmp sktyp1 				; addresses are identical.
end_col:
	jmp colty0

;-----------------------------------------------------------
; Display number, left aligned
; 
; Input:
;  a		= number
;
; global:	-
; local:	a,y,bc,hl,displ0
; calls:	num2ch,dmsg3
;-----------------------------------------------------------

disply:
	sta z80_a
	lda #<displ0				; display workspace.
	sta z80_c
	lda #>displ0
	sta z80_b
	lda z80_a
	jsr num2ch				; convert accumulator to string.
displ1:
	dec z80_c				; back one character.
	bne :+
	dec z80_b
:
	ldy #0
	lda (z80_bc),y				; fetch digit.
	ora #128				; insert end marker.
	sta (z80_bc),y				; new value.

	lda #<displ0				; display space.
	sta z80_l
	lda #>displ0
	sta z80_h
	jmp dmsg3				; display the string.

displ0:	.byte 0,0,0,13+128

;----------------------------------------------------------------
; Initialise screen.
;
; global:	roomtb,scno
; local:	-
; calls:	tstcs
;----------------------------------------------------------------

initsc:
	lda roomtb 			; whereabouts in the map are we?
	jsr tstsc 			; find displacement.
	cmp #255 			; is it valid?
	beq init_end 			; no, it's rubbish.
	sta scno			; store new room number.
init_end:
	rts

;----------------------------------------------------------------
; Test screen.
;
; global:	-
; local:	x
; calls:	-
;----------------------------------------------------------------

tstsc:
	sta tmproom
	clc
	adc #MAPWID 			; add width in case we"re negative.
	tax 				; add displacement to map data.
	lda mapdat-MAPWID,x 		; find room number there.
	rts

;--------------------------
; Screen left.
;--------------------------

scrl:
	lda roomtb 			; present room table pointer.
	sec
	sbc #1				; room left.
scrl0:
	jsr tstsc			; test screen.
	cmp #255			; is there a screen this way?
	bne :+
	rts				; no, return to loop.
:
	lda tmproom			; restore room displacement.
	sta roomtb			; new room table position.
scrl1:
	jsr initsc 			; set new screen.
	lda #2
	sta restfl 			; set it.
	rts
scrr:
	lda roomtb 			; room table pointer.
	clc
	adc #1				; room right.
	jmp scrl0
scru:
	lda roomtb 			; room table pointer.
	sec
	sbc #MAPWID 			; room up.
	jmp scrl0
scrd:
	lda roomtb 			; room table pointer.
	clc
	adc #MAPWID 			; room down.
	jmp scrl0

;-----------------------------------------
; Jump to new screen.
;-----------------------------------------

nwscr:
	ldx #0				; start of map data.
nwscr0:
	cmp mapdat,x
	beq nwscr1			; have we found a match for screen?
	inx 				; next room.
	cpx #80				; zero room count, 80 to search.
	bne nwscr0			; keep looking.
	rts
nwscr1:
	stx roomtb			; set the map position.
	jmp scrl1			; draw new room.


;----------------------------------------------------------
; Gravity processing.
;----------------------------------------------------------

grav:
	ldy #13
	lda (z80_ix),y			; jump pointer low.
	sta z80_l
	ldy #14
	lda (z80_ix),y			; jump pointer high.
	sta z80_h
	ora z80_l			; merge in low byte.
	bne :+
	rts				; if neither is set, we're not in the air.
:
	ldy #0
	lda (z80_hl),y			; pixels to move.
	sta z80_a
	cmp #99				; reached the end?
	bne grav0			; no, continue.
grav2:
	dec z80_l			; go back to previous value.
	cmp #$ff
	bne :+
	dec z80_h
:
	lda (z80_hl),y			; fetch that from table.
	sta z80_a
grav0:
	inc z80_l			; point to next table entry.
	bne :+
	inc z80_h
:
	lda z80_l
	ldy #13
	sta (z80_ix),y			; store new pointer low.
	lda z80_h
	ldy #14
	sta (z80_ix),y			; store new pointer high.
grav1:
	lda z80_a
	bne :+				; any movement required?
	rts				; no, not this time.
:
	lda z80_a
	cmp #128			; is it up or down?
	bcs gravu			; it's up.
gravd:
	sta z80_b			; set pixels to move.
gravd0:
	jsr cangd			; can we go down?
	bne gravst			; can't move down, so stop.
	ldy #8
	lda (z80_ix),y			; adjust new x coord.
	clc
	adc #1
	sta (z80_ix),y
	dec z80_b
	bne gravd0
	rts
gravu:
	eor #$ff			; flip the sign so it's positive.
	clc
	adc #1
	sta z80_b			; set pixels to move.
gravu0:
	jsr cangu			; can we go up?
	bne ifalls			; can't move up, go down next.
	ldy #8
	lda (z80_ix),y
	sec
	sbc #1
	sta (z80_ix),y			; adjust new x coord.
	dec z80_b
	bne gravu0
	rts
gravst:
	ldy #var_jumpLo
	lda (z80_ix),y			; jump pointer low.
	sta z80_l
	ldy #var_jumpHi
	lda (z80_ix),y			; jump pointer high.
	sta z80_h

	lda #0				; null value in pointer.
	ldy #var_jumpLo
	sta (z80_ix),y			; store new pointer low.
	iny
	sta (z80_ix),y			; store new pointer high.

	ldy #0
	lda (z80_hl),y			; fetch byte from table.
	cmp #99				; is it the end marker?
evftf:
	beq :+				; yes, fallen too far.
	rts
:
	jmp evnt15			; EVENT FELLTOOFAR

;------------------------------------------------
; Initiate fall check.
;------------------------------------------------

ifall:
	ldy #var_jumpLo
	lda (z80_ix),y 			; jump pointer low.
	sta z80_l
	ldy #var_jumpHi
	lda (z80_ix),y 			; jump pointer high.
	sta z80_h			; high byte in accumulator.
	ora z80_l			; merge in low byte.
	beq :+
	rts				; if either is set, we're already in the air.
:
	ldy #9
	lda (z80_ix),y			; y coordinate.
	sta dispx
	ldy #8
	lda (z80_ix),y			; look x coordinate.
	clc
numsp7:
	adc #SPR_HGT			; add 16 pixels.
	sta dispy			; set up test coordinates.
	jsr tstbl			; get map address.
	jsr plchk			; block, platform check.
	beq :+
	rts				; it's solid, don't fall.
:
	inc bufaddr			; look right one cell.
	jsr plchk			; block, platform check.
	beq :+
	rts				; it's solid, don't fall.
:
	lda dispx			; y coordinate.
	and #7				; position straddling block cells.
	beq ifalls			; no more checks needed.
	inc bufaddr			; look to third cell.
	jsr plchk			; block, platform check.
	beq :+
	rts				; it's solid, don't fall.
:
ifalls:
.if swrflag
	lda #<(jtab + data_address - data_start)			; jump table start.
	sta z80_l
	lda #>(jtab + data_address - data_start)
	sta z80_h
.else
	lda #<jtab			; jump table start.
	sta z80_l
	lda #>jtab
	sta z80_h
.endif
ifal0:
	inc z80_l			; point to next value.
	bne :+
	inc z80_h
:
	ldy #0
	lda (z80_hl),y			; fetch value.
	beq ifal0			; no, get next value.
	cmp #99				; reached end of table?
	bne :+
	rts				; yes, don't fall.
:
	cmp #128			; is it going up?
	bcs ifal0			; yes, looking for first movement down.

	ldy #13
	lda z80_l
	sta (z80_ix),y 			; set jump low.
	ldy #14
	lda z80_h
	sta (z80_ix),y 			; set jump high.
	rts


;----------------------------------------------------
; Get frame data for a particular sprite.
; Input:
;  a		= framenumer
; Output:
;  hl		= frame address
;
; global:	hl,frmptr
; local:	-
; calls:	-
;----------------------------------------------------

gfrm:
	asl a	 		 	; multiple of 2.
	clc
	adc frmptr 			; frames used by game.
	sta z80_l
	lda frmptr+1
	adc #0
	sta z80_h 			; point to frame start.
	rts

;----------------------------------------------------
; Find sprite list for current room.
;
; global:	hl
; local:	x,y
; calls:	-
;----------------------------------------------------

sprlst:
.if swrflag
	lda #<(nmedat + data_address - data_start) 			; list of enemy sprites.
	sta z80_l
	lda #>(nmedat + data_address - data_start)
	sta z80_h
.else
	lda #<nmedat 			; list of enemy sprites.
	sta z80_l
	lda #>nmedat
	sta z80_h
.endif
	ldx scno 			; screen number.
	bne sprls2 			; is it the first screen?
	rts 				; yes, don't need to search data.
sprls2:
	ldy #0
sprls1:
	lda (z80_hl),y 			; fetch type of sprite.
	cmp #255			; is it an end marker?
	beq sprls0 			; yes, end of this room.

	clc 				; point to next sprite in list.
	lda z80_l
	adc #NMESIZ
	sta z80_l
	bcc :+
	inc z80_h
:
	jmp sprls1 			; continue until end of room.
sprls0:
	inc z80_l 			; point to start of next screen.s
	bne :+
	inc z80_h
:
	dex
	bne sprls1 			; continue until room found.
	rts


;----------------------------------------------------
; Clear all but a single player sprite.
;
; global:	-
; local:	x,y,ix
; calls:	-
;----------------------------------------------------

nspr:
	lda #NUMSPR			; sprite slots in table.
	sta sprcnt
	lda #<sprtab 			; sprite table.
	sta z80_x
	lda #>sprtab
	sta z80_i
nspr0:
	ldy #0 				; fetch sprite type.
	lda (z80_ix),y 			; is it a player?
	beq nspr1 			; yes, keep this one.

	lda #255
	ldy #0 				; fetch sprite type.
	sta (z80_ix),y 			; delete sprite.
	ldy #5
	sta (z80_ix),y 			; remove next type.

	clc	 			; next sprite.
	lda z80_x
	adc #TABSIZ 			; distance to next odd/even entry.
	sta z80_x
	bcc :+
	inc z80_i
:
	dec sprcnt	 			; one less space in the table.
	bne nspr0
	rts
nspr1:
	lda #255
	ldy #0
	sta (z80_ix),y 			; delete sprite.

	clc	 			; point to next sprite.
	lda z80_x
	adc #TABSIZ 			; distance to next odd/even entry.
	sta z80_x
	bcc :+
	inc z80_i
:
	dec sprcnt	 			; one less to do.
	bne nspr2
	rts
nspr2:
	lda #255
	ldy #0
	sta (z80_ix),y 			; delete sprite.
	ldy #5
	sta (z80_ix),y 			; remove next type.

	clc	 			; next sprite.
	lda z80_x
	adc #TABSIZ 			; distance to next odd/even entry.
	sta z80_x
	bcc :+
	inc z80_i
:
	dec sprcnt	 			; one less space in table.
	bne nspr2
	rts

;----------------------------------------------------------
; Two initialisation routines.
; Initialise sprites - copy everything from list to table.
;
; global:	-
; local:	x,y,ix
; calls:	cpsp
;----------------------------------------------------------

ispr:
	lda #NUMSPR			; sprite slots in table.
	sta sprcnt
	lda #<sprtab			; sprite table.
	sta z80_x
	lda #>sprtab
	sta z80_i
ispr2:
	ldy #0
	lda (z80_hl),y 			; fetch byte.
	cmp #255 			; is it an end marker?
	bne :+
	rts 				; yes, no more to do.
:
ispr1:
	ldy #0
	lda (z80_ix),y 			; fetch sprite type.
	cmp #255 			; is it enabled yet?
	bne ispr4			; yes, try another slot.

	ldy #5
	lda (z80_ix),y		 	; next type.
	cmp #255 			; is it enabled yet?
	beq ispr3 			; no, process this one.
ispr4:
	clc 				; next sprite.
	lda z80_x
	adc #TABSIZ		 	; distance to next odd/even entry.
	sta z80_x
	bcc :+
	inc z80_i
:
	dec sprcnt
	bne ispr1 			; repeat for remaining sprites.
	rts  				; no more room in table.
ispr3:
	jsr cpsp			; initialise a sprite.
	dec sprcnt			; one less space in the table.
	bne ispr2
	rts


;-----------------------------------------------------------------------
; Initialise sprites - but not player, we're keeping the old one.
;
; global:	-
; local:	x,y,ix
; calls:	cpsp
;-----------------------------------------------------------------------

kspr:
	ldx #NUMSPR			; sprite slots in table.
	lda #<sprtab 			; sprite table.
	sta z80_x
	lda #>sprtab
	sta z80_i
kspr2:
	ldy #0
	lda (z80_hl),y 			; fetch byte.
	cmp #255 			; is it an end marker?
	bne :+
	rts 				; yes, no more to do.
:
	cmp #0
	bne kspr1 			; no, add to table as normal.

	clc 				; next sprite.
	lda z80_l
	adc #NMESIZ		 	; distance to next odd/even entry.
	sta z80_l
	bcc :+
	inc z80_h
:
	jmp kspr2
kspr1:
	ldy #0 				; fetch sprite type.
	lda (z80_ix),y
	cmp #255 			; is it enabled yet?
	bne kspr4 			; yes, try another slot.

	ldy #5 				; next type.
	lda (z80_ix),y
	cmp #255 			; is it enabled yet?
	beq kspr3 			; no, process this one.
kspr4:
	clc 				; next sprite.
	lda z80_x
	adc #TABSIZ		 	; distance to next odd/even entry.
	sta z80_x
	bcc :+
	inc z80_i
:
	dex	 			; repeat for remaining sprites.
	bne kspr1
	rts  				; no more room in table.
kspr3:
	jsr cpsp 			; copy sprite to table.
	dex	 			; one less space in the table.
	bne kspr2
	rts

;----------------------------------------------
; Copy sprite from list to table.
;
; global:	hl,ix
; local:	y
; calls:	evnt09
;----------------------------------------------

cpsp:
	ldy #0					; fetch byte from table.
	lda (z80_hl),y
	; y=var_Type
	sta (z80_ix),y			; set up type.
	ldy #var_newType
	sta (z80_ix),y 			; set up type.

	inc z80_l 				; move to next byte.
	bne :+
	inc z80_h
:
	ldy #0 					; fetch byte from table.
	lda (z80_hl),y
	ldy #var_newImage
	sta (z80_ix),y			; set up image.

	inc z80_l 				; move to next byte.
	bne :+
	inc z80_h
:
	ldy #0
	lda (z80_hl),y 			; fetch byte from table.
	ldy #var_newY
	sta (z80_ix),y 			; set up coordinate.

	lda #200 				; set initial coordinate off screen.
	ldy #var_Y
	sta (z80_ix),y

	inc z80_l 				; move to next byte.
	bne :+
	inc z80_h
:
	ldy #0 					; fetch byte from table.
	lda (z80_hl),y
	ldy #var_newX
	sta (z80_ix),y 			; set up coordinate.

	inc z80_l 				; move to next byte.
	bne :+
	inc z80_h
:
	lda #0					; zeroes in accumulator.
	ldy #var_newFrame 		; reset frame number.
	sta (z80_ix),y
	ldy #var_Direction 		; reset direction.
	sta (z80_ix),y
	ldy #var_jumpLo			; reset jump pointer low.
	sta (z80_ix),y
	ldy #var_jumpHi	 		; reset jump pointer high.
	sta (z80_ix),y

	lda #255 				; reset data pointer to auto-restore.
	ldy #var_dataHi
	sta (z80_ix),y
evis0:
	lda z80_i
	pha
	lda z80_x
	pha
	lda z80_h
	pha
	lda z80_l
	pha

	jsr evnt09 				; perform event.

	pla
	sta z80_l
	pla
	sta z80_h
	pla
	sta z80_x
	pla
	sta z80_i

	clc
	lda z80_x 			; distance to next odd/even entry.
	adc #TABSIZ		 	; next sprite.
	sta z80_x
	bcc :+
	inc z80_i
:
	rts


;-------------------------------------
; Clear the play area window.
;-------------------------------------

clw:
	lda wintop			; get coordinates of window.
	sta dispy			; put into dispx for calculation.
	lda winlft
	sta dispx

	lda winhgt			; height of window.
	sta rrow			; copy to b register.
clw3:
	lda winwid 			; width of window.
	sta rcol
clw2:
	jsr gprad 			; get print address.
	lda #0				; zero byte to write.
	ldx #7				; pixel height of each cell.
clw1:
	ldy scrtab,x
	sta (scraddr),y 			; copy to screen.
	dex				; next screen row down.
	bpl clw1

	inc dispx			; next column.
	dec rcol			; one less to do.
	bne clw2			; repeat for remaining columns.

	lda winlft			; get left edge.
	sta dispx 			; reset x.
	inc dispy 			; next line down.

	dec rrow
	bne clw3			; repeat down the screen.

	lda wintop			; get coordinates of window.
	sta chary			; put into display position.
	lda winlft
	sta charx
	rts


;----------------------------------------------------------
; Effects code.
; Ticker routine is called 25 times per second.
;
; txtini = text scroller address
; txtscr = left text screen address
; txtwid = scroller width
; txtbit = 128
; X      = message nr
;----------------------------------------------------------

scrollpos:	.byte   0,  8, 16, 24, 32, 40, 48, 56
		.byte  64, 72, 80, 88, 96,104,112,120
		.byte 128,136,144,152,160,168,176,184
		.byte 192,200,208,216,224,232,240,248
.if sflag
scrly:
	rts
	.word txtscr         	; set scr_l = left screen address.
	sta scr_l
	sta tmp_byte		; tmp_byte = lb left screen address
	lda txtscr+1
	sta scr_l+1
	sta scr_r+1		; set hb scr_r = hb left screen address
	
	stx xtmp		; save x

	ldx txtwid
	dex
	ldy scrollpos,x
	tya
	clc
	adc scr_l
	sta scr_r		; set lb scr_r = scr_l + (txtwid - 1) * 8

	lda #8
	sta line_cnt
scrly1:
	ldx txtwid		; set txtwide
	dex
	clc
scrly0:
	ldy scrollpos,x
	lda (scr_l),y		; scroll 1 line
	rol a
	sta (scr_l),y
	dex
	bpl scrly0

	inc scr_l
	dec line_cnt
	bne scrly1		; repeat 8 times

	lda txtpos 		; get text pointer.
	sta scr_txt
	lda txtpos+1
	sta scr_txt+1

	ldy #0
	lda (scr_txt),y 	; find character we're displaying.
	and #127 		; remove end marker bit if applicable.
	cmp #13			; is it newline?
	bne scrly5 		; no, it's okay.
	lda #32			; convert to a space instead.
scrly5:
	sta fntaddr		; calculate char address
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
	sta scrly3+1		; that's the low byte.
	lda fntaddr+1
	adc #>(FONT-256)
	sta scrly3+2		; add displacement.

	ldy #0
scrly3:
	lda $3333,y		; get image of char line.
	and txtbit
	beq scrly2		; don't plot pixel
;	ldy scrline,x
	lda (scr_r),y
	clc
	ora #1
	sta (scr_r),y		; plot pixel
scrly2:
	iny			; next line of char.
	cpy #8
	bne scrly3

	lsr txtbit		; bit of text to display.
	bcs :+
	rts
:
	ldy #0
	lda (scr_txt),y 	; what was the character?
	asl a	  		; end of message?
	bcs scrly4
	inc txtpos
	bne :+
	inc txtpos+1
:
	jmp scrly6 		; not yet - continue.
scrly4:
	lda txtini 		; start of scrolling message.
	sta txtpos
	lda txtini+1
	sta txtpos+1
scrly6:
	lda #128
	sta txtbit
	ldx xtmp
	rts

scrline:	.byte $00,$20,$40,$60,$80,$a0,$c0,$e0
tmp_byte:	.byte 0
line_cnt:	.byte 0

;-------------------------------------------------------
; Entry TICKER command
;
; Entry:
;  z80_b = message nr
;  z80_c = width
;-------------------------------------------------------

iscrly:
	jsr prescr 		; set up display position.
.if swrflag
	lda #<(msgdat + data_address - data_start) 		; text messages.
	sta z80_l
	lda #>(msgdat + data_address - data_start)
	sta z80_h
.else
	lda #<msgdat 		; text messages.
	sta z80_l
	lda #>msgdat
	sta z80_h
.endif
	lda z80_c 		; width.
	sec
	sbc #1			; subtract one.
	cmp #32 		; is it between 1 and 32?
	bcc :+
	lda #$60
	jmp iscrl0		; no, disable messages.
:
	ldx z80_b		; message number.
	jsr getwrd 		; find message start.

	lda z80_l		; set initial text position.
	sta txtini
	lda z80_h
	sta txtini+1

	lda #$ad		; code for lda adrr
iscrl0:
	sta scrly		; enable/disable scrolling routine.

	jsr prescr 		; set up display position.
	jsr gprad 		; get print address.

	lda scraddr 		; set text screen address.
	sta txtscr
	lda scraddr+1
	sta txtscr+1

	lda z80_c		; width.
	sta txtwid		; set width in working storage.

	lda #128 		; start with leftmost bit.
	sta txtbit

	jmp scrly4
.endif

;------------------------------------------------------------------
; Dig routine, conditional assembly depending on dflag
;------------------------------------------------------------------
.if dflag
dig:
	and #3
	beq digr		; dig right
	cmp #1
	beq digl		; dig left
	cmp #2
	beq digd		; dig down

; Dig up.

digu:				; dig up
	ldy #8
	lda (z80_ix),y
	sec
	sbc #2
	sta dispy		; set y

	iny
	lda (z80_ix),y
	sta dispx		; set x
	jmp digv

; Dig down.

digd:
	ldy #9
	lda (z80_ix),y
	sta dispx		; set y

	dey
	clc
	lda (z80_ix),y
	adc #16
	sta dispy		; set y
	jmp digv

; Dig left.

digl:
	ldy #8
	lda (z80_ix),y
	sta dispy		; set y

	iny
	lda (z80_ix),y
	sec
	sbc #2			; x=x-2
	sta dispx		; set x
	jmp digh

; Dig right.

digr:
	ldy #8
	lda (z80_ix),y
	sta dispy		; set y

	iny
	lda (z80_ix),y
	clc
	adc #16
	sta dispx		; set x+16
	jmp digh

; Vertical digging

digv:
	jsr tstbl		; check blocktype in MAP
	jsr fdchk		; test if FODDER

	clc
	lda dispx		; look 1 cell down
	adc #8
	sta dispx
	jsr tstbl		; check blocktype in MAP
	jsr fdchk
	lda dispx
	and #7
	bne :+
	rts
:
	clc
	lda dispx		; look 1 cell down
	adc #8
	sta dispx
	jsr tstbl		; check blocktype in MAP
	jmp fdchk

; Horizontal digging

digh:
	jsr tstbl		; check blocktype in MAP
	jsr fdchk		; test if FODDER

	clc
	lda dispy		; look 1 cell down
	adc #8
	sta dispy
	jsr tstbl		; check blocktype in MAP
	jsr fdchk
	lda dispy
	and #7
	bne :+
	rts
:
	clc
	lda dispy		; look 1 cell down
	adc #8
	sta dispy
	jsr tstbl		; check blocktype in MAP
	jmp fdchk

digcnt:	.byte 0

.endif

;------------------------------------------------------------------
; Sprite table 
;------------------------------------------------------------------


; ix+0  = type.
; ix+1  = sprite image number.
; ix+2  = frame.
; ix+3  = y coord.
; ix+4  = x coord.

; ix+5  = new type.
; ix+6  = new image number.
; ix+7  = new frame.
; ix+8  = new y coord.
; ix+9  = new x coord.

; ix+10 = direction.
; ix+11 = parameter 1.
; ix+12 = parameter 2.
; ix+13 = jump pointer low.
; ix+14 = jump pointer high.
; ix+15 = data pointer low.
; ix+16 = data pointer high.

; block NUMSPR * TABSIZ,255

;sprtab:	.res NUMSPR*TABSIZ,255
ssprit:	.byte 255,255,255,255,255,255,255,0,192,120,0,0,0,255,255,255,255
roomtb:	.byte 7                      ; start room map offset.

; User routine.  Put your own code in here to be called with USER instruction.
; if USER has an argument it will be passed in the accumulator.

user:
	rts

; Everything below here will be generated by the editors.


        rts
WINDOWTOP = 1
WINDOWLFT = 1
WINDOWHGT = 18
WINDOWWID = 30 ;a
MAPWID = 9
        .byte 255,255,255,255,255,255,255,255,255
mapdat:
        .byte 255,255,255,255,0,255,255,255,255,255,255,1,2,3,4,5,255,255,255,6,7,8,9,10,11,255,255,255,12,13,14,15,16,17,18,255,255,19,20,21,255,255,255,255,255
        .byte 255,255,255,255,255,255,255,255,255
stmap:  .byte 4

evnt00:
        lda #CUSTOM	; CUSTOM
        sta z80_b
        jsr tded
        cmp z80_b
        beq :+
        jmp a00046
:
        lda varp
        clc
        adc #1
        sta varp
a00046: lda #40
        cmp varp
        bcc *+5
        jmp a00072
        lda #40
        sta varp
a00072: lda joyval	; KEY
        and #32
        beq :+
        jmp a00111
:
        lda #3		; MENU
        tax
        jsr mmenu
        jsr redraw	; REDRAW
a00111: lda #255
        cmp varb
        beq *+5
        jmp a00173
        lda #255
        cmp varc
        beq *+5
        jmp a00173
        ldy #8
        lda (z80_ix),y
        sta varb
        ldy #9
        lda (z80_ix),y
        sta varc
a00173: ldy #11
        lda (z80_ix),y
        clc
        adc #1
        ldy #11
        sta (z80_ix),y
        lda #2
        ldy #11
        cmp (z80_ix),y
        bcc *+5
        jmp a00235
        lda #0
        ldy #11
        sta (z80_ix),y
a00235: jsr skobj	; DETECTOBJECT
        sta varobj
        lda #3
        cmp varobj
        bcc *+5
        jmp a00297
        lda #10
        cmp varobj
        bcs *+5
        jmp a00297
        lda #90		; BEEP
        asl a
        sta sndtyp
a00297: lda #255
        cmp varobj
        beq *+5
        jmp a00320
        jmp a00369
a00320: lda varobj	; GET
        jsr getob
        lda #<10		; SCORE
        sta z80_l
        lda #>10
        sta z80_h
        jsr addsc
        lda #90		; BEEP
        asl a
        sta sndtyp
a00369: jsr ifall	; TABLEFALL
        lda #12
        ldy #8
        cmp (z80_ix),y
        bcs *+5
        jmp a00427
        lda #126
        ldy #8
        sta (z80_ix),y
        jsr scru	; SCREENUP
        lda #0
        sta vard
a00427: lda #132
        ldy #8
        cmp (z80_ix),y
        bcc *+5
        jmp a00479
        lda #13
        ldy #8
        sta (z80_ix),y
        jsr scrd	; SCREENDOWN
        lda #0
        sta vard
a00479: lda joyval	; KEY
        and #1
        beq :+
        jmp a00663
:
        lda #230
        ldy #9
        cmp (z80_ix),y
        bcc *+5
        jmp a00559
        jsr scrr	; SCREENRIGHT
        lda #0
        sta vard
        lda #16
        ldy #9
        sta (z80_ix),y
        rts		; EXIT
        jmp a00663
a00559: jsr cangr	; CANGORIGHT
        beq :+
        jmp a00663
:
        lda #0
        ldy #6
        sta (z80_ix),y
        lda #0		; ANIMATE
        jsr animsp
        lda #0
        ldy #11
        cmp (z80_ix),y
        beq *+5
        jmp a00637
        lda #5		; BEEP
        asl a
        sta sndtyp
a00637: ldy #9 		; SPRITERIGHT
        lda (z80_ix),y
        clc
        adc #2
        sta (z80_ix),y
a00663: lda joyval	; KEY
        and #2
        beq :+
        jmp a00846
:
        lda #8
        ldy #9
        cmp (z80_ix),y
        bcs *+5
        jmp a00743
        jsr scrl	; SCREENLEFT
        lda #0
        sta vard
        lda #228
        ldy #9
        sta (z80_ix),y
        rts		; EXIT
        jmp a00846
a00743: jsr cangl	; CANGOLEFT
        beq :+
        jmp a00846
:
        lda #1
        ldy #6
        sta (z80_ix),y
        lda #0		; ANIMATE
        jsr animsp
        lda #0
        ldy #11
        cmp (z80_ix),y
        beq *+5
        jmp a00820
        lda #5		; BEEP
        asl a
        sta sndtyp
a00820: ldy #9 		; SPRITELEFT
        lda (z80_ix),y
        sec
        sbc #2
        sta (z80_ix),y
a00846: lda joyval	; KEY
        and #16
        beq :+
        jmp a00908
:
        jsr jump	; TABLEJUMP
        jsr cangd	; CANGODOWN
        beq :+
        jmp a00894
:
        jmp a00908
a00894: lda #60		; BEEP
        asl a
        sta sndtyp
a00908: lda #DEADLY	; DEADLY
        sta z80_b
        jsr tded
        cmp z80_b
        beq :+
        jmp a00948
:
        lda #1 		; KILL
        sta deadf
a00948: jmp grav
evnt01:
        lda #3
        ldy #6
        cmp (z80_ix),y
        beq *+5
        jmp b00042
        lda #1
        ldy #12
        sta (z80_ix),y
        jmp b00055
b00042: lda #1
        ldy #12
        sta (z80_ix),y
b00055: lda #0
        ldy #11
        cmp (z80_ix),y
        beq *+5
        jmp b00288
        ldy #12
        lda (z80_ix),y 	; REPEAT
        sta loopa
b00094: jsr cangl	; CANGOLEFT
        beq :+
        jmp b00241
:
        ldy #9 		; SPRITELEFT
        lda (z80_ix),y
        sec
        sbc #2
        sta (z80_ix),y
        lda #16
        sta z80_c
        ldy #9
        lda (z80_ix),y
        sec
        sbc z80_c
        ldy #9
        sta (z80_ix),y
        jsr cangd	; CANGODOWN
        beq :+
        jmp b00202
:
        lda #1
        ldy #11
        sta (z80_ix),y
b00202: lda #16
        sta z80_c
        ldy #9
        lda (z80_ix),y
        clc
        adc z80_c
        ldy #9
        sta (z80_ix),y
        jmp b00255
b00241: lda #1
        ldy #11
        sta (z80_ix),y
b00255: dec loopa	; ENDREPEAT
        beq :+
        jmp b00094
:
        lda #0		; ANIMATEBACK
        jsr animbk
        jmp b00494
b00288: ldy #12
        lda (z80_ix),y 	; REPEAT
        sta loopa
b00305: jsr cangr	; CANGORIGHT
        beq :+
        jmp b00453
:
        ldy #9 		; SPRITERIGHT
        lda (z80_ix),y
        clc
        adc #2
        sta (z80_ix),y
        lda #16
        sta z80_c
        ldy #9
        lda (z80_ix),y
        clc
        adc z80_c
        ldy #9
        sta (z80_ix),y
        jsr cangd	; CANGODOWN
        beq :+
        jmp b00413
:
        lda #0
        ldy #11
        sta (z80_ix),y
b00413: lda #16
        sta z80_c
        ldy #9
        lda (z80_ix),y
        sec
        sbc z80_c
        ldy #9
        sta (z80_ix),y
        jmp b00466
b00453: lda #0
        ldy #11
        sta (z80_ix),y
b00466: dec loopa	; ENDREPEAT
        beq :+
        jmp b00305
:
        lda #0		; ANIMATE
        jsr animsp
b00494: lda #0 	; COLLISION
        sta z80_b
        jsr sktyp
        bcs :+
        jmp b00529
:
        lda #1 		; KILL
        sta deadf
b00529: rts
evnt02:
        lda #5
        ldy #6
        cmp (z80_ix),y
        beq *+5
        jmp c00042
        lda #2
        ldy #12
        sta (z80_ix),y
        jmp c00055
c00042: lda #1
        ldy #12
        sta (z80_ix),y
c00055: lda #0
        ldy #11
        cmp (z80_ix),y
        beq *+5
        jmp c00175
        ldy #12
        lda (z80_ix),y 	; REPEAT
        sta loopa
c00094: jsr cangu	; CANGOUP
        beq :+
        jmp c00140
:
        ldy #8 		; SPRITEUP
        lda (z80_ix),y
        sec
        sbc #2
        sta (z80_ix),y
        jmp c00154
c00140: lda #1
        ldy #11
        sta (z80_ix),y
c00154: dec loopa	; ENDREPEAT
        beq :+
        jmp c00094
:
        jmp c00269
c00175: ldy #12
        lda (z80_ix),y 	; REPEAT
        sta loopa
c00192: jsr cangd	; CANGODOWN
        beq :+
        jmp c00239
:
        ldy #8 		; SPRITEDOWN
        lda (z80_ix),y
        clc
        adc #2
        sta (z80_ix),y
        jmp c00252
c00239: lda #0
        ldy #11
        sta (z80_ix),y
c00252: dec loopa	; ENDREPEAT
        beq :+
        jmp c00192
:
c00269: lda #0		; ANIMATE
        jsr animsp
        lda #0 	; COLLISION
        sta z80_b
        jsr sktyp
        bcs :+
        jmp c00316
:
        lda #1 		; KILL
        sta deadf
c00316: rts
evnt03:
        lda #0
        ldy #11
        cmp (z80_ix),y
        beq *+5
        jmp d00089
        jsr cangl	; CANGOLEFT
        beq :+
        jmp d00071
:
        ldy #9 		; SPRITELEFT
        lda (z80_ix),y
        sec
        sbc #2
        sta (z80_ix),y
        jmp d00084
d00071: lda #1
        ldy #11
        sta (z80_ix),y
d00084: jmp d00150
d00089: jsr cangr	; CANGORIGHT
        beq :+
        jmp d00137
:
        ldy #9 		; SPRITERIGHT
        lda (z80_ix),y
        clc
        adc #2
        sta (z80_ix),y
        jmp d00150
d00137: lda #0
        ldy #11
        sta (z80_ix),y
d00150: lda #0
        ldy #10
        cmp (z80_ix),y
        beq *+5
        jmp d00237
        jsr cangu	; CANGOUP
        beq :+
        jmp d00218
:
        ldy #8 		; SPRITEUP
        lda (z80_ix),y
        sec
        sbc #2
        sta (z80_ix),y
        jmp d00232
d00218: lda #1
        ldy #10
        sta (z80_ix),y
d00232: jmp d00297
d00237: jsr cangd	; CANGODOWN
        beq :+
        jmp d00284
:
        ldy #8 		; SPRITEDOWN
        lda (z80_ix),y
        clc
        adc #2
        sta (z80_ix),y
        jmp d00297
d00284: lda #0
        ldy #10
        sta (z80_ix),y
d00297: lda #0		; ANIMATE
        jsr animsp
        lda #0 	; COLLISION
        sta z80_b
        jsr sktyp
        bcs :+
        jmp d00344
:
        lda #1 		; KILL
        sta deadf
d00344: rts
evnt04:
        rts
evnt05:
        lda #0		; ANIMATE
        jsr animsp
        rts
evnt06:
        jsr skobj	; DETECTOBJECT
        sta varobj
        lda #254
        cmp varobj
        bcs *+5
        jmp g00053
        lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
        rts		; EXIT
g00053: lda varj
        cmp scno
        beq *+5
        jmp g00157
        lda #0		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp g00099
:
        jmp g00136
g00099: ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #0
        jsr drpob
g00136: lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
        rts		; EXIT
g00157: lda vark
        cmp scno
        beq *+5
        jmp g00261
        lda #1		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp g00203
:
        jmp g00240
g00203: ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #1
        jsr drpob
g00240: lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
        rts		; EXIT
g00261: lda varl
        cmp scno
        beq *+5
        jmp g00365
        lda #2		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp g00307
:
        jmp g00344
g00307: ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #2
        jsr drpob
g00344: lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
        rts		; EXIT
g00365: lda vari
        cmp scno
        beq *+5
        jmp g00469
        lda #3		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp g00411
:
        jmp g00448
g00411: ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #3
        jsr drpob
g00448: lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
        rts		; EXIT
g00469: lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
        rts
evnt07:
        lda #0 	; COLLISION
        sta z80_b
        jsr sktyp
        bcs :+
        jmp h00243
:
        lda #0
        ldy #7
        cmp (z80_ix),y
        beq *+5
        jmp h00053
        jmp h00243
h00053: lda skptr	; OTHER
        sta z80_x
        lda skptr+1
        sta z80_i
        lda #1
        ldy #6
        cmp (z80_ix),y
        beq *+5
        jmp h00148
        ldy #9 		; SPRITERIGHT
        lda (z80_ix),y
        clc
        adc #2
        sta (z80_ix),y
        ldy #9 		; SPRITERIGHT
        lda (z80_ix),y
        clc
        adc #2
        sta (z80_ix),y
h00148: lda #0
        ldy #6
        cmp (z80_ix),y
        beq *+5
        jmp h00221
        ldy #9 		; SPRITELEFT
        lda (z80_ix),y
        sec
        sbc #2
        sta (z80_ix),y
        ldy #9 		; SPRITELEFT
        lda (z80_ix),y
        sec
        sbc #2
        sta (z80_ix),y
h00221: lda ogptr	; ENDSPRITE
        sta z80_x
        lda ogptr+1
        sta z80_i
h00243: lda #0
        ldy #11
        cmp (z80_ix),y
        bcc *+5
        jmp h00299
        lda #4
        ldy #11
        cmp (z80_ix),y
        bcs *+5
        jmp h00299
        lda #0		; ANIMATE
        jsr animsp
h00299: lda #25
        ldy #11
        cmp (z80_ix),y
        bcc *+5
        jmp h00356
        lda #29
        ldy #11
        cmp (z80_ix),y
        bcs *+5
        jmp h00356
        lda #0		; ANIMATEBACK
        jsr animbk
h00356: ldy #11
        lda (z80_ix),y
        clc
        adc #1
        ldy #11
        sta (z80_ix),y
        lda #150
        ldy #11
        cmp (z80_ix),y
        beq *+5
        jmp h00418
        lda #0
        ldy #11
        sta (z80_ix),y
h00418: rts
evnt08:
        lda #0		; ANIMATE
        jsr animsp
        lda #0 	; COLLISION
        sta z80_b
        jsr sktyp
        bcs :+
        jmp i00142
:
        lda #0		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp i00142
:
        lda #1		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp i00142
:
        lda #2		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp i00142
:
        lda #3		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp i00142
:
        lda #1		; ENDGAME
        sta gamwon
i00142: rts
evnt09:
        lda #0
        ldy #5
        cmp (z80_ix),y
        beq *+5
        jmp j00150
        lda #255
        cmp varb
        bne *+5
        jmp j00086
        lda #255
        cmp varc
        bne *+5
        jmp j00086
        lda varb
        ldy #8
        sta (z80_ix),y
        lda varc
        ldy #9
        sta (z80_ix),y
j00086: lda #2
        ldy #5
        cmp (z80_ix),y
        beq *+5
        jmp j00150
        lda vara
        clc
        adc #1
        sta vara
        lda #0
        ldy #11
        sta (z80_ix),y
        lda #69
        ldy #12
        sta (z80_ix),y
j00150: rts
evnt10:
        lda #21
        sta chary
        lda #24
        sta charx
        ldy #0		; SHOWSCORE
        lda #6
        sta z80_b
        jsr dscor
        lda #24
        sta charx
        lda #3 	; REPEAT
        sta loopa
k00058: lda #0 		; PUTBLOCK
        jsr pbpattr
        dec loopa	; ENDREPEAT
        beq :+
        jmp k00058
:
        lda #12
        sta charx
        lda #0		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp k00135
:
        lda #73 		; PUTBLOCK
        jsr pbpattr
        jmp k00147
k00135: lda #70 		; PUTBLOCK
        jsr pbpattr
k00147: lda #1		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp k00187
:
        lda #74 		; PUTBLOCK
        jsr pbpattr
        jmp k00200
k00187: lda #71 		; PUTBLOCK
        jsr pbpattr
k00200: lda #2		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp k00240
:
        lda #75 		; PUTBLOCK
        jsr pbpattr
        jmp k00252
k00240: lda #72 		; PUTBLOCK
        jsr pbpattr
k00252: lda #3		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp k00292
:
        lda #76 		; PUTBLOCK
        jsr pbpattr
        jmp k00305
k00292: lda #77 		; PUTBLOCK
        jsr pbpattr
k00305: lda #0
        cmp varm
        beq *+5
        jmp k00362
        lda #0
        cmp varn
        beq *+5
        jmp k00362
        lda #1
        sta numlif
        lda #1 		; KILL
        sta deadf
        rts		; EXIT
k00362: lda vara
        clc
        adc #1
        sta vara
        lda #10
        cmp vara
        beq *+5
        jmp k00402
        lda #0
        sta vara
k00402: lda #21
        sta chary
        lda #7
        sta charx
        lda #84 		; PUTBLOCK
        jsr pbpattr
        lda charx
        clc
        adc #1
        sta charx
        lda numlif	; DISPLAY
        jsr disply
        lda #21
        sta chary
        lda #19
        sta charx
        lda #89 		; PUTBLOCK
        jsr pbpattr
        lda #0 		; PUTBLOCK
        jsr pbpattr
        lda varp
        sta vars
        lda vars
        lsr a
        lsr a
        lsr a
        sta vars
        lda #0
        cmp vars
        bcc *+5
        jmp k00585
        lda vars 	; REPEAT
        sta loopa
k00556: lda #83 		; PUTBLOCK
        jsr pbpattr
        dec loopa	; ENDREPEAT
        beq :+
        jmp k00556
:
k00585: lda vars
        asl a
        asl a
        asl a
        sta vars
        lda vars
        sta z80_c
        lda varp
        sec
        sbc z80_c
        sta varp
        lda varp
        sta vart
        lda vart
        lsr a
        sta vart
        lda #79
        sta z80_c
        lda vart
        clc
        adc z80_c
        sta vart
        lda #10
        cmp vart
        bcc *+5
        jmp k00708
        lda vart 		; PUTBLOCK
        jsr pbpattr
        jmp k00720
k00708: lda #79 		; PUTBLOCK
        jsr pbpattr
k00720: lda vars
        sta z80_c
        lda varp
        clc
        adc z80_c
        sta varp
        lda varq
        sec
        sbc #1
        sta varq
        lda #30
        cmp varq
        bcs *+5
        jmp k00785
        lda #100
        sta varq
k00785: lda #100
        cmp varq
        beq *+5
        jmp k00818
        lda varp
        sec
        sbc #1
        sta varp
k00818: lda #0
        cmp varp
        beq *+5
        jmp k00862
        lda #1 		; KILL
        sta deadf
        lda #40
        sta varp
        lda #99
        sta varq
k00862: lda varo
        sec
        sbc #1
        sta varo
        lda #255
        cmp varo
        beq *+5
        jmp k00918
        lda #25
        sta varo
        lda varn
        sec
        sbc #1
        sta varn
k00918: lda #255
        cmp varn
        beq *+5
        jmp k00958
        lda #59
        sta varn
        lda varm
        sec
        sbc #1
        sta varm
k00958: lda #0
        cmp varm
        beq *+5
        jmp k01023
        lda #30
        cmp varn
        bcs *+5
        jmp k01023
        lda #25
        cmp varo
        beq *+5
        jmp k01023
        lda #40		; BEEP
        asl a
        sta sndtyp
k01023: lda #21
        sta chary
        lda #2
        sta charx
        lda varm	; DISPLAY
        jsr disply
        lda #5
        cmp varo
        bcc *+5
        jmp k01085
        lda #0 		; PUTBLOCK
        jsr pbpattr
        jmp k01097
k01085: lda #78 		; PUTBLOCK
        jsr pbpattr
k01097: lda #9
        cmp varn
        bcs *+5
        jmp k01125
        lda #0	; DISPLAY
        jsr disply
k01125: lda varn	; DISPLAY
        jsr disply
        lda #0 		; PUTBLOCK
        jsr pbpattr
        lda #8
        cmp varp
        bcs *+5
        jmp k01196
        lda #2
        cmp vara
        bcs *+5
        jmp k01196
        lda #40		; BEEP
        asl a
        sta sndtyp
k01196: rts
evnt11:
        rts
evnt12:
        lda #1
        sta chary
        lda #9
        sta charx
        lda #0  	; MESSAGE
        jsr dmsg 	; INK 	; PAPER
        lda #2
        sta chary
        lda #6
        sta charx
        lda #1		; PRINTMODE
        sta prtmod
        lda #1  	; MESSAGE
        jsr dmsg
        lda #0		; PRINTMODE
        sta prtmod 	; INK 	; PAPER
        lda #5
        sta chary
        lda #7
        sta charx
        lda #9  	; MESSAGE
        jsr dmsg
        lda #9
        sta chary
        lda #0
        sta charx 	; INK 	; PAPER
        lda #2  	; MESSAGE
        jsr dmsg
        lda #22
        sta chary
        lda #6
        sta charx 	; INK 	; PAPER
        lda #8  	; MESSAGE
        jsr dmsg
        lda #99
        sta contrl
m00188: lda #99
        cmp contrl
        beq *+5
        jmp m00427
        ldy #7 	; KEY
        .if swrflag
        lda keys + data_address - data_start,y
        .else
        lda keys,y
        .endif
        jsr ktest
        bcc :+
        jmp m00261
:
        lda #0
        sta contrl
m00261: ldy #8 	; KEY
        .if swrflag
        lda keys + data_address - data_start,y
        .else
        lda keys,y
        .endif
        jsr ktest
        bcc :+
        jmp m00318
:
        lda #1
        sta contrl
m00318: ldy #9 	; KEY
        .if swrflag
        lda keys + data_address - data_start,y
        .else
        lda keys,y
        .endif
        jsr ktest
        bcc :+
        jmp m00374
:
        lda #2
        sta contrl
m00374: ldy #10 	; KEY
        .if swrflag
        lda keys + data_address - data_start,y
        .else
        lda keys,y
        .endif
        jsr ktest
        bcc :+
        jmp m00422
:
m00422: jmp m00188
m00427: rts
evnt13: 	; INK 	; PAPER
        jsr cls		; CLS
        lda #0
        sta vara
        lda #255
        sta varb
        lda #255
        sta varc
        lda #40
        sta varp
        lda #99
        sta varq
        lda #4
        sta varm
        lda #59
        sta varn
        lda #25
        sta varo
        lda #29 		; PUTBLOCK
        jsr pbpattr
        lda #30 	; REPEAT
        sta loopa
n00101: lda #33 		; PUTBLOCK
        jsr pbpattr
        dec loopa	; ENDREPEAT
        beq :+
        jmp n00101
:
        lda #30 		; PUTBLOCK
        jsr pbpattr
        lda #0
        sta chary
        lda #18 	; REPEAT
        sta loopa
n00161: lda chary
        clc
        adc #1
        sta chary
        lda #0
        sta charx
        lda #34 		; PUTBLOCK
        jsr pbpattr
        lda #31
        sta charx
        lda #34 		; PUTBLOCK
        jsr pbpattr
        dec loopa	; ENDREPEAT
        beq :+
        jmp n00161
:
        lda chary
        clc
        adc #1
        sta chary
        lda #0
        sta charx
        lda #37 		; PUTBLOCK
        jsr pbpattr
        lda #30 	; REPEAT
        sta loopa
n00282: lda #33 		; PUTBLOCK
        jsr pbpattr
        dec loopa	; ENDREPEAT
        beq :+
        jmp n00282
:
        lda #37 		; PUTBLOCK
        jsr pbpattr
        lda #3 	; REPEAT
        sta loopa
n00333: lda chary
        clc
        adc #1
        sta chary
        lda #0
        sta charx
        lda #34 		; PUTBLOCK
        jsr pbpattr
        lda #31
        sta charx
        lda #34 		; PUTBLOCK
        jsr pbpattr
        dec loopa	; ENDREPEAT
        beq :+
        jmp n00333
:
        lda #0
        sta charx
        lda #23
        sta chary
        lda #32 		; PUTBLOCK
        jsr pbpattr
        lda #30 	; REPEAT
        sta loopa
n00447: lda #33 		; PUTBLOCK
        jsr pbpattr
        dec loopa	; ENDREPEAT
        beq :+
        jmp n00447
:
        lda #31 		; PUTBLOCK
        jsr pbpattr
        lda #5
        sta numlif
        lda #5
        sta z80_d	; GETRANDOM
        jsr random
        sta z80_h
        jsr imul
        lda z80_h
        sta varrnd
        lda varrnd
        clc
        adc #1
        sta varrnd
        lda varrnd
        sta varj
        lda #5
        sta z80_d	; GETRANDOM
        jsr random
        sta z80_h
        jsr imul
        lda z80_h
        sta varrnd
        lda #7
        sta z80_c
        lda varrnd
        clc
        adc z80_c
        sta varrnd
        lda varrnd
        sta vark
        lda #5
        sta z80_d	; GETRANDOM
        jsr random
        sta z80_h
        jsr imul
        lda z80_h
        sta varrnd
        lda #13
        sta z80_c
        lda varrnd
        clc
        adc z80_c
        sta varrnd
        lda varrnd
        sta varl
        lda #3
        sta z80_d	; GETRANDOM
        jsr random
        sta z80_h
        jsr imul
        lda z80_h
        sta varrnd
        lda #19
        sta z80_c
        lda varrnd
        clc
        adc z80_c
        sta varrnd
        lda varrnd
        sta vari
        rts
evnt14:
        lda #0
        sta vara
        lda #0
        cmp vard
        beq *+5
        jmp o00043
        lda #255
        sta varb
        lda #255
        sta varc
o00043: rts
evnt15:
        lda #9
        ldy #5
        cmp (z80_ix),y
        beq *+5
        jmp p00034
        lda #1 		; KILL
        sta deadf
p00034: rts
evnt16:
        lda #127		; BEEP
        asl a
        sta sndtyp
        lda numlif
        sec
        sbc #1
        sta numlif
        lda #1
        sta vard
        rts
evnt17: 	; INK 	; PAPER
        jsr cls		; CLS
        lda #8
        sta chary
        lda #11
        sta charx
        lda #1		; PRINTMODE
        sta prtmod
        lda #7  	; MESSAGE
        jsr dmsg
        lda #0		; PRINTMODE
        sta prtmod 	; INK 	; PAPER
        lda chary
        clc
        adc #1
        sta chary
        lda #0
        sta charx
        lda #5  	; MESSAGE
        jsr dmsg
        lda #250 	; DELAY
        jsr delay
        lda #200 	; DELAY
        jsr delay
        rts
evnt18:
        lda numlif 	; REPEAT
        sta loopa
s00013: lda #<100		; SCORE
        sta z80_l
        lda #>100
        sta z80_h
        jsr addsc
        dec loopa	; ENDREPEAT
        beq :+
        jmp s00013
:
        lda varm 	; REPEAT
        sta loopa
s00066: lda #<60		; SCORE
        sta z80_l
        lda #>60
        sta z80_h
        jsr addsc
        dec loopa	; ENDREPEAT
        beq :+
        jmp s00066
:
        lda varn 	; REPEAT
        sta loopa
s00118: lda #<1		; SCORE
        sta z80_l
        lda #>1
        sta z80_h
        jsr addsc
        dec loopa	; ENDREPEAT
        beq :+
        jmp s00118
: 	; INK 	; PAPER
        jsr cls		; CLS
        lda #9
        sta chary
        lda #9
        sta charx 	; INK 	; PAPER
        lda #1		; PRINTMODE
        sta prtmod
        lda #6  	; MESSAGE
        jsr dmsg
        lda #0		; PRINTMODE
        sta prtmod
        lda #100 	; DELAY
        jsr delay 	; INK 	; PAPER
        lda #4  	; MESSAGE
        jsr dmsg
        lda #14
        sta chary
        lda #10
        sta charx
        lda varm	; DISPLAY
        jsr disply
        lda #78 		; PUTBLOCK
        jsr pbpattr
        lda #9
        cmp varn
        bcs *+5
        jmp s00317
        lda #0	; DISPLAY
        jsr disply
s00317: lda varn	; DISPLAY
        jsr disply
        lda #65
        sta vara
        lda #250 	; REPEAT
        sta loopa 	; INK 	; PAPER
s00348: lda #16
        sta chary
        lda #17
        sta charx
        ldy #0		; SHOWSCORE
        lda #6
        sta z80_b
        jsr dscor
        lda #2 	; DELAY
        jsr delay
        lda #71
        cmp vara
        beq *+5
        jmp s00425
        lda #65
        sta vara
s00425: lda vara
        clc
        adc #1
        sta vara
        dec loopa	; ENDREPEAT
        beq :+
        jmp s00348
:
        lda #50 	; DELAY
        jsr delay
        rts
evnt19:
        rts
evnt20:
        rts
ptcusr: rts
data_start:
msgdat:
        .byte "ROBOT_1_IN...",141
        .byte "THE_SHIP_OF_DOOM;;",141
        .byte "_THE_CREW_LEFT_YOU_BEHIND....",13
        .byte "_THE_SHIP_WILL_CRASH_INTO_THE",13
        .byte "_SUN_IN_5_MINUTES.",13
        .byte "_TO_SAVE_YOURSELF,_COLLECT_ALL",13
        .byte "_FOUR_KEYS_AND_GET_TO_THE",13
        .byte "_TELEPORT_ROOM.",13
        .byte "_COLLECT_DATA_KEYS_FOR_POINTS.",13
        .byte "_RECHARGE_SOCKETS_ARE_LOCATED",13
        .byte "_THROUGHOUT_THE_SHIP.",13
        .byte 13
        .byte "_RUN_OUT_OF_BATTERY._YOU_DIE.",13
        .byte "__RUN_OUT_OF_TIME._YOU_DIE.",13
        .byte "_________DON",39
        .byte "T_DIE;",141
        .byte "GAME_PAUSED",141
        .byte 13
        .byte "______ROBOT_1_ESCAPED_A",13
        .byte "____FIREY_DEATH_WITH_JUST",13
        .byte "__________XXXX_LEFT",13
        .byte 13
        .byte "_YOU_ALSO_SCORED_XXXXXX_POINTS;",141
        .byte "___________YOU__DIED",13
        .byte 13
        .byte "_EITHER_YOU_DIDNT_GET_OFF_THE_",13
        .byte "_SHIP_IN_TIME_OR_YOU_RAN_OUT_OF",13
        .byte "_LIVES.",13
        .byte "_______EITHER_WAY,_YOU",39
        .byte "RE_DEAD.",13
        .byte 141
        .byte "WELL_DONE;",141
        .byte "GAME_OVER",141
        .byte "2017_Mat_Recardo",13
        .byte "____Music_by_Sergey_Kosov",141
        .byte "1_FOR_KEYS_(ZXP)",13
        .byte "_______2_FOR_KEMPSTON",13
        .byte "_______3_FOR_SINCLAIR",141
nummsg:
        .byte 10
chgfx:
        .byte 0,0,0,0,0,0,0,0
        .byte 151,171,151,171,151,171,151,171
        .byte 213,233,213,233,213,233,213,233
        .byte 255,255,153,102,0,0,0,0
        .byte 255,170,85,255,0,0,0,0
        .byte 255,153,255,0,0,0,0,0
        .byte 255,255,0,255,0,0,0,0
        .byte 255,255,85,170,85,170,0,0
        .byte 255,255,0,255,0,255,0,0
        .byte 255,68,34,255,0,0,0,0
        .byte 128,255,128,255,128,255,128,255
        .byte 255,189,165,165,165,66,0,0
        .byte 255,85,255,0,90,90,24,0
        .byte 255,0,34,102,102,68,255,0
        .byte 255,0,35,19,11,5,2,1
        .byte 255,0,35,19,11,5,2,253
        .byte 252,130,68,40,16,32,64,128
        .byte 98,82,74,70,70,74,82,98
        .byte 70,74,82,98,98,82,74,70
        .byte 255,129,66,36,24,255,0,0
        .byte 129,66,36,24,24,36,66,129
        .byte 255,128,191,191,255,0,127,0
        .byte 255,0,255,255,255,0,255,0
        .byte 255,5,255,255,255,0,254,0
        .byte 128,130,132,130,148,170,128,255
        .byte 128,129,130,133,138,149,170,255
        .byte 0,0,0,0,68,68,238,255
        .byte 63,32,0,0,63,32,0,0
        .byte 252,4,0,0,252,4,0,0
        .byte 0,3,13,29,61,57,113,127
        .byte 0,192,176,200,244,244,250,254
        .byte 254,250,244,244,200,176,192,0
        .byte 127,113,61,61,29,15,3,0
        .byte 0,255,0,255,255,170,85,0
        .byte 122,122,122,122,122,122,122,122
        .byte 122,0,253,253,253,253,0,122
        .byte 122,0,60,0,122,0,60,0
        .byte 52,122,122,122,122,122,0,52
        .byte 0,254,0,254,0,254,0,254
        .byte 85,149,21,229,9,241,2,252
        .byte 85,85,85,85,85,85,85,0
        .byte 0,0,0,0,3,5,10,13
        .byte 0,0,0,0,255,85,170,255
        .byte 0,0,0,0,128,96,176,208
        .byte 176,208,176,208,176,208,176,208
        .byte 176,80,160,192,0,0,0,0
        .byte 255,85,170,255,0,0,0,0
        .byte 11,13,6,3,0,0,0,0
        .byte 11,13,11,13,11,13,11,13
        .byte 207,0,103,103,103,103,103,207
        .byte 254,0,120,184,120,120,180,0
        .byte 0,96,119,59,28,110,119,3
        .byte 0,0,254,254,0,254,126,128
        .byte 125,126,0,127,127,0,127,0
        .byte 222,238,112,186,220,14,246,0
        .byte 255,164,255,0,31,31,15,3
        .byte 255,36,255,0,255,255,255,255
        .byte 255,37,255,0,248,248,240,192
        .byte 255,0,85,255,0,0,0,0
        .byte 255,0,89,173,44,44,44,44
        .byte 44,44,44,44,44,0,44,94
        .byte 85,84,84,83,72,71,32,31
        .byte 255,63,0,31,16,31,31,31
        .byte 128,224,24,252,7,249,254,255
        .byte 0,0,0,0,252,255,248,240
        .byte 0,0,0,0,0,128,64,112
        .byte 0,252,3,0,253,253,253,254
        .byte 224,63,63,127,127,112,191,0
        .byte 255,255,255,255,252,0,255,0
        .byte 252,254,255,255,3,2,252,0
        .byte 255,129,129,129,129,129,129,255
        .byte 60,66,129,129,129,129,66,60
        .byte 24,36,36,66,66,129,129,255
        .byte 255,129,129,129,129,129,129,255
        .byte 60,66,129,129,129,129,66,60
        .byte 24,36,36,66,66,129,129,255
        .byte 60,36,231,129,129,231,36,60
        .byte 60,36,231,129,129,231,36,60
        .byte 0,0,16,0,0,16,0,0
        .byte 0,0,0,0,0,0,0,0
        .byte 0,64,64,64,64,64,64,0
        .byte 0,80,80,80,80,80,80,0
        .byte 0,84,84,84,84,84,84,0
        .byte 0,85,85,85,85,85,85,0
        .byte 14,61,109,237,229,126,129,126
        .byte 7,24,32,64,85,255,128,255
        .byte 255,0,0,0,85,255,0,255
        .byte 240,12,2,1,85,255,1,255
        .byte 254,254,218,250,174,254,254,0
        .byte 8,16,32,126,4,8,16,0
bprop:
        .byte 0
        .byte 2
        .byte 2
        .byte 1
        .byte 2
        .byte 1
        .byte 1
        .byte 1
        .byte 2
        .byte 1
        .byte 2
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 2
        .byte 2
        .byte 1
        .byte 0
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 5
        .byte 3
        .byte 3
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 0
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 0
        .byte 2
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 2
        .byte 2
        .byte 2
        .byte 6
        .byte 6
sprgfx:
        .byte 0,0,64,0,67,239,76,57,80,249,35,251,38,139,78,239,78,14,255,255,128,1,91,108,144,9,43,213,144,8,109,182
        .byte 0,0,64,0,67,239,76,57,80,249,35,251,38,139,78,239,78,14,255,255,128,1,109,182,16,8,171,213,144,9,91,108
        .byte 128,0,131,239,76,57,80,249,35,251,38,139,78,239,78,14,255,255,128,1,0,0,54,218,144,9,171,212,16,9,54,218
        .byte 0,0,0,2,247,194,156,50,159,10,223,196,209,100,247,114,112,114,255,255,128,1,54,218,144,9,171,212,16,9,109,182
        .byte 0,0,0,2,247,194,156,50,159,10,223,196,209,100,247,114,112,114,255,255,128,1,109,182,16,8,171,213,144,9,54,218
        .byte 0,1,247,193,156,50,159,10,223,196,209,100,247,114,112,114,255,255,128,1,0,0,91,108,144,9,43,213,144,8,91,108
        .byte 1,128,6,96,8,16,16,136,32,68,32,36,64,18,64,18,64,2,64,2,32,4,32,4,16,8,8,16,6,96,1,128
        .byte 0,0,3,192,12,48,16,8,32,196,32,36,64,18,64,18,64,2,64,2,32,4,32,4,16,8,12,48,3,192,0,0
        .byte 0,0,3,192,12,48,16,8,32,196,64,34,64,18,128,9,128,9,64,2,64,2,32,4,16,8,12,48,3,192,0,0
        .byte 3,192,12,48,16,8,32,196,64,34,64,18,128,9,128,9,128,1,128,1,64,2,64,2,32,4,16,8,12,48,3,192
        .byte 0,0,0,0,0,0,63,252,32,4,63,252,6,96,1,128,255,255,135,225,51,204,73,146,181,173,165,169,72,18,48,12
        .byte 0,0,0,0,0,0,0,0,63,252,32,4,63,252,6,96,255,255,135,225,51,204,73,146,181,173,149,165,72,18,48,12
        .byte 0,0,0,0,0,0,63,252,32,4,63,252,6,96,1,128,255,255,135,225,51,204,73,146,149,165,181,173,72,18,48,12
        .byte 0,0,0,0,63,252,32,4,63,252,6,96,1,128,6,96,255,255,135,225,51,204,73,146,165,169,181,173,72,18,48,12
        .byte 1,128,191,253,1,128,15,240,16,8,32,20,32,20,32,20,32,4,63,252,20,8,24,8,15,240,4,32,4,32,14,112
        .byte 1,128,15,240,1,128,15,240,16,8,32,68,32,68,32,68,32,4,63,252,17,8,18,8,15,240,4,32,4,112,14,0
        .byte 1,128,3,192,1,128,15,240,16,8,34,4,34,4,34,4,32,4,63,252,16,72,16,136,15,240,4,32,14,112,0,0
        .byte 1,128,15,240,1,128,15,240,16,8,40,4,40,4,40,4,32,4,63,252,16,24,16,40,15,240,4,32,14,32,0,112
        .byte 3,192,12,48,16,8,38,100,41,148,72,18,74,82,127,254,32,4,63,252,8,16,16,8,31,248,0,0,0,0,0,0
        .byte 3,192,12,48,16,8,32,4,32,4,70,98,73,146,127,254,32,4,63,252,8,16,16,8,31,248,2,64,1,128,0,0
        .byte 3,192,12,48,16,8,38,100,41,148,72,18,74,82,127,254,32,4,63,252,8,16,16,8,31,248,4,32,2,64,1,128
        .byte 3,192,12,48,16,8,38,100,41,148,74,82,72,18,127,254,32,4,63,252,8,16,16,8,31,248,16,8,8,16,7,224
        .byte 1,128,3,64,2,64,1,128,1,0,97,134,223,253,149,169,97,6,1,128,1,0,1,128,1,128,3,64,2,64,1,128
        .byte 6,96,13,208,9,144,7,224,1,0,25,152,55,244,37,164,25,24,1,128,1,0,1,128,7,224,13,208,9,144,6,96
        .byte 25,152,55,244,37,36,25,152,1,0,1,128,3,64,2,64,1,128,1,128,1,0,1,128,25,152,55,244,37,100,24,24
        .byte 6,96,13,208,9,144,7,224,1,0,25,152,55,244,37,164,25,24,1,128,1,0,1,128,7,224,13,208,9,144,6,96
        .byte 15,224,23,240,19,248,0,0,126,120,1,144,62,96,0,12,15,176,58,192,68,252,130,194,146,152,162,164,68,52,56,24
        .byte 15,224,23,240,19,248,0,0,126,120,1,144,62,96,0,12,15,176,58,192,68,252,162,194,146,152,130,180,68,36,56,24
        .byte 15,224,23,240,19,248,0,0,126,120,1,144,62,96,0,12,15,176,58,192,68,252,138,194,146,152,130,172,68,36,56,24
        .byte 15,224,23,240,19,248,0,0,126,120,1,144,62,96,0,12,15,176,58,192,68,252,130,194,146,152,138,164,68,44,56,24
        .byte 0,0,32,32,47,160,31,192,54,224,50,96,127,240,112,112,63,224,16,64,32,32,64,16,128,8,192,24,192,24,192,24
        .byte 16,16,23,208,15,224,27,112,25,48,63,248,56,56,31,240,8,32,16,16,32,8,64,4,128,12,192,12,192,12,192,0
        .byte 8,8,11,232,7,240,13,184,12,152,31,252,28,28,15,248,4,16,8,8,16,4,32,2,48,1,48,3,48,3,0,3
        .byte 8,8,11,232,7,240,13,184,12,152,31,252,28,28,15,248,4,16,4,8,4,4,4,2,4,1,6,3,6,3,6,3
        .byte 0,0,4,4,5,244,3,248,6,220,6,76,15,254,14,14,7,252,2,8,4,4,8,2,16,1,24,3,24,3,24,3
        .byte 0,0,4,4,5,244,3,248,6,220,6,76,15,254,14,14,7,252,6,8,24,8,32,8,64,8,96,24,96,24,96,24
        .byte 7,224,24,120,32,52,64,18,64,14,128,7,128,5,255,255,176,195,67,14,44,60,16,216,11,16,4,32,2,192,1,128
        .byte 7,224,24,152,32,68,64,34,64,34,128,17,128,17,255,255,152,97,97,134,38,28,24,120,9,176,6,32,2,64,1,128
        .byte 7,224,24,152,32,132,64,130,64,66,128,65,128,65,255,255,140,49,112,194,35,12,28,56,8,240,7,96,2,64,1,128
        .byte 7,224,25,24,33,4,65,2,66,2,130,1,130,1,255,255,134,25,88,98,33,132,22,24,8,112,5,224,2,192,1,128
        .byte 7,224,25,24,34,4,68,2,68,2,136,1,136,1,255,255,131,13,76,50,48,196,19,8,12,48,4,224,3,64,1,128
        .byte 7,224,30,24,40,4,72,2,112,2,224,1,160,1,255,255,129,135,70,30,56,108,17,136,14,16,4,96,3,192,1,128
        .byte 0,0,1,128,2,64,3,60,0,102,0,114,31,122,32,126,80,60,64,64,80,72,88,92,77,72,32,128,31,0,0,0
        .byte 1,224,3,48,3,144,3,192,3,192,1,152,0,36,7,52,8,152,16,68,20,82,18,90,8,140,7,0,0,0,0,0
        .byte 0,192,1,32,1,160,0,208,0,56,0,16,0,14,6,51,9,57,16,189,16,191,9,30,6,0,0,0,0,0,0,0
        .byte 0,0,0,128,1,192,0,160,0,112,0,32,0,30,0,51,6,57,9,61,9,63,6,30,0,0,0,0,0,0,0,0
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,151,255
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,151,255,0,0,75,254,0,0
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,151,255,0,0,75,254,0,0,175,255,151,254,175,253,151,254
        .byte 0,0,0,0,0,0,0,0,0,0,151,255,0,0,75,254,0,0,175,255,151,254,175,253,151,254,175,253,151,254,175,253
        .byte 151,255,0,0,75,254,0,0,175,255,151,254,175,253,151,254,175,253,151,254,175,253,151,254,175,253,151,254,175,253,151,254
        .byte 0,0,0,0,32,4,0,0,4,32,128,1,128,1,0,0,32,4,32,4,4,32,128,1,129,1,0,0,160,5,255,255
        .byte 0,0,0,0,4,32,0,0,128,1,0,0,32,4,32,4,4,32,129,1,128,1,0,0,32,4,32,4,132,33,255,255
        .byte 0,0,0,0,0,0,0,0,33,4,32,4,4,32,128,1,128,1,0,0,32,4,32,4,4,32,128,1,128,1,255,255
        .byte 0,0,128,0,0,0,2,0,0,1,0,0,0,0,16,0,0,64,0,0,0,0,0,4,0,0,32,0,0,0,0,0
        .byte 0,0,0,2,0,0,8,0,0,4,0,0,0,0,64,0,1,0,0,0,0,0,0,16,0,0,128,0,0,0,0,0
        .byte 0,0,0,8,0,0,32,0,0,16,0,0,0,0,0,1,4,0,0,0,0,0,0,64,0,0,0,2,0,0,0,0
        .byte 0,0,0,32,0,0,128,0,0,64,0,0,0,0,0,4,16,0,0,0,0,0,1,0,0,0,0,8,0,0,0,0
        .byte 0,0,0,128,0,0,0,2,1,0,0,0,0,0,0,16,64,0,0,0,0,0,4,0,0,0,0,32,0,0,0,0
        .byte 0,0,2,0,0,0,0,8,4,0,0,0,0,0,0,64,0,1,0,0,0,0,16,0,0,0,0,128,0,0,0,0
        .byte 0,0,16,0,0,0,0,64,32,0,0,0,0,0,2,0,0,8,0,0,0,0,128,0,0,0,4,0,0,0,0,0
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
frmlst:
        .byte 0,3
        .byte 3,3
        .byte 6,4
        .byte 10,4
        .byte 14,4
        .byte 18,4
        .byte 22,4
        .byte 26,4
        .byte 30,6
        .byte 36,6
        .byte 42,4
        .byte 46,5
        .byte 51,3
        .byte 54,7
        .byte 61,1,62,0
scdat:
        .word 186,215,282,205,209,226,221,179,222,293,222,213,274,189,156,208,211,251,148,152,266,208
        .byte 255,0,38,25,24,25,24,25,24,25,24,25,24,25,24,255,0,18,24,255,0,10,25,255,0,16,37,0,25,255,0,10,24
        .byte 255,0,16,59,0,24,255,0,6,55,56,56,57,25,255,0,16,60,0,25,255,0,7,12,12,0,24,255,0,15,25,24,25
        .byte 24,255,0,7,60,60,0,25,24,25,24,255,0,12,24,8,8,8,0,0,0,255,8,10,25,255,0,12,25,255,0,16,24
        .byte 255,0,12,24,255,0,16,25,255,0,12,25,255,0,16,24,255,0,12,24,255,0,16,25,255,0,12,25,255,8,10,0,0,0
        .byte 8,8,8,24,255,0,12,24,255,0,16,25,255,0,12,25,255,0,16,24,255,0,12,24,255,0,16,25,255,0,12,25,255,0,16
        .byte 24,255,0,12,24,8,8,8,0,0,0,255,8,10,25,255,0,7
        .byte 255,0,16,29,255,10,13,255,0,10,37,255,0,4,29,10,255,25,13,255,0,10,59,0,0,0,29,10,255,0,13,25,255,0,10
        .byte 60,0,0,29,10,255,0,14,25,255,0,9,29,255,10,4,255,0,15,25,255,0,6,29,10,10,10,255,0,8,27,28,255,8,9
        .byte 25,255,0,4,29,10,10,255,0,14,36,255,0,7,25,0,0,0,29,10,255,0,13,27,28,0,36,255,0,7,25,0,0
        .byte 29,10,255,0,17,36,255,0,7,25,0,0,10,255,0,8,27,28,255,8,16,25,0,29,10,255,0,11,36,255,0,14,25
        .byte 0,10,255,0,9,27,28,0,36,255,0,14,25,0,10,255,0,12,36,255,0,14,25,29,10,255,0,5,27,28,255,8,20
        .byte 25,10,25,0,89,255,0,6,36,255,0,18,25,10,25,255,0,5,27,28,0,36,255,0,19,10,25,0,88,255,0,6,36
        .byte 255,0,19,10,25,255,8,22,255,0,4,8,8
        .byte 255,10,31,0,0,40,255,0,7,36,255,0,6,36,255,0,7,40,0,0,10,10,38,38,39,255,0,7,36,255,0,6,36
        .byte 255,0,7,61,38,38,10,10,255,0,10,36,255,0,6,36,255,0,10,10,10,0,0,255,8,10,0,27,28,0,255,8,10
        .byte 0,0,10,10,255,0,4,36,255,0,8,27,28,255,0,8,36,255,0,4,10,10,255,0,4,36,255,0,8,27,28,255,0,8
        .byte 36,255,0,4,10,10,255,0,4,36,255,0,8,27,28,255,0,8,36,255,0,4,10,10,255,8,6,0,0,0,8,8,8
        .byte 0,27,28,0,8,8,8,0,0,0,255,8,6,10,10,255,0,10,36,0,0,27,28,255,0,8,36,255,0,4,10,10,255,0,10
        .byte 36,0,0,27,28,255,0,8,36,255,0,4,10,10,255,0,10,36,0,0,27,28,255,0,8,36,255,0,4,10,10,0,0
        .byte 255,8,10,0,27,28,0,255,8,10,0,0,10,10,255,0,4,34,255,0,4,34,0,0,0,27,28,0,0,0,34,255,0,4
        .byte 34,255,0,4,10,10,255,0,4,32,255,33,4,31,0,0,0,27,28,0,0,0,32,255,33,4,31,255,0,4,10,255,0,14
        .byte 27,28,255,0,28,27,28,255,0,14,255,8,30
        .byte 255,10,9,0,0,0,255,10,19,255,0,28,10,10,255,0,28,10,10,255,0,28,10,10,255,8,17,255,0,8,8,8,8
        .byte 10,10,255,0,15,36,255,0,12,10,10,255,0,15,36,255,0,12,10,10,255,0,15,36,0,0,27,28,8,8,27,28,255,0,4
        .byte 10,10,255,0,15,36,0,0,27,28,0,0,27,28,255,0,5,10,255,0,15,36,255,0,13,10,255,8,17,255,0,8,8,8,8
        .byte 10,10,0,0,34,0,40,255,0,7,40,0,34,255,0,5,27,28,255,0,6,10,10,0,0,35,0,61,255,38,7,39,0
        .byte 35,255,0,5,27,28,255,0,6,10,10,0,0,34,255,0,11,34,255,0,5,27,28,255,0,6,10,10,0,0,32,255,33,11
        .byte 31,255,0,5,27,28,255,0,4,89,0,10,255,0,21,27,28,255,0,6,10,255,0,21,27,28,255,0,4,88,0,10,255,8,30
        .byte 255,10,31,255,0,11,40,0,0,40,40,0,0,40,0,0,34,0,34,0,34,0,0,10,10,0,41,42,42,43,0,41,42,42
        .byte 43,0,40,0,0,40,40,0,0,40,0,0,35,0,35,0,35,0,0,10,10,0,48,0,0,44,0,48,0,0,44,0,61
        .byte 38,38,39,61,38,38,39,0,0,34,0,34,0,34,0,0,10,10,0,48,0,0,44,0,48,0,0,44,255,0,11,32,33
        .byte 37,33,37,33,33,10,10,0,47,46,46,45,0,47,46,46,45,255,0,18,10,10,255,0,28,10,10,255,0,28,10,255,0,29
        .byte 10,255,0,29,10,255,8,7,255,0,22,10,10,255,0,4,36,255,0,23,10,10,255,0,4,36,0,8,8,255,0,20,10,10
        .byte 255,0,4,36,255,0,23,10,10,255,0,4,36,0,0,0,8,8,255,0,18,10,10,255,0,4,36,255,0,24,10,255,0,4
        .byte 36,255,0,24,255,8,30
        .byte 255,10,5,30,255,0,24,10,0,0,0,14,10,30,255,0,6,37,255,0,16,10,255,0,4,14,10,30,255,0,5,59,255,0,16
        .byte 10,255,0,5,14,10,30,255,0,4,60,255,0,16,10,255,0,6,14,255,10,12,30,255,0,9,10,255,0,18,14,10,10,10
        .byte 30,255,0,6,10,255,0,21,14,10,10,30,255,0,4,10,255,0,22,36,14,10,30,0,0,0,10,255,0,22,36,0,14
        .byte 10,30,0,0,10,255,4,5,27,28,255,0,15,36,0,0,14,10,0,0,10,0,89,0,0,0,27,28,255,0,15,36,0,0,0
        .byte 10,30,0,10,255,0,5,27,28,255,0,15,36,0,0,0,14,10,0,10,0,88,0,0,0,27,28,255,0,15,36,255,0,4
        .byte 10,0,10,255,4,12,27,28,255,0,8,21,255,22,4,10,30,10,255,0,12,27,28,255,0,13,14,10,255,0,13,27,28
        .byte 255,0,14,10,255,0,13,27,28,255,0,14,10,255,8,23,255,0,4,8,8,10
        .byte 255,10,31,255,0,28,1,10,255,0,10,41,42,42,43,255,0,14,1,10,255,0,10,48,0,0,44,255,0,14,1,10,255,0,10
        .byte 48,0,0,44,255,0,9,27,28,6,6,6,1,10,255,0,10,47,46,46,45,255,0,9,27,28,0,0,0,1,10,255,0,23
        .byte 27,28,0,0,0,1,10,0,0,255,9,18,0,0,0,27,28,6,6,6,1,10,255,0,23,27,28,0,0,0,1,10,255,0,23
        .byte 27,28,0,0,0,1,10,255,0,23,27,28,0,0,0,1,10,0,0,255,9,18,0,0,0,27,28,6,6,6,1,10,255,0,23
        .byte 27,28,0,0,0,1,10,255,0,6,29,255,33,8,30,255,0,7,27,28,0,0,0,1,10,5,5,255,0,4,34,255,0,8
        .byte 34,255,0,7,27,28,0,0,0,1,10,255,0,6,35,255,0,8,35,255,0,7,27,28,255,0,4,10,255,0,6,34,255,0,8
        .byte 34,255,0,13,255,3,23,255,0,4,3,3,3
        .byte 255,10,24,255,0,4,10,10,2,255,0,8,34,255,0,10,34,255,0,8,1,2,255,0,8,35,255,0,10,35,255,0,8
        .byte 1,2,255,0,8,32,255,33,10,31,255,0,8,1,2,6,6,255,0,23,6,6,6,1,2,255,0,28,1,2,255,0,4
        .byte 255,11,5,0,0,0,255,11,4,0,0,0,255,11,4,255,0,5,1,2,255,0,28,1,2,6,6,255,0,24,6,6,1
        .byte 2,255,0,26,89,0,1,2,255,0,4,255,11,5,0,0,0,255,11,4,0,0,0,255,11,5,255,0,4,1,2,255,0,26
        .byte 88,0,1,2,6,6,255,0,24,6,6,1,2,255,0,28,1,2,255,0,4,255,11,5,0,0,0,255,11,4,0,0,0
        .byte 255,11,5,255,0,4,1,255,0,60,255,3,30
        .byte 255,10,30,2,255,0,5,40,255,0,14,40,0,34,255,0,5,1,2,255,0,5,40,255,0,14,40,0,35,255,0,5,1
        .byte 2,255,38,5,39,255,0,14,61,38,34,255,38,5,1,2,255,0,22,35,255,0,5,1,2,255,0,22,32,255,33,5,1
        .byte 2,255,0,28,1,2,255,0,28,1,2,255,11,9,255,0,4,11,11,11,255,0,4,255,11,8,1,2,255,0,28,1,2
        .byte 255,0,9,27,28,255,0,7,27,28,255,0,8,1,2,255,0,9,27,28,255,0,7,27,28,255,0,8,1,2,255,0,28
        .byte 1,2,0,0,0,49,50,255,9,6,0,0,0,9,0,0,0,255,9,5,49,50,0,0,0,1,2,9,0,0,49,50,27
        .byte 28,255,0,14,27,28,49,50,0,0,9,1,255,0,4,49,50,27,28,255,0,14,27,28,49,50,255,0,8,49,50,27,28
        .byte 0,0,26,26,0,0,0,26,0,0,0,26,26,0,27,28,49,50,255,0,4,255,6,30
        .byte 255,10,30,2,255,0,28,1,2,255,0,23,41,42,42,43,0,1,2,255,0,23,48,0,0,44,0,1,2,255,0,23,48
        .byte 0,0,44,0,1,2,0,0,0,255,3,4,2,3,3,255,0,13,47,46,46,45,0,1,2,255,0,7,2,255,0,20,1
        .byte 2,255,0,7,2,0,0,0,3,3,2,3,3,3,255,0,11,1,2,3,3,255,0,5,2,255,0,5,2,255,0,14,1
        .byte 2,255,0,7,2,0,29,37,30,0,2,255,0,5,3,3,2,255,0,6,1,2,255,0,7,2,0,35,35,35,0,2,255,33,5
        .byte 30,0,2,255,0,6,1,2,0,0,0,3,3,0,0,2,0,34,34,34,0,2,33,33,33,30,0,34,0,2,0,0,0
        .byte 3,3,3,1,2,255,0,7,2,29,37,34,37,30,2,33,30,0,34,0,34,0,2,255,0,6,1,2,255,0,7,2,34
        .byte 0,34,0,34,2,0,34,0,34,0,34,0,2,255,0,6,1,2,3,3,0,0,0,27,28,2,34,0,34,0,34,2,0
        .byte 34,0,34,0,34,0,2,3,3,3,0,0,0,1,255,0,6,27,28,2,35,0,35,0,35,2,0,35,0,35,0,35,0
        .byte 2,255,0,13,27,28,2,34,26,34,26,34,2,26,34,26,34,26,34,26,2,255,0,7,255,9,30
        .byte 255,10,30,2,255,0,5,34,0,0,0,40,255,0,8,40,0,0,0,34,255,0,5,1,2,255,38,5,35,38,38,38,39
        .byte 255,0,8,61,38,38,38,35,255,38,5,1,2,255,0,5,34,255,0,16,34,255,0,5,1,2,255,33,5,31,255,0,16
        .byte 32,33,30,0,0,0,1,2,255,0,24,34,0,0,0,1,2,255,0,24,32,33,33,33,1,2,255,0,28,1,2,255,0,28
        .byte 1,2,255,0,28,1,2,255,0,12,49,50,0,0,0,49,50,255,0,9,1,2,255,0,12,49,50,0,0,0,49,50,255,0,9
        .byte 1,2,255,0,8,58,59,58,59,58,59,58,59,58,59,58,59,58,59,58,255,0,5,1,2,255,0,8,54,60,0,60,0
        .byte 60,0,60,0,60,0,60,0,60,54,255,0,5,1,2,0,0,0,6,6,27,28,255,6,17,1,0,0,0,1,255,0,6
        .byte 27,28,255,0,17,1,255,0,10,27,28,255,0,17,1,255,0,4,255,9,30
        .byte 255,10,23,255,0,4,10,10,10,2,0,34,255,0,26,10,2,0,35,255,0,8,41,42,42,43,255,0,14,10,2,33,31
        .byte 255,0,8,48,0,0,44,255,0,14,10,2,255,0,10,48,0,0,44,255,0,9,27,28,9,9,9,10,2,255,0,10,47
        .byte 46,46,45,255,0,9,27,28,0,0,0,10,2,255,0,23,27,28,0,0,0,10,2,0,0,0,255,9,17,0,0,0,27
        .byte 28,9,9,9,10,2,255,0,23,27,28,0,0,0,10,2,9,9,255,0,19,9,9,27,28,0,0,0,10,2,255,0,23
        .byte 27,28,0,0,0,10,2,0,0,0,255,9,17,0,0,0,27,28,9,9,9,10,2,255,0,23,27,28,0,0,0,10,2
        .byte 255,0,23,27,28,0,0,0,10,2,9,9,255,0,21,27,28,0,89,0,10,255,0,24,27,28,0,0,0,10,255,0,24
        .byte 27,28,0,88,0,10,255,3,23,255,0,4,3,3,3
        .byte 255,10,5,34,10,34,10,10,10,40,255,10,11,255,0,4,10,10,18,10,255,38,4,35,38,35,38,38,38,39,255,0,17
        .byte 18,10,255,0,4,35,0,35,255,0,21,18,10,255,0,4,34,0,34,0,41,42,42,43,255,0,16,18,255,10,7,35,0
        .byte 48,0,0,44,255,0,7,55,255,56,6,27,28,18,30,255,33,5,10,35,0,48,0,0,44,255,0,14,27,28,18,31,255,33,5
        .byte 10,34,0,47,46,46,45,255,0,4,55,57,255,0,8,27,28,18,30,255,33,5,10,35,255,0,19,27,28,18,31,255,33,5
        .byte 10,35,255,0,19,27,28,18,30,255,33,5,10,34,255,0,11,55,57,255,0,6,27,28,18,31,255,33,5,10,35,255,0,21
        .byte 18,30,255,33,5,10,35,255,0,21,18,31,255,33,5,10,31,255,0,12,55,255,56,4,57,0,0,0,18,255,10,7,255,0,5
        .byte 51,52,255,0,15,18,10,255,0,11,53,54,0,89,0,0,21,23,255,0,8,55,18,10,0,49,50,0,49,50,0,49,50
        .byte 0,51,52,51,52,255,0,15,10,0,49,50,0,49,50,0,49,50,0,53,54,53,54,88,255,0,14,10,255,7,16,255,0,4
        .byte 255,7,9
        .byte 255,10,30,18,255,0,28,17,18,255,0,28,17,18,255,0,28,17,18,255,0,28,17,18,255,0,7,49,50,255,0,10,51
        .byte 52,255,0,7,17,18,255,0,7,49,50,255,0,10,53,54,255,0,7,17,18,255,0,6,49,50,49,50,255,0,8,51,52
        .byte 51,52,255,0,6,17,18,255,0,6,49,50,49,50,255,0,8,53,54,53,54,255,0,6,17,18,27,28,14,255,15,8,16
        .byte 255,0,4,14,255,15,8,16,27,28,17,18,27,28,255,0,24,27,28,17,18,27,28,255,0,24,27,28,17,18,27,28,255,0,24
        .byte 27,28,17,18,27,28,255,0,24,27,28,17,18,14,255,15,4,16,27,28,0,0,0,14,255,15,4,16,0,0,0,27,28
        .byte 14,255,15,4,16,17,255,0,7,27,28,255,0,12,27,28,255,0,37,255,7,30
        .byte 255,10,30,18,255,0,29,18,255,0,29,18,255,0,29,18,255,0,22,14,255,15,6,18,255,0,14,14,255,15,5,16,255,0,7
        .byte 17,18,255,0,28,17,18,255,0,7,14,255,15,5,16,255,0,14,17,18,255,0,28,17,18,14,27,28,15,16,255,0,23
        .byte 17,18,0,27,28,255,0,24,14,17,18,0,27,28,255,0,25,17,18,255,0,8,21,255,22,16,23,0,0,17,18,14,15,15,15
        .byte 16,255,0,8,61,38,38,39,255,0,9,27,28,17,18,255,0,14,21,23,255,0,7,89,0,0,27,28,17,255,0,27,27
        .byte 28,17,255,0,24,88,0,0,27,28,17,255,7,14,255,0,4,255,7,12
        .byte 255,10,30,255,0,29,17,255,0,29,17,255,0,29,17,255,22,23,23,255,0,5,17,18,255,0,28,17,18,255,0,25,55
        .byte 56,56,17,18,255,0,5,49,50,255,0,5,41,42,42,43,255,0,12,17,18,255,0,5,49,50,255,0,5,48,0,0,44
        .byte 255,0,12,17,18,255,0,4,49,50,49,50,255,0,4,48,0,0,44,255,0,7,55,255,56,4,17,18,255,0,4,49,50
        .byte 49,50,255,0,4,47,46,46,45,255,0,12,17,18,0,0,0,49,50,49,50,49,50,255,0,19,17,18,0,0,0,49,50
        .byte 49,50,49,50,255,0,13,55,255,56,5,17,18,0,0,49,50,49,50,49,50,49,50,255,0,18,17,18,0,0,49,50,49
        .byte 50,49,50,49,50,255,0,6,55,56,56,57,255,0,8,17,18,0,58,59,255,58,6,59,58,255,0,18,18,0,26,60,255,0,6
        .byte 60,26,255,0,18,255,7,30
        .byte 255,10,30,18,255,0,4,255,37,4,255,0,12,255,37,4,255,0,4,17,18,255,0,4,255,35,4,255,0,12,255,35,4
        .byte 255,0,4,17,18,255,0,4,255,34,4,255,0,12,255,34,4,255,0,4,17,18,255,33,4,31,34,34,34,255,0,12,34,34,34
        .byte 32,255,33,4,17,18,255,0,5,34,34,34,255,0,12,34,34,34,255,0,5,17,18,255,33,5,31,35,35,255,0,12,35,35
        .byte 32,255,33,5,17,18,255,0,6,34,34,255,0,12,34,34,255,0,6,17,18,255,33,6,31,34,255,0,12,34,32,255,33,6
        .byte 17,18,255,0,7,34,255,0,12,34,255,0,7,17,18,255,33,7,31,255,0,12,32,255,33,7,17,18,255,0,28,17,18
        .byte 255,0,28,17,18,255,0,28,17,18,255,0,28,17,255,0,8,58,59,255,58,10,59,58,255,0,15,26,26,60,255,0,10
        .byte 60,26,26,255,0,7,255,7,30
        .byte 255,10,23,255,0,4,10,10,10,18,255,0,28,17,18,255,0,27,89,17,18,255,0,28,17,18,255,0,27,88,17,18,255,0,23
        .byte 14,255,15,4,17,18,255,0,5,27,28,21,255,22,10,23,27,28,255,0,7,17,18,255,0,5,27,28,0,36,255,0,8
        .byte 36,0,27,28,255,0,7,17,18,255,0,5,27,28,0,36,255,0,8,36,0,27,28,255,0,7,17,18,255,0,5,27,28
        .byte 0,36,255,0,8,36,0,27,28,255,0,7,17,18,255,0,5,27,28,0,36,255,0,8,36,0,27,28,255,0,7,17,18
        .byte 0,0,0,49,50,21,255,22,16,23,49,50,0,0,0,17,18,0,0,0,49,50,255,0,18,49,50,0,0,0,17,18,0,0,0
        .byte 49,50,255,0,18,49,50,0,0,0,17,18,16,0,0,49,50,255,0,18,49,50,0,0,14,17,255,0,4,51,52,0,0
        .byte 51,52,0,51,52,0,51,52,0,51,52,0,51,52,0,0,51,52,255,0,8,53,54,0,0,53,54,0,53,54,0,53,54
        .byte 0,53,54,0,53,54,0,0,53,54,255,0,4,255,7,30
        .byte 255,10,16,30,255,0,13,18,255,0,14,32,255,10,4,30,255,0,9,18,255,0,18,32,10,10,10,30,255,0,6,18,255,0,21
        .byte 32,10,10,30,255,0,4,18,255,0,21,255,10,4,30,0,0,0,18,255,0,21,10,62,63,64,65,0,0,0,18,255,0,21
        .byte 10,66,67,68,69,0,0,0,18,255,0,21,255,10,6,30,0,18,255,0,26,32,10,0,18,255,0,27,10,0,18,255,0,27
        .byte 10,30,18,255,0,27,32,10,18,255,0,28,10,18,255,0,28,10,18,255,0,22,85,86,86,87,0,0,10,255,0,29,10
        .byte 255,0,29,255,10,24,255,12,4,10,10,10
        .byte 255,10,17,255,0,4,255,10,10,255,0,28,17,10,255,0,28,17,10,255,0,28,17,10,255,0,28,17,10,255,0,15,14
        .byte 255,15,4,16,255,0,7,17,10,255,0,28,17,10,255,15,12,16,255,0,10,14,255,15,4,17,10,255,0,28,17,10,255,0,28
        .byte 17,10,255,0,14,14,255,15,6,16,255,0,6,17,10,255,0,28,17,10,255,0,28,17,10,255,15,12,16,255,0,10,14
        .byte 255,15,4,17,10,0,49,50,0,49,50,0,49,50,0,49,50,255,0,17,10,0,49,50,0,49,50,0,49,50,0,49,50
        .byte 255,0,17,10,58,59,255,58,9,59,58,255,0,16,10,255,15,29
        .byte 255,10,30,18,255,0,5,40,0,34,0,34,0,34,0,34,255,0,14,18,18,0,41,42,42,43,40,0,35,0,35,0,35
        .byte 0,35,255,0,9,41,42,42,43,0,18,18,0,48,0,0,44,40,0,34,0,34,0,34,0,34,255,0,9,48,0,0,44
        .byte 0,18,18,0,48,0,0,44,40,0,35,0,35,0,35,0,35,255,0,9,48,0,0,44,0,18,18,0,47,46,46,45,40
        .byte 0,32,255,33,13,30,0,47,46,46,45,0,18,18,255,38,5,39,255,0,15,34,255,0,6,18,18,255,0,21,35,255,0,6
        .byte 18,18,255,0,21,34,255,0,6,18,18,255,0,5,49,50,255,0,14,32,33,33,37,33,33,33,18,18,255,0,5,49,50
        .byte 255,0,17,34,0,0,0,18,18,255,0,4,49,50,49,50,255,0,16,35,0,0,0,18,18,255,0,4,49,50,49,50,255,0,16
        .byte 34,0,0,0,18,18,0,0,0,49,50,49,50,49,50,255,0,15,32,33,33,33,18,18,0,0,0,49,50,49,50,49,50
        .byte 255,0,19,18,0,0,0,49,50,49,50,49,50,49,50,255,0,22,49,50,49,50,49,50,49,50,255,0,19,255,15,30
        .byte 255,22,13,23,255,0,4,21,255,22,11,18,0,0,34,255,0,25,18,18,33,33,31,255,0,25,18,18,255,0,28,18,17
        .byte 0,0,0,21,255,22,20,23,0,27,28,18,18,255,0,6,34,255,0,15,34,0,0,0,27,28,18,18,255,0,6,34,255,0,15
        .byte 34,0,0,0,27,28,18,18,16,255,0,5,35,255,0,15,35,0,0,0,27,28,18,18,255,0,6,34,255,0,15,34,0,0,0
        .byte 27,28,18,18,0,0,0,14,255,15,20,16,0,27,28,18,18,255,0,6,34,255,0,15,34,0,0,0,27,28,18,18,255,0,6
        .byte 35,255,0,15,35,0,0,0,27,28,18,18,16,255,0,5,34,255,0,15,34,0,0,0,27,28,18,18,255,0,6,34,255,0,15
        .byte 34,0,0,0,27,28,18,18,0,0,0,14,255,15,20,16,0,27,28,18,255,0,27,27,28,18,255,0,27,27,28,18,255,15,30
numsc:
        .byte 22
nmedat:
        .byte 0,1,24,136,255
        .byte 0,1,128,224,1,3,96,160,6,14,24,224,1,7,64,184,255
        .byte 0,0,128,16,6,14,56,224,1,8,24,160,1,8,88,64,255
        .byte 0,0,24,88,6,14,48,16,1,3,72,80,1,8,128,80,255
        .byte 0,0,72,16,6,14,128,16,2,5,40,112,2,4,64,152,2,9,56,200,7,11,128,80,5,13,32,32,5,13,32,72,255
        .byte 0,0,128,16,6,14,40,16,2,2,56,88,2,2,64,168,2,2,96,144,255
        .byte 0,1,128,224,5,13,32,104,1,3,48,48,1,7,80,64,1,8,128,80,255
        .byte 0,0,128,16,6,14,24,16,2,5,24,32,2,5,48,88,2,5,80,152,7,11,128,72,7,11,128,168,255
        .byte 0,0,128,16,6,14,40,120,1,8,56,16,1,8,56,184,2,5,72,104,2,5,72,144,255
        .byte 0,0,128,16,5,13,32,208,6,14,16,16,2,5,16,152,255
        .byte 0,0,128,16,6,14,128,192,2,5,40,88,2,5,72,176,7,11,128,72,255
        .byte 0,1,24,200,5,13,32,104,6,14,40,16,1,8,48,40,1,8,80,56,1,8,128,72,255
        .byte 0,1,24,200,5,13,40,88,2,9,24,128,255
        .byte 0,0,128,16,6,14,16,40,2,5,72,120,7,11,128,120,255
        .byte 0,0,128,16,6,14,48,224,1,7,88,80,2,5,24,56,255
        .byte 0,1,128,224,5,13,72,120,6,14,112,16,2,9,104,96,2,5,96,152,7,11,128,152,255
        .byte 0,1,128,224,6,14,112,120,2,5,104,88,2,5,104,152,2,4,48,104,2,4,48,136,255
        .byte 0,1,24,200,6,14,80,112,1,8,40,128,1,7,80,72,255
        .byte 0,0,128,16,8,12,128,200,3,2,24,16,3,6,32,80,3,10,48,152,255
        .byte 0,0,24,152,1,3,128,128,1,8,48,40,6,14,96,16,255
        .byte 0,0,128,16,5,13,32,32,5,13,32,208,2,5,64,136,2,5,104,160,3,10,24,160,6,14,64,56,255
        .byte 0,0,128,16,1,7,104,112,1,7,64,120,6,14,24,224,255
NUMOBJ = 24
objdta:
        .byte 31,248,32,4,42,84,42,84,32,4,47,244,40,20,40,20,40,20,40,20,40,20,40,20,47,244,32,4,16,8,15,240,254,128,64,254,128,64
        .byte 31,248,32,4,42,84,42,84,32,4,35,196,36,36,40,20,40,20,40,20,40,20,36,36,35,196,32,4,16,8,15,240,254,8,8,254,8,8
        .byte 31,248,32,4,42,84,42,84,32,4,33,132,34,68,34,68,36,36,36,36,40,20,40,20,47,244,32,4,16,8,15,240,254,8,8,254,8,8
        .byte 31,248,32,4,42,84,42,84,32,4,35,196,34,68,46,116,40,20,40,20,46,116,34,68,35,196,32,4,16,8,15,240,254,8,8,254,8,8
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,1,128,160,1,128,160
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,2,56,96,2,56,96
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,3,128,56,3,128,56
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,4,128,200,4,128,200
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,5,128,64,5,128,64
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,6,128,128,6,128,128
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,7,88,16,7,88,16
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,8,96,120,8,96,120
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,9,48,136,9,48,136
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,10,104,56,10,104,56
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,11,80,224,11,80,224
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,12,104,152,12,104,152
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,13,104,200,13,104,200
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,14,128,168,14,128,168
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,15,88,200,15,88,200
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,16,128,24,16,128,24
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,17,80,192,17,80,192
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,19,128,152,19,128,152
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,20,128,96,20,128,96
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,13,80,10,176,13,80,15,240,4,32,7,224,21,128,208,21,128,208
font:
        .byte 121,62,50,71,205,236,126,205
        .byte 24,129,205,213,220,201,48,0
        .byte 0,108,108,0,0,0,0,0
        .byte 0,108,254,108,108,254,108,0
        .byte 0,24,126,120,126,30,126,24
        .byte 0,230,236,24,48,110,206,0
        .byte 0,48,120,48,126,204,126,0
        .byte 0,24,48,0,0,0,0,0
        .byte 0,12,24,24,24,24,12,0
        .byte 0,96,48,48,48,48,96,0
        .byte 0,0,60,24,126,24,60,0
        .byte 0,0,24,24,126,24,24,0
        .byte 0,0,0,0,0,24,24,48
        .byte 0,0,0,0,126,0,0,0
        .byte 0,0,0,0,0,56,56,0
        .byte 0,0,6,12,24,48,96,0
        .byte 0,124,206,222,246,230,124,0
        .byte 0,56,120,24,24,24,126,0
        .byte 0,124,198,6,124,192,254,0
        .byte 0,124,198,28,6,198,124,0
        .byte 0,24,56,120,216,254,24,0
        .byte 0,254,192,252,6,198,124,0
        .byte 0,124,192,252,198,198,124,0
        .byte 0,254,6,12,24,48,48,0
        .byte 0,124,198,124,198,198,124,0
        .byte 0,124,198,198,126,6,124,0
        .byte 0,0,0,48,0,0,48,0
        .byte 0,48,48,48,48,0,48,0
        .byte 0,0,12,24,48,24,12,0
        .byte 0,0,0,126,0,126,0,0
        .byte 0,0,48,24,12,24,48,0
        .byte 0,124,198,12,24,0,24,0
        .byte 0,124,222,254,254,192,124,0
        .byte 0,252,198,198,254,198,198,0
        .byte 0,252,198,252,198,198,254,0
        .byte 0,126,198,192,192,198,126,0
        .byte 0,248,204,198,198,198,254,0
        .byte 0,254,192,252,192,192,254,0
        .byte 0,254,192,252,192,192,192,0
        .byte 0,126,198,192,222,198,126,0
        .byte 0,198,198,254,198,198,198,0
        .byte 0,126,24,24,24,24,126,0
        .byte 0,6,6,6,198,198,252,0
        .byte 0,198,204,248,222,198,198,0
        .byte 0,192,192,192,192,192,254,0
        .byte 0,198,238,254,198,198,198,0
        .byte 0,198,230,246,222,206,198,0
        .byte 0,126,198,198,198,198,124,0
        .byte 0,252,198,198,254,192,192,0
        .byte 0,126,198,198,246,222,124,0
        .byte 0,254,198,198,252,204,198,0
        .byte 0,252,192,254,6,198,254,0
        .byte 0,254,48,48,48,48,48,0
        .byte 0,198,198,198,198,198,126,0
        .byte 0,198,198,198,198,108,56,0
        .byte 0,198,198,198,198,254,110,0
        .byte 0,198,108,56,56,108,198,0
        .byte 0,134,204,120,48,48,48,0
        .byte 0,254,12,24,48,96,254,0
        .byte 0,30,24,24,24,24,30,0
        .byte 0,0,192,96,48,24,12,0
        .byte 0,240,48,48,48,48,240,0
        .byte 0,48,120,252,48,48,48,0
        .byte 0,0,0,0,0,0,0,0
        .byte 0,60,102,248,96,96,254,0
        .byte 0,0,120,12,124,204,124,0
        .byte 0,96,96,124,102,102,124,0
        .byte 0,0,60,96,96,96,60,0
        .byte 0,12,12,124,204,204,124,0
        .byte 0,0,120,204,248,192,124,0
        .byte 0,28,48,56,48,48,48,0
        .byte 0,0,124,204,204,124,12,120
        .byte 0,192,192,248,204,204,204,0
        .byte 0,48,0,112,48,48,120,0
        .byte 0,12,0,12,12,12,108,56
        .byte 0,96,120,112,112,120,108,0
        .byte 0,48,48,48,48,48,28,0
        .byte 0,0,248,252,252,252,252,0
        .byte 0,0,248,204,204,204,204,0
        .byte 0,0,120,204,204,204,120,0
        .byte 0,0,248,204,204,248,192,192
        .byte 0,0,124,204,204,124,12,14
        .byte 0,0,60,96,96,96,96,0
        .byte 0,0,120,192,120,12,248,0
        .byte 0,48,120,48,48,48,28,0
        .byte 0,0,204,204,204,204,120,0
        .byte 0,0,204,204,120,120,48,0
        .byte 0,0,204,252,252,252,120,0
        .byte 0,0,204,120,48,120,204,0
        .byte 0,0,204,204,204,124,12,120
        .byte 0,0,252,24,48,96,252,0
        .byte 0,30,24,112,24,24,30,0
        .byte 0,24,24,24,24,24,24,0
        .byte 0,240,48,28,48,48,240,0
        .byte 0,60,120,0,0,0,0,0
        .byte 124,198,187,227,227,187,198,124
jtab:
        .byte 248,250,252,254,254,255,255,255,0,0,0,1,1,1,2,2,4,6,8,8,8,99
keys:   .byte 66,97,70,54,55,48,49,48,49,17,18
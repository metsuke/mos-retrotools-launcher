;----------------------------------------------
; BBC AGD Engine
; Z80 conversion by Kees van Oss 2017
; BBC Micro version by Kieran Connell 2018
;----------------------------------------------

;----------------------------------------------------------------------
;BBC configuration

swrflag	= 1				; SW RAM
scrchar = 32				; chars/line

;----------------------------------------------------------------------

.DEFINE asm_code 	$0e00		; assembly address _BEEB
.DEFINE load_address 	$1200		; load address _BEEB

.if swrflag
	.DEFINE data_address $8000	; data address _SWRAM
.endif
	.include "game.cfg" 

;----------------------------------------------------------------------
; BBC MICRO PLATFORM DEFINES
;----------------------------------------------------------------------

; _BEEB MOS calls

	OSBYTE	 = $fff4
	OSFILE	 = $ffdd
	OSWRCH	 = $ffee
	OSASCI	 = $ffe3
	OSWORD	 = $fff1
	OSFIND	 = $ffce
	OSGBPB	 = $ffd1
	OSARGS	 = $ffda

	EVENTV	 = $0220

	PAL_black = 0 ^ 7
	PAL_white = 7 ^ 7

; System constants

	ScreenSize	= scrchar*24*8		; Startaddress video RAM _BEEB
	ScreenAddr 	= $8000 - ScreenSize	; Screen size bytes _BEEB
	ScreenRowBytes	= scrchar*8		; columns
	SpriteMaxY	= 177			; used for clipping bottom of screen

; AGD Engine Workspace

	MAP 		= $300				; properties map buffer (3x256 bytes)
	SCADTB_lb	= MAP + $300
	SCADTB_hb	= SCADTB_lb + $100
.if pflag
    SHRAPN 		= $B00 - (NUMSHR * SHRSIZ)	; shrapnel table (55x6 bytes)
.endif
	sprtab		= $B00				; NUMSPR*TABSIZ

;----------------------------------------------------------------------
; ZERO PAGE SEGMENT
;----------------------------------------------------------------------

.segment "ZEROPAGE"

.include "z80-zp.inc"
.include "engine-zp.inc"

;----------------------------------------------------------------------
; ZCODE SEGMENT
;----------------------------------------------------------------------

.segment "CODE"
.org asm_code

start_asm:

	jmp relocate + load_address - asm_code

boot_game:

; Zero ZP vars

clear_zp:
	ldx #0
	txa
	:
	sta $00, x
	inx
	cpx #$a0
	bne :-

	; Init non-zero vars
	lda #3
	sta numlif

	ldx #255
	stx varrnd
	stx varopt
	stx varblk
	dex
	stx varobj

	jsr bbcinit

	; Call AGD Engine start game
	jsr start_game

	jsr bbckill

    ; Wait for keypress
	ldx #$ff
	ldy #$7f
	lda #$81
	jsr OSBYTE

	; Restart or exit
	jmp boot_game

;----------------------------------------------------------------------
; PLATFORM SPECIFIC ENGINE CODE
;----------------------------------------------------------------------

	.include "z80.asm"
	.include "bbc.inc"

;----------------------------------------------------------------------
; AGD 6502 ENGINE CODE + COMPILED GAME SCRIPT
;----------------------------------------------------------------------

start_game:

	.include "game.inc"

end_asm:

;----------------------------------------------------------------------
; RELOCATION OF BEEB CODE FROM LOAD ADDRESS
;----------------------------------------------------------------------

relocate:
; Issue *TAPE otherwise DFS goes mental that we've overwritten workspace from &E00 - &1100

    lda #$8C
    ldx #$0C
    ldy #$00
    jsr OSBYTE					; *FX &8C,0,0 - *TAPE 1200

	sei
	lda #$7f
	sta $fe4e					; disable all interupts
	lda #$82
	sta $fe4e					; enable vsync interupt only
	cli









.if swrflag
	lda #4						; select SWRAM
	sta $f4
	sta $fe30
.endif





; Other one off initialisation could happen here...

; Relocate all code down to &E00
.if swrflag
	ldx #>(data_start - start_asm) + 1
.else
	ldx #>(end_asm - start_asm) + 1
.endif
	ldy #0
reloop:
	lda load_address, y
	sta asm_code, y
	iny
	bne reloop
	inc reloop + 2 + load_address - asm_code
	inc reloop + 5 + load_address - asm_code
	dex
	bne reloop

.if swrflag
; Relocate all data up to &8000
	ldx #>(end_asm - data_start) + 1
	ldy #0
uploop:
	lda data_start + load_address - asm_code, y
	sta data_address, y
	iny
	bne uploop
	inc uploop + 2 + load_address - asm_code
	inc uploop + 5 + load_address - asm_code
	dex
	bne uploop
.endif

	jmp boot_game





.if swrflag
	.out "- SWRAM mode enabled"
	.out "- Memory:"
	.out .sprintf("   SWRAM : max %6d bytes, used %6d bytes, free %6d bytes", ($c000-$8000)			 , (end_asm - data_start)	,($c000-$8000)-(end_asm - data_start))
	.out .sprintf("   CODE  : max %6d bytes, used %6d bytes, free %6d bytes", (ScreenAddr-$e00)		 , (data_start-$e00)		,(ScreenAddr-data_start))
	.out .sprintf("   TOTAL : max %6d bytes, used %6d bytes, free %6d bytes", ($c000-$8000)+(ScreenAddr-$e00), (end_asm - $e00)		,($c000-$8000)-(end_asm - data_start)+(ScreenAddr-data_start))
.else
	.out "- Memory:"
	.out .sprintf("   CODE  : max %6d bytes, used %6d bytes, free %6d bytes", (ScreenAddr-$e00), (end_asm - start_asm),(ScreenAddr-$e00)-(end_asm - start_asm))
.endif
	.out .sprintf("")
eop:					; End Of Program

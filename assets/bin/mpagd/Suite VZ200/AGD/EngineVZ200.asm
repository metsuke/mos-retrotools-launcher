g; Game engine code --------------------------------------------------------------
;
; Arcade Game Designer.
; (C) 2008 - 2019 Jonathan Cauldwell.
; VZ200 Engine b1.8

; VZ200 specifics --------------------------------------------------------------

; Standard memory usage in MC-1000 with 6KB RAM:
; $0000   $4000   $6800   $7000   $7800 $7AE9  $9000   $ffff
;   |       |       |       |       |      |     |       |
;   XXXXXXXX                                           		16Kb BASIC ROMS
;	    xxxxxxxx						10Kb Reserved for ROM Cartridges 
;		    xxxxxxxx					Keyboard, Cassette, Speaker
;			    xxxxxxxx				2Kb Video RAM
;				    xxxxxxxxxxxxx		6Kb User RAM
;						 xxxxxxxxx  	16Kb Memory expansion module area
;					   				
;           				   XXX-->          	BASIC program + variables.
; 

;----------------------------------------------------------------------------
; Flag options
;
; Change these settings to match the hardware settings in VZem
;----------------------------------------------------------------------------

RFLAG = 1			; 16KB RAM expansion
CFLAG = 0			; Colourset 0/1

;----------------------------------------------------------------------------

IF IFLAG = 1
FILL	equ 255
ELSE
FILL	equ 0
ENDIF

IF CFLAG = 1
COL_OR	equ 8+16
COL_AND	equ $ff
ELSE
COL_OR	equ 8
COL_AND	equ $ef
ENDIF

IF XFLAG = 1
GFX_MOD	equ 2
ELSE
GFX_MOD	equ 0
ENDIF

IF MFLAG = 1
obj_len = 16
ELSE
obj_len = 32
ENDIF


; Global definitions ------------------------------------------------------------

; System addresses.

VRAM	equ $7000           	; Start of video display RAM
ROM	equ $0000           	; start of 16KB ROM.
KEY	equ $687F            	; Memory location to begin keyboard scan
MODE	equ $6800           	; set video mode (MC6847 mode pins):
                           	; - bit 3 = 0: Text-only mode
                           	; - bit 3 = 1: Mode(1) Graphics-only hires mode

RAMTOP	equ $B800 + RFLAG * 16384	; in unexpanded VZ300.
MAP	equ RAMTOP-32*SCRHGT	; properties map buffer. 32*8 = 256, fits exactly into the strings+user areas.
STACK	equ MAP-256		; Set top 256 bytes for Z80 stack.

; Block characteristics.

PLATFM	equ 1			; platform.
WALL	equ PLATFM + 1		; solid wall.
LADDER	equ WALL + 1		; ladder.
FODDER	equ LADDER + 1		; fodder block.
DEADLY	equ FODDER + 1		; deadly block.
CUSTOM	equ DEADLY + 1		; custom block.
WATER	equ CUSTOM + 1		; water block.
COLECT	equ WATER + 1		; collectable block.
NUMTYP	equ COLECT + 1		; number of types.

; Sprites.

NUMSPR	equ 12			; number of sprites.
TABSIZ	equ 17			; size of each entry.
SPRBUF	equ NUMSPR * TABSIZ	; size of entire table.
NMESIZ	equ 4			; bytes stored in nmetab for each sprite.
X	equ 8			; new x coordinate of sprite.
Y	equ X + 1		; new y coordinate of sprite.
PAM1ST	equ 5			; first sprite parameter, old x (ix+5).

IF MFLAG = 1
COLDIST	equ 7			; collision distance
ELSE
COLDIST	equ 15			; collision distance
ENDIF

; Particle engine.

NUMSHR	equ 55			; pieces of shrapnel.
SHRSIZ	equ 6			; bytes per particle.
SHRAPN	equ STACK-NUMSHR*SHRSIZ	; shrapnel table, just below stack area.

;----------------------------------------------------------------------------
; Game starts here.
;----------------------------------------------------------------------------

start	equ $
	di
	ld hl,$7841		; Cursor flashrate
	ld (hl),0

	ld sp,STACK+256		; Set Stackpointer

; Set interrupt flag

	ld hl,$787d		; redirect interrupts to counting routine.
	ld bc,cntint		; address of interrupt counting routine.
	ld (hl),195		; JP opcode
	inc hl
	ld (hl),c		; lb irqroutine
	inc hl
	ld (hl),b		; hb irqroutine
	ei

IF XFLAG = 1
	ld a,(XBANK)
	out (XPORT),a
ENDIF

IF XFLAG = 2
	ld hl,XBANK
	ld a,(hl)
	add a,XMODE
	out (XPORT),a
ENDIF

	ld a,(MODE)
	or COL_OR+GFX_MOD	; select graphic mode 
	and COL_AND		; select background colour	

IF XFLAG = 1
	ld (30779),a
ENDIF

	ld (MODE),a		; enable MODE(1)
reboot
	call cls		; clear screen.
	call game		; start the game.
	jr reboot		; keep looping

joyval	defb 0			; joystick reading.
frmno	defb 0			; selected frame.
XBANK	defb 0			; Extended grahics mod bank

cntint
	push hl
	ld hl,clock         ; game frame.
	inc (hl)            ; advance the frame.
	pop hl
	pop hl
	jp $2ed5


	ret                ; return from interrupt.

;----------------------------------------------------------------------------
; Variables
;----------------------------------------------------------------------------

; Don't change the order of these four.  Menu routine relies on winlft following wintop.

wintop defb WINDOWTOP      ; top of window.
winlft defb WINDOWLFT      ; left edge.
winhgt defb WINDOWHGT      ; window height.
winwid defb WINDOWWID      ; window width.

numob  defb NUMOBJ         ; number of objects in game.

; Variables start here.
; Pixel versions of wintop, winlft, winhgt, winwid.

wntopx defb (8 * WINDOWTOP)
wnlftx defb (8 * WINDOWLFT)
wnbotx defb ((WINDOWTOP * 8) + (WINDOWHGT * 8) - 16)
wnrgtx defb ((WINDOWLFT * 8) + (WINDOWWID * 8) - 16)-2

scno   defb 0              ; present screen number.
numlif defb 3              ; number of lives.
vara   defb 0              ; general-purpose variable.
varb   defb 0              ; general-purpose variable.
varc   defb 0              ; general-purpose variable.
vard   defb 0              ; general-purpose variable.
vare   defb 0              ; general-purpose variable.
varf   defb 0              ; general-purpose variable.
varg   defb 0              ; general-purpose variable.
varh   defb 0              ; general-purpose variable.
vari   defb 0              ; general-purpose variable.
varj   defb 0              ; general-purpose variable.
vark   defb 0              ; general-purpose variable.
varl   defb 0              ; general-purpose variable.
varm   defb 0              ; general-purpose variable.
varn   defb 0              ; general-purpose variable.
varo   defb 0              ; general-purpose variable.
varp   defb 0              ; general-purpose variable.
varq   defb 0              ; general-purpose variable.
varr   defb 0              ; general-purpose variable.
vars   defb 0              ; general-purpose variable.
vart   defb 0              ; general-purpose variable.
varu   defb 0              ; general-purpose variable.
varv   defb 0              ; general-purpose variable.
varw   defb 0              ; general-purpose variable.
varz   defb 0              ; general-purpose variable.
contrl defb 0              ; control: 0 = keyboard, 1 = Joystick A, 2 = Joystick B.
charx  defb 0              ; cursor x position.
chary  defb 0              ; cursor y position.
clock  defb 0              ; last clock reading.
varrnd defb 255            ; last random number.
varobj defb 254            ; last object number.
varopt defb 255            ; last option chosen from menu.
varblk defb 255            ; block type.
nexlev defb 0              ; next level flag.
restfl defb 0              ; restart screen flag.
deadf  defb 0              ; dead flag.
gamwon defb 0              ; game won flag.
dispx  defb 0              ; cursor x position.
dispy  defb 0              ; cursor y position.
loopa  defb 0              ; loop counter system variable.
loopb  defb 0              ; loop counter system variable.
loopc  defb 0              ; loop counter system variable.

; Make sure the next two variables appear together in this exact order.
bgmask defb %00000000      ; background color mask. [00|00|00|00]
fgmask defb %11111111      ; foreground color mask. [11|11|11|11]

; Make sure pointers are arranged in the same order as the data itself.

frmptr defw frmlst         ; sprite frames.
blkptr defw chgfx          ; block graphics.
;<zx>
colptr defw bcol           ; address of char colours.
;</zx>
proptr defw bprop          ; address of char properties.
scrptr defw scdat          ; address of screens.
nmeptr defw nmedat         ; enemy start positions.

;----------------------------------------------------------------------------
; Assorted game routines which can go in contended memory.
;----------------------------------------------------------------------------

; Modify for inventory.

minve  ld hl,invdis        ; routine address.
       ld (mod0+1),hl      ; set up menu routine.
       ld (mod2+1),hl      ; set up count routine.
       ld hl,fopt          ; find option from available objects.
       ld (mod1+1),hl      ; set up routine.
       jr dbox             ; do menu routine.

; Modify for menu.

mmenu  ld hl,always        ; routine address.
       ld (mod0+1),hl      ; set up routine.
       ld (mod2+1),hl      ; set up count routine.
       ld hl,fstd          ; standard option selection.
       ld (mod1+1),hl      ; set up routine.

; Drop through into box routine.

; Work out size of box for message or menu.

dbox   ld hl,msgdat        ; pointer to messages.
       call getwrd         ; get message number.
       push hl             ; store pointer to message.
       ld d,1              ; height.
       xor a               ; start at object zero.
       ld (combyt),a       ; store number of object in combyt.
       ld e,a              ; maximum width.
dbox5  ld b,0              ; this line's width.
mod2   call always         ; item in player's possession?
       jr nz,dbox6         ; not in inventory, skip this line.
       inc d               ; add to tally.
dbox6  ld a,(hl)           ; get character.
       inc hl              ; next character.
       cp ','              ; reached end of line?
       jr z,dbox3          ; yes.
       cp 13               ; reached end of line?
       jr z,dbox3          ; yes.
       inc b               ; add to this line's width.
       and a               ; end of message?
       jp m,dbox4          ; yes, end count.
       jr dbox6            ; repeat until we find the end.
dbox3  ld a,e              ; maximum line width.
       cp b                ; have we exceeded longest so far?
       jr nc,dbox5         ; no, carry on looking.
       ld e,b              ; make this the widest so far.
       jr dbox5            ; keep looking.
dbox4  ld a,e              ; maximum line width.
       cp b                ; have we exceeded longest so far?
       jr nc,dbox8         ; no, carry on looking.
       ld e,b              ; final line is the longest so far.
dbox8  dec d               ; decrement items found.
       jp z,dbox15         ; total was zero.
       ld a,e              ; longest line.
       and a               ; was it zero?
       jp z,dbox15         ; total was zero.
       ld (bwid),de        ; set up size.

; That's set up our box size.

       ld a,(winhgt)       ; window height in characters.
       sub d               ; subtract height of box.
       rra                 ; divide by 2.
       ld hl,wintop        ; top edge of window.
       add a,(hl)          ; add displacement.
       ld (btop),a         ; set up box top.
       ld a,(winwid)       ; window width in characters.
       sub e               ; subtract box width.
       rra                 ; divide by 2.
       inc hl              ; left edge of window.
       add a,(hl)          ; add displacement.
       ld (blft),a         ; box left.
       ld hl,font-256      ; font.
       ld (grbase),hl      ; set up for text display.
       pop hl              ; restore message pointer.
       ld a,(btop)         ; box top.
       ld (dispx),a        ; set display coordinate.
       xor a               ; start at object zero.
       ld (combyt),a       ; store number of object in combyt.
dbox2  ld a,(combyt)       ; get object number.
mod0   call always         ; check inventory for display.
       jp nz,dbox13        ; not in inventory, skip this line.

       ld a,(blft)         ; box left.
       ld (dispy),a        ; set left display position.
       ld a,(bwid)         ; box width.
       ld b,a              ; store width.
dbox0  ld a,(hl)           ; get character.
       cp ','              ; end of line?
       jr z,dbox1          ; yes, next one.
       cp 13               ; end of option?
       jr z,dbox1          ; yes, on to next.
       dec b               ; one less to display.
       and 127             ; remove terminator.
       push bc             ; store characters remaining.
       push hl             ; store address on stack.

       ; no attribute.

       call pchr           ; display on screen.
       pop hl              ; retrieve address of next character.
       pop bc              ; chars left for this line.
       ld a,(hl)           ; get character.
       inc hl              ; next character.
       cp 128              ; end of message?
       jp nc,dbox7         ; yes, job done.
       ld a,b              ; chars remaining.
       and a               ; are any left?
       jr nz,dbox0         ; yes, continue.

; Reached limit of characters per line.

dbox9  ld a,(hl)           ; get character.
       inc hl              ; next one.
       cp ','              ; another line?
       jr z,dbox10         ; yes, do next line.
       cp 13               ; another line?
       jr z,dbox10         ; yes, on to next.
       cp 128              ; end of message?
       jr nc,dbox11        ; yes, finish message.
       jr dbox9

; Fill box to end of line.

dboxf  push hl             ; store address on stack.
       push bc             ; store characters remaining.

       ; no attribute.

       ld a,32             ; space character.
       call pchr           ; display character.
       pop bc              ; retrieve character count.
       pop hl              ; retrieve address of next character.
       djnz dboxf          ; repeat for remaining chars on line.
       ret
dbox1  inc hl              ; skip character.
       call dboxf          ; fill box out to right side.
dbox10 ld a,(dispx)        ; x coordinate.
       inc a               ; down a line.
       ld (dispx),a        ; next position.
       jp dbox2            ; next line.
dbox7  ld a,b              ; chars remaining.
       and a               ; are any left?
       jr z,dbox11         ; no, nothing to draw.
       call dboxf          ; fill message to line.

; Drawn the box menu, now select option.

dbox11 ld a,(btop)         ; box top.
       ld (dispx),a        ; set bar position.
dbox14 call joykey         ; get controls.
       and %00011111       ; anything pressed?
       jr nz,dbox14        ; yes, debounce it.
       call dbar           ; draw bar.
dbox12 call joykey         ; get controls.
       and %00011100       ; anything pressed? (fire, up or down)
       jr z,dbox12         ; no, nothing.
       and %00010000       ; fire button pressed?
mod1   jp nz,fstd          ; yes, job done.
       call dbar           ; delete bar.
       ld a,(joyval)       ; joystick reading.
       and %00001000       ; going up?
       jr nz,dboxu         ; yes, go up.
       ld a,(dispx)        ; vertical position of bar.
       inc a               ; look down.
       ld hl,btop          ; top of box.
       sub (hl)            ; find distance from top.
       dec hl              ; point to height.
       cp (hl)             ; are we at end?
       jp z,dbox14         ; yes, go no further.
       ld hl,dispx         ; coordinate.
       inc (hl)            ; move bar.
       jr dbox14           ; continue.
dboxu  ld a,(dispx)        ; vertical position of bar.
       ld hl,btop          ; top of box.
       cp (hl)             ; are we at the top?
       jp z,dbox14         ; yes, go no further.
       ld hl,dispx         ; coordinate.
       dec (hl)            ; move bar.
       jr dbox14           ; continue.
fstd   ld a,(dispx)        ; bar position.
       ld hl,btop          ; top of menu.
       sub (hl)            ; find selected option.
       ld (varopt),a       ; store the option.
       jp redraw           ; redraw the screen.

; Option not available.  Skip this line.

dbox13 ld a,(hl)           ; get character.
       inc hl              ; next one.
       cp ','              ; another line?
       jp z,dbox2          ; yes, do next line.
       cp 13               ; another line?
       jp z,dbox2          ; yes, on to next line.
       and a               ; end of message?
       jp m,dbox11         ; yes, finish message.
       jr dbox13
dbox15 pop hl              ; pop message pointer from the stack.
       ret

dbar   ld a,(blft)         ; box left.
       ld (dispy),a        ; set display coordinate.
       call gprad          ; get printing address.
       ex de,hl            ; flip into hl register pair.
       ld a,(bwid)         ; box width.
       ld c,a              ; loop counter in c.

       ld de,32            ; distance to next line.
dbar1  push hl             ; store screen address.
       ld b,8              ; pixel height in b.

dbar0  ld a,(hl)           ; get screen byte.
       cpl                 ; reverse all bits.
       ld (hl),a           ; write back to screen.

       add hl,de           ; next line down.
       djnz dbar0          ; draw rest of character.
       pop hl              ; restore screen address.

       inc l               ; one char right.
       dec c               ; decrement character counter.
       jr nz,dbar1         ; repeat for whole line.
       ret

invdis push hl             ; store message text pointer.
       push de             ; store de pair for line count.
       ld hl,combyt        ; object number.
       ld a,(hl)           ; get object number.
       inc (hl)            ; ready for next one.
       call gotob          ; check if we have object.
       pop de              ; retrieve de pair from stack.
       pop hl              ; retrieve text pointer.
       ret

; Find option selected.

fopt
	call joykey		; check for fire
	and $10
	jr z,fopt
rel
	call joykey		; check release fire
	and $10
	jr nz,rel

       ld a,(dispx)
       ld hl,btop          ; top of menu.
       sub (hl)            ; find selected option.
       inc a               ; object 0 needs one iteration, 1 needs 2 and so on.
       ld b,a              ; option selected in b register.
       ld hl,combyt        ; object number.
       ld (hl),0           ; set to first item.
fopt0  push bc             ; store option counter in b register.
       call fobj           ; find next object in inventory.
       pop bc              ; restore option counter.
       djnz fopt0          ; repeat for relevant steps down the list.
       ld a,(combyt)       ; get option.
       dec a               ; one less, due to where we increment combyt.
       ld (varopt),a       ; store the option.
       jp redraw           ; redraw the screen.

fobj   ld hl,combyt        ; object number.
       ld a,(hl)           ; get object number.
       inc (hl)            ; ready for next item.
       ret z               ; in case we loop back to zero.
       call gotob          ; do we have this item?
       ret z               ; yes, it's on the list.
       jr fobj             ; repeat until we find next item in pockets.

bwid   defb 0              ; box/menu width.
blen   defb 0              ; box/menu height.
btop   defb 0              ; box coordinates.
blft   defb 0

; Detect keypress.
; Note that each key causes a logic 0 to appear at the bit position shown, when its row address is read.

;       I/O Address -----------------------------------------------
;     (Selector)     bit 7 bit 6 bit 5  bit 4  bit 3   bit 2  bit 1  bit 0
;row 0  0x68FE        N/A   N/A   R      Q      E              W      T		1111 1110
;row 1  0x68FD        N/A   N/A   F      A      D      CTRL    S      G		1111 1101
;row 2  0x68FB        N/A   N/A   V      Z      C      SHIFT   X      B		1111 1011
;row 3  0x68F7        N/A   N/A   4      1      3              2      5		1111 0111
;row 4  0x68EF        N/A   N/A   M      SPACE  ,              .      N		1110 1111
;row 5  0x68DF        N/A   N/A   7      0      8      -       9      6		1101 1111
;row 6  0x68BF        N/A   N/A   U      P      I      RETURN  O      Y		1011 1111
;row 7  0x687F        N/A   N/A   J      ;      K      :       L      H		0111 1111
;
; If the '2' key were pressed, it would cause bit 1 at address 68F7H to drop to 0.
; The data retrieved by reading that address, neglecting the 2 most significant bits 
; which are not driven by the keyboard, would be 3DH (binary 111101).

; Wait for keypress.

prskey	call vsync
	ld b,1		; reset row
	ld hl,$68fe	; high byte of port to read.

; Check every row

prskey0	ld a,l		; low byte
	rrca		; Adjust lb port address
	ld l,a
	ld a,(hl)	; read key
	and $3f
	cp $3f		; Key pressed?
	jr nz,prskey1	; Yes, exit
	inc b		; increment row counter
	ld a,b
	cp 9		; last row checked?
	jr nz,prskey0	; no, repeat
	jr prskey	; yes, no key pressed, check again

; Determine column

prskey1	ld d,a
	ld c,1		; reset column
prskey2	sra d		; rotate bit out
	jr nc,prskey4	; key pressed, exit
	inc c		; increment column counter
	ld a,c
	cp 7		; last column checked?
	jr nz,prskey2	; no, repeat
prskey3	jr prskey	; yes, no key pressed, exit

; Key pressed, create keycode

prskey4	ld a,c		; high nibble=row
	sla a
	sla a
	sla a
	sla a
	add a,b		; low nibble=column
	push af
debounce
	call $2ef4
	or a
	jr nz,debounce

	pop af
	ret

; Delay routine, b x 1/50 sec.

delay
	call wloop1
delay1
	call wloop
	djnz delay
	ret

; Wait loop, wait 1/50 sec

wloop	ld a,(MODE)
	and $80
	jr nz,wloop
	ret

wloop1	ld a,(MODE)
	and $80
	jr z,wloop1
	ret

; Clear sprite table.

xspr   ld hl,sprtab        ; sprite table.
       ld b,SPRBUF         ; length of table.
xspr0  ld (hl),255         ; clear one byte.
       inc hl              ; move to next byte.
       djnz xspr0          ; repeat for rest of table.
       ret

; Initialise all objects.

iniob  ld ix,objdta       	 ; objects table.
       ld a,(numob)       	 ; number of objects in the game.
       ld b,a             	 ; loop counter.
       ld de,obj_len+7           ; distance between objects.
iniob0 ld a,(ix+obj_len+4)       ; start screen.
       ld (ix+obj_len+1),a       ; set start screen.
       ld a,(ix+obj_len+5)       ; find start x.
       ld (ix+obj_len+2),a       ; set start x.
       ld a,(ix+obj_len+6)       ; get initial y.
       ld (ix+obj_len+3),a       ; set y coord.
       add ix,de          	 ; point to next object.
       djnz iniob0        	 ; repeat.
       ret

; Screen synchronisation.
;
; Located at 345C hex is a routine which can be used to produce sounds 
; via the VZ200/300's internal piezo speaker. Before calling the routine, 
; the HL register pair must be loaded with a number representing the pitch 
; (frequency) of the tone to be produced, while the BC register pair must 
; be loaded with the number of cycles of the tone required (ie the 
; duration in cycles). All registers are used. The frequency coding used
; is inversely proportional to frequency, ie the smaller the number loaded
; into the HL register pair, the higher the frequency. As a guide, the low C
; produced by the VZ200/300's SOUND command in BASIC can be produced using
; the decimal number 526, the middle C using 529 and the high C using 127. 
; Here is how you would use the routine to get say 75 cycles of the middle C:

;LD HL,259  ;set frequency code
;LD BC,75   ;set number of cycles
;CALL 345CH ;& call sound routine

; Produce sound
;   b7
;   b6
;   b5 - speaker b
;   b4 - VDU background colour
;
;   b3 - VDU display mode
;   b2 - cas out msb
;   b1 - cas out lsb
;   b0 - speaker a

vsync
;       ld hl,clock         ; game frame.
;       inc (hl)            ; advance the frame.
vsync0
	call joykey	; read joystick/keyboard.

; 	call wloop	; sync framerate with clock

	ld a,(clock)
	ld b,a
vsloop
	ld a,(clock)
	cp b
	jr z,vsloop
	
	ld a,(clock)	; read clock
	and 1		; filter even frame
	jr z,vsync1	; skip every even frame
	call proshr	; shrapnel and stuff.
vsync1
	push bc
	push de
	push hl
	ld a,(sndtyp)	; sound to play
	and a		; any sound?
	jp z,vsync2	; no, skip.
snd0
	ld a,(sndtyp)
	ld l,a		; set pitch
	ld c,2		; set duration
	call sound	; play sound
	ld a,(sndtyp)
	dec a		; decrement counter
	ld (sndtyp),a
	jr nz,snd0	; repeat if not 0
vsync2 
	pop hl
	pop de
	pop bc

;	ld a,d		; restore graphicsmode
;	or $21
;	or COL_OR+GFX_MOD
;	and COL_AND
;
;IF XFLAG = 1
;	ld (30779),a
;ENDIF
;
;	ld (MODE),a
	ret

; Copy of the sound routine at $345c with some changes:
; - Made the counters 8-bit
; - Changed writing to $6800

sound
	ld a,(MODE)	; read $6800
	ld d,a		; store
	call toggle	; call toggling speaker bits
	dec c		; decrement counter1
	jr nz,sound	; repeat if not 0
	ret

; Toggle speaker bit

toggle
	push bc		; save counter1
	ld a,$20		; set speaker bits
	or COL_OR+GFX_MOD	; restore graphicsmode
	and COL_AND

IF XFLAG = 1
	ld (30779),a
ENDIF

	ld (MODE),a	; set $6800

	push hl		; save counter1
	pop bc		; set counter2
toggle0
	dec c		; decrement counter2
	jr nz,toggle0	; repeat if not 0

	ld a,$01		; get $6800 value
	or COL_OR+GFX_MOD	; restore graphicsmode
	and COL_AND

IF XFLAG = 1
	ld (30779),a
ENDIF

	ld (MODE),a	; set $6800

	push hl		; save counter1
	pop bc		; set counter2
toggle1
	dec c		; decrement counter2
	jr nz,toggle1	; repeat if not 0
	pop bc		; restore counter1
	ret

sndtyp		defb 0
sndcnt		defb 0


framec defb 0              ; interrupt counter (pseudo frame counter).
firmad defw 0


; Redraw the screen.
; Remove old copy of all sprites for redraw.

redraw:
    IF GFLAG
       ld hl,%1111111100000000 ; avoid changing received colors.
       ld (bgmask),hl
    ENDIF
       push ix             ; place sprite pointer on stack.

       call droom          ; show screen layout.
       call shwob          ; draw objects.
numsp0 ld b,NUMSPR         ; sprites to draw.
       ld ix,sprtab        ; sprite table.
redrw0 ld a,(ix+0)         ; old sprite type.
       inc a               ; is it enabled?
       jr z,redrw1         ; no, find next one.
       ld a,(ix+3)         ; sprite x.
       cp SCRHGT * 8 - COLDIST              ; beyond maximum?
       jr nc,redrw1        ; yes, nothing to draw.
       push bc             ; store sprite counter.
       call sspria         ; show single sprite.
       pop bc              ; retrieve sprite counter.
redrw1 ld de,TABSIZ        ; distance to next odd/even entry.
       add ix,de           ; next sprite.
       djnz redrw0         ; repeat for remaining sprites.
rpblc1 call dshrp          ; redraw shrapnel.

    IF AFLAG
       call rbloc          ; draw blocks for this screen
    ENDIF

       pop ix              ; retrieve sprite pointer.
       ret

; Clear screen routine.


cls

IF XFLAG = 1 OR XFLAG = 2
	ld hl,XBANK
	ld (hl),0
cls1
	ld a,(XBANK)
	add a,XMODE
	out (XPORT),a
ENDIF

IF IFLAG = 1
	ld a,FILL
ELSE
	LD A,(bgmask)		   ; In hires clear with 0
ENDIF
        LD HL,VRAM
	LD DE,VRAM+1
	LD BC,$07FF		   ; Size of video ram (2K)
	LD (HL),A		   ; Store 0 in first location
	LDIR			   ; Fill all the screen with 0

IF XFLAG = 1 or XFLAG = 2
	ld a,(XBANK)
	inc a
	ld (XBANK),a
	cp 3
	jr nz,cls1

	ld a,0
	ld (XBANK),a
	add a,XMODE
	out (XPORT),a
ENDIF

       ld hl,0             ; set hl to origin (0, 0).
       ld (charx),hl       ; reset coordinates.
       ret

fdchk  ld a,(hl)           ; fetch cell.
       cp FODDER           ; is it fodder?
       ret nz              ; no.
       ld (hl),0           ; rewrite block type.
       push hl             ; store pointer to block.
       ld de,MAP           ; address of map.
       and a               ; clear carry flag for subtraction.
       sbc hl,de           ; find simple displacement for block.
       ld a,l              ; low byte is y coordinate.
       and 31              ; column position 0 - 31.
       ld (dispy),a        ; set up y position.
       add hl,hl           ; multiply displacement by 8.
       add hl,hl
       add hl,hl
       ld a,h              ; x coordinate now in h.
       ld (dispx),a        ; set the display coordinate.
	ld a,32
       call pattr          ; write block.
       pop hl              ; restore block pointer.
       ret

; Colour a sprite.

cspr   ret                 ; No sprite paiting in VZ200.

; Scrolly text and puzzle variables.

txtbit defb 128            ; bit to write.
txtwid defb 16             ; width of ticker message.
txtpos defw msgdat
txtini defw msgdat

txtscr defw VRAM

; Specialist routines.
; Process shrapnel.

proshr ld ix,SHRAPN        ; table.
       ld b,NUMSHR         ; shrapnel pieces to process.
       ld de,SHRSIZ        ; distance to next.
prosh0 ld a,(ix+0)         ; on/off marker.
       rla                 ; check its status.
proshx call nc,prosh1      ; on, so process it.
       add ix,de           ; point there.
       djnz prosh0         ; round again.
       jp scrly
prosh1 push bc             ; store counter.
       call plot           ; delete the pixel.
       ld a,(ix+0)         ; restore shrapnel type.
       ld hl,shrptr        ; shrapnel routine pointers.
       call prosh2         ; run the routine.
       call chkxy          ; check x and y are good before we redisplay.
       pop bc              ; restore counter.
       ld de,SHRSIZ        ; distance to next.
       ret
prosh2 rlca                ; 2 bytes per address.
       ld e,a              ; copy to de.

       ld d,0              ; reset D as 0 because "plot" changes its value.

       add hl,de           ; point to address of routine.
       ld a,(hl)           ; get address low.
       inc hl              ; point to second byte.
       ld h,(hl)           ; fetch high byte from table.
       ld l,a              ; put low byte in l.
       jp (hl)             ; jump to routine.

shrptr defw laser          ; laser.
       defw trail          ; vapour trail.
       defw shrap          ; shrapnel from explosion.
       defw dotl           ; horizontal starfield left.
       defw dotr           ; horizontal starfield right.
       defw dotu           ; vertical starfield up.
       defw dotd           ; vertical starfield down.
       defw ptcusr         ; user particle.

; Explosion shrapnel.

shrap  ld e,(ix+1)         ; get the angle.
       ld d,0              ; no high byte.
       ld hl,shrsin        ; shrapnel sine table.
       add hl,de           ; point to sine.

       ld e,(hl)           ; fetch value from table.
       inc hl              ; next byte of table.
       ld d,(hl)           ; fetch value from table.
       inc hl              ; next byte of table.
       ld c,(hl)           ; fetch value from table.
       inc hl              ; next byte of table.
       ld b,(hl)           ; fetch value from table.
       ld l,(ix+2)         ; x coordinate in hl.
       ld h,(ix+3)
       add hl,de           ; add sine.
       ld (ix+2),l         ; store new coordinate.
       ld (ix+3),h
       ld l,(ix+4)         ; y coordinate in hl.
       ld h,(ix+5)
       add hl,bc           ; add cosine.
       ld (ix+4),l         ; store new coordinate.
       ld (ix+5),h
       ret

dotl   dec (ix+5)          ; move left.
       ret

dotr   inc (ix+5)          ; move right.
       ret

dotu
       dec (ix+3)          ; move up.
       ret

dotd
       inc (ix+3)          ; move down.
       ret

; Check coordinates are good before redrawing at new position.

chkxy  ld hl,wntopx        ; window top.
       ld a,(ix+3)         ; fetch shrapnel coordinate.
       cp (hl)             ; compare with top window limit.
       jr c,kilshr         ; out of window, kill shrapnel.

       inc hl              ; left edge.
       ld a,(ix+5)         ; fetch shrapnel coordinate.
       cp (hl)             ; compare with left window limit.
       jr c,kilshr         ; out of window, kill shrapnel.
	jr z,kilshr

       inc hl              ; point to bottom.
       ld a,(hl)           ; fetch window limit.
       add a,COLDIST            ; add height of sprite.
       cp (ix+3)           ; compare with shrapnel x coordinate.
       jr c,kilshr         ; off screen, kill shrapnel.

       inc hl              ; point to right edge.
       ld a,(hl)           ; fetch shrapnel y coordinate.
       add a,15            ; add width of sprite.
       cp (ix+5)           ; compare with window limit.
       jr c,kilshr         ; off screen, kill shrapnel.
	jr z,kilshr

; Drop through.
; Display shrapnel.

plot   ld l,(ix+3)         ; x integer.
       ld h,(ix+5)         ; y integer.
       ld (dispx),hl       ; workspace coordinates.
       ld a,(ix+0)         ; type.
       and a               ; is it a laser?
       jr z,plot1          ; yes, draw laser instead.
plot0

IF XFLAG = 0 OR XMODE = 24
	ld a,(dispy)
	and 1
	ret nz
ENDIF
	ld a,l		; test if not offscreen
	cp SCRHGT*8-1
	ret nc
       ld a,h              ; which pixel within byte do we
       and 7               ; want to set first?
       ld d,0              ; no high byte.
       ld e,a              ; copy to de.
       ld hl,dots          ; table of small pixel positions.
       add hl,de           ; hl points to values we want to POKE to screen.
       ld e,(hl)           ; get value.

       call scadd          ; screen address.

IF XFLAG = 1 OR XFLAG = 2
	push hl
	ld a,h
	sub $70
	srl a
	srl a
	srl a
	and 3
	add a,XMODE
	out (XPORT),a
	ld a,h
	and $07
	or $70
	ld h,a
ENDIF

       ld a,(hl)           ; see what's already there.
       xor e               ; merge with pixels.
       ld (hl),a           ; put back on screen.

IF XFLAG = 1 OR XFLAG = 2
	pop hl
ENDIF

       ret

plot1  call scadd          ; screen address.

IF XFLAG = 1 OR XFLAG = 2
	push hl
	ld a,h
	sub $70
	srl a
	srl a
	srl a
	and 3
	add a,XMODE
	out (XPORT),a
	ld a,h
	and $07
	or $70
	ld h,a
ENDIF

       ld a,(hl)           ; fetch byte there.
       cpl                 ; toggle all bits.
       ld (hl),a           ; new byte.

IF XFLAG = 1 OR XFLAG = 2
	pop hl
ENDIF

       ret

kilshr ld (ix+0),128       ; switch off shrapnel.
       ret

shrsin defw 0,1024,391,946,724,724,946,391
       defw 1024,0,946,65144,724,64811,391,64589
       defw 0,64512,65144,64589,64811,64811,64589,65144
       defw 64512,0,64589,391,64811,724,65144,946

trail  dec (ix+1)          ; time remaining.
       jp z,trailk         ; time to switch it off.
       call qrand          ; get a random number.
       rra                 ; x or y axis?
       jr c,trailv         ; use x.
       rra                 ; which direction?
       jr c,traill         ; go left.
       inc (ix+5)          ; go right.
       ret
traill dec (ix+5)          ; go left.
       ret
trailv rra                 ; which direction?
       jr c,trailu         ; go up.
       inc (ix+3)          ; go down.
       ret
trailu dec (ix+3)          ; go up.
       ret
trailk ld (ix+3),200       ; set off-screen to kill vapour trail.
       ret

laser  ld a,(ix+1)         ; direction.
       rra                 ; left or right?
       jr nc,laserl        ; move left.
       ld b,8              ; distance to travel.
       jr laserm           ; move laser.
laserl ld b,248            ; distance to travel.
laserm ld a,(ix+5)         ; y position.
       add a,b             ; add distance.
       ld (ix+5),a         ; set new y coordinate.

; Test new block.

       ld (dispy),a        ; set y for block collision detection purposes.
       ld a,(ix+3)         ; get x.
       ld (dispx),a        ; set coordinate for collision test.
       call tstbl          ; get block type there.
       cp WALL             ; is it solid?
       jr z,trailk         ; yes, it cannot pass.
       cp FODDER           ; is it fodder?
       ret nz              ; no, ignore it.
       call fdchk          ; remove fodder block.
       jr trailk           ; destroy laser.


IF XFLAG = 0
dots   defb $c0,$c0,$30,$30,$c,$c,3,3
ELSE
dots   defb 128,64,32,16,8,4,2,1
ENDIF

; Plot, preserving de.

plotde push de             ; put de on stack.
       call plot           ; plot pixel.
       pop de              ; restore de from stack.
       ret

; Shoot a laser.

shoot  ld c,a              ; store direction in c register.
       ld a,(ix+8)         ; x coordinate.
shoot1 add a,COLDIST/2             ; down 7 pixels.
       ld l,a              ; put x coordinate in l.
       ld h,(ix+9)         ; y coordinate in h.
       push ix             ; store pointer to sprite.
       call fpslot         ; find particle slot.
       jr nc,vapou2        ; failed, restore ix.
       ld (ix+0),0         ; set up a laser.
       ld (ix+1),c         ; set the direction.
       ld (ix+3),l         ; set x coordinate.
       rr c                ; check direction we want.
       jr c,shootr         ; shoot right.
       ld a,h              ; y position.
;       dec a               ; left a pixel.
shoot0 and 248             ; align on character boundary.
       ld (ix+5),a         ; set y coordinate.
       jr vapou0           ; draw first image.
shootr ld a,h              ; y position.
       add a,15            ; look right.
       jr shoot0           ; align and continue.

; Create a bit of vapour trail.

vapour push ix             ; store pointer to sprite.
       ld l,(ix+8)         ; x coordinate.
       ld h,(ix+9)         ; y coordinate.
vapou3 ld de,COLDIST/2*256+COLDIST/2       ; mid-point of sprite.
       add hl,de           ; point to centre of sprite.
       call fpslot         ; find particle slot.
       jr c,vapou1         ; no, we can use it.
vapou2 pop ix              ; restore sprite pointer.
       ret                 ; out of slots, can't generate anything.

vapou1 ld (ix+3),l         ; set up x.
       ld (ix+5),h         ; set up y coordinate.
       call qrand          ; get quick random number.
       and 15              ; random time.
       add a,15            ; minimum time on screen.
       ld (ix+1),a         ; set time on screen.
       ld (ix+0),1         ; define particle as vapour trail.
vapou0 call chkxy          ; plot first position.
       jr vapou2

; Create a user particle.

ptusr  ex af,af'           ; store timer.
       ld l,(ix+8)         ; x coordinate.
       ld h,(ix+9)         ; y coordinate.
       ld de,7*256+7       ; mid-point of sprite.
       add hl,de           ; point to centre of sprite.
       call fpslot         ; find particle slot.
       jr c,ptusr1         ; free slot.
       ret                 ; out of slots, can't generate anything.

ptusr1 ld (ix+3),l         ; set up x.
       ld (ix+5),h         ; set up y coordinate.
       ex af,af'           ; restore timer.
       ld (ix+1),a         ; set time on screen.
       ld (ix+0),7         ; define particle as user particle.
       jp chkxy            ; plot first position.


; Create a vertical or horizontal star.

star   push ix             ; store pointer to sprite.
       call fpslot         ; find particle slot.
       jp c,star7          ; found one we can use.
star0  pop ix              ; restore sprite pointer.
       ret                 ; out of slots, can't generate anything.

star7  ld a,c              ; direction.
       and 3               ; is it left?
       jr z,star1          ; yes, it's horizontal.
       dec a               ; is it right?
       jr z,star2          ; yes, it's horizontal.
       dec a               ; is it up?
       jr z,star3          ; yes, it's vertical.

; Star down

       ld a,(wntopx)       ; get edge of screen.
       inc a               ; down one pixel.
star8  ld (ix+3),a         ; set x coord.
       call qrand          ; get quick random number.

star9  ld (ix+5),a         ; set y position.
       ld a,c              ; direction.
       and 3               ; zero to three.
       add a,3             ; 3 to 6 for starfield.
       ld (ix+0),a         ; define particle as star.
       call chkxy          ; plot first position.
       jp star0

; Star left

star1  call qrand          ; get quick random number.
       ld (ix+3),a         ; set x coord.
       ld a,(wnrgtx)       ; get edge of screen.
       add a,14          ; add width of sprite minus 1.
       jp star9

; Star right

star2  call qrand          ; get quick random number.
       ld (ix+3),a         ; set x coord.
       ld a,(wnlftx)       ; get edge of screen.
	inc a
       jp star9

; Star up

star3  ld a,(wnbotx)       ; get edge of screen.
       add a,15            ; height of sprite minus one pixel.
       jp star8


; Find particle slot for lasers or vapour trail.
; Can't use alternate accumulator.

fpslot ld ix,SHRAPN        ; shrapnel table.
       ld de,SHRSIZ        ; size of each particle.
       ld b,NUMSHR         ; number of pieces in table.
fpslt0 ld a,(ix+0)         ; get type.
       rla                 ; is this slot in use?
       ret c               ; no, we can use it.
       add ix,de           ; point to more shrapnel.
       djnz fpslt0         ; repeat for all shrapnel.
       ret                 ; out of slots, can't generate anything.

; Create an explosion at sprite position.

explod ld c,a              ; particles to create.
       push ix             ; store pointer to sprite.
       ld l,(ix+8)         ; x coordinate.
       ld h,(ix+9)         ; y coordinate.
       ld ix,SHRAPN        ; shrapnel table.
       ld de,SHRSIZ        ; size of each particle.
       ld b,NUMSHR         ; number of pieces in table.
expld0 ld a,(ix+0)         ; get type.
       rla                 ; is this slot in use?
       jr c,expld1         ; no, we can use it.
expld2 add ix,de           ; point to more shrapnel.
       djnz expld0         ; repeat for all shrapnel.
expld3 pop ix              ; restore sprite pointer.
       ret                 ; out of slots, can't generate any more.
expld1 ld a,c              ; shrapnel counter.
       and 15              ; 0 to 15.
       add a,l             ; add to x.
       ld (ix+3),a         ; x coord.
       ld a,(seed3)        ; crap random number.
       and 15              ; 0 to 15.
       add a,h             ; add to y.
       ld (ix+5),a         ; y coord.
       ld (ix+0),2         ; switch it on.
       exx                 ; store coordinates.
       call chkxy          ; plot first position.
       call qrand          ; quick random angle.
       and 60              ; keep within range.
       ld (ix+1),a         ; angle.
       exx                 ; restore coordinates.
       dec c               ; one piece of shrapnel fewer to generate.
       jr nz,expld2        ; back to main explosion loop.
       jr expld3           ; restore sprite pointer and exit.
qrand  ld a,(seed3)        ; random seed.
       ld l,a              ; low byte.
       ld h,0              ; no high byte.
       ld a,r              ; r register.
       xor (hl)            ; combine with seed.
       ld (seed3),a        ; new seed.
       ret
seed3  defb $45

; Display all shrapnel.

dshrp  ld hl,plotde        ; display routine.
       ld (proshx+1),hl    ; modify routine.
       call proshr         ; process shrapnel.
       ld hl,prosh1        ; processing routine.
       ld (proshx+1),hl    ; modify the call.
       ret

; Particle engine.

inishr ld hl,SHRAPN        ; table.
       ld b,NUMSHR         ; shrapnel pieces to process.
       ld de,SHRSIZ        ; distance to next.
inish0 ld (hl),255         ; kill the shrapnel.
       add hl,de           ; point there.
       djnz inish0         ; round again.
       ret

; Check for collision between laser and sprite.

lcol   ld hl,SHRAPN        ; shrapnel table.
       ld de,SHRSIZ        ; size of each particle.
       ld b,NUMSHR         ; number of pieces in table.
lcol0  ld a,(hl)           ; get type.
       and a               ; is this slot a laser?
       jr z,lcol1          ; yes, check collision.
lcol3  add hl,de           ; point to more shrapnel.
       djnz lcol0          ; repeat for all shrapnel.
       ret                 ; no collision, carry not set.
lcol1  push hl             ; store pointer to laser.
       inc hl              ; direction.
       inc hl              ; not used.
       inc hl              ; x position.
       ld a,(hl)           ; get x.
       sub (ix+X)          ; subtract sprite x.
lcolh  cp COLDIST+1               ; within range?
       jr nc,lcol2         ; no, missed.
       inc hl              ; not used.
       inc hl              ; y position.
       ld a,(hl)           ; get y.
       sub (ix+Y)          ; subtract sprite y.
       cp 16               ; within range?
       jr c,lcol4          ; yes, collision occurred.
lcol2  pop hl              ; restore laser pointer from stack.
       jr lcol3
lcol4  pop hl              ; restore laser pointer.
       ret                 ; return with carry set for collision.

; Main game engine code starts here.

game   equ $


       ; no screen address table.



       ; no palette.


rpblc2 call inishr         ; initialise particle engine.
evintr call evnt12         ; call intro/menu event.

       ld hl,MAP           ; block properties.
       ld de,MAP+1         ; next byte.
       ld bc,32 * SCRHGT*8 - 1       ; size of property map.
       ld (hl),WALL        ; write default property.
       ldir
       call iniob          ; initialise objects.
       xor a               ; put zero in accumulator.
       ld (gamwon),a       ; reset game won flag.

       ld hl,score         ; score.
       call inisc          ; init the score.
mapst  ld a,(stmap)        ; start position on map.
       ld (roomtb),a       ; set up position in table, if there is one.

inipbl:
    IF AFLAG
       ld hl,eop          ; reset blockpointer
       ld (pbptr+1),hl
    ENDIF
       call initsc         ; set up first screen.

       ld ix,ssprit        ; default to spare sprite in table.
evini  call evnt13         ; initialisation.

; Two restarts.
; First restart - clear all sprites and initialise everything.

rstrt  call rsevt          ; restart events.
       call xspr           ; clear sprite table.
       call sprlst         ; fetch pointer to screen sprites.
       call ispr           ; initialise sprite table.
       jr rstrt0

; Second restart - clear all but player, and don't initialise him.

rstrtn
       call rsevt          ; restart events.
       call nspr           ; clear all non-player sprites.
       call sprlst         ; fetch pointer to screen sprites.
       call kspr           ; initialise sprite table, no more players.


; Set up the player and/or enemy sprites.

rstrt0
       xor a               ; zero in accumulator.
       ld (nexlev),a       ; reset next level flag.
       ld (restfl),a       ; reset restart flag.
       ld (deadf),a        ; reset dead flag.
       call droom          ; show screen layout.


rpblc0:
    IF AFLAG
       call rbloc          ; draw blocks for this screen
    ENDIF
       call inishr         ; initialise particle engine.

       call shwob          ; draw objects.
       ld ix,sprtab        ; address of sprite table, even sprites.
       call dspr           ; display sprites.
       ld ix,sprtab+TABSIZ ; address of first odd sprite.
       call dspr           ; display sprites.

mloop  call vsync          ; synchronise with display.

       ld ix,sprtab        ; address of sprite table, even sprites.
       call dspr           ; display even sprites.

       call plsnd          ; play sounds.
       call vsync0          ; synchronise with display.
       ld ix,sprtab+TABSIZ ; address of first odd sprite.
       call dspr           ; display odd sprites.
       ld ix,ssprit        ; point to spare sprite for spawning purposes.
evlp1  call evnt10         ; called once per main loop.
       call pspr           ; process sprites.

; Main loop events.

       ld ix,ssprit        ; point to spare sprite for spawning purposes.
evlp2  call evnt11         ; called once per main loop.

bsortx call bsort          ; sort sprites.
       ld a,(nexlev)       ; finished level flag.
       and a               ; has it been set?
       jr nz,newlev        ; yes, go to next level.
       ld a,(gamwon)       ; finished game flag.
       and a               ; has it been set?
       jr nz,evwon         ; yes, finish the game.
       ld a,(restfl)       ; finished level flag.
       dec a               ; has it been set?
       jr z,rstrt          ; yes, go to next level.
       dec a               ; has it been set?
       jr z,rstrtn         ; yes, go to next level.

       ld a,(deadf)        ; dead flag.
       and a               ; is it non-zero?
       jr nz,pdead         ; yes, player dead.

       ld hl,frmno         ; game frame.
       inc (hl)            ; advance the frame.

; Back to start of main loop.

qoff   jp mloop            ; switched to a jp nz,mloop during test mode.
       ret


;----------------------------------------------------------
; Read blocks from list and update screen accordingly.
;----------------------------------------------------------
    IF AFLAG
rbloc:
pbbuf  ld de,eop           ; check for last block
rbloc2 ld hl,(pbptr+1)
       or a
       sbc hl,de
       ret z
rbloc1 ex de,hl
       ld a,(scno)
       cp (hl)             ; pbbuf
       jr nz,rbloc0
       push hl
       inc hl
       ld de,dispx
       ldi                 ; dispx
       ldi                 ; dispy
       ld a,(hl)
       call pattr2         ; draw block
       pop hl
rbloc0 ld de,0004h
       add hl,de           ; point to next block
       ex de,hl
       jr rbloc2
    ENDIF


newlev ld a,(scno)         ; current screen.
       ld hl,numsc         ; total number of screens.
       inc a               ; next screen.
       cp (hl)             ; reached the limit?
       jr nc,evwon         ; yes, game finished.
       ld (scno),a         ; set new level number.
       jp rstrt            ; restart, clearing all aliens.
evwon  call evnt18         ; game completed.
       jp tidyup           ; tidy up and return to BASIC/calling routine.

; Player dead.

pdead  xor a               ; zeroise accumulator.
       ld (deadf),a        ; reset dead flag.
evdie  call evnt16         ; death subroutine.
       ld a,(numlif)       ; number of lives.
       and a               ; reached zero yet?
       jp nz,rstrt         ; restart game.
evfail call evnt17         ; failure event.
tidyup ld hl,hiscor        ; high score.
       ld de,score         ; player's score.
       ld b,6              ; digits to check.
tidyu2 ld a,(de)           ; get score digit.
       cp (hl)             ; are we larger than high score digit?
       jr c,tidyu0         ; high score is bigger.
       jr nz,tidyu1        ; score is greater, record new high score.
       inc hl              ; next digit of high score.
       inc de              ; next digit of score.
       djnz tidyu2         ; repeat for all digits.

;tidyu0 ld hl,(firmad)      ; firmware interrupt address.
;       ld ($0038+1),hl     ; restore interrupts.
;       ld hl,score         ; return pointing to score so programmer can store high-score.



tidyu0


       ret
tidyu1 ld hl,score         ; score.
       ld de,hiscor        ; high score.
       ld bc,6             ; digits to copy.
       ldir                ; copy score to high score.
evnewh call evnt19         ; new high score event.
       jr tidyu0           ; tidy up.

; Restart event.

rsevt  ld ix,ssprit        ; default to spare element in table.
evrs   jp evnt14           ; call restart event.

; Copy number passed in a to string position bc, right-justified.

num2ch ld l,a              ; put accumulator in l.
       ld h,0              ; blank high byte of hl.
       ld a,32             ; leading spaces.
numdg3 ld de,100           ; hundreds column.
       call numdg          ; show digit.
numdg2 ld de,10            ; tens column.
       call numdg          ; show digit.
       or 16               ; last digit is always shown.
       ld de,1             ; units column.
numdg  and 48              ; clear carry, clear digit.
numdg1 sbc hl,de           ; subtract from column.
       jr c,numdg0         ; nothing to show.
       or 16               ; something to show, make it a digit.
       inc a               ; increment digit.
       jr numdg1           ; repeat until column is zero.
numdg0 add hl,de           ; restore total.
       cp 32               ; leading space?
       ret z               ; yes, don't write that.
       ld (bc),a           ; write digit to buffer.
       inc bc              ; next buffer position.
       ret
num2dd ld l,a              ; put accumulator in l.
       ld h,0              ; blank high byte of hl.
       ld a,32             ; leading spaces.
       ld de,100           ; hundreds column.
       call numdg          ; show digit.
       or 16               ; second digit is always shown.
       jr numdg2
num2td ld l,a              ; put accumulator in l.
       ld h,0              ; blank high byte of hl.
       ld a,48             ; leading spaces.
       jr numdg3

inisc  ld b,6              ; digits to initialise.
inisc0 ld (hl),'0'         ; write zero digit.
       inc hl              ; next column.
       djnz inisc0         ; repeat for all digits.
       ret


; Multiply h by d and return in hl.

imul   ld e,d              ; HL = H * D
       ld c,h              ; make c first multiplier.
imul0  ld hl,0             ; zeroise total.
       ld d,h              ; zeroise high byte.
       ld b,8              ; repeat 8 times.
imul1  rr c                ; rotate rightmost bit into carry.
       jr nc,imul2         ; wasn't set.
       add hl,de           ; bit was set, so add de.
       and a               ; reset carry.
imul2  rl e                ; shift de 1 bit left.
       rl d
       djnz imul1          ; repeat 8 times.
       ret

; Divide d by e and return in d, remainder in a.

idiv   xor a
       ld b,8              ; bits to shift.
idiv0  sla d               ; multiply d by 2.
       rla                 ; shift carry into remainder.
       cp e                ; test if e is smaller.
       jr c,idiv1          ; e is greater, no division this time.
       sub e               ; subtract it.
       inc d               ; rotate into d.
idiv1  djnz idiv0
       ret

; Initialise a sound.

isnd
;       ld de,(ch1ptr)      ; first pointer.
;       ld a,(de)           ; get first byte.
;       inc a               ; reached the end?
;       jr z,isnd1          ; that'll do.
;       ld de,(ch2ptr)      ; second pointer.
;       ld a,(de)           ; get first byte.
;       inc a               ; reached the end?
;       jr z,isnd2          ; that'll do.
;       ld de,(ch3ptr)      ; final pointer.
;       ld a,(de)           ; get first byte.
;       inc a               ; reached the end?
;       jr z,isnd3          ; that'll do.
;       ret
;isnd1  ld (ch1ptr),hl      ; set up the sound.
;       ret
;isnd2  ld (ch2ptr),hl      ; set up the sound.
;       ret
;isnd3  ld (ch3ptr),hl      ; set up the sound.
       ret
;
;
;ch1ptr defw spmask
;ch2ptr defw spmask
;ch3ptr defw spmask
;
;plsnd  call plsnd1         ; first channel.
;       call plsnd2         ; second one.
;       call plsnd3         ; final channel.
;
;
plsnd
	ret
;
;
; Write the contents of our AY buffer to the AY registers.
;
;w8912  ld hl,snddat        ; start of AY-3-8912 register data.
;       ld de,14*256        ; start with register 0, 14 to write.
;
;w8912a ld a,e              ; AY register to write to.
;       out (REG),a         ; select AY register to write to.
;       ld a,(hl)           ; value to write.
;       out (WR),a          ; write value to selected AY register.
;
;       inc e               ; next sound chip register.
;       inc hl              ; next byte to write.
;       dec d               ; decrement loop counter.
;       jp nz,w8912a        ; repeat until done.
;       ret
;
;snddat defw 0              ; tone registers, channel A.
;       defw 0              ; channel B tone registers.
;       defw 0              ; as above, channel C.
;sndwnp defb 0              ; white noise period.
;
;sndmix defb %01111100      ; tone/noise mixer control.
;                           ; [%01......] I/O port A and I/O port B configuration for MC-1000.
;                           ; [%..1.....] channel C white noise silent.
;                           ; [%...1....] channel B white noise silent.
;                           ; [%....1...] channel A white noise silent.
;                           ; [%.....1..] channel C tone silent.
;                           ; [%......0.] channel B tone active.
;                           ; [%.......0] channel A tone active.
;
;sndmix defb %01111100      ; tone/noise mixer control.
;
; <VZ?> Not sure what to do here. Will take a lot of work
;sndv1  defb 0              ; channel A amplitude/envelope generator.
;sndv2  defb 0              ; channel B amplitude/envelope.
;sndv3  defb 0              ; channel C amplitude/envelope.
;       defw 0              ; duration of each note.
;       defb 0
;
;plwn   inc hl              ; next byte of sound.
;       and %00111000       ; check if we're bothering with white noise.
;       ret nz              ; we're not.
;       ld a,(hl)           ; fetch byte.
;       ld (sndwnp),a       ; set white noise period.
;       ret
;
;
;plsnd2 call cksnd2         ; check sound for second channel.
;       cp 255              ; reached end?
;       jr z,silen2         ; silence this channel.
;       and %00001111       ; sound bits.
;       ld (sndv2),a        ; set volume for channel.
;       ld a,(sndmix)       ; mixer byte.
;       and %11101101       ; remove bits for this channel.
;       ld b,a              ; store in b register.
;       call plmix          ; fetch mixer details.
;       and %00010010       ; mixer bits we want.
;       or b                ; combine with mixer bits.
;       ld (sndmix),a       ; new mixer value.
;       call plwn           ; white noise check.
;       inc hl              ; tone low.
;       ld e,(hl)           ; fetch value.
;       inc hl              ; tone high.
;       ld d,(hl)           ; fetch value.
;       ld (snddat+2),de    ; set tone.
;       inc hl              ; next bit of sound.
;       ld (ch2ptr),hl      ; set pointer.
;       ret
;
;plsnd3 call cksnd3         ; check sound for third channel.
;       cp 255              ; reached end?
;       jr z,silen3         ; silence last channel.
;       and %00001111       ; sound bits.
;       ld (sndv3),a        ; set volume for channel.
;       ld a,(sndmix)       ; mixer byte.
;       and %11011011       ; remove bits for this channel.
;       ld b,a              ; store in b register.
;       call plmix          ; fetch mixer details.
;       and %00100100       ; mixer bits we want.
;       or b                ; combine with mixer bits.
;       ld (sndmix),a       ; new mixer value.
;       call plwn           ; white noise check.
;       inc hl              ; tone low.
;       ld e,(hl)           ; fetch value.
;       inc hl              ; tone high.
;       ld d,(hl)           ; fetch value.
;       ld (snddat+4),de    ; set tone.
;       inc hl              ; next bit of sound.
;       ld (ch3ptr),hl      ; set pointer.
;       ret
;
;plmix  ld a,(hl)           ; fetch mixer byte.
;       and %11000000       ; mix bits are d6 and d7.
;       rlca                ; rotate into d0 and d1.
;       rlca
;       ld e,a              ; displacement in de.
;       ld d,0
;       push hl             ; store pointer on stack.
;       ld hl,mixtab        ; mixer table.
;       add hl,de           ; point to mixer byte.
;       ld a,(hl)           ; fetch mixer value.
;       pop hl              ; restore pointer.
;       ret
;mixtab defb %00111111,%00111000,%00000111,%00000000 ; mixer byte settings.
;
;silen1 xor a               ; zero.
;       ld (sndv1),a        ; sound off.
;       ld a,(sndmix)       ; mixer byte.
;       or %00001001        ; mix bits off.
;       ld (sndmix),a       ; mixer setting for channel.
;       ret
;silen2 xor a               ; zero.
;       ld (sndv2),a        ; sound off.
;       ld a,(sndmix)       ; mixer byte.
;       or %00010010        ; mix bits off.
;       ld (sndmix),a       ; mixer setting for channel.
;       ret
;silen3 xor a               ; zero.
;       ld (sndv3),a        ; sound off.
;       ld a,(sndmix)       ; mixer byte.
;       or %00100100        ; mix bits off.
;       ld (sndmix),a       ; mixer setting for channel.
;       ret
;cksnd1 ld hl,(ch1ptr)      ; pointer to sound.
;       ld a,(hl)           ; fetch mixer/flag.
;       ret
;cksnd2 ld hl,(ch2ptr)      ; pointer to sound.
;       ld a,(hl)           ; fetch mixer/flag.
;       ret
;cksnd3 ld hl,(ch3ptr)      ; pointer to sound.
;       ld a,(hl)           ; fetch mixer/flag.
;       ret
;
;plsnd1 call cksnd1         ; check sound for first channel.
;       cp 255              ; reached end?
;       jr z,silen1         ; silence first channel.
;       and %00001111       ; sound bits.
;       ld (sndv1),a        ; set volume for channel.
;       ld a,(sndmix)       ; mixer byte.
;       and %11110110       ; remove bits for this channel.
;       ld b,a              ; store in b register.
;       call plmix          ; fetch mixer details.
;       and %00001001       ; mixer bits we want.
;       or b                ; combine with mixer bits.
;       ld (sndmix),a       ; new mixer value.
;       call plwn           ; white noise check.
;       inc hl              ; tone low.
;       ld e,(hl)           ; fetch value.
;       inc hl              ; tone high.
;       ld d,(hl)           ; fetch value.
;       ld (snddat),de      ; set tone.
;       inc hl              ; next bit of sound.
;       ld (ch1ptr),hl      ; set pointer.
;       ret


; Objects handling.
; 32 bytes for image
; 1 for colour
; 3 for room, x and y
; 3 for starting room, x and y.
; 254 = disabled.
; 255 = object in player's pockets.

; Show items present.

shwob  ld hl,objdta        ; objects table.
;<zx>

       ld de,obj_len+1            ; distance to room number.

;</zx>
       add hl,de           ; point to room data.
       ld a,(numob)        ; number of objects in the game.
       ld b,a              ; loop counter.
shwob0 push bc             ; store count.
       push hl             ; store item pointer.
       ld a,(scno)         ; current location.
       cp (hl)             ; same as an item?
;<zx>
       call z,dobjc        ; yes, display object in colour.
;</zx>
       pop hl              ; restore pointer.
       pop bc              ; restore counter.
;<zx>
       ld de,obj_len+7            ; distance to next item.
;</zx>
       add hl,de           ; point to it.
       djnz shwob0         ; repeat for others.
       ret

; Display object.
; hl must point to object's room number.


dobjc: ; no object/sprite coloring. just display it.


dobj   inc hl              ; point to x.
dobj0  ld de,dispx         ; coordinates.
       ldi                 ; transfer x coord.
       ldi                 ; transfer y too.
;<zx>
       ld de,65536-obj_len-4         ; minus 36.
;</zx>
       add hl,de           ; point to image.
dobj1  jp sprite           ; draw this sprite.


       ; no object/sprite coloring.


; Remove an object.

remob  ld hl,numob         ; number of objects in game.
       cp (hl)             ; are we checking past the end?
       ret nc              ; yes, can't get non-existent item.
       push af             ; remember object.
       call getob          ; pick it up if we haven't already got it.
       pop af              ; retrieve object number.
       call gotob          ; get its address.
       ld (hl),254         ; remove it.
       ret

; Pick up object number held in the accumulator.
; hl 7dd8
; de ffdc

getob  ld hl,numob         ; number of objects in game.
       cp (hl)             ; are we checking past the end?
       ret nc              ; yes, can't get non-existent item.
       call gotob          ; check if we already have it.
       ret z               ; we already do.
       ex de,hl            ; object address in de.
       ld hl,scno          ; current screen.
       cp (hl)             ; is it on this screen?
       ex de,hl            ; object address back in hl.
       jr nz,getob0        ; not on screen, so nothing to delete.
       ld (hl),255         ; pick it up.
       inc hl              ; point to x coord.
getob1 ld e,(hl)           ; x coord.
       inc hl              ; back to y coord.
       ld d,(hl)           ; y coord.
       ld (dispx),de       ; set display coords.
;<zx>
       ld de,65536-obj_len-3        ; minus graphic size.65501 
;</zx>
       add hl,de           ; point to graphics.
       call dobj1          ; delete object sprite.

       ; no sprite coloring.

getob0 ld (hl),255         ; pick it up.
       ret

; Got object check.
; Call with object in accumulator, returns zero set if in pockets.

gotob  ld hl,numob         ; number of objects in game.
       cp (hl)             ; are we checking past the end?
       jr nc,gotob0        ; yes, we can't have a non-existent object.
       call findob         ; find the object.
gotob1 cp 255              ; in pockets?
       ret
gotob0 ld a,254            ; missing.
       jr gotob1

findob ld hl,objdta        ; objects.
;<zx>
       ld de,obj_len+7            ; size of each object.
;</zx>
       and a               ; is it zero?
       jr z,fndob1         ; yes, skip loop.
       ld b,a              ; loop counter in b.
fndob2 add hl,de           ; point to next one.
       djnz fndob2         ; repeat until we find address.
;<zx>
fndob1 ld e,obj_len+1             ; distance to room it's in.
;</zx>
       add hl,de           ; point to room.
       ld a,(hl)           ; fetch status.
       ret

; Drop object number at (dispx, dispy).

drpob  ld hl,numob         ; number of objects in game.
       cp (hl)             ; are we checking past the end?
       ret nc              ; yes, can't drop non-existent item.
       call gotob          ; make sure object is in inventory.
       ld a,(scno)         ; screen number.
       cp (hl)             ; already on this screen?
       ret z               ; yes, nothing to do.
       ld (hl),a           ; bring onto screen.
       inc hl              ; point to x coord.
       ld a,(dispx)        ; sprite x coordinate.
       ld (hl),a           ; set x coord.
       inc hl              ; point to object y.
       ld a,(dispy)        ; sprite y coordinate.
       ld (hl),a           ; set the y position.
;<zx>
       ld de,65536-obj_len-3         ; minus graphic size.
;</zx>
       add hl,de           ; point to graphics.

       jp dobj1            ; delete object sprite.
       ; no sprite/object coloring.


; Seek objects at sprite position.

skobj  ld hl,objdta        ; pointer to objects.
       ld de,obj_len+1            ; distance to room number.
       add hl,de           ; point to room data.
       ld de,obj_len+7            ; size of each object.
       ld a,(numob)        ; number of objects in game.
       ld b,a              ; set up the loop counter.
skobj0 ld a,(scno)         ; current room number.
       cp (hl)             ; is object in here?
       call z,skobj1       ; yes, check coordinates.
       add hl,de           ; point to next object in table.
       djnz skobj0         ; repeat for all objects.
       ld a,255            ; end of list and nothing found, return 255.
       ret
skobj1 inc hl              ; point to x coordinate.
       ld a,(hl)           ; get coordinate.
       sub (ix+8)          ; subtract sprite x.
       add a,COLDIST            ; add sprite height minus one.
       cp 2 * COLDIST + 1              ; within range?
       jp nc,skobj2        ; no, ignore object.
       inc hl              ; point to y coordinate now.
       ld a,(hl)           ; get coordinate.
       sub (ix+9)          ; subtract the sprite y.
       add a,15            ; add sprite width minus one.
       cp 31               ; within range?
       jp nc,skobj3        ; no, ignore object.
       pop de              ; remove return address from stack.
       ld a,(numob)        ; objects in game.
       sub b               ; subtract loop counter.
       ret                 ; accumulator now points to object.
skobj3 dec hl              ; back to y position.
skobj2 dec hl              ; back to room.
       ret


; Spawn a new sprite.

spawn  ld hl,sprtab        ; sprite table.
numsp1 ld a,NUMSPR         ; number of sprites.
       ld de,TABSIZ        ; size of each entry.
spaw0  ex af,af'           ; store loop counter.
       ld a,(hl)           ; get sprite type.
       inc a               ; is it an unused slot?
       jr z,spaw1          ; yes, we can use this one.
       add hl,de           ; point to next sprite in table.
       ex af,af'           ; restore loop counter.
       dec a               ; one less iteration.
       jr nz,spaw0         ; keep going until we find a slot.

; Didn't find one but drop through and set up a dummy sprite instead.

spaw1  push ix             ; existing sprite address on stack.
       ld (spptr),hl       ; store spawned sprite address.
       ld (hl),c           ; set the type.
       inc hl              ; point to image.
       ld (hl),b           ; set the image.
       inc hl              ; next byte.
       ld (hl),0           ; frame zero.
       inc hl              ; next byte.
       ld a,(ix+X)         ; x coordinate.
       ld (hl),a           ; set sprite coordinate.
       inc hl              ; next byte.
       ld a,(ix+Y)         ; y coordinate.
       ld (hl),a           ; set sprite coordinate.
       inc hl              ; next byte.
       ex de,hl            ; swap address into de.
       ld hl,(spptr)       ; restore address of details.
       ld bc,5             ; number of bytes to duplicate.
       ldir                ; copy first version to new version.
       ex de,hl            ; swap address into de.
       ld a,(ix+10)        ; direction of original.
       ld (hl),a           ; set the direction.
       inc hl              ; next byte.
       ld (hl),b           ; reset parameter.
       inc hl              ; next byte.
       ld (hl),b           ; reset parameter.
       inc hl              ; next byte.
       ld (hl),b           ; reset parameter.
       inc hl              ; next byte.
       ld (hl),b           ; reset parameter.
rtssp  ld ix,(spptr)       ; address of new sprite.
evis1  call evnt09         ; call sprite initialisation event.
       ld ix,(spptr)       ; address of new sprite.
       call sspria         ; display the new sprite.
       pop ix              ; address of original sprite.
       ret

spptr  defw 0              ; spawned sprite pointer.
seed   defb 0              ; seed for random numbers.
score  defb '000000'       ; player's score.
hiscor defb '000000'       ; high score.
bonus  defb '000000'       ; bonus.

grbase defw 0              ; graphics base address.


checkx ld a,e              ; x position.
       cp SCRHGT           ; off screen?
       ret c               ; no, it's okay.
       pop hl              ; remove return address from stack.
       ret

; Displays the current score.

dscor  call preprt         ; set up font and print position.
       call checkx         ; make sure we're in a printable range.
       ld a,(prtmod)       ; get print mode.
       and a               ; standard size text?
       jp nz,bscor0        ; no, show double-height.
dscor0 push bc             ; place counter onto the stack.
       push hl
       ld a,(hl)           ; fetch character.
       call pchar          ; display character.

       ; no attribute.

       ld hl,dispy         ; y coordinate.
       inc (hl)            ; move along one.
       pop hl
       inc hl              ; next score column.
       pop bc              ; retrieve character counter.
       djnz dscor0         ; repeat for all digits.
       ld hl,(blkptr)      ; blocks.
       ld (grbase),hl      ; set graphics base.
dscor2 ld hl,(dispx)       ; general coordinates.
       ld (charx),hl       ; set up display coordinates.
       ret

; Displays the current score in double-height characters.

bscor0 push bc             ; place counter onto the stack.
       push hl
       ld a,(hl)           ; fetch character.
       call bchar          ; display big char.
       pop hl
       inc hl              ; next score column.
       pop bc              ; retrieve character counter.
       djnz bscor0         ; repeat for all digits.
       jp dscor2           ; tidy up line and column variables.

; Adds number in the hl pair to the score.

addsc  ld de,score+1       ; ten thousands column.
       ld bc,10000         ; amount to add each time.
       call incsc          ; add to score.
       inc de              ; thousands column.
       ld bc,1000          ; amount to add each time.
       call incsc          ; add to score.
       inc de              ; hundreds column.
       ld bc,100           ; amount to add each time.
       call incsc          ; add to score.
       inc de              ; tens column.
       ld bc,10            ; amount to add each time.
       call incsc          ; add to score.
       inc de              ; units column.
       ld bc,1             ; units.
incsc  push hl             ; store amount to add.
       and a               ; clear the carry flag.
       sbc hl,bc           ; subtract from amount to add.
       jr c,incsc0         ; too much, restore value.
       pop af              ; delete the previous amount from the stack.
       push de             ; store column position.
       call incsc2         ; do the increment.
       pop de              ; restore column.
       jp incsc            ; repeat until all added.
incsc0 pop hl              ; restore previous value.
       ret
incsc2 ld a,(de)           ; get amount.
       inc a               ; add one to column.
       ld (de),a           ; write new column total.
       cp '9'+1            ; gone beyond range of digits?
       ret c               ; no, carry on.
       ld a,'0'            ; mae it zero.
       ld (de),a           ; write new column total.
       dec de              ; back one column.
       jr incsc2

; Add bonus to score.

addbo  ld de,score+5       ; last score digit.
       ld hl,bonus+5       ; last bonus digit.
       and a               ; clear carry.
       ld bc,6*256+48      ; 6 digits to add, ASCII '0' in c.
addbo0 ld a,(de)           ; get score.
       adc a,(hl)          ; add bonus.
       sub c               ; 0 to 18.
       ld (hl),c           ; zeroise bonus.
       dec hl              ; next bonus.
       cp 58               ; carried?
       jr c,addbo1         ; no, do next one.
       sub 10              ; subtract 10.
addbo1 ld (de),a           ; write new score.
       dec de              ; next score digit.
       ccf                 ; set carry for next digit.
       djnz addbo0         ; repeat for all 6 digits.
       ret

; Swap score and bonus.

swpsb  ld de,score         ; first score digit.
       ld hl,bonus         ; first bonus digit.
       ld b,6              ; digits to add.
swpsb0 ld a,(de)           ; get score and bonus digits.
       ld c,(hl)
       ex de,hl            ; swap pointers.
       ld (hl),c           ; write bonus and score digits.
       ld (de),a
       inc hl              ; next score and bonus.
       inc de
       djnz swpsb0         ; repeat for all 6 digits.
       ret

; Get print address.
; Returns screen address in DE.


gprad  push hl             ; store HL registers.
       ld hl,(dispx)       ; get coordinates.
       ld d,l              ; DE = vertical position * 32 * 8...

IF XFLAG = 1 or XFLAG = 2
	ld a,l
	srl a
	srl a
	srl a
	and 3
	add a,XMODE
	out (XPORT),a
	ld a,l
	and $07
	ld d,a
ENDIF

       ld e,h              ; + horizontal position...
       ld hl,VRAM          ; + address of start of VRAM.
       add hl,de

       ex de,hl
       pop hl              ; restore HL registers.

       ret

; Get property buffer address of char at (dispx, dispy) in hl.

pradd  ld a,(dispx)        ; x coordinate.
       rrca                ; multiply by 32.
       rrca
       rrca
       ld l,a              ; store shift in l.
       and 3               ; high byte bits.

       add a,MAP >> 8      ; start of properties map.

       ld h,a              ; that's our high byte.
       ld a,l              ; restore shift result.
       and 224             ; only want low bits.
       ld l,a              ; put into low byte.
       ld a,(dispy)        ; fetch y coordinate.
       and 31              ; should be in range 0 - 31.
       add a,l             ; add to low byte.
       ld l,a              ; new low byte.
       ret


       ; no attributes.


; Display character block on screen.


pchar  rlca                ; multiply char by 8.
       rlca
       rlca
       ld e,a              ; store shift in e.
       and %00000111       ; only want high byte bits.
       ld d,a              ; store in d.
       ld a,e              ; restore shifted value.
       and %11111000       ; only want low byte bits.
       ld e,a              ; that's the low byte. (DE=char*8)
       ld hl,(grbase)      ; address of graphics.
       add hl,de           ; add displacement, hl=chardata
    IF GFLAG
       ld de,(bgmask)	; de=fg/bgmask
    ENDIF
       exx		; save de and hl
       call gprad          ; get screen address (in DE).
       ex de,hl            ; hl=scraddr
       ld de,32            ; de=distance to next line (in bytes).
       ld b,8              ; height of character in font.
pchar0 exx
       ld a,(hl)           ; get image byte.
       inc hl              ; next image byte.
    IF GFLAG
       and d               ; apply foreground color.
       xor e               ; apply background color.
    ENDIF

IF IFLAG = 1
	xor 255
ENDIF
       exx
       ld (hl),a           ; copy to screen.
       add hl,de           ; next screen row down.
       djnz pchar0         ; repeat until all rows have been copied.
       ret

pcnt	defb 0

; Print attributes, properties and pixels.

colpat defb 0


pattr:
    IF AFLAG
       call wbloc          ; save blockinfo
    ENDIF
pattr2 ld b,a              ; store cell in b register for now.

       ld e,a              ; displacement in e.
       ld d,0              ; no high byte.
       ld hl,(proptr)      ; pointer to properties.
       add hl,de           ; property cell address.
       ld c,(hl)           ; fetch byte.
       ld a,c              ; put into accumulator.
       cp COLECT           ; is it a collectable?
       jp nz,pattr1        ; no, carry on as normal.
       ld a,b              ; restore cell.
       ld (colpat),a
pattr1 call pradd          ; get property buffer address.
       ld (hl),c           ; write property.
       ld a,b              ; get block number.

; Print attributes, no properties.


       ; no attributes.


; Print character pixels, no more.
	ld hl,(blkptr)      ; address of graphics.
	ld (grbase),hl

pchr:
;    IFDEF GFLAG
;       ld hl,%1111111100000000 ; avoid changing received colors.
;       ld (bgmask),hl
;    ENDIF
	
       call pchar          ; show character in accumulator.

       ld hl,dispy         ; y coordinate.
       inc (hl)            ; move along one.
       ret


setfgm defb $01            ; opcode for "LD BC,nn" will interpret next instruction as value: $010e, thus C=14, will cause "DJNZ" below to jump.
setbgm ld c,1              ; C=1 will cause "DJNZ" below not to jump.
       ld b,c              ; copy to B.
       and %00000011       ; limit colour value to the 0 -- 3 range.
       add a,cmsktb & $ff  ; get corresponding color mask. (note: this code requires an ALIGNed table.)
       ld l,a
       ld h,cmsktb >> 8
       ld a,(hl)
       ld hl,(bgmask)      ; get previous background (L) and foreground (H) masks.
       djnz calfgm         ; B is not 1 (setfgm)? jump forward to calculate only foreground mask.
       ld c,a              ; store background mask to set.
       ld a,h              ; calculate previous non-XORed foreground mask.
       xor l
       ld l,c              ; store new background mask in L.
calfgm xor l               ; calculate new XORed foreground mask.
       ld h,a              ; store new foreground mask in H.
       ld (bgmask),hl      ; set variables.
       ret
    ALIGN 4
cmsktb defb %00000000      ; color masks for colors 0 -- 3.
       defb %01010101
       defb %10101010
       defb %11111111



;----------------------------------------------
; Write block
;----------------------------------------------
    IF AFLAG
wbloc:
pbptr  ld de,0000h
       ld hl,scno
       ldi                 ; write screen.
       ld hl,dispx
       ldi                 ; write x position of block.
       ldi                 ; write y position of block.
       ld (de),a           ; store block number
       inc de
       ld (pbptr+1),de     ; auto-modifying code
       ret
    ENDIF


; Shifter sprite routine for objects.

sprit7 xor 7
       inc a
sprit3 rl l                ; shift into position.
       rl c
       rl h
       dec a               ; one less iteration.
       jp nz,sprit3
       ld a,l
       ld l,c
       ld c,h
       ld h,a
       jp sprit0           ; now apply to screen.

; Sprite routine for objects.

sprite push hl             ; store sprite graphic address.
       call scadd          ; get screen address in hl.
       ex de,hl            ; switch to de.
       pop hl              ; restore graphic address.
       ld a,(dispy)        ; y position.
       and 7               ; position straddling cells.
       ld b,a              ; store in b register.
       ld a,COLDIST + 1             ; pixel height.
sprit1 ex af,af'
       ld c,(hl)           ; fetch first byte.
       inc hl              ; next byte.
       push hl             ; store source address.
       ld l,(hl)
       ld h,0
       ld a,b              ; position straddling cells.
       and a               ; is it zero?
       jr z,sprit0         ; yes, apply to screen.
       cp 5
       jr nc,sprit7
       and a               ; clear carry.
sprit2 rr c
       rr l
       rr h
       dec a
       jp nz,sprit2
sprit0 

;       ld a,(MODBUF)       ; get video mode with bit 0 = 0 to bank-switch VRAM on.
;       out (COL32),a       ; (set video mode and) bank-switch VRAM on.


IF XFLAG = 1 or XFLAG = 2
	push de
	ld a,d
	sub $70
	srl a
	srl a
	srl a
	and 3
	add a,XMODE
	out (XPORT),a
	ld a,d
	and $07
	or $70
	ld d,a
ENDIF

       ld a,(de)           ; fetch screen image.
       xor c               ; merge with graphic.
       ld (de),a           ; write to screen.
       inc e               ; next screen byte.
       ld a,(de)           ; fetch screen image.
       xor l               ; combine with graphic.
       ld (de),a           ; write to screen.
       inc de              ; next screen address.
       ld a,(de)           ; fetch screen image.
       xor h               ; combine with graphic.
       ld (de),a           ; write to screen.


IF XFLAG = 1 or XFLAG = 2
	pop de
ENDIF

;       ld a,(MODBUF)       ; get video mode with bit 0 = 0.
;       inc a               ; set bit 0 = 1 to bank-switch VRAM off.
;       out (COL32),a       ; (set video mode and) bank-switch VRAM off.


IF XFLAG = 0
       ld a,-2+32          ; back to start byte and down to next line.
ENDIF

IF XFLAG = 1 or XFLAG = 2
       ld a,+32          ; back to start byte and down to next line.
ENDIF

       add a,e
       ld e,a
       ld a,0
       adc a,d
       ld d,a
sprit6 pop hl              ; restore source address.
       inc hl              ; next source byte.
       ex af,af'
       dec a
       jp nz,sprit1
       ret

; Get room address.

groom  ld a,(scno)         ; screen number.
groomx ld de,0             ; start at zero.
       ld hl,(scrptr)      ; pointer to screens.
       and a               ; is it the first one?
groom1 jr z,groom0         ; no more screens to skip.
       ld c,(hl)           ; low byte of screen size.
       inc hl              ; point to high byte.
       ld b,(hl)           ; high byte of screen size.
       inc hl              ; next address.
       ex de,hl            ; put total in hl, pointer in de.
       add hl,bc           ; skip a screen.
       ex de,hl            ; put total in de, pointer in hl.
       dec a               ; one less iteration.
       jr groom1           ; loop until we reach the end.
groom0 ld hl,(scrptr)      ; pointer to screens.
       add hl,de           ; add displacement.
       ld a,(numsc)        ; number of screens.
       ld d,0              ; zeroise high byte.
       ld e,a              ; displacement in de.
       add hl,de           ; add double displacement to address.
       add hl,de
       ret

; Draw present room.

droom  ld a,(wintop)       ; window top.
       ld (dispx),a        ; set x coordinate.
droom2 ld hl,(blkptr)      ; blocks.
       ld (grbase),hl      ; set graphics base.
       call groom          ; get address of current room.
       xor a               ; zero in accumulator.
       ld (comcnt),a       ; reset compression counter.
       ld a,(winhgt)       ; height of window.
droom0 push af             ; store row counter.
       ld a,(winlft)       ; window left edge.
       ld (dispy),a        ; set cursor position.
       ld a,(winwid)       ; width of window.
droom1 push af             ; store column counter.
       call flbyt          ; decompress next byte on the fly.
       push hl             ; store address of cell.

       call pattr2         ; show attributes and block.

       pop hl              ; restore cell address.
       pop af              ; restore loop counter.
       dec a               ; one less column.
       jr nz,droom1        ; repeat for entire line.
       ld a,(dispx)        ; x coord.
       inc a               ; move down one line.
       ld (dispx),a        ; set new position.
       pop af              ; restore row counter.
       dec a               ; one less row.
       jr nz,droom0        ; repeat for all rows.
       ret

; Decompress bytes on-the-fly.

flbyt  ld a,(comcnt)       ; compression counter.
       and a               ; any more to decompress?
       jr nz,flbyt1        ; yes.
       ld a,(hl)           ; fetch next byte.
       inc hl              ; point to next cell.
       cp 255              ; is this byte a control code?
       ret nz              ; no, this byte is uncompressed.
       ld a,(hl)           ; fetch byte type.
       ld (combyt),a       ; set up the type.
       inc hl              ; point to quantity.
       ld a,(hl)           ; get quantity.
       inc hl              ; point to next byte.
flbyt1 dec a               ; one less.
       ld (comcnt),a       ; store new quantity.
       ld a,(combyt)       ; byte to expand.
       ret

combyt defb 0              ; byte type compressed.
comcnt defb 0              ; compression counter.

; Ladder down check.

laddd  ld a,(ix+8)         ; x coordinate.
;       and %11111110       ; make it even.
;       ld (ix+8),a         ; reset it.
       ld h,(ix+9)         ; y coordinate.
       add a,COLDIST +1            ; look down 16 pixels.
       ld l,a              ; coords in hl.
       jr laddv

; Ladder up check.

laddu  ld a,(ix+8)         ; x coordinate.
       and %11111110       ; make it even.
       ld (ix+8),a         ; reset it.
       ld h,(ix+9)         ; y coordinate.
       add a,COLDIST - 1            ; look 2 pixels above feet.
       ld l,a              ; coords in hl.
laddv  ld (dispx),hl       ; set up test coordinates.
       call tstbl          ; get map address.
       call ldchk          ; standard ladder check.
       ret nz              ; no way through.
       inc hl              ; look right one cell.
       call ldchk          ; do the check.
       ret nz              ; impassable.
       ld a,(dispy)        ; y coordinate.
       and %00000111       ; position straddling block cells.
       ret z               ; no more checks needed.
;       inc hl              ; look to third cell.
;       call ldchk          ; do the check.
       ret                 ; return with zero flag set accordingly.

; Can go up check.

cangu  ld a,(ix+8)         ; x coordinate.
       ld h,(ix+9)         ; y coordinate.
       sub 1               ; look up 2 pixels.
       ld l,a              ; coords in hl.
       ld (dispx),hl       ; set up test coordinates.
       call tstbl          ; get map address.
       call lrchk          ; standard left/right check.
       ret nz              ; no way through.
       inc hl              ; look right one cell.
       call lrchk          ; do the check.
       ret nz              ; impassable.
       ld a,(dispy)        ; y coordinate.
       and %00000111       ; position straddling block cells.
       ret z               ; no more checks needed.
       inc hl              ; look to third cell.
       call lrchk          ; do the check.
       ret                 ; return with zero flag set accordingly.

; Can go down check.

cangd  ld a,(ix+8)         ; x coordinate.
       ld h,(ix+9)         ; y coordinate.
       add a,COLDIST + 1            ; look down 16 pixels.
       ld l,a              ; coords in hl.
       ld (dispx),hl       ; set up test coordinates.
       call tstbl          ; get map address.
       call plchk          ; block, platform check.
       ret nz              ; no way through.
       inc hl              ; look right one cell.
       call plchk          ; block, platform check.
       ret nz              ; impassable.
       ld a,(dispy)        ; y coordinate.
       and %00000111       ; position straddling block cells.
       ret z               ; no more checks needed.
       inc hl              ; look to third cell.
       call plchk          ; block, platform check.
       ret                 ; return with zero flag set accordingly.

; Can go left check.

cangl  ld l,(ix+8)         ; x coordinate.
       ld a,(ix+9)         ; y coordinate.
       sub 2               ; look left 2 pixels.
       ld h,a              ; coords in hl.
       jr cangh            ; test if we can go there.

; Can go right check.

cangr  ld l,(ix+8)         ; x coordinate.
       ld a,(ix+9)         ; y coordinate.
       add a,16            ; look right 16 pixels.
       ld h,a              ; coords in hl.

cangh  ld (dispx),hl       ; set up test coordinates.

IF MFLAG = 1
cangh2 ld b,2              ; default rows to write.
ELSE
cangh2 ld b,3              ; default rows to write.
ENDIF

       ld a,l              ; x position.
       and %00000111       ; does x straddle cells?
       jr nz,cangh0        ; yes, loop counter is good.
       dec b               ; one less row to write.
cangh0 call tstbl          ; get map address.
       ld de,32            ; distance to next cell.
cangh1 call lrchk          ; standard left/right check.
       ret nz              ; no way through.
       add hl,de           ; look down.
       djnz cangh1
       ret

; Check left/right movement is okay.

lrchk  ld a,(hl)           ; fetch map cell.
       cp WALL             ; is it passable?
       jr z,lrchkx         ; no.
       cp FODDER           ; fodder has to be dug.
       jr z,lrchkx         ; not passable.
always xor a               ; report it as okay.
       ret
lrchkx xor a               ; reset all bits.
       inc a
       ret

; Check platform or solid item is not in way.

plchk  ld a,(hl)           ; fetch map cell.
       cp WALL             ; is it passable?
       jr z,lrchkx         ; no.
       cp FODDER           ; fodder has to be dug.
       jr z,lrchkx         ; not passable.
       cp PLATFM           ; platform is solid.
       jr z,plchkx         ; not passable.
       cp LADDER           ; is it a ladder?
       jr z,lrchkx         ; on ladder, deny movement.
plchk0

IF CRFLAG = 1
	cp 9
;	jr nc,lrchkx 
	jr nc,plchkx 
ENDIF

       xor a               ; report it as okay.
       ret

plchkx ld a,(dispx)        ; x coordinate.
       and 7               ; position straddling blocks.
       jr z,lrchkx         ; on platform, deny movement.
       jr plchk0

; Check ladder is available.

ldchk  ld a,(hl)           ; fetch cell.
       cp LADDER           ; is it a ladder?
       ret                 ; return with zero flag set accordingly.

; Get collectables.

getcol ld b,COLECT         ; collectable blocks.
       call tded           ; test for collectable blocks.
       cp b                ; did we find one?
       ret nz              ; none were found, job done.
       call gtblk          ; get block.
       call evnt20         ; collected block event.
       jr getcol           ; repeat until none left.

; Get collectable block.

gtblk  ld (hl),0           ; make it empty now.
       ld de,MAP           ; map address.
       and a               ; clear carry.
       sbc hl,de           ; find cell number.
       ld a,l              ; get low byte of cell number.
       and 31              ; 0 - 31 is column.
       ld d,a              ; store y in d register.
       add hl,hl           ; multiply by 8.
       add hl,hl
       add hl,hl           ; x is now in h.
       ld e,h              ; put x in e.
       ld (dispx),de       ; set display coordinates.

       ld a,(colpat)
       rlca                ; multiply char by 8.
       rlca
       rlca
       ld e,a              ; store shift in e.
       and %00000111       ; only want high byte bits.
       ld d,a              ; store in d.
       ld a,e              ; restore shifted value.
       and %11111000       ; only want low byte bits.
       ld e,a              ; that's the low byte.
       ld hl,(blkptr)      ; address of graphics.
       add hl,de           ; add displacement.
       call gprad          ; get screen address.


;       ld a,(MODBUF)       ; get video mode with bit 0 = 0 to bank-switch VRAM on.
;       out (COL32),a       ; (set video mode and) bank-switch VRAM on.
       ex de,hl            ; screen address in hl, image address in de
       ld bc,32            ; distance to next screen row.

       ld a,(de)           ; get image byte.
       xor (hl)            ; erase in screen.
       ld (hl),a
       inc de              ; next image byte.
       add hl,bc           ; next screen row down.

       ld a,(de)           ; get image byte.
       xor (hl)            ; erase in screen.
       ld (hl),a
       inc de              ; next image byte.
       add hl,bc           ; next screen row down.

       ld a,(de)           ; get image byte.
       xor (hl)            ; erase in screen.
       ld (hl),a
       inc de              ; next image byte.
       add hl,bc           ; next screen row down.

       ld a,(de)           ; get image byte.
       xor (hl)            ; erase in screen.
       ld (hl),a
       inc de              ; next image byte.
       add hl,bc           ; next screen row down.

       ld a,(de)           ; get image byte.
       xor (hl)            ; erase in screen.
       ld (hl),a
       inc de              ; next image byte.
       add hl,bc           ; next screen row down.

       ld a,(de)           ; get image byte.
       xor (hl)            ; erase in screen.
       ld (hl),a
       inc de              ; next image byte.
       add hl,bc           ; next screen row down.

       ld a,(de)           ; get image byte.
       xor (hl)            ; erase in screen.
       ld (hl),a
       inc de              ; next image byte.
       add hl,bc           ; next screen row down.

       ld a,(de)           ; get image byte.
       xor (hl)            ; erase in screen.
       ld (hl),a

;       ld a,(MODBUF)       ; get video mode with bit 0 = 0.
;       inc a               ; set bit 0 = 1 to bank-switch VRAM off.
;       out (COL32),a       ; (set video mode and) bank-switch VRAM off.

       ret

; Touched deadly block check.
; Returns with DEADLY (must be non-zero) in accumulator if true.

tded   ld l,(ix+8)         ; x coordinate.
       ld h,(ix+9)         ; y coordinate.
       ld (dispx),hl       ; set up test coordinates.
       call tstbl          ; get map address.
       ld de,31            ; default distance to next line down.
       cp b                ; is tl this the required block?
       ret z               ; yes.

       inc hl              ; next cell.
       ld a,(hl)           ; fetch type.
       cp b                ; is ml this deadly/custom?
       ret z               ; yes.

       ld a,(dispy)        ; horizontal position.
       ld c,a              ; store column in c register.
       and %00000111       ; is it straddling cells?
       jr z,tded0          ; no.
       inc hl              ; last cell.
       ld a,(hl)           ; fetch type.
       cp b                ; is tr this the block?
       ret z               ; yes.

       dec de              ; one less cell to next row down.
tded0

IF MFLAG = 0
       add hl,de           ; point to next row.
       ld a,(hl)           ; fetch left cell block.
       cp b                ; is this fatal?
       ret z               ; yes.

       inc hl              ; next cell.
       ld a,(hl)           ; fetch type.
       cp b                ; is this fatal?
       ret z               ; yes.

       ld a,c              ; horizontal position.
       and %00000111       ; is it straddling cells?
       jr z,tded1          ; no.
       inc hl              ; last cell.
       ld a,(hl)           ; fetch type.
       cp b                ; is this fatal?
       ret z               ; yes.
ENDIF

tded1  ld a,(dispx)        ; vertical position.
       and 7               ; is it straddling cells?
       ret z               ; no, job done.
       add hl,de           ; point to next row.
       ld a,(hl)           ; fetch left cell block.
       cp b                ; is this fatal?
       ret z               ; yes.

       inc hl              ; next cell.
       ld a,(hl)           ; fetch type.
       cp b                ; is this fatal?
       ret z               ; yes.

       ld a,c              ; horizontal position.
       and %00000111       ; is it straddling cells?
       ret z               ; no.
       inc hl              ; last cell.
       ld a,(hl)           ; fetch final type.
       ret                 ; return with final type in accumulator.


; Fetch block type at (dispx, dispy).

tstbl  ld a,(dispx)        ; fetch x coord.
       rlca                ; divide by 8,
       rlca                ; and multiply by 32.
       ld d,a              ; store in d.
       and 224             ; mask off high bits.
       ld e,a              ; low byte.
       ld a,d              ; restore shift result.
       and 3               ; high bits.
       ld d,a              ; got displacement in de.
       ld a,(dispy)        ; y coord.
       rra                 ; divide by 8.
       rra
       rra
       and 31              ; only want 0 - 31.
       add a,e             ; add to displacement.
       ld e,a              ; displacement in de.
       ld hl,MAP           ; position of dummy screen.
       add hl,de           ; point to address.
       ld a,(hl)           ; fetch byte there.
       ret

; Jump - if we can.
; Requires initial speed to be set up in accumulator prior to call.

jump   neg                 ; switch sign so we jump up.
       ld c,a              ; store in c register.
;       ld a,(ix+8)         ; x coordinate.
;       ld h,(ix+9)         ; y coordinate.
;numsp4 add a,16            ; look down 16 pixels.
;       ld l,a              ; coords in hl.
;       and 7               ; are we on platform boundary?
;       ret nz              ; no, cannot jump.
;       ld (dispx),hl       ; set up test coordinates.
;       ld b,a              ; copy to b register.
;       call tstbl          ; get map address.
;       call plchk          ; block, platform check.
;       jr nz,jump0         ; it's solid, we can jump.
;       inc hl              ; look right one cell.
;       call plchk          ; block, platform check.
;       jr nz,jump0         ; it's solid, we can jump.
;       ld a,b              ; y coordinate.
;       call rem5           ; position straddling block cells.
;       ret z               ; no more checks needed.
;       inc hl              ; look to third cell.
;       call plchk          ; block, platform check.
;       ret z               ; not solid, don't jump.
jump0  ld a,(ix+13)        ; jumping flag.
       and a               ; is it set?
       ret nz              ; already in the air.
       inc (ix+13)         ; set it.
       ld (ix+14),c        ; set jump height.
       ret

hop    ld a,(ix+13)        ; jumping flag.
       and a               ; is it set?
       ret nz              ; already in the air.
       ld (ix+13),255      ; set it.
       ld (ix+14),0        ; set jump table displacement.
       ret


; Random numbers code.
; Pseudo-random number generator, 8-bit.

random ld hl,seed          ; set up seed pointer.
       ld a,(hl)           ; get last random number.
       ld b,a              ; copy to b register.
       rrca                ; multiply by 32.
       rrca
       rrca
       xor 31
       add a,b
       sbc a,255
       ld (hl),a           ; store new seed.
       ld (varrnd),a       ; return number in variable.
       ret

; Joystick and keyboard reading routines.

; Keyboard test routine.

kget	ld b,1		; reset row

	ld hl,$68fe	; high byte of port to read.
	ld a,l		; low byte
kget0	rrca		; rotate '0' into position.
	ld l,a		; set low byte

	ld a,(hl)	; read key
	cp $ff		; No key pressed?
	jr nz,kget1	; Key pressed, skip
	inc b		; Increment row counter
	ld a,b		; last row?
	cp 8
	jr nz,kget0	; no, repeat
	jr kget2	; yes, end

kget1	ld c,1		; reset column
kget3	rra		; rotate bit out
	jr nc,kget2	; if key pressed, end
	inc c		; increment column counter

;	cp c,6		; end of row?
;	jr z,kget2	; yes, end
;        jr kget3	; if key pressed, end
;kget2	inc b		; next row
;	cp b,8		; all rows read?
;	jr nz kget0	; no, repeat
;	jr kget		; no key pressed, scan again
;
;kget3	sla b		; key pressed, high nibble=row
;	sla b
;	sla b
;	sla b
;	ld a,c
;	add a,b		; low nibble=column
kget2	ret

; Keyboard test routine
; Input:  A = keycode, bit0-3 = row, bit4-7 = column
; Output: Carry set = key not pressed, Carry clear = key pressed


ktest
       push hl
       ld c,a              ; key to test in c.
       and 7               ; mask bits d0-d2 for row.
       ld b,a              ; place in b.

       srl c               ; divide c by 8
       srl c               ; to find position within row.
       srl c               ; column
       srl c

       ld hl,$68fe         ; high byte of port to read.
       ld a,l
ktest0 rrca                ; rotate into position.
       djnz ktest0         ; repeat until we've found relevant row.
       ld l,a
       ld a,(hl)           ; read key
ktest1 rra                 ; rotate bit out of result.
       dec c               ; loop counter.
       jp nz,ktest1        ; repeat until bit for position in carry.
       pop hl
       ret
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
; Joystick and keyboard reading routines.
; The two Joystick units are connected to a plug-in module that
; contains I/O address decoding and switch matrix encoding.
; IC U2 (74LS138) enables I/O reads between 20 - 2F Hex.
; Address lines AO - A3 are used separately to generate active LOW signals
; on the joystick or switch to be read.
; Switch state is then read at the resultant address from Data bits DO - D4.
; When a switch is ON it provides an active-low Data bit. 
;
; JOY1 0x2E    JOY2 0x2B
; U    0xFE    U    0xFE   1111 1110	
; D    0xFD    D    0xFD   1111 1101   
; L    0xFB    L    0xFB   1111 1011   
; R    0xF7    R    0xF7   1111 0111   
; FIRE 0xEF    FIRE 0xEF   1110 1111   

; 'Arm'0x2D (joy1 button 2)
; FIRE 0xEF                1110 1111   

; 'Arm'0x27 (joy2 button 2)
;              FIRE 0xEF   1110 1111 

joykey
       ld a,(contrl)       ; control flag.
       dec a               ; is it joystick 1?
       jr z,joy1           ; yes, read it.
       dec a               ; is it joystick 2?
       jr z,joy2           ; yes, read it.

; Keyboard controls.

       ld hl,keys+6        ; address of last key.
       ld e,0              ; zero reading.
       ld d,7              ; keys to read.
joyke0 ld a,(hl)           ; get key from table.
       call ktest          ; is key pressed?
       ccf                 ; complement the result (0=not pressed,1=pressed).
       rl e                ; rotate into reading.
       dec hl
       dec d
       jp nz,joyke0         ; repeat for all keys.
joyke1 ld a,e              ; copy e register to accumulator.
       ld (joyval),a       ; remember value.
       ret

; Joystick 1.

joy1	in a,($2e)	; read joystick1
	call readjoy	; convert to joyval
	in a,($2d)	; Read arm button joystick1
	jr readarm

; Joystick 2.

joy2	in a,($2b)	; read joystick2
	call readjoy	; convert to joyval
	in a,($27)	; Read arm button joystick1
	jr readarm

readjoy	ld b,5		; read 5 bits from joystick
read0	sra a	
	ccf		; complement the result (0=not pressed,1=pressed).
	rl e
	djnz read0
	rrc e		; convert VZ values to Kempston
	jr nc,rstfire
	set 4,e
	jr joyexit
rstfire res 4,e
joyexit ret

readarm	and $10		; read arm button
	jr z, joy1a
	res 5,e		; Not pressed, carry clear
	jr joy1b
joy1a	set 5,e		; Pressed, carry set

joy1b	ld hl,keys+6	; Read FIRE3 key
	ld a,(hl)
	call ktest
	jr c,joy1c
	set 6,e
	jr joy1d
joy1c	res 6,e
joy1d	jr joyke1

; Display message.

dmsg   ld hl,msgdat        ; pointer to messages.
       call getwrd         ; get message number.
dmsg3  call preprt         ; pre-printing stuff.
       call checkx         ; make sure we're in a printable range.
       ld a,(prtmod)       ; print mode.
       and a               ; standard size?
       jp nz,bmsg1         ; no, double-height text.
dmsg0  push hl             ; store string pointer.
       ld a,(hl)           ; fetch byte to display.
       and 127             ; remove any end marker.
       cp 13               ; newline character?
       jr z,dmsg1
       call pchar          ; display character.

       ; no attribute.

       call nexpos         ; display position.
       jr nz,dmsg2         ; not on a new line.
       call nexlin         ; next line down.
dmsg2  pop hl
       ld a,(hl)           ; fetch last character.
       rla                 ; was it the end?
       jp c,dscor2         ; yes, job done.
       inc hl              ; next character to display.
       jr dmsg0
dmsg1  ld hl,dispx         ; x coordinate.
       inc (hl)            ; newline.
       ld a,(hl)           ; fetch position.
       cp SCRHGT               ; past screen edge?
       jr c,dmsg4          ; no, it's okay.
       ld (hl),0           ; restart at top.
dmsg4  inc hl              ; y coordinate.
       ld (hl),0           ; carriage return.
       jr dmsg2
prtmod defb 0              ; print mode, 0 = standard, 1 = double-height.

; Display message in big text.

bmsg1  ld a,(hl)           ; get character to display.
       push hl             ; store pointer to message.
       and 127             ; only want 7 bits.
       cp 13               ; newline character?
       jr z,bmsg2          ; newline instead.
       call bchar          ; display big char.
bmsg3  pop hl              ; retrieve message pointer.
       ld a,(hl)           ; look at last character.
       inc hl              ; next character in list.
       rla                 ; was terminator flag set?
       jr nc,bmsg1         ; no, keep going.
       ret
bmsg2  ld hl,charx         ; x coordinate.
       inc (hl)            ; newline.
       inc (hl)            ; newline.
       ld a,(hl)           ; fetch position.
       cp 23               ; past screen edge?
       jr c,bmsg3          ; no, it's okay.
       ld (hl),0           ; restart at top.
       inc hl              ; y coordinate.
       ld (hl),0           ; carriage return.
       jr bmsg3

; Big character display.


bchar  rlca                ; multiply char by 8.
       rlca
       rlca
       ld e,a              ; store shift in e.
       and %00000111       ; only want high byte bits.
       ld d,a              ; store in d.
       ld a,e              ; restore shifted value.
       and %11111000       ; only want low byte bits.
       ld e,a              ; that's the low byte. (DE=char*8).
       ld hl,(grbase)      ; address of graphics.
       add hl,de           ; add displacement.
    IF GFLAG
       ld de,(bgmask)
    ENDIF
       exx
       call gprad          ; get screen address (in DE).
       ex de,hl            ; (in HL).
       ld de,32            ; distance to next line.
       ld b,8              ; height of character in font.

;       ld a,(MODBUF)       ; get current screen mode. (bit 0 = 0 to bank-switch VRAM on.)
;       out (COL32),a       ; (set screen mode and) bank-switch VRAM on.
;       ld c,a              ; store screen mode.

bchar0 exx
       ld a,(hl)           ; get a byte of the font.
       inc hl              ; next line of font.
    IF GFLAG
       and d               ; apply foreground color.
       xor e               ; apply background color.
    ENDIF

IF IFLAG = 1
	xor 255
ENDIF

       exx
       ld (hl),a           ; write to screen.
       add hl,de           ; down a line.
       ld (hl),a           ; write to screen.
       add hl,de           ; next line down.
       ld a,h

       cp (VRAM + SCRHGT * 8 * 32) >>> 8 ; past lower edge of screen?
       jr nc,bchar1        ; yes, stop copying.
       djnz bchar0         ; repeat.
bchar1

;       ld a,c              ; restore screen mode.
;       inc a               ; set bit 0 to bank-switch VRAM off.
;       out (COL32),a       ; (set screen mode and) bank-switch VRAM off.

       call nexpos         ; display position.
       jp nz,bchar2        ; not on a new line.
bchar3 inc (hl)            ; newline.
       call nexlin         ; next line check.
bchar2 jp dscor2           ; tidy up line and column variables.

; Display a character.

achar  ld b,a              ; copy to b.
       call preprt         ; get ready to print.
       ld a,(prtmod)       ; print mode.
       and a               ; standard size?
       ld a,b              ; character in accumulator.
       jp nz,bchar         ; no, double-height text.
       call pchar          ; display character.

       jp bchar1            ; advance position.


; Get next print column position.

nexpos ld hl,dispy         ; display position.
       ld a,(hl)           ; get coordinate.
       inc a               ; move along one position.
       and 31              ; reached edge of screen?
       ld (hl),a           ; set new position.
       dec hl              ; point to x now.
       ret                 ; return with status in zero flag.

; Get next print line position.

nexlin inc (hl)            ; newline.
       ld a,(hl)           ; vertical position.
       cp 64               ; past screen edge?
       ret c               ; no, still okay.
       ld (hl),0           ; restart at top.
       ret

; Pre-print preliminaries.


preprt ld de,font-256      ; font pointer.
       ld (grbase),de      ; set up graphics base.
prescr ld de,(charx)       ; display coordinates.

       ld (dispx),de       ; set up general coordinates.
       ret

; On entry: hl points to word list
;           a contains word number.

getwrd and a               ; first word in list?
       ret z               ; yep, don't search.
       ld b,a
getwd0 ld a,(hl)
       inc hl
       cp %10000000        ; found end?
       jr c,getwd0         ; no, carry on.
       djnz getwd0         ; until we have right number.
       ret


; Bubble sort.

bsort  ld b,NUMSPR - 1     ; sprites to swap.
       ld ix,sprtab        ; sprite table.
bsort0 push bc             ; store loop counter for now.

       ld a,(ix+0)         ; first sprite type.
       inc a               ; is it switched off?
       jr z,swemp          ; yes, may need to switch another in here.

       ld a,(ix+TABSIZ)    ; check next slot exists.
       inc a               ; is it enabled?
       jr z,bsort2         ; no, nothing to swap.

       ld a,(ix+(3+TABSIZ)); fetch next sprite's coordinate.
       cp (ix+3)           ; compare with this x coordinate.
       jr c,bsort1         ; next sprite is higher - may need to switch.
bsort2 ld de,TABSIZ        ; distance to next odd/even entry.
       add ix,de           ; next sprite.
       pop bc              ; retrieve loop counter.
       djnz bsort0         ; repeat for remaining sprites.
       ret

bsort1 ld a,(ix+TABSIZ)    ; sprite on/off flag.
       inc a               ; is it enabled?
       jr z,bsort2         ; no, nothing to swap.
       call swspr          ; swap positions.
       jr bsort2

swemp  ld a,(ix+TABSIZ)    ; next table entry.
       inc a               ; is that one on?
       jr z,bsort2         ; no, nothing to swap.
       call swspr          ; swap positions.
       jr bsort2

; Swap sprites.

swspr  push ix             ; table address on stack.
       pop hl              ; pop into hl pair.
       ld d,h              ; copy to de pair.
       ld e,l
       ld bc,TABSIZ        ; distance to second entry.
       add hl,bc           ; point to second sprite entry.
       ld b,TABSIZ         ; bytes to swap.
swspr0 ld c,(hl)           ; fetch second byte.
       ld a,(de)           ; fetch first byte.
       ld (hl),a           ; copy to second.
       ld a,c              ; second byte in accumulator.
       ld (de),a           ; copy to first sprite entry.
       inc de              ; next byte.
       inc hl              ; next byte.
       djnz swspr0         ; swap all bytes in table entry.
       ret


; Process sprites.

pspr   ld b,NUMSPR         ; sprites to process.
       ld ix,sprtab        ; sprite table.
pspr1  push bc             ; store loop counter for now.
       ld a,(ix+0)         ; fetch sprite type.
       cp 9                ; within range of sprite types?
       call c,pspr2        ; yes, process this one.
       ld de,TABSIZ        ; distance to next odd/even entry.
       add ix,de           ; next sprite.
       pop bc              ; retrieve loop counter.
       djnz pspr1          ; repeat for remaining sprites.
       ret
pspr2  ld (ogptr),ix       ; store original sprite pointer.
       call pspr3          ; do the routine.
rtorg  ld ix,(ogptr)       ; restore original pointer to sprite.
rtorg0 ret
pspr3  ld hl,evtyp0        ; sprite type events list.
pspr4  add a,a             ; double accumulator.
       ld e,a              ; copy to de.
       ld d,0              ; no high byte.
       add hl,de           ; point to address of routine.
       ld e,(hl)           ; address low.
       inc hl              ; next byte of address.
       ld d,(hl)           ; address high.
       ex de,hl            ; swap address into hl.
       jp (hl)             ; go there.
ogptr  defw 0              ; original sprite pointer.

; Address of each sprite type's routine.

evtyp0 defw evnt00
evtyp1 defw evnt01
evtyp2 defw evnt02
evtyp3 defw evnt03
evtyp4 defw evnt04
evtyp5 defw evnt05
evtyp6 defw evnt06
evtyp7 defw evnt07
evtyp8 defw evnt08


; Display sprites.

dspr   ld b,NUMSPR/2       ; number of sprites to display.
dspr0  push bc             ; store loop counter for now.
       ld a,(ix+0)         ; get sprite type.
       inc a               ; is it enabled?
       jr nz,dspr1         ; yes, it needs deleting.
dspr5  ld a,(ix+5)         ; new type.
       inc a               ; is it enabled?
       jr nz,dspr3         ; yes, it needs drawing.

dspr2  push ix             ; put ix on stack.
       pop hl              ; pop into hl.
       ld e,l              ; copy to de.
       ld d,h

;dspr2  ld e,ixl            ; copy ix to de.
;       ld d,ixh
;       ld l,e              ; copy to hl.
;       ld h,d
       ld bc,5             ; distance to new type.
       add hl,bc           ; point to new properties.
       ldi                 ; copy to old positions.
       ldi
       ldi
       ldi
       ldi
       ld c,TABSIZ*2       ; distance to next odd/even entry.
       add ix,bc           ; next sprite.
       pop bc              ; retrieve loop counter.
       djnz dspr0          ; repeat for remaining sprites.
       ret

;dspr1  ld a,(ix+3)         ; old x coord.
;       cp 177              ; beyond maximum?
;       jr nc,dspr5         ; yes, don't delete it.
dspr1  ld a,(ix+5)         ; type of new sprite.
       inc a               ; is this enabled?
       jr nz,dspr4         ; yes, display both.
dspr6  call sspria         ; show single sprite.
       jp dspr2

; Displaying two sprites.  Don't bother redrawing if nothing has changed.

dspr4  ld a,(ix+4)         ; old y.
       cp (ix+9)           ; compare with new value.
       jr nz,dspr7         ; they differ, need to redraw.
       ld a,(ix+3)         ; old x.
       cp (ix+8)           ; compare against new value.
       jr nz,dspr7         ; they differ, need to redraw.
       ld a,(ix+2)         ; old frame.
       cp (ix+7)           ; compare against new value.
       jr nz,dspr7         ; they differ, need to redraw.
       ld a,(ix+1)         ; old image.
       cp (ix+6)           ; compare against new value.
       jp z,dspr2          ; everything is the same, don't redraw.
dspr7  call sspric         ; delete old sprite, draw new one simultaneously.
       jp dspr2
dspr3  call ssprib         ; show single sprite.
       jp dspr2


; Get sprite address calculations.
; gspran = new sprite, gsprad = old sprite.

gspran ld l,(ix+8)         ; new x coordinate.
       ld h,(ix+9)         ; new y coordinate.
       ld (dispx),hl       ; set display coordinates.
       ld a,(ix+6)         ; new sprite image.
       call gfrm           ; fetch start frame for this sprite.
       ld a,(hl)           ; frame in accumulator.
       add a,(ix+7)        ; new add frame number.
       jp gspra0

gsprad ld l,(ix+3)         ; x coordinate.
       ld h,(ix+4)         ; y coordinate.
       ld (dispx),hl       ; set display coordinates.
       ld a,(ix+1)         ; sprite image.
       call gfrm           ; fetch start frame for this sprite.
       ld a,(hl)           ; frame in accumulator.
       add a,(ix+2)        ; add frame number.

gspra0

IF MFLAG = 0
       rrca                ; multiply by 128.
       ld d,a              ; store in d.
       and 128             ; low byte bit.
       ld e,a              ; got low byte.
       ld a,d              ; restore result.
       and 127             ; high byte bits.
       ld d,a              ; displacement high byte.
ELSE
       rrca                ; multiply by 64.
       rrca                ; .
       ld d,a              ; store in d.
       and $c0             ; low byte bit 0,1
       ld e,a              ; got low byte.
       ld a,d              ; restore result.
       and $3f             ; high byte bits.
       ld d,a              ; displacement high byte.
ENDIF

       ld hl,sprgfx        ; address of play sprites.
       add hl,de           ; point to frame.

       ld a,(dispy)        ; y coordinate.
       and 6               ; position within byte boundary.
       ld c,a              ; low byte of table displacement.

IF MFLAG = 0
       rlca                ; multiply by 32.
ENDIF
       rlca                ; already a multiple
       rlca                ; of 2, so just 4
       rlca                ; shifts needed.
       ld e,a              ; put displacement in low byte of de.
       ld d,0              ; zero the high byte.
       ld b,d              ; no high byte for mask displacement either.
       add hl,de           ; add to sprite address.
       ex de,hl            ; need it in de for now.
       ld hl,spmask        ; pointer to mask table.
       add hl,bc           ; add displacement to pointer.
       ld c,(hl)           ; left mask.
       inc hl
       ld b,(hl)           ; right mask.

; Drop into screen address routine.
; This routine returns a screen address for (dispx, dispy) in hl.


scadd
       ld hl,(dispx)       ; get vertical pixel coordinate in L, horizontal in H.

       ld a,h              ; copy horizontal coordinate in A.
       srl l               ; shift the pair LA 3 times to the right, inserting zeroes to the left.
       rra                 ; (making LA = 32 * vertical coordinate + horizontal coordinate / 8)
       srl l
       rra
       srl l
       rra
       ld h,l              ; copy the result in LA to HL.
       ld l,a
       ld a,VRAM >> 8      ; Add address of start of VRAM to HL.
       add a,h
       ld h,a

	ld a,(dispx)
	cp 191
	ret c

	ld a,h
	sub $20
	ld h,a

       ret


spmask defb %11111111,%00000000
       defb %00111111,%11000000
       defb %00001111,%11110000
       defb %00000011,%11111100


; These are the sprite routines.
; sspria = single sprite, old (ix).
; ssprib = single sprite, new (ix+5).
; sspric = both sprites, old (ix) and new (ix+5).

sspria call gsprad         ; get old sprite address.
sspri2 ld a,COLDIST + 1             ; vertical lines
sspri0 ex af,af'           ; store line counter away in alternate registers.
       call dline          ; draw a line.
       ex af,af'           ; restore line counter.
       dec a               ; one less to go.
       jp nz,sspri0
       ret

ssprib call gspran         ; get new sprite address.
       jp sspri2

sspric call gsprad         ; get old sprite address.
       exx                 ; store addresses.
       call gspran         ; get new sprite addresses.
       call dline          ; draw a line.
       exx                 ; restore old addresses.
       call dline          ; delete a line.
       exx                 ; flip to new sprite addresses.
       call dline          ; draw a line.
       exx                 ; restore old addresses.
       call dline          ; delete a line.
       exx                 ; flip to new sprite addresses.
       call dline          ; draw a line.
       exx                 ; restore old addresses.
       call dline          ; delete a line.
       exx                 ; flip to new sprite addresses.
       call dline          ; draw a line.
       exx                 ; restore old addresses.
       call dline          ; delete a line.
       exx                 ; flip to new sprite addresses.
       call dline          ; draw a line.
       exx                 ; restore old addresses.
       call dline          ; delete a line.
       exx                 ; flip to new sprite addresses.
       call dline          ; draw a line.
       exx                 ; restore old addresses.
       call dline          ; delete a line.
       exx                 ; flip to new sprite addresses.
       call dline          ; draw a line.
       exx                 ; restore old addresses.
       call dline          ; delete a line.
       exx                 ; flip to new sprite addresses.
       call dline          ; draw a line.
       exx                 ; restore old addresses.

IF MFLAG = 0
       call dline          ; delete a line.
       exx                 ; flip to new sprite addresses.
       call dline          ; draw a line.
       exx                 ; restore old addresses.
       call dline          ; delete a line.
       exx                 ; flip to new sprite addresses.
       call dline          ; draw a line.
       exx                 ; restore old addresses.
       call dline          ; delete a line.
       exx                 ; flip to new sprite addresses.
       call dline          ; draw a line.
       exx                 ; restore old addresses.
       call dline          ; delete a line.
       exx                 ; flip to new sprite addresses.
       call dline          ; draw a line.
       exx                 ; restore old addresses.
       call dline          ; delete a line.
       exx                 ; flip to new sprite addresses.
       call dline          ; draw a line.
       exx                 ; restore old addresses.
       call dline          ; delete a line.
       exx                 ; flip to new sprite addresses.
       call dline          ; draw a line.
       exx                 ; restore old addresses.
       call dline          ; delete a line.
       exx                 ; flip to new sprite addresses.
       call dline          ; draw a line.
       exx                 ; restore old addresses.
       call dline          ; delete a line.
       exx                 ; flip to new sprite addresses.
       call dline          ; draw a line.
       exx                 ; restore old addresses.
ENDIF

; Drop through.
; Line drawn, now work out next target address.


dline
IF XFLAG = 1 or XFLAG = 2
	push hl
ENDIF
	ld a,h
	cp $70
	jr nc,goon
	inc de
	inc de
	jr skipline
goon
IF XFLAG = 1 or XFLAG = 2
	sub $70
	srl a
	srl a
	srl a
	and 3
	cp 3
	jr z,skipline
	add a,XMODE
	out (XPORT),a
	ld a,h
	and $07
	or $70
	ld h,a
ENDIF

       ld a,(de)           ; graphic data.

       and c               ; mask away what's not needed.
       xor (hl)            ; XOR with what's there.
       ld (hl),a           ; bung it in.
       inc l               ; next screen address.
       inc l               ; next screen address.
       ld a,(de)           ; fetch data.
       and b               ; mask away unwanted bits.
       xor (hl)            ; XOR with what's there.
       ld (hl),a           ; bung it in.
       inc de              ; next graphic.
       dec l               ; one character cell to the left.
       ld a,(de)           ; second bit of data.
       xor (hl)            ; XOR with what's there.
       ld (hl),a           ; bung it in.
       inc de              ; point to next line of data.
       dec l               ; another char left.

skipline
IF XFLAG = 1 or XFLAG = 2
	pop hl
ENDIF

;       ld a,(MODBUF)       ; get video mode with bit 0 = 0.
;       inc a               ; set bit 0 = 1 to bank-switch VRAM off.
;       out (COL32),a       ; (set video mode and) bank-switch VRAM off.


; Line drawn, now work out next target address.


nline  ld a,32             ; next screen line.
       add a,l
       ld l,a
       ld a,0
       adc a,h
       ld h,a

IF XFLAG = 0
       cp (VRAM + SCRHGT * 8 * 32) >> 8 ; reached end of screen?
       ret c               ; not yet.
       ld h,ROM >> 8       ; take a ROM address, so next tries to write sprite bytes into VRAM will be lost.
ENDIF

       ret

; Animates a sprite.

animsp ld hl,frmno         ; game frame.
       and (hl)            ; is it time to change the frame?
       ret nz              ; not this frame.
       ld a,(ix+6)         ; sprite image.
       call gfrm           ; get frame data.
       inc hl              ; point to frames.
       ld a,(ix+7)         ; sprite frame.
       inc a               ; next one along.
       cp (hl)             ; reached the last frame?
       jr c,anims0         ; no, not yet.
       xor a               ; start at first frame.
anims0 ld (ix+7),a         ; new frame.
       ret

animbk ld hl,frmno         ; game frame.
       and (hl)            ; is it time to change the frame?
       ret nz              ; not this frame.
       ld a,(ix+6)         ; sprite image.
       call gfrm           ; get frame data.
       inc hl              ; point to frames.
       ld a,(ix+7)         ; sprite frame.
       and a               ; first one?
       jr nz,rtanb0        ; yes, start at end.
       ld a,(hl)           ; last sprite.
rtanb0 dec a               ; next one along.
       jr anims0           ; set new frame.

; Check for collision with other sprite, strict enforcement.

sktyp  ld hl,sprtab        ; sprite table.
;<zx>
numsp2 ld a,NUMSPR         ; number of sprites.
sktyp0 ex af,af'           ; store loop counter.
       ld (skptr),hl       ; store pointer to sprite.
       ld a,(hl)           ; get sprite type.
       cp b                ; is it the type we seek?
       jr z,coltyp         ; yes, we can use this one.
sktyp1 ld hl,(skptr)       ; retrieve sprite pointer.
       ld de,TABSIZ        ; size of each entry.
       add hl,de           ; point to next sprite in table.
       ex af,af'           ; restore loop counter.
       dec a               ; one less iteration.
       jp nz,sktyp0        ; keep going until we find a slot.
       ld hl,0             ; default to ROM address - no sprite.
       ld (skptr),hl       ; store pointer to sprite.
       or h                ; don't return with zero flag set.
       ret                 ; didn't find one.
skptr  defw 0              ; search pointer.

coltyp ld a,(ix+0)         ; current sprite type.
       cp b                ; seeking sprite of same type?
       jr z,colty1         ; yes, need to check we're not detecting ourselves.
colty0 ld de,X             ; distance to x position in table.
       add hl,de           ; point to coords.
       ld e,(hl)           ; fetch x coordinate.
       inc hl              ; now point to y.
       ld d,(hl)           ; that's y coordinate.

; Drop into collision detection.

colc16 ld a,(ix+X)         ; x coord.
       sub e               ; subtract x.
       jr nc,colc1a        ; result is positive.
       neg                 ; make negative positive.
colc1a cp COLDIST +1              ; within x range?
       jr nc,sktyp1        ; no - they've missed.
       ld c,a              ; store difference.
       ld a,(ix+Y)         ; y coord.
       sub d               ; subtract y.
       jr nc,colc1b        ; result is positive.
       neg                 ; make negative positive.
colc1b cp 16               ; within y range?
       jr nc,sktyp1        ; no - they've missed.
       add a,c             ; add x difference.
       cp 26               ; only 5 corner pixels touching?
       ret c               ; carry set if there's a collision.
       jp sktyp1           ; try next sprite in table.

colty1 push ix             ; base sprite address onto stack.
       pop de              ; pop it into de.
       ex de,hl            ; flip hl into de.
       sbc hl,de           ; compare the two.
       ex de,hl            ; restore hl.
       jr z,sktyp1         ; addresses are identical.
       jp colty0

; Display number.

disply ld bc,displ0        ; display workspace.
       call num2ch         ; convert accumulator to string.
displ1 dec bc              ; back one character.
       ld a,(bc)           ; fetch digit.
       or 128              ; insert end marker.
       ld (bc),a           ; new value.
       ld hl,displ0        ; display space.
       jp dmsg3            ; display the string.
displ0 defb 0,0,0,13+128


; Initialise screen.

initsc ld a,(roomtb)       ; whereabouts in the map are we?
       call tstsc          ; find displacement.
       cp 255              ; is it valid?
       ret z               ; no, it's rubbish.
       ld (scno),a         ; store new room number.
       ret

; Test screen.

tstsc  ld hl,mapdat-MAPWID ; start of map data, subtract width for negative.
       ld b,a              ; store room in b for now.
       add a,MAPWID        ; add width in case we're negative.
       ld e,a              ; screen into e.
       ld d,0              ; zeroise d.
       add hl,de           ; add displacement to map data.
       ld a,(hl)           ; find room number there.
       ret

; Screen left.

scrl   ld a,(roomtb)       ; present room table pointer.
       dec a               ; room left.
scrl0  call tstsc          ; test screen.
       inc a               ; is there a screen this way?
       ret z               ; no, return to loop.
       ld a,b              ; restore room displacement.
       ld (roomtb),a       ; new room table position.
scrl1  call initsc         ; set new screen.
       ld hl,restfl        ; restart screen flag.
       ld (hl),2           ; set it.
       ret
scrr   ld a,(roomtb)       ; room table pointer.
       inc a               ; room right.
       jr scrl0
scru   ld a,(roomtb)       ; room table pointer.
       sub MAPWID          ; room up.
       jr scrl0
scrd   ld a,(roomtb)       ; room table pointer.
       add a,MAPWID        ; room down.
       jr scrl0

; Jump to new screen.

nwscr  ld hl,mapdat        ; start of map data.
       ld bc,256*80        ; zero room count, 80 to search.
nwscr0 cp (hl)             ; have we found a match for screen?
       jr z,nwscr1         ; yes, set new point in map.
       inc hl              ; next room.
       inc c               ; count rooms.
       djnz nwscr0         ; keep looking.
       ret
nwscr1 ld a,c              ; room displacement.
       ld (roomtb),a       ; set the map position.
       jr scrl1            ; draw new room.


; Gravity processing.

grav   ld a,(ix+13)        ; in-air flag.
       and a               ; are we in the air?
       ret z               ; no we are not.
       inc a               ; increment it.
       jp z,ogrv           ; set to 255, use old gravity.
       ld (ix+13),a        ; write new setting.
       rra                 ; every other frame.
       jr nc,grav0         ; don't apply gravity this time.
       ld a,(ix+14)        ; pixels to move.
       cp 16               ; reached maximum?
       jr z,grav0          ; yes, continue.
       inc (ix+14)         ; slow down ascent/speed up fall.
grav0  ld a,(ix+14)        ; get distance to move.
       sra a               ; divide by 2.
       and a               ; any movement required?
       ret z               ; no, not this time.
       cp 128              ; is it up or down?
       jr nc,gravu         ; it's up.
gravd  ld b,a              ; set pixels to move.
gravd0 call cangd          ; can we go down?
       jr nz,gravst        ; can't move down, so stop.
       inc (ix+8)          ; adjust new x coord.
       djnz gravd0
       ret
gravu  neg                 ; flip the sign so it's positive.
       ld b,a              ; set pixels to move.
gravu0 call cangu          ; can we go up?
       jp nz,ifalls        ; can't move up, go down next.
       dec (ix+8)          ; adjust new x coord.
       djnz gravu0
       ret
gravst ld a,(ix+14)        ; jump pointer high.
       ld (ix+13),0        ; reset falling flag.
       ld (ix+14),0        ; store new speed.
       cp 8                ; was speed the maximum?
evftf  jp z,evnt15         ; yes, fallen too far.
       ret

; Old gravity processing for compatibility with Spectrum versions 4.6 and 4.7.

ogrv   ld e,(ix+14)        ; get index to table.
       ld d,0              ; no high byte.
       ld hl,jtab          ; jump table.
       add hl,de           ; hl points to jump value.
       ld a,(hl)           ; pixels to move.
       cp 99               ; reached the end?
       jr nz,ogrv0         ; no, continue.
       dec hl              ; go back to previous value.
       ld a,(hl)           ; fetch that from table.
       jr ogrv1
ogrv0  inc (ix+14)         ; point to next table entry.
ogrv1  and a               ; any movement required?
       ret z               ; no, not this time.
       cp 128              ; is it up or down?
       jr nc,ogrvu         ; it's up.
ogrvd  ld b,a              ; set pixels to move.
ogrvd0 call cangd          ; can we go down?
       jr nz,ogrvst        ; can't move down, so stop.
       inc (ix+8)          ; adjust new x coord.
       djnz ogrvd0
       ret
ogrvu  neg                 ; flip the sign so it's positive.
       ld b,a              ; set pixels to move.
ogrvu0 call cangu          ; can we go up?
       jr nz,ogrv2         ; can't move up, go down next.
       dec (ix+8)          ; adjust new x coord.
       djnz ogrvu0
       ret
ogrvst ld e,(ix+14)        ; get index to table.
       ld d,0              ; no high byte.
       ld hl,jtab          ; jump table.
       add hl,de           ; hl points to jump value.
       ld a,(hl)           ; fetch byte from table.
       cp 99               ; is it the end marker?
       ld (ix+13),0        ; reset jump flag.
       ld (ix+14),0        ; reset pointer.
       jp evftf
ogrv2  ld hl,jtab          ; jump table.
       ld b,0              ; offset into table.
ogrv4  ld a,(hl)           ; fetch table byte.
       cp 100              ; hit end or downward move?
       jr c,ogrv3          ; yes.
       inc hl              ; next byte of table.
       inc b               ; next offset.
       jr ogrv4            ; keep going until we find crest/end of table.
ogrv3  ld (ix+14),b        ; set next table offset.
       ret

; Initiate fall check.

ifall  ld a,(ix+13)        ; jump pointer flag.
       and a               ; are we in the air?
       ret nz              ; if set, we're already in the air.
       ld h,(ix+9)         ; y coordinate.
       ld a,COLDIST + 1            ; look down 16 pixels.
       add a,(ix+8)        ; add x coordinate.
       ld l,a              ; coords in hl.
       ld (dispx),hl       ; set up test coordinates.
       call tstbl          ; get map address.
       call plchk          ; block, platform check.
       ret nz              ; it's solid, don't fall.
       inc hl              ; look right one cell.
       call plchk          ; block, platform check.
       ret nz              ; it's solid, don't fall.
       ld a,(dispy)        ; y coordinate.
       and 7               ; position straddling block cells.
       jr z,ifalls         ; no more checks needed.
       inc hl              ; look to third cell.
       call plchk          ; block, platform check.
       ret nz              ; it's solid, don't fall.
ifalls inc (ix+13)         ; set in air flag.
       ld (ix+14),0        ; initial speed = 0.
       ret
tfall  ld a,(ix+13)        ; jump pointer flag.
       and a               ; are we in the air?
       ret nz              ; if set, we're already in the air.
       call ifall          ; do fall test.
       ld a,(ix+13)        ; get falling flag.
       and a               ; is it set?
       ret z               ; no.
       ld (ix+13),255      ; we're using the table.
       jr ogrv2            ; find position in table.


; Get frame data for a particular sprite.

gfrm   rlca                ; multiple of 2.
       ld e,a              ; copy to de.
       ld d,0              ; no high byte as max sprite is 128.
       ld hl,(frmptr)      ; frames used by game.
       add hl,de           ; point to frame start.
       ret

; Find sprite list for current room.

sprlst ld a,(scno)         ; screen number.
sprls2 ld hl,(nmeptr)      ; pointer to enemies.
       ld b,a              ; loop counter in b register.
       and a               ; is it the first screen?
       ret z               ; yes, don't need to search data.
       ld de,NMESIZ        ; bytes to skip.
sprls1 ld a,(hl)           ; fetch type of sprite.
       inc a               ; is it an end marker?
       jr z,sprls0         ; yes, end of this room.
       add hl,de           ; point to next sprite in list.
       jr sprls1           ; continue until end of room.
sprls0 inc hl              ; point to start of next screen.
       djnz sprls1         ; continue until room found.
       ret


; Clear all but a single player sprite.

nspr   ld b,NUMSPR         ; sprite slots in table.
       ld ix,sprtab        ; sprite table.
       ld de,TABSIZ        ; distance to next odd/even entry.
nspr0  ld a,(ix+0)         ; fetch sprite type.
       and a               ; is it a player?
       jr z,nspr1          ; yes, keep this one.
       ld (ix+0),255       ; delete sprite.
       ld (ix+5),255       ; remove next type.
       add ix,de           ; next sprite.
       djnz nspr0          ; one less space in the table.
       ret
nspr1  ld (ix+0),255       ; delete sprite.
       add ix,de           ; point to next sprite.
       djnz nspr2          ; one less to do.
       ret
nspr2  ld (ix+0),255       ; delete sprite.
       ld (ix+5),255       ; remove next type.
       add ix,de           ; next sprite.
       djnz nspr2          ; one less space in the table.
       ret


; Two initialisation routines.
; Initialise sprites - copy everything from list to table.

ispr   ld b,NUMSPR         ; sprite slots in table.
       ld ix,sprtab        ; sprite table.
ispr2  ld a,(hl)           ; fetch byte.
       cp 255              ; is it an end marker?
       ret z               ; yes, no more to do.
ispr1  ld a,(ix+0)         ; fetch sprite type.
       cp 255              ; is it enabled yet?
       jr nz,ispr4         ; yes, try another slot.
       ld a,(ix+5)         ; next type.
       cp 255              ; is it enabled yet?
       jr z,ispr3          ; no, process this one.
ispr4  ld de,TABSIZ        ; distance to next odd/even entry.
       add ix,de           ; next sprite.
       djnz ispr1          ; repeat for remaining sprites.
       ret                 ; no more room in table.
ispr3  call cpsp           ; initialise a sprite.
       djnz ispr2          ; one less space in the table.
       ret

; Initialise sprites - but not player, we're keeping the old one.

kspr   ld b,NUMSPR         ; sprite slots in table.
       ld ix,sprtab        ; sprite table.
kspr2  ld a,(hl)           ; fetch byte.
       cp 255              ; is it an end marker?
       ret z               ; yes, no more to do.
       and a               ; is it a player sprite?
       jr nz,kspr1         ; no, add to table as normal.
       ld de,NMESIZ        ; distance to next item in list.
       add hl,de           ; point to next one.
       jr kspr2
kspr1  ld a,(ix+0)         ; fetch sprite type.
       cp 255              ; is it enabled yet?
       jr nz,kspr4         ; yes, try another slot.
       ld a,(ix+5)         ; next type.
       cp 255              ; is it enabled yet?
       jr z,kspr3          ; no, process this one.
kspr4  ld de,TABSIZ        ; distance to next odd/even entry.
       add ix,de           ; next sprite.
       djnz kspr1          ; repeat for remaining sprites.
       ret                 ; no more room in table.
kspr3  call cpsp           ; copy sprite to table.
       djnz kspr2          ; one less space in the table.
       ret

; Copy sprite from list to table.

cpsp   ld a,(hl)           ; fetch byte from table.
       ld (ix+0),a         ; set up type.
       ld (ix+PAM1ST),a    ; set up type.
       inc hl              ; move to next byte.
       ld a,(hl)           ; fetch byte from table.
       ld (ix+6),a         ; set up image.
       inc hl              ; move to next byte.
       ld a,(hl)           ; fetch byte from table.
       ld (ix+3),200       ; set initial coordinate off screen.
       ld (ix+8),a         ; set up coordinate.
       inc hl              ; move to next byte.
       ld a,(hl)           ; fetch byte from table.
       ld (ix+9),a         ; set up coordinate.
       inc hl              ; move to next byte.
       xor a               ; zeroes in accumulator.
       ld (ix+7),a         ; reset frame number.
       ld (ix+10),a        ; reset direction.
;       ld (ix+12),a        ; reset parameter B.
       ld (ix+13),a        ; reset jump pointer low.
       ld (ix+14),a        ; reset jump pointer high.
       ld (ix+16),255      ; reset data pointer to auto-restore.
       push ix             ; store ix pair.
       push hl             ; store hl pair.
       push bc
evis0  call evnt09         ; perform event.
       pop bc
       pop hl              ; restore hl.
       pop ix              ; restore ix.
       ld de,TABSIZ        ; distance to next odd/even entry.
       add ix,de           ; next sprite.
       ret

; Clear the play area window.

clw    ld hl,(wintop)      ; get top-left coordinates of window.
       ld (dispy),hl       ; put into dispx for calculation.

       call gprad          ; get address of top-left byte of window in VRAM.
       ex de,hl            ; store in HL.
       ld a,(winhgt)       ; get height of window...
       add a,a             ; ...in pixels (winhgt * 8).
       add a,a
       add a,a
       ld b,a              ; initialize vertical counter.
       ld a,(winwid)       ; get width of window in bytes.
       ld c,a              ; store initial value for horizontal counter.
clw0   push bc             ; store vertical counter and initial value for horizontal counter.
       ld b,c              ; initialize horizontal counter.
       ld e,l              ; store address of leftmost byte of window in RAM.
       ld d,h
clw1
IF XFLAG = 1 OR XFLAG = 2
	push hl
	ld a,h
	sub $70
	srl a
	srl a
	srl a
	and 3
	add a,XMODE
	out (XPORT),a
	ld a,h
	and $07
	or $70
	ld h,a
ENDIF
       ld a,FILL       ; get background color.
       ld (hl),a           ; write background color to VRAM.

IF XFLAG = 1 OR XFLAG = 2
	pop hl
ENDIF

       inc hl              ; advance 1 byte to the right.
       djnz clw1           ; repeat until window row filled.
       ld hl,32            ; advance to leftmost byte of next row of window.
       add hl,de
       pop bc              ; restore vertical counter and initial value for horizontal counter.
       djnz clw0           ; repeat until all window rows filled.

       ld hl,(wintop)      ; get coordinates of window.
       ld (charx),hl       ; put into display position.
       ret

;<cpc>
;; Redefine key.
;; Go through table of 80 keys testing each one.
;; Return code for first detected keypress.
;
;redky  call debkey         ; debounce previous key.
       ;ld b,80             ; codes to check.
;redky0 ld a,b              ; put code in accumulator.
       ;dec a               ; test 0 - 79, not 1 - 80.
       ;call 47902          ; check if key pressed.
       ;jr nz,redky1        ; pressed.
       ;djnz redky0         ; repeat until we've scanned them all.
       ;jr redky            ; repeat until something is pressed.
;redky1 dec b               ; always one less.
       ;ret
;</cpc>

; Effects code.
; Ticker routine is called 25 times per second.

scrly	ret
       defw txtscr         ; get screen address.

; Scroll 8 lines txtwid

	ld b,8		; 8 pixel rows.
	push hl		; store screen address.
scrly1	push bc		; store rows on stack.
	push hl		; store screen address.

; Scroll 1 line txtwid

	ld a,(txtwid)	; characters wide.
	ld b,a		; put into the loop counter.

;	IF GFLAG
;	  xor a		; reset carry flag and msbit of accumulator.
;	ELSE
	  and a		; reset carry flag.
;	ENDIF

scrly0
	rl (hl)		; rotate left.

	IF GFLAG
	  rla		; in color mode, there are two bits to be transfered.
	  rl (hl)
	  rra
	ENDIF

	dec l		; char left.
	djnz scrly0	; repeat for width of ticker message.

	pop hl		; retrieve screen address from stack.
	ld de,32	; next row down.
	add hl,de
	pop bc		; retrieve row counter from stack.
	djnz scrly1	; repeat for all rows.

; Calculate character address

	ld hl,(txtpos)	; get text pointer.
	ld a,(hl)	; find character we're displaying.
	and %01111111	; remove end marker bit if applicable.
	cp 13		; is it newline?
	jr nz,scrly5	; no, it's okay.
	ld a,32		; convert to a space instead.
scrly5
	rlca
	rlca
	rlca		; multiply by 8 to find char.
	ld b,a		; store shift in b.
	and 3		; keep within 768-byte range of font.
	ld d,a		; that's our high byte.

	ld a,b		; restore the shift.
	and %11111000
	ld e,a

	ld hl,font-256	; font address.
	add hl,de	; point to image of character.
	ex de,hl	; need the address in de.
	pop hl		; restore screen address.

; Add pixelrow on right side scroller

	ld a,(txtbit)
	ld c,a
	ld b,8
scrly3	ld a,(de)	; get image of char line.
	and c		; test relevant bit of char.

	IF GFLAG
	  jr z,scrl3g	; not set - skip.
	  inc (hl)	; set bit 1.
	  inc (hl)
scrl3g	  rrc c		; next bit of char to use.
	  ld a,(de)	; get image of char line.
	  and c		; test relevant bit of char.
	ENDIF

	jr z,scrly2	; not set - skip.
	inc (hl)	; set bit 0.
scrly2:
	IF GFLAG
	  rlc c		; restore mask position for next line.
	ENDIF

	ld a,32		; next line of window hl+32
	add a,l
	ld l,a
	ld a,0
	adc h
	ld h,a

	inc de		; next line of char de+1
	djnz scrly3

       ld hl,txtbit        ; bit of text to display.
       rrc (hl)            ; next bit of char to use.
    IF GFLAG
       rrc (hl)            ; next bit of char to use.
    ENDIF
       ret nc              ; not reached end of character yet.

       ld hl,(txtpos)      ; text pointer.
       ld a,(hl)           ; what was the character?
       inc hl              ; next character in message.
       rla                 ; end of message?
       jr nc,scrly6        ; not yet - continue.
scrly4 ld hl,(txtini)      ; start of scrolling message.
scrly6 ld (txtpos),hl      ; new text pointer position.
       ret


iscrly call preprt         ; set up display position.

       ld hl,msgdat        ; text messages.
       ld a,b              ; width.
       dec a               ; subtract one.
       cp 32               ; is it between 1 and 32?
;<fix>
       ld a,201            ; code for ret.
;</fix>
       jr nc,iscrl0        ; no, disable messages.
       ld a,c              ; message number.
       ld d,b              ; copy width to d.
       call getwrd         ; find message start.
       ld b,d              ; restore width to b register.
       ld (txtini),hl      ; set initial text position.
       ld a,42             ; code for ld hl,(nn).
iscrl0 ld (scrly),a        ; enable/disable scrolling routine.

       call preprt         ; set up display position.

       call gprad          ; get print address.
       ld l,b              ; width in b so copy to hl.
       ld h,0              ; no high byte.
       dec hl              ; width minus one.
       add hl,de           ; add width.
       ld (txtscr),hl      ; set text screen address.
       ld a,b              ; width.
       ld (txtwid),a       ; set width in working storage.
       ld hl,txtbit        ; bit of text to display.
       ld (hl),%10000000   ; start with leftmost bit.
       jr scrly4

IF CRFLAG = 1
crumble
;	ld a,(vard)
;	and 03h
;	ret nz
	ld h,(ix+09h)
	ld a,(ix+08h)
	add COLDIST+1
	ld l,a
	ld (dispx),hl
	and 06h
	ret nz
	push hl
	call tstbl
	pop de
	ld a,e
	rra
	rra
	rra
	and 1fh
	ld e,a
	ld a,d
	rra
	rra
	rra
	and 1fh
	ld d,a
	ld (dispx),de
	ld a,(hl)
	cp 09h
	call nc,crumble5	; 7a55
crumble1
	ld a,(dispy)
	inc a
	ld (dispy),a
	inc hl
	ld a,(hl)
	cp 09h
	call nc,crumble5	; 7a55
crumble2
	ld a,(dispy)
	inc a
	ld (dispy),a
	inc hl
	ld a,(ix+09h)
	and 07h
	ret z
	ld a,(hl)
	cp 09h
	call nc,crumble5        ; 7a55
crumble3
	ret

crumble5			; 7a55
	push hl
	inc a
	cp 11h
	jr c,crumble6		; 7a5c
	xor a
crumble6			; 7a5c
	ld (hl),a
	call pattr		; 879e
	dec (hl)
	pop hl
	ret
ENDIF

;<cpc>
;lstrnd equ $               ; end of "random" area.
;</cpc>

; Sprite table.
; ix+0  = type.
; ix+1  = sprite image number.
; ix+2  = frame.
; ix+3  = x coord.
; ix+4  = y coord.

; ix+5  = new type.
; ix+6  = new image number.
; ix+7  = new frame.
; ix+8  = new x coord.
; ix+9  = new y coord.

; ix+10 = direction.
; ix+11 = parameter 1.
; ix+12 = parameter 2.
; ix+13 = jump pointer low.
; ix+14 = jump pointer high.
; ix+15 = data pointer low.
; ix+16 = data pointer high.


sprtab equ $

;       block NUMSPR * TABSIZ,255

       defb 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
       defb 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
       defb 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
       defb 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
       defb 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
       defb 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
       defb 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
       defb 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
       defb 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
       defb 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
       defb 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
       defb 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
ssprit defb 255,255,255,255,255,255,255,0,192,120,0,0,0,255,255,255,255

roomtb defb 34                     ; room number.
;<cpc>
;nosnd  defb 255
;</cpc>

; Everything below here will be generated by the editors.

; Sounds.

;fx1    defb 128+15         ; volume and mixer.
;       defb 31             ; white noise.
;       defw 1000           ; tone register.
;       defb 128+15         ; volume and mixer.
;       defb 25             ; white noise.
;       defw 1000           ; tone register.
;       defb 128+14         ; volume and mixer.
;       defb 19             ; white noise.
;       defw 1000           ; tone register.
;       defb 128+13         ; volume and mixer.
;       defb 13             ; white noise.
;       defw 1000           ; tone register.
;       defb 128+12         ; volume and mixer.
;       defb 7              ; white noise.
;       defw 1000           ; tone register.
;       defb 128+11         ; volume and mixer.
;       defb 0              ; white noise.
;       defw 1000           ; tone register.
;       defb 128+10         ; volume and mixer.
;       defb 6              ; white noise.
;       defw 1000           ; tone register.
;       defb 128+8          ; volume and mixer.
;       defb 12             ; white noise.
;       defw 1000           ; tone register.
;       defb 128+6          ; volume and mixer.
;       defb 18             ; white noise.
;       defw 1000           ; tone register.
;       defb 128+3          ; volume and mixer.
;       defb 24             ; white noise.
;       defw 1000           ; tone register.
;       defb 255
;
;fx2    defb 064+15         ; volume and mixer.
;       defb 27             ; white noise.
;       defw 1000           ; tone register.
;       defb 064+14         ; volume and mixer.
;       defb 31             ; white noise.
;       defw 2000           ; tone register.
;       defb 064+13         ; volume and mixer.
;       defb 28             ; white noise.
;       defw 3000           ; tone register.
;       defb 064+12         ; volume and mixer.
;       defb 31             ; white noise.
;       defw 2000           ; tone register.
;       defb 064+11         ; volume and mixer.
;       defb 29             ; white noise.
;       defw 1000           ; tone register.
;       defb 064+10         ; volume and mixer.
;       defb 31             ; white noise.
;       defw 2000           ; tone register.
;       defb 064+9          ; volume and mixer.
;       defb 30             ; white noise.
;       defw 3000           ; tone register.
;       defb 064+8          ; volume and mixer.
;       defb 31             ; white noise.
;       defw 2000           ; tone register.
;       defb 064+7          ; volume and mixer.
;       defb 31             ; white noise.
;       defw 1000           ; tone register.
;       defb 064+6          ; volume and mixer.
;       defb 31             ; white noise.
;       defw 2000           ; tone register.
;       defb 255
;
;fx3    defb 064+15         ; volume and mixer.
;       defb 0              ; white noise.
;       defw 4000           ; tone register.
;       defb 064+15         ; volume and mixer.
;       defb 0              ; white noise.
;       defw 4100           ; tone register.
;       defb 064+14         ; volume and mixer.
;       defb 0              ; white noise.
;       defw 4200           ; tone register.
;       defb 064+14         ; volume and mixer.
;       defb 0              ; white noise.
;       defw 4300           ; tone register.
;       defb 064+13         ; volume and mixer.
;       defb 0              ; white noise.
;       defw 4400           ; tone register.
;       defb 064+13         ; volume and mixer.
;       defb 0              ; white noise.
;       defw 4500           ; tone register.
;       defb 064+12         ; volume and mixer.
;       defb 0              ; white noise.
;       defw 4600           ; tone register.
;       defb 064+12         ; volume and mixer.
;       defb 0              ; white noise.
;       defw 4700           ; tone register.
;       defb 064+11         ; volume and mixer.
;       defb 0              ; white noise.
;       defw 4800           ; tone register.
;       defb 064+10         ; volume and mixer.
;       defb 0              ; white noise.
;       defw 4900           ; tone register.
;       defb 255
;
;       defb 99             ; temporary marker.

; User routine.  Put your own code in here to be called with USER instruction.
; if USER has an argument it will be passed in the accumulator.

sndbit_port   = 26624      ; this is a memory address, not a port !
sndbit_bit    = 0
sndbit_mask   = 33         ; bit 0 (Speaker A) and 5 (Speaker B)

expl
	ld hl,450
expl0
          push    hl
          push    af
          ld      a,sndbit_mask
          ld      h,0
          and     (hl)
          ld      l,a
          pop     af
          xor     l
	or COL_OR
	and COL_AND
          ld      (sndbit_port),a
          pop     hl
          push    af
          ld      b,h
          ld      c,l
dly      dec     bc
          ld      a,b
          or      c
          jr      nz,dly
          pop     af
          
          inc     hl
          bit     1,h
          jr      z,expl0
	ret         

	include "user.inc"

eop    equ $


; Game-specific data and events code generated by the compiler ------------------



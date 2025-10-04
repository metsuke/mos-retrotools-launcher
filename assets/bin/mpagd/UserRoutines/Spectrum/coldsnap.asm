; ColdSnap Audio
;
; Z80/AY music player for Amstrad CPC and ZX Spectrum 128.
;
; Copyright (c) 2016 Source Solutions, Inc.
; Copyright (c) 2004, 2007 Sergey Bulba
; Refactored by Matt Westcott
; Bugfixed by Lee du Caine
; CPC driver by Grim/Arkos^Semilanceata
; Adapted for MPAGD by Jonathan Cauldwell

; Variables are 541 bytes long and follow the player.
; Music is in ProTracker 3.6 format (without the 100 byte header).

; Assemble with SjAsmPlus.


; comment out if interrupts are not disabled
;no_interrupts equ 1

; USER start address.

       and a                   ; check argument.
       jr z,musoff             ; it's zero, switch off music.

muson  di                      ; interrupts off.
       call csa_init           ; initialise music.
       ld hl,63488             ; creates interrupt table (256 bytes) at maximum address;
       ld a,h                  ; high byte of table.
       ld i,a                  ; point there.
       im 2                    ; select interrupt mode 2.
       dec a
       ld (hl),a
       inc l
       jr nz,$-2
       inc h
       ld (hl),a
       ld l,a                  ; presets music playback
       ld h,a
       ld de,im2_proc          ; loads execution part (im2_proc)
       ld (hl),195             ; jp instruction.
       inc l
       ld (hl),e               ; address low byte.
       inc l
       ld (hl),d               ; address high byte.
       ei                      ; iterrupts on again.
       ret

musoff di                      ; interrupts off.
       call csa_init           ; silence AY chip.
       ld a,63                 ; restore default im1 interrupt state.
       ld i,a
       im 1
       ei                      ; interrupts back on.
       ret

im2_proc:                      ; start of interrupt routines
       di
       push af                 ; store registers.
       push hl
       push bc
       push de
       push ix
       push iy
       exx
       ex af,af'
       push af
       push hl
       push bc
       push de
       call csa_play           ; music playback
       ld hl,23672             ; game timer.
       inc (hl)                ; increment for vsync routine.
       pop de                  ; restore registers.
       pop bc
       pop hl
       pop af
       ex af,af'
       exx
       pop iy
       pop ix
       pop de
       pop bc
       pop hl
       pop af
       ei
       reti

setup:
	defb 0;						// set bit 0 to 1 if to play without looping
;	     						// bit 7 is set each time loop point is passed

checklp:
	ld hl, setup;
	set 7, (hl);
	bit 0, (hl);
	ret z;
	pop hl;
	ld hl, DelyCnt;
	inc (hl);
	ld hl, ChanA + NtSkCn;
	inc (hl);

csa_stop:
	xor a;
	ld h, a;
	ld l, a  ;
	ld (AYregs + AmplA), a;
	ld (AYregs + AmplB), hl;
	jp csa_to_ay;

csa_init:
	ld hl, music - 100; 		// music address
	ld (modaddr + 1), hl;
	ld (mdaddr2 + 1), hl;
	push hl;
	ld de, 100;
	add hl, de;
	ld a, (hl);
	ld (Delay + 1), a;
	push hl;
	pop ix;
	add hl, de;
	ld (CrPsPtr), hl;
	ld e, (ix + 2);
	add hl, de;
	inc hl;
	ld (LPosPtr + 1), hl;
	pop de;
	ld l, (ix + 3);
	ld h, (ix + 4);
	add hl, de;
	ld (PatsPtr + 1), hl;
	ld hl, 169;
	add hl, de;
	ld (ornptrs + 1), hl;
	ld hl, 105;
	add hl, de;
	ld (samptrs + 1), hl;
	ld hl, setup;
	res 7, (hl);

	xor a;
	ld hl, csvars;
	ld (hl), a;
	ld de, csvars + 1;
	ld bc, var0end - csvars - 1;
	ldir;
	inc a;
	ld (DelyCnt), a;
	ld hl, 0xf001;				// H = volume, L = NtSkCn
	ld (ChanA + NtSkCn), hl;
	ld (ChanB + NtSkCn), hl;
	ld (ChanC + NtSkCn), hl;
	ld hl, emptysamorn;
	ld (AdInPtA + 1), hl;
	ld (ChanA + OrnPtr), hl;
	ld (ChanB + OrnPtr), hl;
	ld (ChanC + OrnPtr), hl;

	jp csa_to_ay;

pd_orsm:
	ld (ix + 8), 0; 			// pattern decoder
	call setorn;
	ld a, (bc);
	inc bc;
	rrca;

pd_sam:
	add a, a;

pd_sam_:
	ld e, a;
	ld d, 0;

samptrs:
	ld hl, 0x2121;
	add hl, de;
	ld e, (hl);
	inc hl;
	ld d, (hl);

modaddr;
	ld hl, 0x2121;
	add hl, de;
	ld (ix + 3), l;
	ld (ix + 4), h;
	jr pd_loop;

pd_vol:
	rlca;
	rlca;
	rlca;
	rlca;
	ld (ix + 16), a;
	jr pd_lp2;

pd_eoff:
	ld (ix + 8), a;
	ld (ix - 12), a;
	jr pd_lp2;

pd_sore:
	dec a;
	jr nz, pd_env;
	ld a, (bc);
	inc bc;
	ld (ix + 5), a;
	jr pd_lp2;

pd_env:
	call setenv;
	jr pd_lp2;

pd_orn:
	call setorn;
	jr pd_loop;

pd_esam:
	ld (ix + 8), a;
	ld (ix - 12), a;
	call nz, setenv;
	ld a, (bc);
	inc bc;
	jr pd_sam_

ptdecod:
	ld a, (ix + 6);
	ld (prnote + 1), a;
	ld l, (ix - 6);
	ld h, (ix - 5);
	ld (PrSlide + 1), hl;

pd_loop:
	ld de, 0x2010;

pd_lp2:
	ld a, (bc);
	inc bc;
	add a, e;
	jr c, pd_orsm;
	add a, d;
	jr z, pd_fin;
	jr c, pd_sam;
	add a, e;
	jr z, pd_rel;
	jr c, pd_vol;
	add a, e;
	jr z, pd_eoff;
	jr c, pd_sore;
	add a, 0x60;
	jr c, pd_note;
	add a, e;
	jr c, pd_orn;
	add a, d;
	jr c, pd_nois;
	add a, e;
	jr c, pd_esam;
	add a, a;
	ld e, a;
	ld hl, spccoms - 8416;
	add hl, de;
	ld e, (hl);
	inc hl;
	ld d, (hl);
	push de;
	jr pd_loop;

pd_nois:
	ld (ns_base), a;
	jr pd_lp2;

pd_rel:
	res 0, (ix + 9);
	jr pd_res;

pd_note:
	ld (ix + 6), a;
	set 0, (ix + 9);
	xor a;

       ifndef no_interrupts
	di
       endif

pd_res:
	ld (pdsp_ + 1), sp;
	ld sp, ix;
	ld h, a;
	ld l, a;
	push hl;
	push hl;
	push hl;
	push hl;
	push hl;
	push hl;

pdsp_;
	ld sp, 0x3131

       ifndef no_interrupts
	ei
       endif

pd_fin:
	ld a, (ix + 5);
	ld (ix + 15), a;
	ret

c_portm:
	res 2, (ix + 9);
	ld a, (bc);
	inc bc;						// skip precalculated tone delta because cannot
	inc bc;						// be right after PT3 compilation
	inc bc;
	ld (ix + 10), a;
	ld (ix - 7), a;
	ld de, nt_;
	ld a, (ix + 6);
	ld (ix + 7), a;
	add a, a;
	ld l, a;
	ld h, 0;
	add hl, de;
	ld a, (hl);
	inc hl;
	ld h, (hl);
	ld l, a;
	push hl

prnote:
	ld a, 62;
	ld (ix + 6), a;
	add a, a;
	ld l, a;
	ld h, 0;
	add hl, de;
	ld e, (hl);
	inc hl;
	ld d, (hl);
	pop hl;
	sbc hl, de;
	ld (ix + 13), l;
	ld (ix + 14), h;
	ld e, (ix - 6);
	ld d, (ix - 5)

PrSlide:
	ld de, 0x1111;
	ld (ix - 6), e;
	ld (ix - 5), d;
	ld a, (bc);					// signed tone step
	inc bc;
	ex af, af';					//'
	ld a, (bc);
	inc bc;
	and a;
	jr z, nosig;
	ex de, hl

nosig:
	sbc hl, de;
	jp p, set_stp;
	cpl;
	ex af, af';					//'
	neg;
	ex af, af';					//'

set_stp:
	ld (ix + 12), a;
	ex af, af' ;				//'
	ld (ix + 11), a;
	ld (ix - 2), 0;
	ret;

c_gliss:
	set 2, (ix + 9);
	ld a, (bc);
	inc bc;
	ld (ix + 10), a;
	and a;
	jr nz, gl36;
	ld a, 6;					// version is always PT3.6
	cp 7;
	sbc a, a;
	inc a;

gl36:
	ld (ix - 7), a;
	ld a, (bc);
	inc bc;
	ex af, af' ;				//'
	ld a, (bc);
	inc bc;
	jr set_stp

c_smpos:
	ld a, (bc);
	inc bc;
	ld (ix - 11), a;
	ret;

c_orpos:
	ld a, (bc);
	inc bc;
	ld (ix - 12), a;
	ret;

c_vibrt:
	ld a, (bc);
	inc bc;
	ld (ix - 1), a;
	ld (ix - 2), a;
	ld a, (bc);
	inc bc;
	ld (ix + 0), a;
	xor a;
	ld (ix - 7), a;
	ld (ix - 6), a;
	ld (ix - 5), a;
	ret;

c_engls;
	ld a, (bc);
	inc bc;
	ld (env_del + 1), a;
	ld (CurEDel), a;
	ld a, (bc);
	inc bc;
	ld l, a;
	ld a, (bc);
	inc bc;
	ld h, a;
	ld (esldadd + 1), hl;
	ret;

c_delay:
	ld a, (bc);
	inc bc;
	ld (Delay + 1), a;
	ld (DelyCnt), a;			// bugfix by Lee du Caine;
	ret;

setenv:
	ld (ix + 8), e;
	ld (AYregs + EnvTp), a;
	ld a, (bc);
	inc bc;
	ld h, a;
	ld a, (bc);
	inc bc;
	ld l, a;
	ld (EnvBase), hl;
	xor a;
	ld (ix - 12), a;
	ld (CurEDel), a;
	ld h, a;
	ld l, a;
	ld (CurESld), hl

c_nop:
	ret;

setorn:
	add a, a;
	ld e, a;
	ld d, 0;
	ld (ix - 12), d

ornptrs:
	ld hl, 0x2121;
	add hl, de;
	ld e, (hl);
	inc hl;
	ld d, (hl)

mdaddr2:
	ld hl, 0x2121;
	add hl, de;
	ld (ix + 1), l;
	ld (ix + 2), h;
	ret;

;all 16 addresses to protect from broken PT3 modules

spccoms:
	defw c_nop;
	defw c_gliss;
	defw c_portm;
	defw c_smpos;
	defw c_orpos;
	defw c_vibrt;
	defw c_nop;
	defw c_nop;
	defw c_engls;
	defw c_delay;
	defw c_nop;
	defw c_nop;
	defw c_nop;
	defw c_nop;
	defw c_nop;
	defw c_nop

chregs:
	xor a;
	ld (ampl), a;
	bit 0, (ix + 21);
	push hl;
	jp z, ch_exit;
	ld (csp_ + 1), sp;
	ld l, (ix + 13);
	ld h, (ix + 14);

       ifndef no_interrupts
	di
       endif

	ld sp, hl;
	pop de;
	ld h, a;
	ld a, (ix + 0);
	ld l, a;
	add hl, sp;
	inc a;
	cp d;
	jr c, ch_orps;
	ld a, e

ch_orps:
	ld (ix + 0), a;
	ld a, (ix + 18);
	add a, (hl);
	jp p, ch_ntp;
	xor a

ch_ntp:
	cp 96;
	jr c, ch_nok;
	ld a, 95

ch_nok:
	add a, a;
	ex af, af' ;';
	ld l, (ix + 15);
	ld h, (ix + 16);
	ld sp, hl;
	pop de;
	ld h, 0;
	ld a, (ix + 1);
	ld b, a;
	add a, a;
	add a, a;
	ld l, a;
	add hl, sp;
	ld sp, hl;
	ld a, b;
	inc a;
	cp d;
	jr c, ch_smps;
	ld a, e

ch_smps:
	ld (ix + 1), a;
	pop bc;
	pop hl;
	ld e, (ix + 8);
	ld d, (ix + 9);
	add hl, de;
	bit 6, b;
	jr z, ch_noac;
	ld (ix + 8), l;
	ld (ix + 9), h

ch_noac:
	ex de, hl;
	ex af, af' ;';
	ld l, a;
	ld h, 0;
	ld sp, nt_;
	add hl, sp;
	ld sp, hl;
	pop hl;
	add hl, de;
	ld e, (ix + 6);
	ld d, (ix + 7);
	add hl, de

csp_:
	ld sp, 03131h;
	ex (sp), hl;
	xor a;
	or (ix + 5);
	jr z, ch_amp;
	dec (ix + 5);
	jr nz, ch_amp;
	ld a, (ix + 22);
	ld (ix + 5), a;
	ld l, (ix + 23);
	ld h, (ix + 24);
	ld a, h;
	add hl, de;
	ld (ix + 6), l;
	ld (ix + 7), h;
	bit 2, (ix + 21);
	jr nz, ch_amp;
	ld e, (ix + 25);
	ld d, (ix + 26);
	and a;
	jr z, ch_stpp;
	ex de, hl

ch_stpp:
	sbc hl, de;
	jp m, ch_amp;
	ld a, (ix + 19);
	ld (ix + 18), a;
	xor a;
	ld (ix + 5), a;
	ld (ix + 6), a;
	ld (ix + 7), a

ch_amp:
	ld a, (ix + 2);
	bit 7, c;
	jr z, ch_noam;
	bit 6, c;
	jr z, ch_amin;
	cp 15;
	jr z, ch_noam;
	inc a;
	jr ch_svam

ch_amin:
	cp 241;
	jr z, ch_noam;
	dec a

ch_svam:
	ld (ix + 2), a

ch_noam:
	ld l, a;
	ld a, b;
	and 15;
	add a, l;
	jp p, ch_apos;
	xor a

ch_apos:
	cp 16;
	jr c, ch_vol;
	ld a, 15

ch_vol:
	or (ix + 28);
	ld l, a;
	ld h, 0;
	ld de, vt_;
	add hl, de;
	ld a, (hl)

ch_env:
	bit 0, c;
	jr nz, ch_noen;
	or (ix + 20)

ch_noen:
	ld (ampl), a;
	bit 7, b;
	ld a, c;
	jr z, no_ensl;
	rla;
	rla;
	sra a;
	sra a;
	sra a;
	add a, (ix + 4);
	bit 5, b;
	jr z, no_enac;
	ld (ix + 004h), a

no_enac:
	ld hl, AddToEn + 1;
	add a, (hl);
	ld (hl), a;
	jr ch_mix

no_ensl:
	rra;
	add a, (ix + 3);
	ld (addtons), a;
	bit 5, b;
	jr z, ch_mix;
	ld (ix + 3), a

ch_mix:
	ld a, b;
	rra;
	and 72

ch_exit:
	ld hl, AYregs + Mixer;
	or (hl);
	rrca;
	ld (hl), a;
	pop hl;
	xor a;
	or (ix + 10);
	ret z;
	dec (ix + 10);
	ret nz;
	xor (ix + 21);
	ld (ix + 21), a;
	rra;
	ld a, (ix + 11);
	jr c, ch_ondl;
	ld a, (ix + 12)

ch_ondl:
	ld (ix + 10), a;
	ret

csa_play:
	xor a;
	ld (AddToEn + 1), a;
	ld (AYregs + Mixer), a;
	dec a;
	ld (AYregs + EnvTp), a;
	ld hl, DelyCnt;
	dec (hl);
	jr nz, pl2;
	ld hl, ChanA + NtSkCn;
	dec (hl);
	jr nz, pl1b

AdInPtA:
	ld bc, 0x0101;
	ld a, (bc);
	and a;
	jr nz, pl1a;
	ld d, a;
	ld (ns_base), a
CrPsPtr equ $ + 1;
	ld hl, 0;
	inc hl;
	ld a, (hl);
	inc a;
	jr nz, plnlp;
	call checklp

LPosPtr:
	ld hl, 8481;
	ld a, (hl);
	inc a
plnlp:
	ld (CrPsPtr), hl;
	dec a;
	add a, a;
	ld e, a;
	rl d

PatsPtr:
	ld hl, 8481;
	add hl, de;
	ld de, (modaddr + 1);
	ld (psp_ + 1), sp;
	ld sp, hl;
	pop hl;
	add hl, de;
	ld b, h;
	ld c, l;
	pop hl;
	add hl, de;
	ld (adinptb + 1), hl;
	pop hl;
	add hl, de;
	ld (adinptc + 1), hl

psp_:
	ld sp, 12593

pl1a:
	ld ix, ChanA + 12;
	call ptdecod;
	ld (AdInPtA + 1), bc

pl1b:
	ld hl, ChanB + NtSkCn;
	dec (hl);
	jr nz, pl1c;
	ld ix, ChanB + 12

adinptb:
	ld bc, 257;
	call ptdecod;
	ld (adinptb + 1), bc

pl1c:
	ld hl, ChanC + NtSkCn;
	dec (hl);
	jr nz, Delay;
	ld ix, ChanC + 12

adinptc:
	ld bc, 257;
	call ptdecod;
	ld (adinptc + 1), bc

Delay:
	ld a, 62;
	ld (DelyCnt), a

pl2:
	ld ix, ChanA;
	ld hl, (AYregs + ar_tona);
	call chregs;
	ld (AYregs + ar_tona), hl;
	ld a, (ampl);
	ld (AYregs + AmplA), a;
	ld ix, ChanB;
	ld hl, (AYregs + ar_tonb);
	call chregs;
	ld (AYregs + ar_tonb), hl;
	ld a, (ampl);
	ld (AYregs + AmplB), a;
	ld ix, ChanC;
	ld hl, (AYregs + ar_tonc);
	call chregs;
	ld (AYregs + ar_tonc), hl;
	ld hl, (ns_base_addtons);
	ld a, h;
	add a, l;
	ld (AYregs + ar_noise), a

AddToEn:
	ld a, 62;
	ld e, a;
	add a, a;
	sbc a, a;
	ld d, a;
	ld hl, (EnvBase);
	add hl, de;
	ld de, (CurESld);
	add hl, de;
	ld (AYregs + ar_env), hl;
	xor a;
	ld hl, CurEDel;
	or (hl);
	jr z, csa_to_ay;
	dec (hl);
	jr nz, rout

env_del:
	ld a, 62;
	ld (hl), a

esldadd:
	ld hl, 8481;
	add hl, de;
	ld (CurESld), hl

rout:
	xor a

csa_to_ay:

	ld de, 0xffbf
	ld bc, 0xfffd
	ld hl, AYregs
lout:
	out (c), a;
	ld b, e;
	outi;
	ld b, d;
	inc a;
	cp 13;
	jr nz, lout;
	out (c), a;
	ld a, (hl);
	and a;
	ret m;
	ld b, e;
	out (c), a;
	ret

t_:

tcold_0:
	defb 0x00 + 1, 0x04 + 1, 0x08 + 1, 0x0a + 1, 0x0c + 1, 0x0e + 1;
	defb 0x12 + 1, 0x14 + 1, 0x18 + 1, 0x24 + 1, 0x3c + 1, 0

tcold_1:
	defb 0x5c + 1, 0

tcold_2:
	defb 0x30 + 1, 0x36 + 1, 0x4c + 1, 0x52 + 1, 0x5e + 1, 0x70 + 1;
	defb 0x82, 0x8c, 0x9c, 0x9e, 0xa0, 0xa6, 0xa8, 0xaa, 0xac, 0xae, 0xae, 0

tcnew_3:
	defb 0x56 + 1

tcold_3:
	defb 0x1e + 1, 0x22 + 1, 0x24 + 1, 0x28 + 1, 0x2c + 1, 0x2e + 1;
	defb 0x32 + 1, 0xbe + 1, 0

tcnew_0:
	defb 0x1c + 1, 0x20 + 1, 0x22 + 1, 0x26 + 1, 0x2a + 1, 0x2c + 1;
	defb 0x30 + 1, 0x54 + 1, 0xbc + 1, 0xbe + 1, 0

tcnew_1 equ tcold_1

tcnew_2:
	defb 0x1a + 1, 0x20 + 1, 0x24 + 1, 0x28 + 1, 0x2a + 1, 0x3a + 1;
	defb 0x4c + 1, 0x5e + 1, 0xba + 1, 0xbc + 1, 0xbe + 1, 0

emptysamorn equ $ - 1
;
	defb 1, 0;, 0x90;			// uncomment 0x90 if you need default sample

; channel data offsets

chp_psinor equ 0
chp_psinsm equ 1
chp_cramsl equ 2
chp_crnssl equ 3
chp_crensl equ 4
chp_tslcnt equ 5
chp_crtnsl equ 6
chp_tnacc equ 8
chp_conoff equ 10
chp_onoffd equ 11; 				// ix for ptdecod here ( + 12)
chp_offond equ 12
OrnPtr equ 13
chp_samptr equ 15
chp_nntskp equ 17
chp_note equ 18
chp_sltont equ 19
chp_env_en equ 20
chp_flags equ 21;				// enabled - 0, simplegliss - 2
chp_tnsldl equ 22
chp_tslstp equ 23
chp_tndelt equ 25
NtSkCn equ 27
chp_volume equ 28

csvars:
ChanA ds 29
ChanB ds 29
ChanC ds 29
DelyCnt defb 0;					// globalvars
CurESld defw 0
CurEDel defb 0

ns_base_addtons:
ns_base defb 0
addtons defb 0

ar_tona equ 0;					// word 1
ar_tonb equ 2;					// word 1
ar_tonc equ 4;					// word 1
ar_noise equ 6;					// byte 1
Mixer equ 7;					// byte 1
AmplA equ 8;					// byte 1
AmplB equ 9;					// byte 1
ar_amplc equ 10;				// byte 1
ar_env equ 11;					// word 1
EnvTp equ 13;					// byte 1
ar_size equ 14;					// byte 1

AYregs equ $

vt_:
       defb 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
       defb 0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1
       defb 0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2
       defb 0,0,0,1,1,1,1,1,2,2,2,2,2,3,3,3
       defb 0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4
       defb 0,0,1,1,1,2,2,2,3,3,3,4,4,4,5,5
       defb 0,0,1,1,2,2,2,3,3,4,4,4,5,5,6,6
       defb 0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7
       defb 0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8
       defb 0,1,1,2,2,3,4,4,5,5,6,7,7,8,8,9
       defb 0,1,1,2,3,3,4,5,5,6,7,7,8,9,9,10
       defb 0,1,1,2,3,4,4,5,6,7,7,8,9,10,10,11
       defb 0,1,2,2,3,4,5,6,6,7,8,9,10,10,11,12
       defb 0,1,2,3,3,4,5,6,7,8,9,10,10,11,12,13
       defb 0,1,2,3,4,5,6,7,7,8,9,10,11,12,13,14
       defb 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15

EnvBase equ vt_ + 14

nt_:
       defb 61,13,127,12,204,11,34,11,130,10,235,9,93,9,214,8
       defb 87,8,223,7,110,7,3,7,159,6,64,6,230,5,145,5
       defb 65,5,246,4,174,4,107,4,44,4,240,3,183,3,130,3
       defb 79,3,32,3,243,2,201,2,161,2,123,2,87,2,54,2
       defb 22,2,248,1,220,1,193,1,168,1,144,1,121,1,100,1
       defb 80,1,61,1,44,1,27,1,11,1,252,0,238,0,224,0
       defb 212,0,200,0,189,0,178,0,168,0,159,0,150,0,141,0
       defb 133,0,126,0,119,0,112,0,106,0,100,0,94,0,89,0
       defb 84,0,79,0,75,0,71,0,67,0,63,0,59,0,56,0
       defb 53,0,50,0,47,0,45,0,42,0,40,0,37,0,35,0
       defb 33,0,31,0,30,0,28,0,26,0,25,0,24,0,22,0
       defb 21,0,20,0,19,0,18,0,17,0,16,0,15,0,14,0

;local vars
ampl equ AYregs + ar_amplc
var0end equ vt_ + 16;			// csa_init zeros from vars to var0end - 1
varsend equ $
mdladdr equ $

music: ; do not alter this comment ~

       incbin "c:\zx\code\winagd\coldsnap\zx\RaysReprisal.p3x"
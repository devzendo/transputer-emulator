j	macro	val		; jump (can deschedule)
	db	000h or (val and 00fh)
	endm

prfx	macro	val,modif,cnt	; prefix instruction
	db	020h or (val modif cnt and 00fh)
	endm


ldnl	macro	val		; load local opcode
	db	030h or (val and 00fh)
	endm

ldc	macro	val		; load constant opcode
	db	040h or (val and 00fh)
	endm

call	macro			; call opcode
	db	090h or (val and 00fh)
	endm

cj	macro	val		; conditional jump
	db	0A0h or (val and 00fh)
	endm

ajw	macro	val		; adjust workspace
	db	0B0h or (val and 00fh)
	endm

eqc macro val
	db  0c0h or (val and 00fh)
	endm

stl	macro	val		; store local opcode
	db	0d0h or (val and 00fh)
	endm

stnl	macro	val		; store non-local opcode
	db	0e0h or (val and 00fh)
	endm

ldpi macro
	db	021h,0fbh	;; ldpi 	make prg rel -> absolute
	endm

stopp macro
	db 21h, f5h, 11h
	endm

mint macro
	db 24h, f2h
	endm

outword macro
	db 0ffh
	endm

outbyte macro
	db 0feh
	endm

lb macro
	db 0f1h
	endm

sb macro
	db 3bh
	endm

START:
	j   MAIN ???

HWSTR:
	DB  "hello world\12\0" ; 13

MAIN:
	ldc HWSTR-LF5
	ldpi
LF5:
	; call _putConsoleCString
	call _putConsoleCString - LF6
LF6:

FIN:
	stopp

_putConsoleCString:
	ajw -9
	ldc 0
	stl 3
	ldlp 1
	stl 4
	ldl 3
	ldl 4
	sb
	ldl 10
	call _localStrlen
	stl 2
	mint
	stl 5
	ldc 258
	stl 6
	ldl 5
	ldl 6
	outword

	mint
	stl 7
	ldl 10
	ldl 7
	ldl 2
	out

	mint
	stl 8
	ldl 8
	ldl 1
	outbyte
	ajw +9
	ret

_localStrlen:
	ajw -6
	ldc 0
	stl 1
	ldl 1
	stl 0
LS1:
	ldl 7
	stl 2
	ldl 2
	lb
	stl 3
	ldl 3
	eqc 0
	cj LS3
	j LS2
LS3:
	ldl 0
	adc 1
	stl 4
	ldl 4
	stl 0
	ldl 7
	adc 1
	stl 5
	ldl 5
	stl 7
	j LS1
LS2:
	ldl 0
	ajw +6
	ret
	ajw +6
	ret

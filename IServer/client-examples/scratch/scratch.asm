TITLE Scratch assembly file, for writing sequences for testcpu

PAGE 60,132

	.TRANSPUTER
	MemStart    EQU 0x800000FA

	ORG         MemStart

; Standard Transputer startup code is assembled first.



	STDOUT_STREAMID EQU 0x01
	LINK0_OUTPUT    EQU 0x80000000
	LINK0_INPUT     EQU 0x80000010

MAIN:
	ajw     0x100    ; allow for 64 stack frames

    mint             ; initialise channel
	ldc     CHAN - _M1
	ldpi
_M1:
    ; a=chan, b=mint
    stnl    0


	ldc     HWSTR - _M2 ; Output string data to channel
	ldpi
_M2:
    ldc     CHAN - _M3
    ldpi
_M3:
    ; (a=chan, b=string address)
    ldc     11
	; (a=string length; b=chan, c=string address)
	out
	; ()

    ldc 42  ; nop-ish

HWSTR:
	DB      "hello world", 0x00 ; length 11

CHAN:
    DW      0

	END

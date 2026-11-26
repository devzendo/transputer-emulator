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

	ldc     HWSTR - _M1
	ldpi
_M1:

    ; (a=string address)
	; Output string data
	ldc     LINK0_OUTPUT
	ldc     11
	; (a=string length; b=LINK0_OUTPUT, c=string address)
	out
	; ()

    ldc 42
HWSTR:
	DB      "hello world", 0x00 ; length 11

	END

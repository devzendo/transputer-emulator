TITLE Transputer Call and Return as a ROM

PAGE 60,132

	.T800
	MemStart    EQU 0x80000070
	ResetCode   EQU 0x7FFFFFFE

	ORG         ResetCode - 0x20

START:

	call        FUNC - L1
L1:

STOP:
	stopp

FUNC:
	ret

	db  0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa

	ORG     ResetCode - 8
BIGJUMP:
	j       START - BJ2
BJ2:

	ORG     ResetCode
	j       BIGJUMP - FIN
FIN:
    END


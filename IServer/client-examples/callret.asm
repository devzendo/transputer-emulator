TITLE Transputer Call and Return

PAGE 60,132

	.T800
	MemStart    EQU 0x80000070

	ORG         MemStart - 1
	DB          STOP - START
	ORG         MemStart

START:

	call        FUNC - L1
L1:

FIN:
	stopp

FUNC:
	ret

	db  0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa
	db  0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa

	db  0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa
	db  0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa
STOP:
	END


;   WPtr is set to #80000079 after this boots. But the call overwrites this code!
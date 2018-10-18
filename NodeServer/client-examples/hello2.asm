TITLE Transputer Hello World

PAGE 60,132

; Second version without macros.

	.TRANSPUTER
	MemStart    EQU 0x80000070
	TptrLoc0    EQU 0x80000024
	TptrLoc1    EQU 0x80000028

	start_time  EQU 0

;#define LDPI(LABEL)     ldc LABEL-9f; ldpi; 9: ;
LDPIL   MACRO   label
		ldc     label-LDPIL1
		ldpi
LDPIL1:
		ENDM

;#define LDS(LABEL)      LDPI(LABEL); ldnl 0;
LDS     MACRO   label
		LDPIL   label
		ldnl    0
		ENDM

;#define STS(LABEL)      LDPI(LABEL); stnl 0;
STS     MACRO   label
		LDPIL   label
		stnl    0
		ENDM

;#define LDSB(LABEL)     LDPI(LABEL); lb;
LDSB    MACRO   label
		LDPIL   label
		lb
		ENDM

;#define STSB(LABEL)     LDPI(LABEL); sb;
STSB    MACRO   label
		LDPIL   label
		sb
		ENDM

;#define LEND(CTRL,LABEL) ldlp CTRL; ldc 9f-LABEL; lend; 9: ;
LENDL   MACRO   ctrl label
		ldlp    ctrl
		ldc     LENDL1-label
		lend
LENDL1:
		ENDM

;#define NEQC(CONST)     eqc CONST; eqc 0;
NEQC    MACRO   const
		eqc     const
		eqc     0
		ENDM

;#define LE              gt; eqc 0;
LE      MACRO
		gt
		eqc     0
		ENDM

;#define GE              rev; LE;
GE      MACRO
		rev
		LE
		ENDM

;#define LT              rev; gt;
LT      MACRO
		rev
		gt
		ENDM

;#define BR              eqc 0; cj   /* traditional "branch if true" */
BR      MACRO
		eqc     0
		cj
		ENDM

	ORG         MemStart - 1
	DB          STOP - START
	ORG         MemStart

START:
	mint                        ; load A with NotProcess.p = mint = 0x80000000
	sthf                        ; initialize high priority queue front pointer
    mint                        ; load A with NotProcess.p
    stlf                        ; initialize low priority process queue pointer
                                ; This must be done before starting any processes or attempting any message passing.
                                ; Because of the way queues are implemented there is no need to initialise the back
                                ; pointers.
                                ; CWG p75

    mint                        ; load A with NotProcess.p
    ldc     TPtrLoc0            ; load high priority timer queue pointer offset
    stnl    0                   ; initialize high priority timer queue
    mint                        ; load A with NotProcess.p
    ldc     TPtrLoc1            ; load low priority timer queue pointer offset
    stnl    0                   ; initialize high priority timer queue
                                ; This must be done before any attempt to wait on the timer. Also before using the
                                ; clock at any priority level, the timer must be started...

    ldc     start_time          ; load time to initialize clocks at (usually zero)
    sttimer                     ; start the clocks

    testerr                     ; clears the Error flag and HaltOnError flag
    clrhalterr                  ; or use sethalterr here depending what you want

    fptesterr                   ; Reset floating point error flag and set rounding mode to Round-to-Nearest

    mint                        ; load A with NotProcess.p
    ldc     0x80000020          ; load address of Event channel
    stnl    0                   ; store NotProcess.p to Event channel
    mint                        ; load A with NotProcess.p
    ldc     0x8000001C          ; load address of Link 3 input channel
    stnl    0                   ; store NotProcess.p to Link 3 input channel
    mint                        ; load A with NotProcess.p
    ldc     0x80000018          ; load address of Link 2 input channel
    stnl    0                   ; store NotProcess.p to Link 2 input channel
    mint                        ; load A with NotProcess.p
    ldc     0x80000014          ; load address of Link 1 input channel
    stnl    0                   ; store NotProcess.p to Link 1 input channel
    mint                        ; load A with NotProcess.p
    ldc     0x80000010          ; load address of Link 0 input channel
    stnl    0                   ; store NotProcess.p to Link 0 input channel
    mint                        ; load A with NotProcess.p
    ldc     0x8000000C          ; load address of Link 3 output channel
    stnl    0                   ; store NotProcess.p to Link 3 output channel
    mint                        ; load A with NotProcess.p
    ldc     0x80000008          ; load address of Link 2 output channel
    stnl    0                   ; store NotProcess.p to Link 2 output channel
    mint                        ; load A with NotProcess.p
    ldc     0x80000004          ; load address of Link 1 output channel
    stnl    0                   ; store NotProcess.p to Link 1 output channel
    mint                        ; load A with NotProcess.p
    ldc     0x80000000          ; load address of Link 0 output channel
    stnl    0                   ; store NotProcess.p to Link 0 output channel



	j   MAIN - ST1
ST1:

	NSS_CONSOLE     EQU 0x0100
	CONSOLE_PUT_PSTR    EQU (NSS_CONSOLE | 0x01)
	LINK0_OUTPUT    EQU 0x80000000
	LINK0_INPUT     EQU 0x80000010


HWSTR:
	DB  "hello world", 0x0a, 0x00 ; length 13

MAIN:
	ldc HWSTR - LF5
	ldpi
LF5:
	; call putConsolePString
	; a=800000FB (HWSTR), b=00000001 (irrelevant)
	call putConsolePString - LF6
	; W 4 [80000158]=8000010D
    ; W 4 [8000015C]=800000FB (HWSTR)
    ; W 4 [80000160]=00000001
    ; W 4 [80000164]=80000000
LF6:

FIN:
	stopp

; Stack effect notation used here:
; fun(a=A, b=B, c=C): (a=A', b=B', c=C')
; a, b, c are the three registers; A, B, C are their initial contents.
; A', B', C' are their return values (which may be A, B, C if these are still on the stack).
; Return values can be denoted as 'useless' indicating some internal value that's not useful to the caller.

; putConsolePString(a=string address, b=B, c=C): (a=, b=, c=)
; Precondition: string is <= 255 bytes long. Only a single protocol frame will be sent to the server; If the string
; is longer, this function could issue several frames for each segment. A future version could..
; If the string is empty, no frame is emitted.
putConsolePString:
	PPS_WLEN        EQU 2

	PPS_STRINGADDR  EQU 0
	PPS_STRINGLEN   EQU 1

	ajw     PPS_WLEN

; idea for later, store CONSOLE_PUT_PSTR then the length next to each other in the workspace, then out them as a 5-byte
; message (LSB of the length will be next to the CONSOLE_PUT_PSTR), then the string text, so we have two outputs,
; rather than the three we have at the moment....?

	; Store the string pointer for later
	stl     PPS_STRINGADDR
	; (a=B, b=C, c=C)
	ldl     PPS_STRINGADDR
	; (a=string address, b=B, c=C)

	call strlen - _PPS1
_PPS1:
	; (a=length, b=useless, c=useless)
	stl     PPS_STRINGLEN
	; (a,b,c=useless)

	; Empty string?
	ldl     PPS_STRINGLEN
	; (a=length, b=useless, c=useless)
	cj      _PPS_END

	; (a,b,c=useless)

	ldc     LINK0_OUTPUT
	; (a=LINK0_OUTPUT, b,c=useless)
	ldc     CONSOLE_PUT_PSTR
	; (a=CONSOLE_PUT_PSTR, b=LINK0_OUTPUT, c=useless)
	outword     ; CONSOLE_PUT_PSTR WORD32 -->
	; (a=CONSOLE_PUT_PSTR, b=LINK0_OUTPUT, c=useless)
	rev
	; (a=LINK0_OUTPUT, b=CONSOLE_PUT_PSTR, c=useless)
	ldl     PPS_STRINGLEN
	; (a=string length, b=LINK0_OUTPUT, c=CONSOLE_PUT_PSTR)
	outbyte     ; string length BYTE -->
	; (a=string length, b=LINK0_OUTPUT, c=CONSOLE_PUT_PSTR)
	; WOZERE - need to set c in a reasonable manner... perhaps with some ROT
; a=len, b=channel, c=addr

_PPS_END:
	; Empty string => (a=length, b=useless, c=useless)
	ajw     -PPS_WLEN
	ret



strlen: ; look, no workspace!! all registers!
	; strlen(a=string address, b=B, c=C): (a=length, b=useless, c=useless)
	ldc 0
	; (a=length (0), b=string address, c=B)
_sr_loop:
	rev
	; (a=string address, b=length, c=B or length [undefined])
	dup
	; (a=string address, b=string address, c=length)
	lb
	; (a=char, b=string address, c=length)
	cj      _sr_end

	; (a=string address, b=length, c=length [undefined])
	; not end of string
	adc     1
	; (a=string address+1, b=length, c=length [undefined])
	rev
	; (a=length, b=string address+1, c=length [undefined])
	adc     1
	; (a=length+1, b=string address+1, c=length [undefined])
	j       _sr_loop

_sr_end
	; (a=char, b=string address, c=length)
	or    ; side-effect, don't care about address|char, but want length. 2 x pop would be nice but that's T801/T805
	; (a=address|char, b=length, c=length [undefined])
	rev
	; (a=length, b=address|char, c=length [undefined])
	ret


filler:
	db  0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa
	db  0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa

STOP:
	END

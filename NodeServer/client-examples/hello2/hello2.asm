TITLE Transputer Hello World

PAGE 60,132

; Second version without macros.

	.TRANSPUTER
	MemStart    EQU 0x80000070
	TptrLoc0    EQU 0x80000024
	TptrLoc1    EQU 0x80000028

	start_time  EQU 0

	ORG         MemStart - 1
	DB          STOP - START
	ORG         MemStart

; Standard Transputer startup code.

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



	j       MAIN

	NSS_CONSOLE     EQU 0x0100
	CONSOLE_PUT_PSTR    EQU NSS_CONSOLE OR 0x01
	LINK0_OUTPUT    EQU 0x80000000
	LINK0_INPUT     EQU 0x80000010


HWSTR:
	DB      "hello world", 0x0a, 0x00 ; length 13

MAIN:
    ajw     0x100    ; allow for 64 stack frames

	ldc     HWSTR - _M1
	ldpi
_M1:
	; call putConsolePString
	; a=800000FB (HWSTR), b=00000001 (irrelevant)
	call    putConsolePString

	; W 4 [80000158]=8000010D
    ; W 4 [8000015C]=800000FB (HWSTR)
    ; W 4 [80000160]=00000001
    ; W 4 [80000164]=80000000

FIN:
	terminate

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

    ; before call, w=80000169
    ; now this routine has been called, here, w=80000159
	ajw     -PPS_WLEN
    ; now w=80000151

    ; the call generates the following return stack
    ; DEBUG memory.cpp:281 W 4 [80000158]=8000010D (return address)
    ; DEBUG memory.cpp:281 W 4 [8000015C]=800000FB (old a, the string address)
    ; DEBUG memory.cpp:281 W 4 [80000160]=00000001 (old b)
    ; DEBUG memory.cpp:281 W 4 [80000164]=80000000 (old c)
    ; get the old areg (string address) off the return stack
    ldl     0x03
    ; 5 gets w+5word => 80000150+14 => 80000164 (old c)
    ; 4 gets w+4word => 80000150+10 => 80000160 (old b)
    ; 3 gets w+3word => 80000150+C = 8000015C (which is old a, the string address, which we want)
    ; 2 gets w+2word => 80000150+8 = 80000158 (the return address)

; idea for later, store CONSOLE_PUT_PSTR then the length next to each other in the workspace, then out them as a 5-byte
; message (LSB of the length will be next to the CONSOLE_PUT_PSTR), then the string text, so we have two outputs,
; rather than the three we have at the moment....?

	; Store the string pointer for later
    dup
    ; (a=string address, b=string address, c=B)
	stl     PPS_STRINGADDR
	; (a=string address, b=B, c=B)

	call    strlen

	; (a=length, b=useless, c=useless)
	dup
	; (a=length, b=length, c=useless)
	stl     PPS_STRINGLEN
	; (a=length, b,c=useless)

	; Empty string?
	cj      _PPS_END

	; (a,b,c=useless)

    ; Output CONSOLE_PUT_PSTR word
	ldc     LINK0_OUTPUT
	; (a=LINK0_OUTPUT, b,c=useless)
	ldc     CONSOLE_PUT_PSTR
	; (a=CONSOLE_PUT_PSTR, b=LINK0_OUTPUT, c=useless)
	outword     ; CONSOLE_PUT_PSTR WORD32 -->
	; (a=CONSOLE_PUT_PSTR, b=LINK0_OUTPUT, c=useless)

	; Output length byte
	pop
	; (a=LINK0_OUTPUT, b=useless, c=CONSOLE_PUT_PSTR)
	ldl     PPS_STRINGLEN
	; (a=string length, b=LINK0_OUTPUT, c=useless)
	outbyte     ; string length BYTE -->
	; (a=string length, b=LINK0_OUTPUT, c=useless)

    ; WANT:
    ; (a=string length; b=LINK0_OUTPUT, c=string address)

    ldl     PPS_STRINGADDR
    ; (a=string address; b=string length; c=LINK0_OUTPUT)
    pop
    ; (a=string length; b=LINK0_OUTPUT; c=string_address)
    out
    ; (a=string length; b=LINK0_OUTPUT; c=string_address)

_PPS_END:
	; Empty string => (a=length, b=useless, c=useless)
	ajw     PPS_WLEN
	ret



strlen: ; look, no workspace!! all registers!

    ; get the old areg (string address) off the return stack
    ; this routine is workspace free, so no ajw....
    ldl     0x01

	; strlen(a=string address, b=B, c=C): (a=length, b=useless, c=useless)
	ldc     0
	; (a=length (0), b=string address, c=B)
_sl_loop:
	rev
	; (a=string address, b=length, c=B or length [undefined])
	dup
	; (a=string address, b=string address, c=length)
	lb
	; (a=char, b=string address, c=length)
	cj      _sl_end

	; (a=string address, b=length, c=length [undefined])
	; not end of string
	adc     1
	; (a=string address+1, b=length, c=length [undefined])
	rev
	; (a=length, b=string address+1, c=length [undefined])
	adc     1
	; (a=length+1, b=string address+1, c=length [undefined])
	j       _sl_loop

_sl_end:
	; (a=char, b=string address, c=length)
	pop
	; (a=string address, b=length, c=char)
	pop
	; (a=length, b=char, c=string address)
	ret


STOP:
	END

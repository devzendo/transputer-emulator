TITLE Transputer Hello World

PAGE 60,132

; Third version without macros. Uses the IServer protocol.

	.TRANSPUTER
	MemStart    EQU 0x80000070
	MemLength   EQU MemStop - MemStart
	TptrLoc0    EQU 0x80000024
	TptrLoc1    EQU 0x80000028

	start_time  EQU 0

	ORG         MemStart - 1
	DB          MemLength
	ORG         MemStart

; Standard Transputer startup code.

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

	REQ_PUTS        EQU 0x0f
	STDOUT_STREAMID EQU 0x01
	LINK0_OUTPUT    EQU 0x80000000
	LINK0_INPUT     EQU 0x80000010


HWSTR:
	DB      "hello world", 0x00 ; length 11

MAIN:
	ajw     0x100    ; allow for 64 stack frames

	ldc     HWSTR - _M1
	ldpi
_M1:
	
	; a=800000FB (HWSTR), b=00000001 (irrelevant)
	call    putConsolePString
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
	PCPS_WLEN       EQU 2

	PPS_STRINGADDR  EQU 0
	PPS_STRINGLEN   EQU 1

	ajw     -PCPS_WLEN

	; Workspace following call here...
	; 5 => (old c)
	; 4 => (old b)
	; 3 => (which is old a, the string address, which we want)
	; 2 => (the return address)
	; 1 => PPS_STRINGLEN
	; 0 => PPS_STRINGADDR
	; get the old areg (string address) off the return stack
	WS_STR_ADDR EQU 0x03

	ldl     WS_STR_ADDR
	; (a=string address)

	; Store the string pointer for later
	dup
	; (a=string address, b=string address)
	stl     PPS_STRINGADDR   ; TODO don't need a separate WS element for this, it's at 0x03
	; (a=string address)

	call    strlen
	; (a=len)

	dup
	; (a=len, b=len)
	stl     PPS_STRINGLEN
	; (a=len)

	; Frame length is 2 bytes (a short), then the data for the frame itself:
	; 1 byte for frame type (REQ_PUTS)
	; 4 bytes for stream id 0
	; 2 bytes (short) for string length
	; then <len> bytes of string.
	; So, frame length is 7 + <len>. (The whole transmission includes the frame length short, so would be 9 + <len>.
	; By chance our message has an odd length, so the whole frame will have an even length.
	adc     7
	; (a=frame length)
	call    outshort0
	; ()

	; Output REQ_PUTS byte
	ldc     LINK0_OUTPUT
	; (a=LINK0_OUTPUT)
	ldc     REQ_PUTS
	; (a=REQ_PUTS, b=LINK0_OUTPUT)
	outbyte
	; ()

	; Output Stdout stream id word32
	ldc     LINK0_OUTPUT
	; (a=LINK0_OUTPUT)
	ldc     STDOUT_STREAMID
	; (a=STDOUT_STREAMID, b=LINK0_OUTPUT)
	outword
	; ()

	; Output length word16
	ldl     PPS_STRINGLEN
	; (a=string length)
	call    outshort0
	; ()

	; Output string data
	ldl     PPS_STRINGADDR
	ldc     LINK0_OUTPUT
	ldl     PPS_STRINGLEN
	; (a=string length; b=LINK0_OUTPUT, c=string address)
	out
	; ()

	ajw     PCPS_WLEN
	ret

; Output the LSB and MSB of a short word to LINK0_OUTPUT.
; (a=short word): ()
outshort0:
	; Workspace following call here...
	; 3 => (old c)
	; 2 => (old b)
	; 1 => (old a=short word)
	; 0 => (the return address)
	WSOS0_WORD EQU 0x01

	ldc     LINK0_OUTPUT
	; (a=link address)
	ldlp    WSOS0_WORD
	; (a=address of short word, b=link address)
	rev
	; (a=link address, b=address of short word)
	ldc     2
	; (a=length=2, b=link address, c=address of short word)
	out
	ret

; strlen(a=string address): (a=length)
strlen: ; look, no workspace!! all registers!

	; get the old areg (string address) off the return stack
	; this routine is workspace free, so no ajw....
	ldl     0x01

	ldc     0
	; (a=length (0), b=string address)
_sl_loop:
	rev
	; (a=string address, b=length)
	dup
	; (a=string address, b=string address, c=length)
	lb
	; (a=char, b=string address, c=length)
	cj      _sl_end

	; (a=string address, b=length)
	; not end of string
	adc     1
	; (a=string address+1, b=length)
	rev
	; (a=length, b=string address+1)
	adc     1
	; (a=length+1, b=string address+1)
	j       _sl_loop

_sl_end:
	; (a=char, b=string address, c=length)
	pop
	; (a=string address, b=length, c=char)
	pop
	; (a=length, b=char, c=string address)
	ret

MemStop:
	END

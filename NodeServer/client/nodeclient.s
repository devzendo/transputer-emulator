.globl _CONSOLE_PUT_CSTR_BUF_LIMIT;
.text
	.align 4;
_CONSOLE_PUT_CSTR_BUF_LIMIT:
	.word 16384
.globl _getServerVersion;
_getServerVersion:
	ajw -8
	ldc 0
	stl 2
	ldl 2
	stl 1
	mint
	stl 3
	ldc 0
	stl 4
	ldl 3
	ldl 4
	outword
	
	ldc -2147483632
	stl 5
	ldc 4
	stl 6
	ldlp 1
	stl 7
	ldl 7
	ldl 5
	ldl 6
	in
	ldl 1
	ajw +8
	ret
	ajw +8
	ret
.globl _exitServer;
_exitServer:
	ajw -3
	mint
	stl 1
	ldc 1
	stl 2
	ldl 1
	ldl 2
	outword
	
	ajw +3
	ret
.globl _putConsoleChar;
_putConsoleChar:
	ajw -7
	ldl 8
	stl 2
	ldlp 1
	stl 3
	ldl 2
	ldl 3
	sb
	mint
	stl 4
	ldc 256
	stl 5
	ldl 4
	ldl 5
	outword
	
	mint
	stl 6
	ldl 6
	ldl 1
	outbyte
	
	ajw +7
	ret
.globl _putConsolePString;
_putConsolePString:
	ajw -8
	ldl 9
	stl 2
	ldlp 1
	stl 3
	ldl 2
	ldl 3
	sb
	mint
	stl 4
	ldc 257
	stl 5
	ldl 4
	ldl 5
	outword
	
	mint
	stl 6
	ldl 6
	ldl 1
	outbyte
	
	mint
	stl 7
	ldl 10
	ldl 7
	ldl 1
	out
	ajw +8
	ret
_localStrlen:
	ajw -6
	ldc 0
	stl 1
	ldl 1
	stl 0
L@1:
	ldl 7
	stl 2
	ldl 2
	lb
	stl 3
	ldl 3
	eqc 0
	cj L@3
	j L@2
L@3:
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
	j L@1
L@2:
	ldl 0
	ajw +6
	ret
	ajw +6
	ret
.globl _putConsoleCString;
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
.globl _isConsolePutAvailable;
_isConsolePutAvailable:
	ajw -10
	ldc 0
	stl 2
	ldlp 1
	stl 3
	ldl 2
	ldl 3
	sb
	mint
	stl 4
	ldc 259
	stl 5
	ldl 4
	ldl 5
	outword
	
	ldc -2147483632
	stl 6
	ldc 1
	stl 7
	ldlp 1
	stl 8
	ldl 8
	ldl 6
	ldl 7
	in
	ldlp 1
	lb
	stl 9
	ldl 9
	eqc 1
	stl 9
	ldl 9
	ajw +10
	ret
	ajw +10
	ret
.globl _isConsoleGetAvailable;
_isConsoleGetAvailable:
	ajw -10
	ldc 0
	stl 2
	ldlp 1
	stl 3
	ldl 2
	ldl 3
	sb
	mint
	stl 4
	ldc 260
	stl 5
	ldl 4
	ldl 5
	outword
	
	ldc -2147483632
	stl 6
	ldc 1
	stl 7
	ldlp 1
	stl 8
	ldl 8
	ldl 6
	ldl 7
	in
	ldlp 1
	lb
	stl 9
	ldl 9
	eqc 1
	stl 9
	ldl 9
	ajw +10
	ret
	ajw +10
	ret
.globl _getConsoleChar;
_getConsoleChar:
	ajw -10
	ldc 0
	stl 2
	ldlp 1
	stl 3
	ldl 2
	ldl 3
	sb
	mint
	stl 4
	ldc 261
	stl 5
	ldl 4
	ldl 5
	outword
	
	ldc -2147483632
	stl 6
	ldc 1
	stl 7
	ldlp 1
	stl 8
	ldl 8
	ldl 6
	ldl 7
	in
	ldlp 1
	lb
	stl 9
	ldl 9
	ajw +10
	ret
	ajw +10
	ret
.globl _getTimeMillis;
_getTimeMillis:
	ajw -8
	ldc 0
	stl 2
	ldl 2
	stl 1
	mint
	stl 3
	ldc 1280
	stl 4
	ldl 3
	ldl 4
	outword
	
	ldc -2147483632
	stl 5
	ldc 4
	stl 6
	ldlp 1
	stl 7
	ldl 7
	ldl 5
	ldl 6
	in
	ldl 1
	ajw +8
	ret
	ajw +8
	ret
.globl _getTimeUTC;
_getTimeUTC:
	ajw -5
	mint
	stl 1
	ldc 1281
	stl 2
	ldl 1
	ldl 2
	outword
	
	ldc -2147483632
	stl 3
	ldc 28
	stl 4
	ldl 6
	ldl 3
	ldl 4
	in
	ajw +5
	ret

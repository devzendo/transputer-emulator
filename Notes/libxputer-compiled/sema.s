.globl ___semaphore_wait;
.globl ___semaphore_signal;
.text
.globl ___semaphore_wait;
___semaphore_wait:
	ajw -2
	ldl 3
	stl 0
L@25:
	ldl 0
	ldnl 0
	eqc 0
	cj L@26
	ldtimer
	stl 1
	ldl 1
	tin
	j L@25
L@26:
	ldl 0
	ldnl 0
	adc -1
	ldl 0
	stnl 0
	ldl 0
	ldnl 0
	ajw +2
	ret
.globl ___semaphore_signal;
___semaphore_signal:
	ldl 1
	ldnl 0
	adc 1
	ldl 1
	stnl 0
	ldl 1
	ldnl 0
	ret

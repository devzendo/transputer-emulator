.globl ___semaphore_wait;
.globl ___semaphore_signal;
.text
.globl _lddevid;
_lddevid:
	ajw -2
	ldc 2
	ldc 1
	ldc 0
	lddevid
	rev
	stl 0
	ldl 0
	eqc 0
	cj L@25
	stl 1
	j L@26
L@25:
	ldc 0
	ldc 1
	ldl 0
	adc -1
	ldiff
	rev
	eqc 0
	cj L@27
	ldl 0
	not
	adc 1
	stl 1
	j L@26
L@27:
	ldc -3
	stl 1
L@26:
	ldl 1
	ajw +2
	ret

.globl ___semaphore_wait;
.globl ___semaphore_signal;
.text
.globl ___alt;
___alt:
	ajw -4
	  alt                           
	  ldl 6; stl 1              
	0:                              
	  ldl 1; ldnl 0; cj 1f        
	  ldl 1; ldnl 0; ldc 1; enbc  
	  ldl 1; ldnlp 1; stl 1     
	  ldc 0; cj 0b                  
	1:                              
	  ldl 5; enbs                 
	  altwt                         
	  ldl 6; stl 1              
	0:                              
	  ldl 1; ldnl 0; cj 1f        
	  ldl 1; ldnl 0; ldc 1; ldl 1; disc 
	  eqc 0; cj 1f                  
	  ldl 1; ldnlp 1; stl 1     
	  ldc 0; cj 0b                  
	1:                              
	  ldc 5; ldc 0; diss          
	  ldl 0                         
	
	stl 2
	ldl 2
	cj L@25
	ldl 2
	ldl 6
	diff
	xdble
	ldc 2
	lshr
	stl 3
	j L@26
L@25:
	ldc -1
	stl 3
L@26:
	ldl 3
	ajw +4
	ret
.globl ___talt;
___talt:
	ajw -4
	  talt                          
	  ldl 5; ldc 1; enbt          
	  ldl 6; stl 1              
	0:                              
	  ldl 1; ldnl 0; cj 1f        
	  ldl 1; ldnl 0; ldc 1; enbc  
	  ldl 1; ldnlp 1; stl 1     
	  ldc 0; cj 0b                  
	1:                              
	  taltwt                        
	  ldl 6; stl 1              
	0:                              
	  ldl 1; ldnl 0; cj 1f        
	  ldl 1; ldnl 0; ldc 1; ldl 1; disc 
	  eqc 0; cj 2f                  
	  ldl 1; ldnlp 1; stl 1     
	  ldc 0; cj 0b                  
	1:                              
	  ldl 5; ldc 1; ldc 0; dist   
	2:                              
	  ldl 0                         
	
	stl 2
	ldl 2
	cj L@27
	ldl 2
	ldl 6
	diff
	xdble
	ldc 2
	lshr
	stl 3
	j L@28
L@27:
	ldc -1
	stl 3
L@28:
	ldl 3
	ajw +4
	ret
.globl _alt;
_alt:
	ajw -12
	ldtimer
	stl 3
	ldl 13
	eqc 0
	cj L@31
	ldl 14
	ldc 0
	call ___alt
	ajw +12
	ret
L@31:
	ldl 13
	ldnl 0
	eqc 0
	cj L@32
	ldl 13
	ldnl 1
	eqc 0
	cj L@32
	ldl 14
	ldc 1
	call ___alt
	ajw +12
	ret
L@32:
	ldpri
	cj L@33
	ldc 15625
	ldl 13
	ldnl 0
	prod
	stl 5
	ldl 13
	ldnl 1
	stl 6
	ldc 0
	ldl 6
	gt
	cj L@37
	ldl 6
	adc 63
	stl 6
L@37:
	ldl 6
	xdble
	ldc 6
	lshr
	ldl 5
	add
	stl 4
	j L@34
L@33:
	ldc 1000000
	ldl 13
	ldnl 0
	prod
	ldl 13
	ldnl 1
	add
	stl 4
L@34:
	ldl 4
	xdble
	stl 1
	stl 2
	ldl 2
	ldc 0
	gt
	eqc 0
	cj L@40
	ldl 2
	eqc 0
	cj L@39
	ldc 0
	ldc 2099999999
	ldl 1
	ldiff
	rev
	stl 7
	ldl 7
	cj L@39
L@40:
	ldc -2100000000
	stl 8
	ldc -1
	stl 9
	ldl 8
	ldl 1
	add
	stl 10
	ldc 0
	ldl 10
	ldl 1
	ldiff
	rev
	ldl 2
	adc -1
	stl 11
	ldl 11
	add
	stl 11
	ldl 10
	stl 10
	ldl 11
	stl 11
	ldl 10
	stl 1
	ldl 11
	stl 2
	ldl 3
	ldnlp 525000000
	stl 3
	ldl 14
	ldl 3
	call ___talt
	stl 0
	ldc 0
	ldl 0
	gt
	cj L@45
	ldl 2
	ldc 0
	gt
	eqc 0
	cj L@40
	ldl 2
	eqc 0
	cj L@39
	ldc 0
	ldc 2099999999
	ldl 1
	ldiff
	rev
	stl 7
	ldl 7
	eqc 0
	cj L@40
L@39:
	ldl 1
	eqc 0
	cj L@44
	ldl 2
	eqc 0
	cj L@44
	ldc -1
	ajw +12
	ret
L@44:
	ldl 1
	ldl 3
	add
	ldl 14
	rev
	call ___talt
	ajw +12
	ret
L@45:
	ldl 0
	ajw +12
	ret

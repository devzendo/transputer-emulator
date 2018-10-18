.globl ___semaphore_wait;
.globl ___semaphore_signal;
.text
.globl _process_tolow;
_process_tolow:
	ldlp  0  
	ldc   1  
	or       
	runp     
	stopp    
	ret
_starter:
	ajw -2
	ldl 3
	ldnl 0
	stl 1
	ldc -2
	ldl 1
	and
	ldl 3
	stnl 0
	ldl 3
	ldc 0
	outbyte
	ajw +2
	ret
.globl _process_tohigh;
_process_tohigh:
	ajw -20
	mint
	stl 17
	ldlp 17
	stl 18
	ldl 18
	stl 16
	ldlp 15
	stl 19
	ldl 19
	ldc _starter-LF44
	ldpi
LF44:
	  ldc 9f-8f  
	  ldpi       
	8:           
	  diff       
	  rev        
	  startp;    
	9: 
	ldl 18
	ldc 1
	alt    
	enbc   
	altwt  
	ajw +20
	ret

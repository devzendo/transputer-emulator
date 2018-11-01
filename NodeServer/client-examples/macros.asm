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
LENDL   MACRO   ctrl,label
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


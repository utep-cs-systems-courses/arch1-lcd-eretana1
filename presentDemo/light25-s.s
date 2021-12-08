	.arch msp430g2553
	.p2align 1,0
	.text

jt:		
	.word case0		;jt[0]
	.word case1		;jt[1]
	.word case2		;jt[2]
	.word case3		;jt[3]
	.word default		;jt[4]
	
	.global light_25
	.extern P1OUT

light_25:
	cmp #5, r12		;jmp if state > 4
	jhs default
	add r12, r12
	mov jt(r12), r0
	

case0:
case1:
case2:
	and.b #~64, &P1OUT
	bis.b #1, &P1OUT
	jmp exit

case3:
	bis.b #64, &P1OUT
	and.b #~1, &P1OUT

default:
exit:
	pop r0

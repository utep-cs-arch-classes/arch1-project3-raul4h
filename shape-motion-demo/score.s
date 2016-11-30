	.arch msp430g2553
	.text

	.global increaseScoreRed
	.global increaseScoreBlue

increaseScoreRed:
	add.b #1,5(r12)		;scoreRed[5]++
	ret
increaseScoreBlue:
	add.b #1,6(r12)		;scoreBlue[6]++
	ret

	.arch msp430g2553
	.text

	.global increaseScoreRed
	.global increaseScoreBlue

increaseScoreRed:
	add.b #1,5(r12)
	ret
increaseScoreBlue:
	add.b #1,6(r12)
	ret

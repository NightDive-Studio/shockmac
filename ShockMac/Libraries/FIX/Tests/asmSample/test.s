	export PPC_ASM_TEST

PPC_ASM_TEST:
  stmw     r24,-32(SP)
  srawi    r31,r3,16
  srawi    r30,r4,16
  andi.    r29,r3,$FFFF
  andi.    r28,r4,$FFFF
  mullw    r5,r31,r30
  slwi     r27,r5,16
  mullw    r26,r31,r28
  mullw    r25,r30,r29
  mullw    r6,r29,r28
  srawi    r6,r6,16
  andi.    r24,r6,$FFFF
  add      r7,r26,r25
  add      r7,r7,r24
  add      r3,r27,r7
  lmw      r24,-32(SP)
  blr

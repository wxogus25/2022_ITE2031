        lw      0       1       ten     load reg1 with 10 (symbolic address)
        lw      1       2       1       load reg2 with -3 (numeric address)
        lw      0       3       plus1   load reg3 with 1 (symbolic address)
        add     2       3       2       add reg2, reg3 (-3 + 1 = -2), save at reg2
        lw      0       4       stAddr  load reg4 with start(symbolic address)
start   add     1       2       1       decrement reg1 by -2
        beq     0       1       2       goto end of program when reg1==0
        jalr    4       5               go back to the beginning of the loop
        noop
done    halt                            end of program
ten     .fill   10
neg3    .fill   -3
stAddr  .fill   start                   will contain the address of start (2)
plus1   .fill   1

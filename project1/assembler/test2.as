        lw      0       1       five    undefined label (should be occur error)
        lw      1       2       0       load reg2 with -3 (numeric address)
        lw      0       3       plus1   load reg3 with 1 (symbolic address)
        add     2       3       2       add reg2, reg3 (-3 + 1 = -2), save at reg2     
start   add     1       2       1       decrement reg1 by -2
        beq     0       1       2       goto end of program when reg1==0
        beq     0       0       start   go back to the beginning of the loop
        noop
done    halt                            end of program
ten     .fill   10
neg3    .fill   -3
stAddr  .fill   start                   will contain the address of start (2)
plus1   .fill   1

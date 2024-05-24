        .syntax     old
        LHL
        CAL sub
        .syntax     new
        MOV H,L
        CALL sub
sub:


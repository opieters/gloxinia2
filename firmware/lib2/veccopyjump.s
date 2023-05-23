.global _vector_copy_jump
_vector_copy_jump:
    ; w0 = number of coefficients in source array
    ; w1 = destination array
    ; w2 = source array
    ; w3 = step size
    dec w0, w0
    add w3, w3, w3
    ; setup read page to EDS memory
    push DSRPAG
    mov #0x0001, w4
    mov w4, DSRPAG
    do w0, _final_copy_conv
    mov [w2], [w1++] ; [w1] = [w2]
_final_copy_conv:
    add w2, w3, w2
    pop DSRPAG
    return

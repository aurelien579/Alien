global __switch_pagedir
__switch_pagedir:
    mov dword eax, [esp+4]
    mov cr3, eax
    ret
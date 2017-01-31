global __switch_page_dir
__switch_page_dir:
    mov dword eax, [esp+4]
    mov cr3, eax
    ret

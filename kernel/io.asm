[bits 32]

section .text

global inb ;将 inb 导出
inb:
    push ebp
    mov ebp, esp 

    xor eax, eax
    mov edx, [ebp + 8]   ;edx里是端口号
    in al, dx ;将端口号的8bit输入到al中 

    jmp $+2
    jmp $+2
    jmp $+2

    leave
    ret 

global outb ;将 outb 导出
outb:
    push ebp
    mov ebp, esp 

    mov edx, [ebp + 8]   ;edx里是端口号
    mov eax, [ebp + 12]   ;eax里是输出值
    out dx, al ;将 al 中的 8bit 值 输出到 dx 端口号

    jmp $+2
    jmp $+2
    jmp $+2
    
    leave
    ret 

global inw ;将 inw 导出
inw:
    push ebp
    mov ebp, esp 

    xor eax, eax
    mov edx, [ebp + 8]   ;edx里是端口号
    in ax, dx ;将端口号的8bit输入到ax中 

    jmp $+2
    jmp $+2
    jmp $+2

    leave
    ret 

global outw ;将 outw 导出
outw:
    push ebp
    mov ebp, esp 

    mov edx, [ebp + 8]   ;edx里是端口号
    mov eax, [ebp + 12]   ;eax里是输出值
    out dx, ax ;将 al 中的 8bit 值 输出到 dx 端口号

    jmp $+2
    jmp $+2
    jmp $+2
    
    leave
    ret 
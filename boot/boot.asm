[org 0x7c00]

; 设置屏幕模式为文本模式，清除屏幕
mov ax, 3
int 0x10

; 初始化段寄存器
mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7c00

; 0xb800 文本显示器的内存区域
;mov ax, 0xb800
;mov ds, ax
;mov byte [0], 'H'

mov si, booting
call print

mov edi, 0x1000 ;读取的目标内存
mov ecx, 2 ;起始扇区
mov bl, 4 ;扇区数量

call read_disk

cmp word [0x1000], 0x55aa
jnz error

jmp 0:0x1002

; 阻塞
jmp $

read_disk:
    ;0x1f2: 设置读取扇区的数量
    mov dx, 0x01f2
    mov al, bl 
    out dx, al

    ;0x1f3: 设置起始扇区地址的低8位
    inc dx ;0x01f3
    mov al, cl 
    out dx, al

    ;0x1f4: 设置起始扇区地址的中8位
    inc dx ;0x01f4
    shr ecx, 8
    mov al, cl 
    out dx, al

    ;0x1f5: 设置起始扇区地址的高8位
    inc dx ;0x01f5
    shr ecx, 8
    mov al, cl 
    out dx, al

    inc dx ;0x01f6
    shr ecx, 8
    and cl, 0b0000_1111
    mov al, 0b1110_0000
    or al, cl
    out dx, al

    inc dx ; 0x1f7
    mov al, 0x20
    out dx, al

    xor ecx, ecx ;将ecx清空
    mov cl, bl

    .read:
        push cx
        call .waits
        call .reads
        pop cx
        loop .read
    ret

    .waits:
        mov dx, 0x01f7
        .check:
            in al, dx
            jmp $+2
            jmp $+2
            jmp $+2
            and al, 0b1000_1000
            cmp al, 0b0000_1000
            jnz .check
        ret

    .reads:
        mov dx, 0x01f0
        mov cx, 256
        .readw:
            in ax, dx
            jmp $+2
            jmp $+2
            jmp $+2
            mov [edi], ax
            add edi, 2
            loop .readw
        ret

print:
    mov ah, 0x0e
    .next:
        mov al, [si]
        cmp al, 0
        jz .done
        int 0x10
        inc si
        jmp .next
    .done:
        ret

booting:
    db "Hello Y1OS...", 10, 13, 0;

error:
    mov si, .msg
    call print
    hlt; 让 CPU 停止
    jmp $
    .msg db "Booting Error!!!", 10, 13, 0

; 填充0
times 510 - ($-$$) db 0
;主引导扇区的最后两个字节必须是0x55 0xaa
db 0x55, 0xaa

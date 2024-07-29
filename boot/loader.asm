[org 0x1000]

dw 0x55aa

mov si, loading
call print

;断点
;xchg bx, bx 

;以下内容为检测内存
detect_memory:
    xor ebx, ebx
    mov ax, 0
    mov es, ax
    mov edi, ards_buffer
    mov edx, 0x534d4150

    .next:
        mov eax, 0xe820
        mov ecx, 20
        int 0x15

        ;jc  助记符 检查eflags寄存器CF位 当CF位为1时jc 可以进行跳转 表示进位跳转
        ;jnc 助记符 检查eflags寄存器CF位 当CF位为0时jnc可以进行跳转 表示无进位进位跳转
        jc error

        ;将缓存指针指向下一个ards结构体
        add di, cx

        ; 将结构体数量加1
        inc dword [ards_count]

        ; 当ebx是0的时候就相当于检测完毕，当ebx不是0的时候就继续检测
        cmp ebx, 0
        jnz .next

        mov si, detecting
        call print
        
    ;mov cx, [ards_count]
    ;mov si, 0
    ;.show:
        ;mov eax, [ards_buffer + si]
        ;mov ebx, [ards_buffer + si + 8]
        ;mov edx, [ards_buffer + si + 16]
        ;add si, 20
        ;xchg bx, bx
        ;loop .show

    jmp prepare_protected_mode


; 准备保护模式
prepare_protected_mode:
    cli  ;关闭中断

    ; 打开 A20 线
    in al, 0x92 ;从0x92 端口读取数据到al寄存器中
    or al, 0b10
    out 0x92, al ;将 al寄存中的值写到0x92端口中

    ; 加载gdt
    lgdt [gdt_ptr]

    ; 启动保护模式 即是将cr0寄存器的0位置为1
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 用跳转来刷新缓存
    jmp dword code_selector:protect_mode


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

loading:
    db "Loading Y1OS...", 10, 13, 0;

detecting:
    db "Detecting Memory Success...", 10, 13, 0

error:
    mov si, .msg
    call print
    hlt
    jmp $
    .msg db "Loading Error!!!", 10, 13, 0



[bits 32]
protect_mode:
    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax  ; 初始化段寄存器

    mov esp, 0x10000 ; 修改栈顶

    mov edi, 0x10000 ;读取的目标内存
    mov ecx, 10 ;起始扇区
    mov bl, 200 ;扇区数量

    call read_disk ;读取内核

    mov eax, 0x20230322 ;内核魔数
    mov ebx, ards_count ;ards 数量指针

    jmp dword code_selector:0x10040

    ud2  ; 表示出错

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


code_selector equ (1 << 3)
data_selector equ (2 << 3)

memory_base equ 0   ; 内存开始的位置 基地址
memory_limit equ ((1024 * 1024 * 1024 * 4) / (1024 * 4)) -1 ; 内存界限

gdt_ptr:
    dw (gdt_end - gdt_base) - 1
    dd gdt_base

gdt_base:
    ;dd 是double word 双字 4个字节 写了两个0 就是8个字节
    dd 0, 0   ;NULL 描述符
gdt_code:
    dw memory_limit & 0xffff  ;段界限 0 ~ 15 位
    dw memory_base & 0xffff ;基地址 0 ~ 15位
    db (memory_base >> 16) & 0xff ;基地址 16 ~ 23位

    db 0b_1_00_1_1_0_1_0  ;存在 、DPL为0 、 S 、 代码 、 非依从 、 可读 、 没有被访问过
    db 0b_1_1_0_0_0000 | (memory_limit >> 16) & 0xf ;4K 、 32位 、 段界限 16 ~ 19 位

    db (memory_base >> 24) & 0xff ;基地址 24 ~ 31 位

gdt_data:
    dw memory_limit & 0xffff  ;段界限 0 ~ 15 位
    dw memory_base & 0xffff ;基地址 0 ~ 15位
    db (memory_base >> 16) & 0xff ;基地址 16 ~ 23位

    db 0b_1_00_1_0_0_1_0  ;存在 、DPL为0 、 S 、 数据 、 非依从 、 可写 、 没有被访问过
    db 0b_1_1_0_0_0000 | (memory_limit >> 16) & 0xf ;4K 、 32位 、 段界限 16 ~ 19 位

    db (memory_base >> 24) & 0xff ;基地址 24 ~ 31 位
gdt_end:

ards_count:
    dd 0
ards_buffer:
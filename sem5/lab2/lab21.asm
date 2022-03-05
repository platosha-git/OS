.386P 

include macro.inc

descr struc 
	lim 	dw 0 
	base_l 	dw 0
	base_m 	db 0
	attr_1 	db 0 
	attr_2 	db 0
	base_h 	db 0 
descr ends

trap struc
	offs_l	dw 0
	sel 	dw 16
	cntr 	db 0
	dtype 	db 8Fh
	offs_h 	dw 0
trap ends 	

STACK SEGMENT PARA STACK 'STACK'
	stack_start db 100h dup(0)
	stack_size=

data SEGMENT USE16
	gdt_null descr <0, 0, 0, 0, 0, 0>
	gdt_data descr <data_size - 1, 0, 0, 92h, 0, 0>
	gdt_code descr <code_size - 1, 0, 0, 98h, 0, 0>
	gdt_stack descr <255, 0, 0, 92h, 0, 0>
	gdt_screen descr <3999, 8000h, 0Bh, 92h, 0, 0>
	gdt_size=$-gdt_null

	idt label word
	trap 13 dup (<dummy>)
	trap <exc13>
	trap 18 dup (<dummy>)
	trap <new_08,,8Eh>
	trap <new_09,,8Eh>
	idt_size=$-idt

	pdescr 	df 0
	msg1 	db 27, '[31;42m In Real Mode! ', 27, '[0m$'
	msg2 	db 10, 27, '[31;42m Back To Real Mode! ', 27, '[0m$'
	string 	db '**** ****-**** ****-**** ****'
	len=$-string

	mark_08	dw 1600
	time_08 db 0
	
	mark_09	dd 1E0h
	
	master	db 0
	slave	db 0	
	
	data_size=$-gdt_null
data ends

text SEGMENT USE16
	assume CS:text, DS:data

textseg label word
exc13 proc
	pop eax
	iret
exc13 endp

dummy proc
	mov ax, 5555h
	jmp home
dummy endp

new_08 proc
	push ax
	push bx
	mov al, time_08
	add al, 30h
	mov ah, 71h
	mov bx, mark_08
	mov cx, 5
	mov es:[bx], ax
	add mark_08, 2

	inc time_08
	mov al, 20h
	out 20h, al
	pop bx
	pop ax
	db 66h
	iret
new_08 endp

new_09 proc
	push ax
	in al, 60h
	in al, 61h
	mov al, 'A'
	or al, 80h
	out 61h, al
	and al, 7Fh
	out 61h, al
	mov al, 20h
	out 20h, al	
	pop ax
	db 66h
	iret
new_09 endp

main proc
	xor eax, eax
	mov ax, data
	mov DS, ax
	shl eax, 4
	mov ebp, eax
	mov bx, offset gdt_data
	mov [bx].base_l, ax
	shr eax, 16
	mov [bx].base_m, al
	
	xor eax, eax
	mov ax, CS
	shl eax, 4
	mov bx, offset gdt_code
	mov [bx].base_l, ax
	shr eax, 16
	mov [bx].base_m, al
	
	xor eax, eax
	mov ax, SS
	shl eax, 4
	mov bx, offset gdt_stack
	mov [bx].base_l, ax
	shr eax, 16
	mov [bx].base_m, al

	mov dword ptr pdescr+2, ebp
	mov word ptr pdescr, gdt_size-1
	lgdt pdescr

	mov ah, 09h
	mov dx, offset msg1
	int 21h

	mov	ax, 3
    int	10h

	cli

	in al, 21h
	mov master, al
	in al, 0A1h
	mov slave, al

	mov al, 11h
	out 20h, al
	mov al, 32
	out 21h, al
	mov al, 4
	out 21h, al
	mov al, 1
	out 21h, al
	mov al, 0FCh
	out 21h, al

	mov al, 0FFh
	out 0A1h, al

	mov word ptr pdescr, idt_size-1
	xor eax, eax
	mov ax, offset idt
	add eax, ebp
	mov dword ptr pdescr+2, eax
	lidt pdescr

;Переход в защищенный режим
;Открытие линии A20
	mov al, 0D1h
	out 64h, al
	mov al, 0DFh
	out 60h, al

;Установка PE = 1
	mov eax, cr0
	or eax, 1
	mov cr0, eax

;Процессор работает в защищенном режиме

	db 0EAh 
	dw offset continue
	dw 16
continue:
	mov ax, 8
	mov DS, ax

	mov ax, 24
	mov SS, ax

	mov ax, 32
	mov ES, ax

	mov di, 3870
	mov ah, 00000010b

;Вместо таблицы выводим сообщение "В защищенном режиме"
    	mov al, 'I' 
	stosw 
    	mov al, 'n' 
    	stosw 
    	mov al, ' ' 
    	stosw 
    	mov al, 'P' 
	stosw 
    	mov al, 'r' 
    	stosw 
    	mov al, 'o' 
    	stosw 
    	mov al, 't' 
    	stosw 
    	mov al, 'e' 
    	stosw 
    	mov al, 'c' 
    	stosw 
    	mov al, 't' 
    	stosw
    	mov al, 'e' 
    	stosw 
    	mov al, 'd' 
    	stosw
    	mov al, ' ' 
    	stosw
    	mov al, 'M' 
    	stosw 
    	mov al, 'o' 
    	stosw 
    	mov al, 'd' 
    	stosw 
    	mov al, 'e' 
    	stosw 

	
	sti
	mov di, 1920
	mov cx, 320
	mov ax, 1E01h
scrn:
	stosw
	inc al
	push cx
	mov ecx, 200000
delay:
	db 67h
	loop delay
	pop cx
	loop scrn

	mov ax, 0FFFFh
home:
	mov si, offset string
	call wrd_asc

	mov si, offset string
	mov cx, len
	mov ah, 74h
	mov di, 1280
	
scrn1:
	lodsb
	stosw
	loop scrn1

;Закрытие линии А20
	mov al, 0D1h
	out 64h, al
	mov al, 0DDh
	out 60h, al

;Возврат в реальный режим(модификация)
	cli
	mov gdt_data.lim, 0FFFFh
	mov gdt_code.lim, 0FFFFh
	mov gdt_stack.lim, 0FFFFh
	mov gdt_screen.lim, 0FFFFh
	push DS
	pop DS
	push SS
	pop SS
	push ES
	pop ES

	db 0EAh
	dw offset go 
	dw 16
go: 
	mov eax, cr0
	and eax,  0FFFFFFFEh
	mov cr0, eax
	db 0EAh
	dw offset return
	dw text

;Процессор работает в реальном режиме
return:
	mov ax, data
	mov DS, ax
	mov ax, stk
	mov SS, ax 
	mov SP, 256
	
	mov ax, 3FFh
	mov word ptr pdescr, ax
	mov eax, 0
	mov dword ptr pdescr+2, eax
	lidt pdescr 
 
	mov al, 11h
	out 20h, al
	mov al, 8
	out 21h, al
	mov al, 4
	out 21h, al
	mov al, 1
	out 21h, al

	mov al, master
	out 21h, al
	mov al, slave
	out 0A1h, al
	sti

	;mov	ax, 3
    ;int	10h

	mov ah, 09h
	mov dx, offset msg2
	int 21h

	mov ax, 4C00h
	int 21h
main endp

wrd_asc proc
	push ax
	and ax, 0F000h
	mov cl, 12
	shr ax, cl
	call bin_asc
	mov byte ptr [SI], al
	pop ax
	push ax
	and ax, 0F00h
	mov cl, 8
	shr ax, cl
	inc si
	call bin_asc
	mov byte ptr [SI], al
	pop ax
	push ax
	and ax, 0F0h
	mov cl, 4
	shr ax, cl
	inc si
	call bin_asc
	mov byte ptr [SI], al
	pop ax
	push ax
	and ax, 0Fh
	inc si
	call bin_asc
	mov byte ptr [SI], al
	pop ax
	ret	
wrd_asc endp

bin_asc proc
	cmp al, 9
	ja lettr
	add al, 30h
	jmp ok
lettr:
	add al, 37h
ok:
	ret
bin_asc endp

code_size=$-textseg 
text ends 

stk segment stack use16
	db 256 dup ('^')
stk ends
	end main

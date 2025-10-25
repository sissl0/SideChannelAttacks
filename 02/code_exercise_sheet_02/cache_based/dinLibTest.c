// clang -g -O0 -no-pie -fno-pic cache_based/dinLibTest.c -o cache_based/dinLibTest -Wl,-z,lazy
#include <stdio.h>
int main() {
    puts("1");
    puts("2");
    return 0;
}

// Breakpoint 1, main () at dinLibTest.c:3
// 3	    puts("1");
// (gdb) disassemble 'puts@plt'
// Dump of assembler code for function puts@plt:
//    0x0000000000401030 <+0>:	jmp    *0x2fca(%rip)        # 0x404000 <puts@got.plt>
//    0x0000000000401036 <+6>:	push   $0x0
//    0x000000000040103b <+11>:	jmp    0x401020
// End of assembler dump.
// (gdb) x/6i 'puts@plt'
//    0x401030 <puts@plt>:	jmp    *0x2fca(%rip)        # 0x404000 <puts@got.plt>
//    0x401036 <puts@plt+6>:	push   $0x0
//    0x40103b <puts@plt+11>:	jmp    0x401020
//    0x401040 <_start>:	endbr64
//    0x401044 <_start+4>:	xor    %ebp,%ebp
//    0x401046 <_start+6>:	mov    %rdx,%r9
// (gdb) set $plt = (unsigned char*)'puts@plt'
// (gdb) set $rel = *(int32_t*)($plt+2)
// set $gotp = (void**)($plt + 6 + $rel)
// A syntax error in expression, near `set $gotp = (void**)($plt + 6 + $rel)'.
// (gdb) set $rel = *(int32_t*)($plt+2)
// (gdb) set $gotp = (void**)($plt + 6 + $rel)
// (gdb) p/x $gotp
// $1 = 0x404000
// (gdb) p/x *$gotp
// $2 = 0x401036
// (gdb) next
// 1
// 4	    puts("2");
// (gdb) si
// 0x0000000000401158	4	    puts("2");
// (gdb) finish
// "finish" not meaningful in the outermost frame.
// (gdb) p/x *$gotp
// $3 = 0x7ffff7c87be0
// (gdb) 

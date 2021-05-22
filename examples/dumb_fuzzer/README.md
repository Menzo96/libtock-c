Dumb Fuzzer
==========

This application is a very simple fuzzer that tries a few Tock OS system calls (allow_readwrite, subscribe, command) with random values / uninitialise as parameters and logs the fuzzing process on the console.

This application is tested and developed for the Microbit V2 boards.
It uses the latest version of TockOS kernel (version on master branch) without bootloader (**tock/boards/microbit_v2/layout.ld**).

After flashing the kernel, go to the corresponding dumb_fuzzer application folder: **libtock-c/examples/dumb_fuzzer** and run **make**.

Load the application onto the board with tockloader:
```
tockloader install --board microbit_v2 --openocd ./build/dumb_fuzzer.tab
```

Open the serial to see the logging:
```
tockloader listen
```

After a few lines, the application will crash with a similar output as the one below:

```
panicked at 'Process dumb_fuzzer had a fault', kernel/src/process_standard.rs:287:17
	Kernel version release-1.6-1488-gbf54e789c

---| No debug queue found. You can set it with the DebugQueue component.

---| Cortex-M Fault Status |---
Data Access Violation:              true
Forced Hard Fault:                  true
Faulting Memory Address:            0x00000000
Fault Status Register (CFSR):       0x00000082
Hard Fault Status Register (HFSR):  0x40000000

---| App Status |---
𝐀𝐩𝐩: dumb_fuzzer   -   [Faulted]
 Events Queued: 0   Syscall Count: 314   Dropped Upcall Count: 0
 Restart Count: 0
 Last Syscall: Some(Memop { operand: 1, arg0: 108 })


 ╔═══════════╤══════════════════════════════════════════╗
 ║  Address  │ Region Name    Used | Allocated (bytes)  ║
 ╚0x20006000═╪══════════════════════════════════════════╝
             │ ▼ Grant        1280 |   1280          
  0x20005B00 ┼───────────────────────────────────────────
             │ Unused
  0x200057D0 ┼───────────────────────────────────────────
             │ ▲ Heap         3300 |   4116               S
  0x20004AEC ┼─────────────────────────────────────────── R
             │ Data            748 |    748               A
  0x20004800 ┼─────────────────────────────────────────── M
             │ ▼ Stack        2048 |   2048          
  0x20004000 ┼───────────────────────────────────────────
             │ Unused
  0x20004000 ┴───────────────────────────────────────────
             .....
  0x00044000 ┬─────────────────────────────────────────── F
             │ App Flash     16336                        L
  0x00040030 ┼─────────────────────────────────────────── A
             │ Protected        48                        S
  0x00040000 ┴─────────────────────────────────────────── H

  R0 : 0x00000001    R6 : 0x00000000
  R1 : 0x00000001    R7 : 0x20005140
  R2 : 0x20005140    R8 : 0x20004AE4
  R3 : 0x0000001E    R10: 0x00000002
  R4 : 0x20004588    R11: 0x00000000
  R5 : 0x00042135    R12: 0x0D9E67A2
  R9 : 0x20004800 (Static Base Register)
  SP : 0x20004558 (Process Stack Pointer)
  LR : 0x00042055
  PC : 0x0004069A
 YPC : 0x00040700

 APSR: N 0 Z 0 C 1 V 0 Q 0
       GE 0 0 0 0
 EPSR: ICI.IT 0x00
       ThumbBit true 

 Total number of grant regions defined: 14
  Grant  0: --          Grant  5: --          Grant 10: --        
  Grant  1: --          Grant  6: --          Grant 11: --        
  Grant  2: 0x20005fd0  Grant  7: 0x20005fe4  Grant 12: --        
  Grant  3: --          Grant  8: --          Grant 13: --        
  Grant  4: 0x20005fd8  Grant  9: 0x20005fec

 Cortex-M MPU
  Region 0: [0x20004000:0x20006000], length: 8192 bytes; ReadWrite (0x3)
    Sub-region 0: [0x20004000:0x20004400], Enabled
    Sub-region 1: [0x20004400:0x20004800], Enabled
    Sub-region 2: [0x20004800:0x20004C00], Enabled
    Sub-region 3: [0x20004C00:0x20005000], Enabled
    Sub-region 4: [0x20005000:0x20005400], Enabled
    Sub-region 5: [0x20005400:0x20005800], Enabled
    Sub-region 6: [0x20005800:0x20005C00], Disabled
    Sub-region 7: [0x20005C00:0x20006000], Disabled
  Region 1: [0x00040000:0x00044000], length: 16384 bytes; UnprivilegedReadOnly (0x2)
    Sub-region 0: [0x00040000:0x00040800], Enabled
    Sub-region 1: [0x00040800:0x00041000], Enabled
    Sub-region 2: [0x00041000:0x00041800], Enabled
    Sub-region 3: [0x00041800:0x00042000], Enabled
    Sub-region 4: [0x00042000:0x00042800], Enabled
    Sub-region 5: [0x00042800:0x00043000], Enabled
    Sub-region 6: [0x00043000:0x00043800], Enabled
    Sub-region 7: [0x00043800:0x00044000], Enabled
  Region 2: Unused
  Region 3: Unused
  Region 4: Unused
  Region 5: Unused
  Region 6: Unused
  Region 7: Unused

To debug, run `make debug RAM_START=0x20004000 FLASH_INIT=0x40059`
in the app's folder and open the .lst file.
```

When inspecting the .lst file, we can see that the program had a fault in **allow_readonly** function, at the driver number cmp instruction:

```
0004068c <allow_readonly>:
allow_ro_return_t allow_readonly(uint32_t driver, uint32_t allow, const void* ptr, size_t size) {
   4068c:       b510            push    {r4, lr}
   4068e:       4604            mov     r4, r0
  register uint32_t r0 __asm__ ("r0")       = driver;
   40690:       4608            mov     r0, r1
  register uint32_t r1 __asm__ ("r1")       = allow;
   40692:       4611            mov     r1, r2
  register const void*    r2 __asm__ ("r2") = ptr;
   40694:       461a            mov     r2, r3
  register size_t r3 __asm__ ("r3")         = size;
   40696:       9b02            ldr     r3, [sp, #8]
  __asm__ volatile (
   40698:       df04            svc     4
  if (rtype == TOCK_SYSCALL_SUCCESS_U32_U32) {
   ***4069a:       2882            cmp     r0, #130        ; 0x82***
   4069c:       d107            bne.n   406ae <allow_readonly+0x22>
    return rv;
   4069e:       2301            movs    r3, #1
   406a0:       e9c4 1201       strd    r1, r2, [r4, #4]
   406a4:       7023            strb    r3, [r4, #0]
   406a6:       2300            movs    r3, #0
   406a8:       7323            strb    r3, [r4, #12]
}
```
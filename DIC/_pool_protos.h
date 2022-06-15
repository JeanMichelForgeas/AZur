extern void * __asm AsmCreatePool(register __d0 ULONG,register __d1 ULONG,register __d2 ULONG,register __a6 struct ExecBase *);
extern void __asm AsmDeletePool(register __a0 void *,register __a6 struct ExecBase *);
extern void * __asm AsmAllocPooled(register __a0 void *,register __d0 ULONG,register __a6 struct ExecBase *);
extern void __asm AsmFreePooled(register __a0 void *,register __a1 void *,register __d0 ULONG,register __a6 struct ExecBase *);

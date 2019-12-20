#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Yuki");

#define isb1() __asm__ __volatile__("isb sy")
#define dsb1() __asm__ __volatile("dsb sy")
#define EL3 0
#define EL1_NONSECURE 1
#define XREG_CONTROL_DCACHE_BIT	(0X00000001U<<2U)
#define XREG_CONTROL_ICACHE_BIT	(0X00000001U<<12U)
#define IRQ_FIQ_MASK 0xC0U	/* Mask IRQ and FIQ interrupts in cpsr */
int DCacheDisable(void)
{
	register u32 CsidReg;
	register u32 C7Reg;
	register u32 LineSize;
	register u32 NumWays;
	register u32 Way;
	register u32 WayIndex;
	register u32 WayAdjust;
	register u32 Set;
	register u32 SetIndex;
	register u32 NumSet;
	register u32 CacheLevel;
	
	dsb1();
	asm(
	"mov 	x0, #0\n\t"
#if EL3==1
	"mrs	x0, sctlr_el3 \n\t"
	"and	w0, w0, #0xfffffffb\n\t"
	"msr	sctlr_el3, x0\n\t"
#elif EL1_NONSECURE==1
	"mrs	x0, sctlr_el1 \n\t"
	"and	w0, w0, #0xfffffffb\n\t"
	"msr	sctlr_el1, x0\n\t"
#endif
	"dsb sy\n\t"
	);
	/* Number of level of cache*/
	CacheLevel = 0U;
	/* Select cache level 0 and D cache in CSSR */
	__asm__ __volatile__("msr CSSELR_EL1,%0"::"r"(CacheLevel));
	isb1();
	__asm__ __volatile__("mrs %0, ccsidr_el1":"=r"(CsidReg));
	/* Get the cacheline size, way size, index size from csidr */
	LineSize = (CsidReg & 0x00000007U) + 0x00000004U;

	/* Number of Ways */
	NumWays = (CsidReg & 0x00001FFFU) >> 3U;
	NumWays += 0x00000001U;

	/*Number of Set*/
	NumSet = (CsidReg >> 13U) & 0x00007FFFU;
	NumSet += 0x00000001U;

	__asm__ __volatile__("clz %0, %1":"=r"(WayAdjust):"r"(NumWays));
	WayAdjust = WayAdjust - (u32)0x0000001FU;
	
	Way = 0U;
	Set = 0U;

	/* Flush all the cachelines */
	for (WayIndex = 0U; WayIndex < NumWays; WayIndex++) {
		for (SetIndex = 0U; SetIndex < NumSet; SetIndex++) {
			C7Reg = Way | Set | CacheLevel;
			__asm__ __volatile__("dc CISW,%0"::"r"(C7Reg));
			Set += (0x00000001U << LineSize);
		}
		Set = 0U;
		Way += (0x00000001U << WayAdjust);
	}

	/* Wait for Flush to complete */
	dsb1();

	/* Select cache level 1 and D cache in CSSR */
	CacheLevel += (0x00000001U << 1U);
	__asm__ __volatile__("msr CSSELR_EL1,%0"::"r"(CacheLevel));
	isb1();

	__asm__ __volatile__("mrs %0, ccsidr_el1":"=r"(CsidReg));

	/* Get the cacheline size, way size, index size from csidr */
	LineSize = (CsidReg & 0x00000007U) + 0x00000004U;

	/* Number of Ways */
	NumWays = (CsidReg & 0x00001FFFU) >> 3U;
	NumWays += 0x00000001U;

	/* Number of Sets */
	NumSet = (CsidReg >> 13U) & 0x00007FFFU;
	NumSet += 0x00000001U;

	__asm__ __volatile__("clz %0, %1":"=r"(WayAdjust):"r"(NumWays));
	WayAdjust = WayAdjust - (u32)0x0000001FU;

	Way = 0U;
	Set = 0U;

	/* Flush all the cachelines */
	for (WayIndex =0U; WayIndex < NumWays; WayIndex++) {
		for (SetIndex =0U; SetIndex < NumSet; SetIndex++) {
			C7Reg = Way | Set | CacheLevel;
			__asm__ __volatile__("dc CISW,%0"::"r"(C7Reg));
			Set += (0x00000001U << LineSize);
		}
		Set=0U;
		Way += (0x00000001U<<WayAdjust);
	}
	/* Wait for Flush to complete */
	dsb1();

	asm(
#if EL3==1
		"tlbi 	ALLE3\n\t"
#elif EL1_NONSECURE==1
		"tlbi 	VMALLE1\n\t"
#endif
		"dsb sy\r\n"
		"isb\n\t"
	);
	printk(KERN_INFO "DCache has been disabled!\r\n");
	return 0;
}

void DCacheInvalidate(void)
{
	register u32 CsidReg, C7Reg;
	u32 LineSize, NumWays;
	u32 Way, WayIndex,WayAdjust, Set, SetIndex, NumSet, CacheLevel;
	u32 currmask;

	__asm__ __volatile__("mrs %0, DAIF":"=r"(currmask));
	__asm__ __volatile__ ("msr DAIF, %0" : : "r" (currmask | IRQ_FIQ_MASK));


	/* Number of level of cache*/

	CacheLevel=0U;
	/* Select cache level 0 and D cache in CSSR */
	__asm__ __volatile__ ("msr CSSELR_EL1, %0" : : "r" (CacheLevel));
	isb1();

	__asm__ __volatile__("mrs %0, CCSIDR_EL1":"=r"(CsidReg));

	/* Get the cacheline size, way size, index size from csidr */
	LineSize = (CsidReg & 0x00000007U) + 0x00000004U;

	/* Number of Ways */
	NumWays = (CsidReg & 0x00001FFFU) >> 3U;
	NumWays += 0X00000001U;

	/*Number of Set*/
	NumSet = (CsidReg >> 13U) & 0x00007FFFU;
	NumSet += 0X00000001U;

	__asm__ __volatile__("clz %0, %1":"=r"(WayAdjust):"r"(NumWays));
	WayAdjust = WayAdjust - (u32)0x0000001FU;

	Way = 0U;
	Set = 0U;

	/* Invalidate all the cachelines */
	for (WayIndex =0U; WayIndex < NumWays; WayIndex++) {
		for (SetIndex =0U; SetIndex < NumSet; SetIndex++) {
			C7Reg = Way | Set | CacheLevel;
			__asm__ __volatile__("dc ISW,%0" : : "r" (C7Reg));
			Set += (0x00000001U << LineSize);
		}
		Set = 0U;
		Way += (0x00000001U << WayAdjust);
	}

	/* Wait for invalidate to complete */
	dsb1();

	/* Select cache level 1 and D cache in CSSR */
	CacheLevel += (0x00000001U<<1U) ;
	__asm__ __volatile__ ("msr CSSELR_EL1, %0" : : "r" (CacheLevel));
	isb1();

	__asm__ __volatile__("mrs %0, CCSIDR_EL1":"=r"(CsidReg));

	/* Get the cacheline size, way size, index size from csidr */
		LineSize = (CsidReg & 0x00000007U) + 0x00000004U;

	/* Number of Ways */
	NumWays = (CsidReg & 0x00001FFFU) >> 3U;
	NumWays += 0x00000001U;

	/* Number of Sets */
	NumSet = (CsidReg >> 13U) & 0x00007FFFU;
	NumSet += 0x00000001U;

	__asm__ __volatile__("clz %0, %1":"=r"(WayAdjust):"r"(NumWays));
	WayAdjust = WayAdjust - (u32)0x0000001FU;

	Way = 0U;
	Set = 0U;

	/* Invalidate all the cachelines */
	for (WayIndex = 0U; WayIndex < NumWays; WayIndex++) {
		for (SetIndex = 0U; SetIndex < NumSet; SetIndex++) {
			C7Reg = Way | Set | CacheLevel;
			__asm__ __volatile__("dc ISW,%0" : : "r" (C7Reg));
			Set += (0x00000001U << LineSize);
		}
		Set = 0U;
		Way += (0x00000001U << WayAdjust);
	}
	/* Wait for invalidate to complete */
	dsb1();

	__asm__ __volatile__ ("msr DAIF, %0" : : "r" (currmask));
}

void DCacheEnable(void)
{
	u32 CtrlReg;

	if (EL3 == 1) {
		__asm__ __volatile__("mrs %0, SCTLR_EL3":"=r"(CtrlReg));
	} else if (EL1_NONSECURE == 1) {
		__asm__ __volatile__("mrs %0, SCTLR_EL1":"=r"(CtrlReg));
	} else {
		CtrlReg = 0U;
	}

	/* enable caches only if they are disabled */
	if((CtrlReg & XREG_CONTROL_DCACHE_BIT) == 0X00000000U){

		/* invalidate the Data cache */
		DCacheInvalidate();

		CtrlReg |= XREG_CONTROL_DCACHE_BIT;

		if (EL3 == 1) {
			/* enable the Data cache for el3*/
			__asm__ __volatile__("msr SCTLR_EL3,%0"::"r"(CtrlReg));
		} else if (EL1_NONSECURE == 1) {
			/* enable the Data cache for el1*/
			__asm__ __volatile__("msr SCTLR_EL1,%0"::"r"(CtrlReg));
		}
	}
	printk(KERN_INFO "DCache has been enabled!\r\n");
}

module_init(DCacheDisable);
module_exit(DCacheEnable);

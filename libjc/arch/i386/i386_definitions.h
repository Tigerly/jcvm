
/*
 * @COPYRIGHT@
 *
 * $Id: i386_definitions.h,v 1.6 2005/05/08 21:12:07 archiecobbs Exp $
 */

#ifndef _ARCH_I386_DEFINITIONS_H_
#define _ARCH_I386_DEFINITIONS_H_

#if !defined(__i386__)
#error "This include file is for the i386 architecture only"
#endif

#define	_JC_PAGE_SHIFT		12		/* 4096 byte pages */

#define _JC_ELF_CLASS		ELFCLASS32
#define _JC_ELF_DATA		ELFDATA2LSB
#define _JC_ELF_MACHINE		EM_386

#define _JC_STACK_ALIGN		2

#define _JC_BIG_ENDIAN		0

#ifdef __FreeBSD__

#define	_JC_REGISTER_OFFS	{					\
	_JC_OFFSETOF(mcontext_t, mc_gs),				\
	_JC_OFFSETOF(mcontext_t, mc_fs),				\
	_JC_OFFSETOF(mcontext_t, mc_es),				\
	_JC_OFFSETOF(mcontext_t, mc_ds),				\
	_JC_OFFSETOF(mcontext_t, mc_edi),				\
	_JC_OFFSETOF(mcontext_t, mc_esi),				\
	_JC_OFFSETOF(mcontext_t, mc_ebp),				\
	_JC_OFFSETOF(mcontext_t, mc_isp),				\
	_JC_OFFSETOF(mcontext_t, mc_ebx),				\
	_JC_OFFSETOF(mcontext_t, mc_edx),				\
	_JC_OFFSETOF(mcontext_t, mc_ecx),				\
	_JC_OFFSETOF(mcontext_t, mc_eax),				\
	_JC_OFFSETOF(mcontext_t, mc_cs),				\
	_JC_OFFSETOF(mcontext_t, mc_esp),				\
	_JC_OFFSETOF(mcontext_t, mc_ss),				\
    }

#elif defined(__linux__)

#define	_JC_REGISTER_OFFS	{					\
	_JC_OFFSETOF(mcontext_t, gregs) + REG_GS  * sizeof(greg_t),	\
	_JC_OFFSETOF(mcontext_t, gregs) + REG_FS  * sizeof(greg_t),	\
	_JC_OFFSETOF(mcontext_t, gregs) + REG_ES  * sizeof(greg_t),	\
	_JC_OFFSETOF(mcontext_t, gregs) + REG_DS  * sizeof(greg_t),	\
	_JC_OFFSETOF(mcontext_t, gregs) + REG_EDI * sizeof(greg_t),	\
	_JC_OFFSETOF(mcontext_t, gregs) + REG_ESI * sizeof(greg_t),	\
	_JC_OFFSETOF(mcontext_t, gregs) + REG_EBP * sizeof(greg_t),	\
	_JC_OFFSETOF(mcontext_t, gregs) + REG_ESP * sizeof(greg_t),	\
	_JC_OFFSETOF(mcontext_t, gregs) + REG_EBX * sizeof(greg_t),	\
	_JC_OFFSETOF(mcontext_t, gregs) + REG_EDX * sizeof(greg_t),	\
	_JC_OFFSETOF(mcontext_t, gregs) + REG_ECX * sizeof(greg_t),	\
	_JC_OFFSETOF(mcontext_t, gregs) + REG_EAX * sizeof(greg_t),	\
	_JC_OFFSETOF(mcontext_t, gregs) + REG_CS  * sizeof(greg_t),	\
	_JC_OFFSETOF(mcontext_t, gregs) + REG_UESP* sizeof(greg_t),	\
	_JC_OFFSETOF(mcontext_t, gregs) + REG_SS  * sizeof(greg_t),	\
    }

#else
#error "Unsupported O/S for i386 _JC_REGISTER_OFFS"
#endif

#endif	/* _ARCH_I386_DEFINITIONS_H_ */


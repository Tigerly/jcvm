
/*
 * @COPYRIGHT@
 *
 * $Id: jc_defs.h,v 1.40 2005/12/11 19:26:47 archiecobbs Exp $
 */

#ifndef _JC_DEFS_H_
#define _JC_DEFS_H_

#ifndef __GNUC__		/* XXX check gcc version here too */
#error "GCC is required"
#endif

#include "jc_machdep.h"
#include "jc_arraydefs.h"

/************************************************************************
 *			      Definitions				*
 ************************************************************************/

/*
 * Binary compatibility version for this header file. This should
 * be incremented after each incompatible change to this file.
 */
#define _JC_LIBJC_VERSION		0x000c

/* C stuff */
#define _JC_NULL			((void *)0)
#define _JC_OFFSETOF(s, f)		((_jc_word)&((s *)0)->f)

/* Access flags */
#define _JC_ACC_PUBLIC			0x0001
#define _JC_ACC_PRIVATE			0x0002
#define _JC_ACC_PROTECTED		0x0004
#define _JC_ACC_STATIC			0x0008
#define _JC_ACC_FINAL			0x0010
#define _JC_ACC_SUPER			0x0020
#define _JC_ACC_SYNCHRONIZED		0x0020
#define _JC_ACC_VOLATILE		0x0040
#define _JC_ACC_TRANSIENT		0x0080
#define _JC_ACC_NATIVE			0x0100
#define _JC_ACC_INTERFACE		0x0200
#define _JC_ACC_ABSTRACT		0x0400
#define _JC_ACC_STRICT			0x0800
#define _JC_ACC_MASK			0x0fff

/* Additional flags stored with access flags */
#define _JC_ACC_JCNI			0x1000	/* JCNI native method (!JNI) */
#define _JC_ACC_INTERP			0x2000	/* interpreted class/method */
#define _JC_ACC_PCMAP			0x4000	/* method pc map created */

/*
 * Flags for 'flags' field of the '_jc_type' structure,
 * and unique indicies for the various primitive Java types.
 */
#define _JC_TYPE_INVALID		0x0000
#define _JC_TYPE_BOOLEAN		0x0001
#define _JC_TYPE_BYTE			0x0002
#define _JC_TYPE_CHAR			0x0003
#define _JC_TYPE_SHORT			0x0004
#define _JC_TYPE_INT			0x0005
#define _JC_TYPE_LONG			0x0006
#define _JC_TYPE_FLOAT			0x0007
#define _JC_TYPE_DOUBLE			0x0008
#define _JC_TYPE_VOID			0x0009
#define _JC_TYPE_REFERENCE		0x000a
#define _JC_TYPE_MAX			0x000b
#define _JC_TYPE_MASK			0x000f
#define _JC_TYPE_ARRAY			0x0010

#define _JC_TYPE_SKIPWORD		0x0020
#define _JC_TYPE_LOADED			0x0040
#define _JC_TYPE_INIT_ERROR		0x0080		/* class init error */

#define _JC_TYPE_VERIFIED		0x0100
#define _JC_TYPE_PREPARED		0x0200
#define _JC_TYPE_RESOLVED		0x0400
#define _JC_TYPE_INITIALIZED		0x8000

/* Fixed parameters for hash tables */
#define _JC_IMETHOD_HASHSIZE		128		/* must be power of 2 */
#define _JC_INSTANCEOF_HASHSIZE		128		/* must be power of 2 */

/************************************************************************
 *				Typedefs				*
 ************************************************************************/

typedef struct _jc_object		_jc_object;

typedef struct _jc_env			_jc_env;
typedef struct _jc_splay_node		_jc_splay_node;
typedef struct _jc_class_loader		_jc_class_loader;
typedef struct _jc_array_type		_jc_array_type;
typedef struct _jc_nonarray_type	_jc_nonarray_type;
typedef struct _jc_classfile		_jc_classfile;
typedef struct _jc_super_info		_jc_super_info;
typedef struct _jc_elf			_jc_elf;

typedef struct _jc_type			_jc_type;
typedef struct _jc_method		_jc_method;
typedef struct _jc_method_exec		_jc_method_exec;
typedef struct _jc_method_code		_jc_method_code;
typedef struct _jc_linenum_info		_jc_linenum_info;
typedef struct _jc_pc_map_info		_jc_pc_map_info;
typedef struct _jc_array		_jc_array;
typedef struct _jc_object_array		_jc_object_array;
typedef union _jc_value			_jc_value;
typedef struct _jc_linenum		_jc_linenum;
typedef struct _jc_catch_frame		_jc_catch_frame;
typedef struct _jc_pc_map		_jc_pc_map;
typedef struct _jc_env_head		_jc_env_head;

typedef struct _jc_field		_jc_field;
typedef struct _jc_trap_info		_jc_trap_info;
typedef struct _jc_class_depend		_jc_class_depend;
typedef struct _jc_inner_class		_jc_inner_class;

typedef struct _jc_lookup		_jc_lookup;
typedef struct _jc_iinc			_jc_iinc;
typedef struct _jc_invoke		_jc_invoke;
typedef struct _jc_linemap		_jc_linemap;
typedef struct _jc_lookupswitch		_jc_lookupswitch;
typedef union _jc_insn_info		_jc_insn_info;
typedef struct _jc_multianewarray	_jc_multianewarray;
typedef struct _jc_tableswitch		_jc_tableswitch;
typedef struct _jc_interp_trap		_jc_interp_trap;

/************************************************************************
 *				Structures				*
 ************************************************************************/

/*
 * Java value. This differs from a JNI 'jvalue' in that if the value is a
 * Java object, then the 'l' field points directly to the object, whereas
 * in a 'jvalue' the 'l' field is a native reference to the object.
 */
union _jc_value {
	jboolean	z;
	jbyte		b;
	jchar		c;
	jshort		s;
	jint		i;
	jlong		j;
	jfloat		f;
	jdouble		d;
	_jc_object	*l;
	_jc_word	_dummy;			/* ensure size/alignment */
};

/*
 * One splay search tree node.
 */
struct _jc_splay_node {
	_jc_splay_node	*left;
	_jc_splay_node	*right;
	jboolean	inserted;
};

/* Object head */
struct _jc_object {
	_jc_word	lockword;
	_jc_type	*type;
};

/* Array head */
struct _jc_array {
	_jc_word	lockword;
	_jc_type	*type;
	const jint	length;
};

/*
 * Scalar arrays.
 */
#define _JC_DECLARE_SCALAR_ARRAY(type0, ctype)				\
struct _jc_ ## type0 ## _array {					\
	_jc_word	lockword;					\
	_jc_type	*type;						\
	const jint	length;						\
	ctype		elems[0];					\
};									\
typedef struct _jc_ ## type0 ## _array	_jc_ ## type0 ## _array;

_JC_DECLARE_SCALAR_ARRAY(boolean, jboolean);
_JC_DECLARE_SCALAR_ARRAY(byte, jbyte);
_JC_DECLARE_SCALAR_ARRAY(char, jchar);
_JC_DECLARE_SCALAR_ARRAY(short, jshort);
_JC_DECLARE_SCALAR_ARRAY(int, jint);
_JC_DECLARE_SCALAR_ARRAY(long, jlong);
_JC_DECLARE_SCALAR_ARRAY(float, jfloat);
_JC_DECLARE_SCALAR_ARRAY(double, jdouble);

/*
 * Reference arrays.
 *
 * Elements are indexed backwards, i.e., 0 is elems[-1], 1 is elems[-2], etc.
 * An easy way to convert between the two is one's complement, i.e., ~i.
 * See also _JC_REF_ELEMENT() macro.
 */
struct _jc_object_array {
	_jc_object		*elems[0];
	_jc_word		lockword;
	_jc_type		*type;
	const jint		length;
};

/************************************************************************
 *			Interpreted Method Info				*
 ************************************************************************/

struct _jc_iinc {
	jint		index;
	jint	  	value;
};

struct _jc_invoke {
	_jc_method	*method;
	_jc_uint16  	pop;
};

struct _jc_multianewarray {
	_jc_type	*type;
	unsigned char	dims;
};

struct _jc_lookup {
	jint		match;
	_jc_uint16	target;
};

struct _jc_lookupswitch {
	_jc_uint16	default_target;
	_jc_uint16	num_pairs;
	_jc_lookup	pairs[0];
};

struct _jc_tableswitch {
	jint		low;
	jint		high;
	_jc_uint16	default_target;
	_jc_uint16	targets[0];
};

struct _jc_interp_trap {
	_jc_uint16	start;
	_jc_uint16	end;
	_jc_uint16	target;
	_jc_type	*type;
};

struct _jc_linemap {
	_jc_uint16	index;
	_jc_uint16	line;
};

union _jc_insn_info {
	_jc_invoke		invoke;
	_jc_field		*field;
	_jc_iinc		iinc;
	_jc_multianewarray	multianewarray;
	_jc_type		*type;
	_jc_lookupswitch	*lookupswitch;
	_jc_tableswitch		*tableswitch;
	_jc_uint16		target;
	_jc_uint16		local;
	_jc_value		constant;
	const char		*utf8;
};

/************************************************************************
 *			Methods, Fields, Types, Etc.			*
 ************************************************************************/

/* Field descriptor */
struct _jc_field {
	const char		*name;
	const char		*signature;
	_jc_type		*class;
	_jc_type		*type;
	_jc_uint16		access_flags;
	int			offset;		/* in object or class_fields */
	void			*initial_value;	/* initialized static only */
};

/* Executable method info */
struct _jc_linenum_info {
	_jc_linenum		*table;
	_jc_uint16		len;
};

struct _jc_pc_map_info {
	_jc_pc_map		*map;
	_jc_uint16		len;
};

struct _jc_method_exec {
	union {
		_jc_linenum_info	linenum;    /* from generated code */
		_jc_pc_map_info		pc_map;	    /* computed at runtime */
	}			u;
	_jc_uint16		trap_table_len;
	_jc_trap_info		*trap_table;
	const void		*function_end;
	_jc_splay_node		node;		/* in vm->method_tree */
};

/* Interpreted method info */
struct _jc_method_code {
	unsigned char	*opcodes;
	_jc_insn_info	*info;
	_jc_interp_trap	*traps;
	_jc_linemap	*linemaps;
	_jc_uint16	max_stack;
	_jc_uint16	max_locals;
	_jc_uint16	num_traps;
	_jc_uint16	num_linemaps;
	_jc_uint16	num_insns;
	_jc_uint16	num_params2;
};

/* Method descriptor */
struct _jc_method {
	const char		*name;
	const char		*signature;
	_jc_type		*class;
	_jc_type		**param_types;
	_jc_type		*return_type;
	unsigned char		*param_ptypes;
	jlong			signature_hash;
	_jc_type		**exceptions;
	const void		*function;	/* code or interp trampoline */
	_jc_word		vtable_index;	/* index in vtable, mtable */
	_jc_uint16		access_flags;
	_jc_uint16		num_parameters;
	_jc_uint16		num_exceptions;
	const void		*native_function;
	union {
		_jc_method_exec		exec;		/* if executable */
		_jc_method_code		code;		/* if interpreted */
	}			u;
};

/* Type info specific to array types */
struct _jc_array_type {
	jint			dimensions;
	_jc_type		*base_type;
	_jc_type		*element_type;
};

/*
 * Type info specific to non-array types
 *
 * The field and method lists must be sorted by name, then signature.
 */
struct _jc_nonarray_type {
	jshort			block_size_index;	/* _JC_LIBJC_VERSION */
	_jc_uint16		num_vmethods;
	_jc_uint16		num_fields;
	_jc_uint16		num_methods;
	_jc_uint16		num_virtual_refs;
	_jc_uint16		num_inner_classes;
	_jc_uint32		instance_size;
	jlong			hash;
	const char		*source_file;
	_jc_field		**fields;	
	_jc_method		**methods;
	_jc_type		***instanceof_hash_table;
	void			*class_fields;
	_jc_inner_class		*inner_classes;
	_jc_type		*outer_class;
	int			num_class_depends;
	_jc_class_depend	*class_depends;

	/* These fields are filled in at run-time */
	union {
		_jc_elf		*elf;			/* if executable */
		_jc_classfile	*cfile;			/* if interpreted */
	}			u;
	_jc_env			*initializing_thread;
	_jc_super_info		*supers;
	_jc_method		**mtable;		/* "method vtable" */
};

/* Java type info */
struct _jc_type {
	const char		*name;
	_jc_type		*superclass;
	_jc_uint16		access_flags;
	_jc_uint16		flags;

	/* Interfaces info */
	_jc_uint16		num_interfaces;
	_jc_type		**interfaces;
	_jc_method		***imethod_hash_table;
	const void		**imethod_quick_table;

	/* Specific array/non-array info */
	union {
	    _jc_nonarray_type	nonarray;
	    _jc_array_type	array;
	}			u;

	/* These fields are filled in at run-time */
	_jc_class_loader	*loader;
	_jc_word		initial_lockword;
	_jc_object		*instance;	/* java.lang.Class instance */
	_jc_splay_node		node;		/* in loader->defined_types */

	/* Pointers to method functions */
	const void		*vtable[0];
};

/* Class dependency info */
struct _jc_class_depend {
	const char		*name;
	jlong			hash;
};

/* Inner class info */
struct _jc_inner_class {
	_jc_type		*type;
	_jc_uint16		access_flags;
};

/* Line number table entry */
struct _jc_linenum {
	_jc_uint32		cline;		/* c line number */
	_jc_uint16		jline;		/* java line number */
};

/* Trap table entry */
struct _jc_trap_info {
	_jc_type		*type;		/* (sub)class of Throwable */
	_jc_uint16		start;		/* starting trap region */
	_jc_uint16		end;		/* ending trap region */
};

/* Exception catcher */
struct _jc_catch_frame {
	_jc_catch_frame		*next;		/* next deeper catch frame */
	const _jc_method	*method;	/* method doing the catching */
	volatile _jc_uint16	region;		/* current method trap region */
	sigjmp_buf		context;	/* how to catch the exception */
};

/* Thread information made visible to generated code */
struct _jc_env_head {
	_jc_catch_frame		*catch_list;	/* exception traps */
	_jc_object		*pending;	/* posted exception */
	_jc_object		*caught;	/* caught exception */
};

/************************************************************************
 *				Macros					*
 ************************************************************************/

/*
 * Define the start of the line number table. We force it into the .data
 * section and build it one element at a time via _JC_LINE_NUMBER().
 * This macro must immediately preceed the function itself. The function
 * must not contain any static data that would go into the .data section.
 */
#define _JC_LINENUM_TABLE(class, method)				\
static _jc_linenum _jc_ ## class ## $linenum_table$ ## method[]		\
	__attribute__ ((section(".data"))) = { }

/*
 * Line number table entry. This allows us to map a C source line number
 * to a Java source line number for exception traces and to determine if
 * thrown exceptions are caught within the method's various trap ranges.
 * In turn, ELF debug sections allow us to map PC values to C line numbers.
 */
#define _JC_LINE_NUMBER(_jline)						\
    do {								\
	static volatile _jc_linenum _linenum				\
	    __attribute__ ((section(".data"))) = {			\
		.cline=		__LINE__,				\
		.jline=		_jline,					\
	};								\
	asm volatile ("" : : );						\
    } while (0)

/*
 * Trap table entry.
 */
#define _JC_TRAP(_type, _start, _end)					\
    {									\
    	.type=		(_type),					\
	.start=		(_start),					\
	.end=		(_end),						\
    }

/*
 * Set the current trap region.
 */
#define _JC_TRAP_REGION(catch, _region)					\
    do {								\
	asm volatile ("" : : );						\
	(catch).region = (_region);					\
	asm volatile ("" : : );						\
    } while (0)

/*
 * Macro to establish a return point for C functions catching exceptions.
 * The second parameter points to a local _jc_catch_frame structure. The
 * _JC_CANCEL_TRAPS() macro must be called before this function exits.
 *
 * Because the target list is 'const' it will go into the .rodata section
 * and so won't interfere with the line number table.
 */
#define _JC_DEFINE_TRAPS(env, catch, method_info, targets...)		\
    do {								\
	_jc_env_head *const _eh = (_jc_env_head *)(env);		\
	static const void *const _targets[]				\
	    /* __attribute__ ((section(".rodata"))) */			\
	    = { _JC_NULL, targets };					\
	int _target_index;						\
									\
	(catch).next = _eh->catch_list;					\
	_eh->catch_list = &(catch);					\
	(catch).method = (method_info);					\
	(catch).region = 0;						\
	if ((_target_index = sigsetjmp((catch).context, 0)) != 0) 	\
		goto *_targets[_target_index];				\
    } while (0)

/*
 * Methods that use _JC_DEFINE_TRAPS() must use this macro before
 * returning to the caller.
 */
#define _JC_CANCEL_TRAPS(env, catch)					\
    do {								\
	_jc_env_head *const _eh = (_jc_env_head *)(env);		\
									\
	_eh->catch_list = (catch).next;					\
    } while (0)

/*
 * Retrieve the last exception caught. The "last caught exception" is
 * not traced by GC and so must be copied into a safe place immediately
 * after the exception is caught.
 */
#define _JC_CAUGHT_EXCEPTION(env)					\
    ({									\
	_jc_env_head *const _eh = (_jc_env_head *)(env);		\
									\
	_eh->caught;							\
    })

/*
 * Explicit null pointer check. This is used in cases where we must check
 * for a null pointer but where invoking the method won't necessarily
 * cause a signal to be generated (the only case is invoke-nonvirtual).
 */
#define _JC_EXPLICIT_NULL_CHECK(env, obj)				\
    do {								\
	_jc_object *const _obj0 = (_jc_object *)(obj);			\
									\
	*((volatile char *)_obj0);					\
    } while (0)

/*
 * Stack overflow check.
 *
 * This is performed at the beginning of each method. We simply try
 * to reference a memory address further "up" the (unallocated) stack.
 * When the stack gets almost full, this reference will hit the guard
 * page and cause a fault which we trap with a signal handler.
 */
#if _JC_DOWNWARD_STACK
#define _JC_STACK_OVERFLOW_CHECK(env)					\
	(void)(((volatile char *)&(env))[-_JC_STACK_OVERFLOW_MARGIN])
#else
#define _JC_STACK_OVERFLOW_CHECK(env)					\
	(void)(((volatile char *)&(env))[_JC_STACK_OVERFLOW_MARGIN])
#endif

/*
 * Thread periodic check.
 *
 * When one thread needs to cause other threads to do a periodic check,
 * we unmap a page of memory referenced by all executing threads when
 * taking backward branches. This causes them to quickly receive a signal.
 */
#define _JC_PERIODIC_CHECK(env)						\
    do {								\
	extern volatile _jc_word _jc_check_address;			\
									\
	(void)(_jc_check_address);					\
    } while (0)

/*
 * Notify of an "active use" of a class or interface, which causes
 * the class or interface to be initialized if it hasn't been already.
 *
 * This is required before the first static field access, static
 * method invocation, or instance creation of a class.
 *
 * Warning: this macro is included in other macros and so must use
 * unique internal variable names.
 */
#define _JC_ACTIVE_USE(env, class)					\
    do {								\
	_jc_type *const _type0 = &_jc_ ## class ## $type.type;		\
									\
	if ((_type0->flags & _JC_TYPE_INITIALIZED) == 0)		\
		_jc_cs_initialize_type((env), _type0);			\
    } while (0)

/*
 * Invoke the virtual method 'meth' of class 'class' using object 'obj'.
 */
#define _JC_INVOKE_VIRTUAL(env, class, meth, obj, args...)		\
    ({									\
	_jc_ ## class ## $object *const _obj = (obj);			\
									\
	(*_obj->vtype->vtable.class.meth)((env), _obj , ## args);	\
    })

/*
 * Invoke the non-virtual method 'meth' of class 'class' using object 'obj'.
 * If 'nullchk' is non-zero, an explicit null pointer check will be done.
 */
#define _JC_INVOKE_NONVIRTUAL(env, class, meth, nullchk, obj, args...)	\
    ({									\
	_jc_env *const _env = (env);					\
	_jc_ ## class ## $object *const _obj				\
	    = (struct _jc_ ## class ## $object *)(obj);			\
									\
	if (nullchk)							\
		_JC_EXPLICIT_NULL_CHECK(_env, _obj);			\
	_jc_ ## class ## $method$ ## meth(_env, _obj , ## args);	\
    })

/*
 * Invoke a static method 'meth' of class 'class'. If 'omitchk' is
 * non-zero then no class initialization is performed.
 */
#define _JC_INVOKE_STATIC(env, class, meth, omitchk, args...)		\
    ({									\
	_jc_env *const _env = (env);					\
									\
	if (!(omitchk))							\
		_JC_ACTIVE_USE(_env, class);				\
	_jc_ ## class ## $method$ ## meth(_env , ## args);		\
    })

/*
 * Invoke an interface method with parameter types 'pdecl', returning
 * 'rtype', and having method signature hash 'sighash', on object 'obj'.
 *
 * First we try the "quick" lookup table, and if that fails, fall back
 * to searching the interface method hash table.
 */
#define _JC_INVOKE_INTERFACE(env, obj, hash, rtype, pdecl, args...)	\
    ({									\
	_jc_env *const _env = (env);					\
	const jlong _hash = (hash);					\
	_jc_object *const _obj = (_jc_object *)(obj);			\
	_jc_type *const _type = _obj->type;				\
	const jint _bucket = (jint)(_hash) & (_JC_IMETHOD_HASHSIZE - 1);\
	rtype (*_func) pdecl _JC_JCNI_ATTR;				\
									\
	if ((_func = _type->imethod_quick_table[_bucket]) == _JC_NULL)	\
		_func = _jc_cs_lookup_interface(_env, _obj, _hash);	\
	(*_func)(_env, _obj , ## args);					\
    })

/*
 * Access a static field 'field' of class 'class'. If 'omitchk' is
 * non-zero then no class initialization is performed.
 */
#define _JC_STATIC_FIELD(env, class, field, omitchk)			\
    (*({								\
	if (!(omitchk))							\
		_JC_ACTIVE_USE((env), class);				\
	&_jc_ ## class ## $class_fields.field;				\
    }))

/*
 * Access a reference instance field 'field' of class 'class' from
 * object 'obj'.
 */
#define _JC_REF_FIELD(env, obj, class, field)				\
    (*({								\
	_jc_ ## class ## $object *const _obj				\
	    = (struct _jc_ ## class ## $object *)(obj);			\
									\
	&_obj->refs[-1].class.field;					\
    }))

/*
 * Access a non-reference instance field 'field' of class 'class' from
 * object 'obj'.
 */
#define _JC_PRIM_FIELD(env, obj, class, field)				\
    (*({								\
	_jc_ ## class ## $object *const _obj				\
	    = (struct _jc_ ## class ## $object *)(obj);			\
									\
	&_obj->nonrefs.class.field;					\
    }))

/*
 * Return an intern'd String created from the supplied UTF-8 C string.
 * This macro is used for embedding String constants in the code.
 *
 * Note the static variable which constitutes an implicit object
 * reference within the code that must be accounted for during GC.
 *
 * We must put this reference in the .rodata section so it won't
 * interfere with the line number table. Since we don't actually load
 * the .rodata with read-only protection, this won't cause problems.
 */
#define _JC_STRING(env, utf8)						\
    ({									\
	static _jc_object *_obj __attribute__ ((section(".rodata")));	\
									\
	if (_obj == _JC_NULL)						\
		_obj = _jc_cs_intern_string_utf8((env), &_obj, (utf8));	\
	(struct _jc_java_lang_String$object *)_obj;			\
    })

/*
 * Compare two long values.
 */
#define _JC_LCMP(x, y)							\
    ({									\
	const jlong _x = (x);						\
	const jlong _y = (y);						\
									\
	(_x > _y) - (_x < _y);						\
    })

/*
 * Compare two floating point values, with result 'greater than' if
 * either value is NaN.
 */
#define _JC_FCMPG(x, y)							\
    ({									\
	const jfloat _x = (x);						\
	const jfloat _y = (y);						\
									\
	(_x != _x || _y != _y) ? 1 : (_x > _y) - (_x < _y);		\
    })
#define _JC_DCMPG(x, y)							\
    ({									\
	const jdouble _x = (x);						\
	const jdouble _y = (y);						\
									\
	(_x != _x || _y != _y) ? 1 : (_x > _y) - (_x < _y);		\
    })

/*
 * Compare two floating point values, with result 'less than' if
 * either value is NaN.
 */
#define _JC_FCMPL(x, y)							\
    ({									\
	const jfloat _x = (x);						\
	const jfloat _y = (y);						\
									\
	(_x != _x || _y != _y) ? -1 : (_x > _y) - (_x < _y);		\
    })
#define _JC_DCMPL(x, y)							\
    ({									\
	const jdouble _x = (x);						\
	const jdouble _y = (y);						\
									\
	(_x != _x || _y != _y) ? -1 : (_x > _y) - (_x < _y);		\
    })

/*
 * Zero-filled arithmetic right shift for 'int' and 'long'.
 */
#define _JC_IUSHR(x, y)							\
    ({									\
	const jint _x = (x);						\
	const jint _y = (y) & 0x1f;					\
									\
	(jint)(((_jc_uint32)_x) >> _y);					\
    })
#define _JC_LUSHR(x, y)							\
    ({									\
	const jlong _x = (x);						\
	const jlong _y = (y) & 0x3f;					\
									\
	(jlong)(((_jc_uint64)_x) >> _y);				\
    })

/*
 * Signed-extended arithmetic right shift for 'int' and 'long'.
 */
#if _JC_SIGNED_RIGHT_SHIFT
#define _JC_ISHR(x, y)							\
    ({									\
	const jint _x = (x);						\
	const jint _y = (y) & 0x1f;					\
									\
	_x >> _y;							\
    })
#define _JC_LSHR(x, y)							\
    ({									\
	const jlong _x = (x);						\
	const jlong _y = (y) & 0x3f;					\
									\
	_x >> _y;							\
    })
#else	/* !_JC_SIGNED_RIGHT_SHIFT */
#define _JC_ISHR(x, y)							\
    ({									\
	const jint _x = (x);						\
	const jint _y = (y) & 0x1f;					\
	jint _result;							\
									\
	_result = _x >> _y;						\
	if (_x < 0)							\
		_result |= ~0 << (32 - _y);				\
	_result;							\
    })
#define _JC_LSHR(x, y)							\
    ({									\
	const jlong _x = (x);						\
	const jlong _y = (y) & 0x3f;					\
	jlong _result;							\
									\
	_result = _x >> _y;						\
	if (_x < 0)							\
		_result |= ~0 << (64 - _y);				\
	_result;							\
    })
#endif	/* !_JC_SIGNED_RIGHT_SHIFT */

/*
 * Conversion from floating point -> integral types.
 */
#define _JC_CAST_FLT2INT(env, vtype, ctype, value)			\
    ({									\
	const vtype _val = (value);					\
	const ctype _min = (ctype)(_JC_JLONG(1)				\
				<< (sizeof(ctype) * 8 - 1));		\
	const ctype _max = ~_min;					\
									\
	_val >= _max ? _max :						\
	_val <= _min ? _min :						\
	_val != _val ? 0 :						\
	(ctype)_val;							\
    })

/*
 * Access an element in an array of type 'type', where 'type' is one of
 * 'boolean', 'byte', 'char', 'short', 'int', 'long', 'float', or 'double'.
 */
#define _JC_PRIM_ELEMENT(env, type, array, index, chklo, chkhi)		\
    (*({								\
	_jc_env *const _env = (env);					\
	_jc_ ## type ## _array *const _obj				\
	    = (struct _jc_ ## type ## _array *)(array);			\
	const jint _index = (index);					\
									\
	if ((chklo) && _index < 0)					\
		_jc_cs_throw_array_index_exception(_env, _index);	\
	if ((chkhi) && _index >= _obj->length)				\
		_jc_cs_throw_array_index_exception(_env, _index);	\
	&_obj->elems[index];						\
    }))

/*
 * Access an element in an array of objects. Note that the array grows
 * backwards in memory from the base of the object.
 */
#define _JC_REF_ELEMENT(env, array, index, chklo, chkhi)		\
    (*({								\
	_jc_env *const _env = (env);					\
	_jc_object_array *const _obj					\
	    = (struct _jc_object_array *)(array);			\
	const jint _index = (index);					\
									\
	if ((chklo) && _index < 0)					\
		_jc_cs_throw_array_index_exception(_env, _index);	\
	if ((chkhi) && _index >= _obj->length)				\
		_jc_cs_throw_array_index_exception(_env, _index);	\
	&_obj->elems[~(index)];						\
    }))

/*
 * Check type compatibility before storing to a reference array.
 * This will also cause a NullPointerException if 'array' is null.
 */
#define _JC_ARRAYSTORE_CHECK(env, ary, obj)				\
    do {								\
	_jc_env *const _env = (env);					\
	_jc_type *const _etype = (ary)->type->u.array.element_type;	\
	_jc_object *const _obj = (obj);					\
									\
	if (_obj != _JC_NULL && !_jc_cs_instanceof(_env, _obj, _etype))	\
		_jc_cs_throw_array_store_exception(_env, _obj, _etype);	\
    } while (0)

/*
 * Get the length of an array. This is a read-only value.
 */
#define _JC_ARRAY_LENGTH(env, array)					\
    ({									\
	_jc_array *const _obj = (_jc_array *)(array);			\
									\
	_obj->length;							\
    })

/*
 * Create a new object of type 'class'.
 */
#define _JC_NEW(env, class)						\
    ({									\
	_jc_cs_new_object(env, &_jc_ ## class ## $type.type);		\
    })

/*
 * Same as _JC_NEW() but for stack-allocated objects.
 */
#define _JC_STACK_NEW(env, mem, class)					\
    ({									\
	_jc_cs_init_object(env, mem, &_jc_ ## class ## $type.type);	\
    })

/*
 * Create a new N-dimensional array of 'type', where 'type' is one of
 * 'boolean', 'byte', 'char', 'short', 'int', 'long', 'float', or 'double',
 * and having length 'size', where N is 'dims'.
 *
 * Only the first array dimension is instantiated.
 *
 * Note: 'dims' must be a plain decimal integer constant with no
 * leading zeroes.
 */
#define _JC_NEW_PRIM_ARRAY(env, type, dims, size)			\
	((_jc_ ## type ## _array *)_jc_cs_new_array(env,		\
	    &_jc_ ## type ## $array ## dims ## $prim$type, size))

/*
 * Same as _JC_NEW_PRIM_ARRAY() but for stack-allocated arrays.
 */
#define _JC_STACK_NEW_PRIM_ARRAY(env, mem, type, dims, size)		\
    ({									\
	const jint _size = (size);					\
	    								\
	(_jc_ ## type ## _array *)_jc_cs_init_array(env,		\
	    mem, &_jc_ ## type ## $array ## dims ## $prim$type, _size);	\
    })

/*
 * Create a new N-dimensional array of objects of class 'class'
 * having length 'size', where N is 'dims'.
 *
 * Only the first array dimension is instantiated.
 *
 * Note: 'dims' must be a plain decimal integer constant with no
 * leading zeroes.
 */
#define _JC_NEW_REF_ARRAY(env, class, dims, size)			\
	((_jc_object_array *)_jc_cs_new_array(env,			\
	    &_jc_ ## class ## $array ## dims ## $type, size))

/*
 * Same as _JC_NEW_REF_ARRAY() but for stack-allocated arrays.
 */
#define _JC_STACK_NEW_REF_ARRAY(env, mem, class, dims, size)		\
    ({									\
	const jint _size = (size);					\
	    								\
	(_jc_object_array *)_jc_cs_init_array(env,			\
	    mem, &_jc_ ## class ## $array ## dims ## $type, _size);	\
    })

/*
 * Create a new multi-dimensional array of 'base_class' objects having
 * dimensions 'dims'. There must be 'dims' subsequent parameters to the
 * macro giving the sub-array sizes (or -1 for no subarray creation).
 * If -1 appears, all following sizes must also be -1.
 *
 * Note: 'dims' must be a plain decimal integer constant with no
 * leading zeroes.
 */
#define _JC_NEW_REF_MULTIARRAY(env, base_class, dims, nsizes, sizes...)	\
    ({									\
	const jint _sizes[] = { sizes };				\
									\
	_jc_cs_new_multiarray((env),					\
	    &_jc_ ## base_class ## $array ## dims ## $type,		\
	    (nsizes), _sizes);						\
    })

/*
 * Same as _JC_NEW_REF_MULTIARRAY() but for stack-allocated arrays.
 */
#define _JC_STACK_NEW_REF_MULTIARRAY(env, mem, base_class, dims, nsizes, sizes...)\
    ({									\
	const jint _sizes[] = { sizes };				\
									\
	_jc_cs_init_multiarray((env),					\
	    mem, &_jc_ ## base_class ## $array ## dims ## $type,	\
	    (nsizes), _sizes);						\
    })

/*
 * Create a new multi-dimensional array of 'type' having dimensions 'dims'
 * and lengths 'sizes', where 'type' is one of 'boolean', 'byte', 'char',
 * 'short', 'int', 'long', 'float', or 'double'.
 *
 * Note: 'dims' must be a plain decimal integer constant.
 */
#define _JC_NEW_PRIM_MULTIARRAY(env, type, dims, nsizes, sizes...)	\
    ({									\
	int _sizes[] = { sizes };					\
									\
	_jc_cs_new_multiarray((env),					\
	    &_jc_ ## type ## $array ## dims ## $prim$type,		\
	    (nsizes), _sizes);						\
    })

/*
 * Same as _JC_NEW_PRIM_MULTIARRAY() but for stack-allocated arrays.
 */
#define _JC_STACK_NEW_PRIM_MULTIARRAY(env, mem, type, dims, nsizes, sizes...)\
    ({									\
	int _sizes[] = { sizes };					\
									\
	_jc_cs_init_multiarray((env),					\
	    mem, &_jc_ ## type ## $array ## dims ## $prim$type,	\
	    (nsizes), _sizes);						\
    })

/*
 * Determine if the object 'obj' is an instance of 'type'.
 */
#define _JC_INSTANCEOF(env, obj, type)					\
	_jc_cs_instanceof((env), (_jc_object *)(obj), (type))

/*
 * Determine if the object 'obj' is an instance of 'type'
 * when 'type' is known to be a final class.
 */
#define _JC_INSTANCEOF_FINAL(env, obj, typ)				\
    ({									\
	_jc_object *const _obj = (obj);					\
	_jc_type *const _type = (typ);					\
									\
	_obj != _JC_NULL && _obj->type == _type;			\
    })

/*
 * Cast 'obj' to type 'type', which must not be primitive.
 */
#define _JC_CAST(env, type, obj)					\
    ({									\
	_jc_env *const _env = (env);					\
	_jc_object *const _obj = (obj);					\
	_jc_type *const _type = (type);					\
									\
	if (_obj != _JC_NULL && !_jc_cs_instanceof(_env, _obj, _type))	\
		_jc_cs_throw_class_cast_exception(_env, _obj, _type);	\
	_obj;								\
    })

/*
 * Optimization of _JC_CAST() where 'type' is a type with no sub-types.
 */
#define _JC_CAST_FINAL(env, typ, obj)					\
    ({									\
	_jc_env *const _env = (env);					\
	_jc_object *const _obj = (obj);					\
	_jc_type *const _type = (typ);					\
									\
	if (_obj != _JC_NULL && _obj->type != _type)			\
		_jc_cs_throw_class_cast_exception(_env, _obj, _type);	\
	_obj;								\
    })

/*
 * Resolve and invoke a native method.
 *
 * 'jvalp' points to a _jc_value into which the return value is copied.
 * This should only be used within the generated native method function.
 * 'params' should include the instance object iff method is not static.
 */
#define _JC_INVOKE_NATIVE_METHOD(env, minfo, jvalp, params...)	\
	_jc_cs_invoke_native_method((env), (minfo), (jvalp) , ## params)

/*
 * This macro "implements" abstract methods.
 * It causes an AbstractMethodError to be thrown.
 */
#define _JC_ABSTRACT_METHOD(env, minfo)					\
	_jc_cs_throw_abstract_method_error((env), (minfo))

/*
 * Enter/exit object monitor.
 */
#define _JC_MONITOR_ENTER(env, obj)					\
	_jc_cs_monitorenter((env), (_jc_object *)(obj))
#define _JC_MONITOR_EXIT(env, obj)					\
	_jc_cs_monitorexit((env), (_jc_object *)(obj))

/*
 * Throw an exception.
 */
#define _JC_THROW(env, obj)						\
	_jc_cs_throw((env), (_jc_object *)(obj))

/************************************************************************
 *				External data				*
 ************************************************************************/

/* Primitive classes */
extern _jc_type _jc_boolean$prim$type;
extern _jc_type _jc_byte$prim$type;
extern _jc_type _jc_char$prim$type;
extern _jc_type _jc_short$prim$type;
extern _jc_type _jc_int$prim$type;
extern _jc_type _jc_long$prim$type;
extern _jc_type _jc_float$prim$type;
extern _jc_type _jc_double$prim$type;
extern _jc_type _jc_void$prim$type;

/* Primitive array classes */
_JC_DECL_ARRAYS(boolean, prim$type);
_JC_DECL_ARRAYS(byte, prim$type);
_JC_DECL_ARRAYS(char, prim$type);
_JC_DECL_ARRAYS(short, prim$type);
_JC_DECL_ARRAYS(int, prim$type);
_JC_DECL_ARRAYS(long, prim$type);
_JC_DECL_ARRAYS(float, prim$type);
_JC_DECL_ARRAYS(double, prim$type);

/* C support functions */
extern void		_jc_cs_initialize_type(_jc_env *env, _jc_type *type)
				_JC_JCNI_ATTR;
extern jboolean		_jc_cs_instanceof(_jc_env *env, _jc_object *obj,
				_jc_type *type) _JC_JCNI_ATTR;
extern _jc_object	*_jc_cs_intern_string_utf8(_jc_env *env,
				_jc_object **refp, const char *utf8)
				_JC_JCNI_ATTR;
extern void		_jc_cs_invoke_native_method(_jc_env *env,
				_jc_method *minfo, _jc_value *retval, ...)
				_JC_JCNI_ATTR;
extern const void	*_jc_cs_lookup_interface(_jc_env *env, _jc_object *obj,
				jlong sig_hash) _JC_JCNI_ATTR;
extern void		_jc_cs_monitorenter(_jc_env *env, _jc_object *obj)
				_JC_JCNI_ATTR;
extern void		_jc_cs_monitorexit(_jc_env *env, _jc_object *obj)
				_JC_JCNI_ATTR;
extern _jc_array	*_jc_cs_new_array(_jc_env *env, _jc_type *type,
				jint size) _JC_JCNI_ATTR;
extern _jc_array	*_jc_cs_init_array(_jc_env *env, void *mem,
				_jc_type *type, jint size) _JC_JCNI_ATTR;
extern _jc_array	*_jc_cs_new_multiarray(_jc_env *env, _jc_type *type,
				jint num_sizes, const jint *sizes)
				_JC_JCNI_ATTR;
extern _jc_array	*_jc_cs_init_multiarray(_jc_env *env, void *mem,
				_jc_type *type, jint num_sizes,
				const jint *sizes) _JC_JCNI_ATTR;
extern _jc_object	*_jc_cs_new_object(_jc_env *env, _jc_type *type)
				_JC_JCNI_ATTR;
extern _jc_object	*_jc_cs_init_object(_jc_env *env, void *mem,
				_jc_type *type) _JC_JCNI_ATTR;
extern void		_jc_cs_panic(_jc_env *env, const char *fmt, ...)
				__attribute__ ((noreturn)) _JC_JCNI_ATTR;
extern void		_jc_cs_throw(_jc_env *env, _jc_object *obj)
				__attribute__ ((noreturn)) _JC_JCNI_ATTR;
extern void		_jc_cs_throw_abstract_method_error(_jc_env *env,
				_jc_method *minfo) __attribute__ ((noreturn))
				_JC_JCNI_ATTR;
extern void		_jc_cs_throw_array_index_exception(_jc_env *env,
				jint indx) __attribute__ ((noreturn))
				_JC_JCNI_ATTR;
extern void		_jc_cs_throw_array_store_exception(_jc_env *env,
				_jc_object *obj, _jc_type *type)
				__attribute__ ((noreturn)) _JC_JCNI_ATTR;
extern void		_jc_cs_throw_class_cast_exception(_jc_env *env,
				_jc_object *obj, _jc_type *type)
				__attribute__ ((noreturn)) _JC_JCNI_ATTR;
extern void		_jc_cs_throw_null_pointer_exception(_jc_env *env)
				__attribute__ ((noreturn)) _JC_JCNI_ATTR;
extern jdouble		_jc_cs_fmod(jdouble x, jdouble y) _JC_JCNI_ATTR;

/* Empty interface method lookup tables */
extern const		void *_jc_empty_quick_table[_JC_IMETHOD_HASHSIZE];
extern _jc_method	**_jc_empty_imethod_table[_JC_IMETHOD_HASHSIZE];

#endif	/* _JC_DEFS_H_ */



/*
 * @COPYRIGHT@
 *
 * $Id: interp.c,v 1.8 2005/12/11 19:26:47 archiecobbs Exp $
 */

#include "libjc.h"

/* Internal functions */
static jint	_jc_interp(_jc_env *env, _jc_method *const method,
			_jc_object *this, _jc_word *args);
static int	_jc_lookup_compare(const void *v1, const void *v2);
static void	_jc_vinterp(_jc_env *env, va_list args);
static void	_jc_vinterp_native(_jc_env *env, va_list args);

/*
 * Macros used in _jc_interp()
 */
#ifndef NDEBUG

#define JUMP(_pc)							\
    do {								\
	pc = (_pc);							\
	_JC_ASSERT(sp >= state.locals + code->max_locals);		\
	_JC_ASSERT(sp <= state.locals					\
	    + code->max_locals + code->max_stack);			\
	_JC_ASSERT(pc >= 0 && pc < code->num_insns);			\
	_JC_ASSERT(actions[code->opcodes[pc]] != NULL);			\
	_JC_ASSERT(ticker > 0);						\
	if (--ticker == 0)						\
		goto periodic_check;					\
	goto *actions[code->opcodes[pc]];				\
    } while (0)

#define NEXT()		JUMP(pc + 1)

#else	/* !NDEBUG */

#define JUMP(_pc)							\
    do {								\
	pc = (_pc);							\
	if (--ticker == 0)						\
		goto periodic_check;					\
	goto *actions[code->opcodes[pc]];				\
    } while (0)

#define NEXT()								\
    do {								\
	goto *actions[code->opcodes[++pc]];				\
    } while (0)

#endif	/* !NDEBUG */

#define STACKI(i)	(*(jint *)(sp + (i)))
#define STACKF(i)	(*(jfloat *)(sp + (i)))
#define STACKJ(i)	(*(jlong *)(sp + (i)))
#define STACKD(i)	(*(jdouble *)(sp + (i)))
#define STACKL(i)	(*(_jc_object **)(sp + (i)))
#define LOCALI(i)	(*(jint *)(state.locals + i))
#define LOCALF(i)	(*(jfloat *)(state.locals + i))
#define LOCALJ(i)	(*(jlong *)(state.locals + i))
#define LOCALD(i)	(*(jdouble *)(state.locals + i))
#define LOCALL(i)	(*(_jc_object **)(state.locals + i))
#define PUSHI(v)	do { STACKI(0) = (v); sp++; } while (0)
#define PUSHF(v)	do { STACKF(0) = (v); sp++; } while (0)
#define PUSHJ(v)	do { STACKJ(0) = (v); sp += 2; } while (0)
#define PUSHD(v)	do { STACKD(0) = (v); sp += 2; } while (0)
#define PUSHL(v)	do { STACKL(0) = (v); sp++; } while (0)
#define POP(i)		(sp -= (i))
#define POP2(i)		(sp -= 2 * (i))

#define INFO(f)		(code->info[pc].f)

#define ARRAYCHECK(array, i)						\
    do {								\
	_jc_array *const _array = (_jc_array *)(array);			\
	jint _i = (i);							\
									\
	if (_array == NULL)						\
		goto null_pointer_exception;				\
	if (_i < 0 || _i >= _array->length) {				\
		_jc_post_exception_msg(env,				\
		    _JC_ArrayIndexOutOfBoundsException, "%d", _i);	\
		goto exception;						\
	}								\
    } while (0)
#define _JC_INTERP_PRIM_ELEM(type, array, i)				\
	(((_jc_ ## type ## _array *)(array))->elems[(i)])
#define _JC_INTERP_REF_ELEM(array, i)					\
	(((_jc_object_array *)(array))->elems[~(i)])

#define PERIODIC_CHECK_TICKS	32

/*
 * Java interpreter. The "args" must contain two elements for long/double.
 *
 * If successful, return value is stored in env->retval and JNI_OK returned.
 * Otherwise, an exception is posted and JNI_ERR is returned.
 */
static jint
_jc_interp(_jc_env *const env, _jc_method *const method,
	_jc_object *const this, _jc_word *args)
{
#define ACTION(name)  [_JC_ ## name]= &&do_ ## name
	static const void *const actions[0x100] = {
		ACTION(aaload),
		ACTION(aastore),
		ACTION(aload),
		ACTION(anewarray),
		ACTION(areturn),
		ACTION(arraylength),
		ACTION(astore),
		ACTION(athrow),
		ACTION(baload),
		ACTION(bastore),
		ACTION(caload),
		ACTION(castore),
		ACTION(checkcast),
		ACTION(d2f),
		ACTION(d2i),
		ACTION(d2l),
		ACTION(dadd),
		ACTION(daload),
		ACTION(dastore),
		ACTION(dcmpg),
		ACTION(dcmpl),
		ACTION(ddiv),
		ACTION(dload),
		ACTION(dmul),
		ACTION(dneg),
		ACTION(drem),
		ACTION(dreturn),
		ACTION(dstore),
		ACTION(dsub),
		ACTION(dup),
		ACTION(dup_x1),
		ACTION(dup_x2),
		ACTION(dup2),
		ACTION(dup2_x1),
		ACTION(dup2_x2),
		ACTION(f2d),
		ACTION(f2i),
		ACTION(f2l),
		ACTION(fadd),
		ACTION(faload),
		ACTION(fastore),
		ACTION(fcmpg),
		ACTION(fcmpl),
		ACTION(fdiv),
		ACTION(fload),
		ACTION(fmul),
		ACTION(fneg),
		ACTION(frem),
		ACTION(freturn),
		ACTION(fstore),
		ACTION(fsub),
		ACTION(getfield),
		ACTION(getstatic),
		ACTION(goto),
		ACTION(i2b),
		ACTION(i2c),
		ACTION(i2d),
		ACTION(i2f),
		ACTION(i2l),
		ACTION(i2s),
		ACTION(iadd),
		ACTION(iaload),
		ACTION(iand),
		ACTION(iastore),
		ACTION(idiv),
		ACTION(if_acmpeq),
		ACTION(if_acmpne),
		ACTION(if_icmpeq),
		ACTION(if_icmpne),
		ACTION(if_icmplt),
		ACTION(if_icmpge),
		ACTION(if_icmpgt),
		ACTION(if_icmple),
		ACTION(ifeq),
		ACTION(ifne),
		ACTION(iflt),
		ACTION(ifge),
		ACTION(ifgt),
		ACTION(ifle),
		ACTION(ifnonnull),
		ACTION(ifnull),
		ACTION(iinc),
		ACTION(iload),
		ACTION(imul),
		ACTION(ineg),
		ACTION(instanceof),
		ACTION(invokeinterface),
		ACTION(invokespecial),
		ACTION(invokestatic),
		ACTION(invokevirtual),
		ACTION(ior),
		ACTION(irem),
		ACTION(ireturn),
		ACTION(ishl),
		ACTION(ishr),
		ACTION(istore),
		ACTION(isub),
		ACTION(iushr),
		ACTION(ixor),
		ACTION(jsr),
		ACTION(l2d),
		ACTION(l2f),
		ACTION(l2i),
		ACTION(ladd),
		ACTION(laload),
		ACTION(land),
		ACTION(lastore),
		ACTION(lcmp),
		ACTION(ldc),
		ACTION(ldc_string),
		ACTION(ldc2_w),
		ACTION(ldiv),
		ACTION(lload),
		ACTION(lmul),
		ACTION(lneg),
		ACTION(lookupswitch),
		ACTION(lor),
		ACTION(lrem),
		ACTION(lreturn),
		ACTION(lshl),
		ACTION(lshr),
		ACTION(lstore),
		ACTION(lsub),
		ACTION(lushr),
		ACTION(lxor),
		ACTION(monitorenter),
		ACTION(monitorexit),
		ACTION(multianewarray),
		ACTION(new),
		ACTION(newarray),
		ACTION(nop),
		ACTION(pop),
		ACTION(pop2),
		ACTION(putfield),
		ACTION(putstatic),
		ACTION(ret),
		ACTION(return),
		ACTION(saload),
		ACTION(sastore),
		ACTION(swap),
		ACTION(tableswitch),
	};
	_jc_method_code *const code = &method->u.code;
	int ticker = PERIODIC_CHECK_TICKS;
	_jc_interp_stack state;
	_jc_object *lock = NULL;
	_jc_word *sp;
	int pc = 0;

	/* Stack overflow check */
#if _JC_DOWNWARD_STACK
	if ((char *)&env < env->stack_limit)
#else
	if ((char *)&env > env->stack_limit)
#endif
	{
		_jc_post_exception(env, _JC_StackOverflowError);
		return JNI_ERR;
	}

	/* Is method abstract? */
	if (_JC_ACC_TEST(method, ABSTRACT)) {
		_jc_post_exception_msg(env, _JC_AbstractMethodError,
		    "%s.%s%s", method->class->name, method->name,
		    method->signature);
		return JNI_ERR;
	}

	/* Sanity check */
	_JC_ASSERT((this == NULL) == _JC_ACC_TEST(method, STATIC));
	_JC_ASSERT(!_JC_ACC_TEST(method->class, INTERFACE)
	    || strcmp(method->name, "<clinit>") == 0);
	_JC_ASSERT(_JC_FLG_TEST(method->class, RESOLVED));
	_JC_ASSERT(_JC_ACC_TEST(method, INTERP));
	_JC_ASSERT(!_JC_ACC_TEST(method, NATIVE));

	/* Push Java stack frame */
	memset(&state, 0, sizeof(state));
	state.jstack.interp = JNI_TRUE;
	state.jstack.next = env->java_stack;
	state.method = method;
	state.pcp = &pc;
	env->java_stack = &state.jstack;

	/* Allocate combined space for locals and stack */
	if ((state.locals = _JC_STACK_ALLOC(env,
	    (code->max_locals + code->max_stack)
	      * sizeof(*state.locals))) == NULL) {
		_jc_post_exception_info(env);
		goto exception;
	}
	sp = &state.locals[code->max_locals];

	/* Sanity check */
	_JC_ASSERT(code->opcodes != NULL);
	_JC_ASSERT(code->num_insns > 0);

	/* Copy method parameters into local variables */
	if (!_JC_ACC_TEST(method, STATIC)) {
		state.locals[0] = (_jc_word)this;
		memcpy(state.locals + 1, args,
		    code->num_params2 * sizeof(*args));
	} else
		memcpy(state.locals, args, code->num_params2 * sizeof(*args));

	/* Synchronize */
	if (_JC_ACC_TEST(method, SYNCHRONIZED)) {
		lock = _JC_ACC_TEST(method, STATIC) ?
		    method->class->instance : this;
		if (_jc_lock_object(env, lock) != JNI_OK) {
			lock = NULL;
			goto exception;
		}
	}

	/* Start */
	JUMP(0);

do_aaload:
    {
	_jc_object_array *array;
	jint index;

	POP(2);
	array = (_jc_object_array *)STACKL(0);
	index = STACKI(1);
	ARRAYCHECK(array, index);
	_JC_ASSERT(_JC_LW_TEST(array->lockword, ARRAY)
	    && _JC_LW_EXTRACT(array->lockword, TYPE) == _JC_TYPE_REFERENCE);
	PUSHL(array->elems[~index]);
	NEXT();
    }
do_aastore:
    {
	_jc_object_array *array;
	_jc_object *obj;
	jint index;

	POP(3);
	array = (_jc_object_array *)STACKL(0);
	index = STACKI(1);
	ARRAYCHECK(array, index);
	obj = STACKL(2);
	if (obj != NULL) {
		switch (_jc_assignable_from(env, obj->type,
		    array->type->u.array.element_type)) {
		case 1:
			break;
		case 0:
			_jc_post_exception_msg(env, _JC_ArrayStoreException,
			    "can't store object of type `%s' into array"
			    " of `%s'", obj->type->name,
			    array->type->u.array.element_type->name);
			/* FALLTHROUGH */
		case -1:
			goto exception;
		}
	}
	array->elems[~index] = obj;
	NEXT();
    }
do_aload:
	PUSHL(LOCALL(INFO(local)));
	NEXT();
do_anewarray:
    {
	_jc_array *array;
	jint length;

	POP(1);
	length = STACKI(0);
	if ((array = _jc_new_array(env, INFO(type), length)) == NULL)
		goto exception;
	PUSHL((_jc_object *)array);
	NEXT();
    }
do_areturn:
	POP(1);
	env->retval.l = STACKL(0);
	goto done;
do_arraylength:
    {
	_jc_array *array;

	POP(1);
	array = (_jc_array *)STACKL(0);
	if (array == NULL)
		goto null_pointer_exception;
	PUSHI(array->length);
	NEXT();
    }
do_astore:
	POP(1);
	LOCALL(INFO(local)) = STACKL(0);
	NEXT();
do_athrow:
	POP(1);
	if (STACKL(0) == NULL)
		goto null_pointer_exception;
	_jc_post_exception_object(env, STACKL(0));
	goto exception;
do_baload:
    {
	_jc_array *array;
	jint index;
	int ptype;

	POP(2);
	array = (_jc_array *)STACKL(0);
	index = STACKI(1);
	ARRAYCHECK(array, index);
	ptype = _JC_LW_EXTRACT(array->lockword, TYPE);
	_JC_ASSERT(ptype == _JC_TYPE_BOOLEAN || ptype == _JC_TYPE_BYTE);
	if (ptype == _JC_TYPE_BOOLEAN)
		PUSHI(((_jc_boolean_array *)array)->elems[index]);
	else
		PUSHI(((_jc_byte_array *)array)->elems[index]);
	NEXT();
    }
do_bastore:
    {
	_jc_array *array;
	jint index;
	int ptype;

	POP(3);
	array = (_jc_array *)STACKL(0);
	index = STACKI(1);
	ARRAYCHECK(array, index);
	ptype = _JC_LW_EXTRACT(array->lockword, TYPE);
	_JC_ASSERT(ptype == _JC_TYPE_BOOLEAN || ptype == _JC_TYPE_BYTE);
	if (ptype == _JC_TYPE_BOOLEAN)
		((_jc_boolean_array *)array)->elems[index] = STACKI(2) & 0x1;
	else
		((_jc_byte_array *)array)->elems[index] = STACKI(2) & 0xff;
	NEXT();
    }
do_caload:
    {
	_jc_char_array *array;
	jint index;

	POP(2);
	array = (_jc_char_array *)STACKL(0);
	index = STACKI(1);
	ARRAYCHECK(array, index);
	PUSHI(array->elems[index]);
	NEXT();
    }
do_castore:
    {
	_jc_char_array *array;
	jint index;

	POP(3);
	array = (_jc_char_array *)STACKL(0);
	index = STACKI(1);
	ARRAYCHECK(array, index);
	array->elems[index] = STACKI(2) & 0xffff;
	NEXT();
    }
do_checkcast:
    {
	_jc_object *const obj = STACKL(-1);

	if (obj != NULL) {
		_jc_type *const type = INFO(type);

		switch (_jc_instance_of(env, obj, type)) {
		case 1:
			break;
		case 0:
			_jc_post_exception_msg(env, _JC_ClassCastException,
			    "can't cast `%s' to `%s'", obj->type->name,
			    type->name);
			/* FALLTHROUGH */
		case -1:
			goto exception;
		default:
			_JC_ASSERT(JNI_FALSE);
		}
	}
	NEXT();
    }
do_d2f:
	POP2(1);
	PUSHF(STACKD(0));
	NEXT();
do_d2i:
	POP2(1);
	PUSHI(_JC_CAST_FLT2INT(env, jdouble, jint, STACKD(0)));
	NEXT();
do_d2l:
	POP2(1);
	PUSHJ(_JC_CAST_FLT2INT(env, jdouble, jlong, STACKD(0)));
	NEXT();
do_dadd:
	POP2(2);
	PUSHD(STACKD(0) + STACKD(2));
	NEXT();
do_daload:
    {
	_jc_double_array *array;
	jint index;

	POP(2);
	array = (_jc_double_array *)STACKL(0);
	index = STACKI(1);
	ARRAYCHECK(array, index);
	PUSHD(array->elems[index]);
	NEXT();
    }
do_dastore:
    {
	_jc_double_array *array;
	jint index;

	POP(4);
	array = (_jc_double_array *)STACKL(0);
	index = STACKI(1);
	ARRAYCHECK(array, index);
	array->elems[index] = STACKD(2);
	NEXT();
    }
do_dcmpg:
	POP2(2);
	PUSHI(_JC_DCMPG(STACKD(0), STACKD(2)));
	NEXT();
do_dcmpl:
	POP2(2);
	PUSHI(_JC_DCMPL(STACKD(0), STACKD(2)));
	NEXT();
do_ddiv:
	POP2(2);
	PUSHD(STACKD(0) / STACKD(2));
	NEXT();
do_dload:
	PUSHD(LOCALD(INFO(local)));
	NEXT();
do_dmul:
	POP2(2);
	PUSHD(STACKD(0) * STACKD(2));
	NEXT();
do_dneg:
	POP2(1);
	PUSHD(-STACKD(0));
	NEXT();
do_drem:
	POP2(2);
	PUSHD(fmod(STACKD(0), STACKD(2)));
	NEXT();
do_dreturn:
	POP2(1);
	env->retval.d = STACKD(0);
	goto done;
do_dstore:
	POP2(1);
	LOCALD(INFO(local)) = STACKD(0);
	NEXT();
do_dsub:
	POP2(2);
	PUSHD(STACKD(0) - STACKD(2));
	NEXT();
do_dup:
	STACKI(0) = STACKI(-1);
	POP(-1);
	NEXT();
do_dup_x1:
	STACKI(0) = STACKI(-1);
	STACKI(-1) = STACKI(-2);
	STACKI(-2) = STACKI(0);
	POP(-1);
	NEXT();
do_dup_x2:
	STACKI(0) = STACKI(-1);
	STACKI(-1) = STACKI(-2);
	STACKI(-2) = STACKI(-3);
	STACKI(-3) = STACKI(0);
	POP(-1);
	NEXT();
do_dup2:
	STACKI(1) = STACKI(-1);
	STACKI(0) = STACKI(-2);
	POP(-2);
	NEXT();
do_dup2_x1:
	STACKI(1) = STACKI(-1);
	STACKI(0) = STACKI(-2);
	STACKI(-1) = STACKI(-3);
	STACKI(-2) = STACKI(1);
	STACKI(-3) = STACKI(0);
	POP(-2);
	NEXT();
do_dup2_x2:
	STACKI(1) = STACKI(-1);
	STACKI(0) = STACKI(-2);
	STACKI(-1) = STACKI(-3);
	STACKI(-2) = STACKI(-4);
	STACKI(-3) = STACKI(1);
	STACKI(-4) = STACKI(0);
	POP(-2);
	NEXT();
do_f2d:
	POP(1);
	PUSHD(STACKF(0));
	NEXT();
do_f2i:
	POP(1);
	PUSHI(_JC_CAST_FLT2INT(env, jfloat, jint, STACKF(0)));
	NEXT();
do_f2l:
	POP(1);
	PUSHJ(_JC_CAST_FLT2INT(env, jfloat, jlong, STACKF(0)));
	NEXT();
do_fadd:
	POP(2);
	PUSHF(STACKF(0) + STACKF(1));
	NEXT();
do_faload:
    {
	_jc_float_array *array;
	jint index;

	POP(2);
	array = (_jc_float_array *)STACKL(0);
	index = STACKI(1);
	ARRAYCHECK(array, index);
	PUSHF(array->elems[index]);
	NEXT();
    }
do_fastore:
    {
	_jc_float_array *array;
	jint index;

	POP(3);
	array = (_jc_float_array *)STACKL(0);
	index = STACKI(1);
	ARRAYCHECK(array, index);
	array->elems[index] = STACKF(2);
	NEXT();
    }
do_fcmpg:
	POP(2);
	PUSHI(_JC_FCMPG(STACKF(0), STACKF(1)));
	NEXT();
do_fcmpl:
	POP(2);
	PUSHI(_JC_FCMPL(STACKF(0), STACKF(1)));
	NEXT();
do_fdiv:
	POP(2);
	PUSHF(STACKF(0) / STACKF(1));
	NEXT();
do_fload:
	PUSHF(LOCALF(INFO(local)));
	NEXT();
do_fmul:
	POP(2);
	PUSHF(STACKF(0) * STACKF(1));
	NEXT();
do_fneg:
	POP(1);
	PUSHF(-STACKF(0));
	NEXT();
do_frem:
	POP(2);
	PUSHF(fmod(STACKF(0), STACKF(1)));
	NEXT();
do_freturn:
	POP(1);
	env->retval.f = STACKF(0);
	goto done;
do_fstore:
	POP(1);
	LOCALF(INFO(local)) = STACKF(0);
	NEXT();
do_fsub:
	POP(2);
	PUSHF(STACKF(0) - STACKF(1));
	NEXT();
do_getfield:
    {
	_jc_field *const field = INFO(field);
	_jc_object *obj;
	void *data;

	POP(1);
	obj = STACKL(0);

	if (obj == NULL)
		goto null_pointer_exception;
	data = (char *)obj + field->offset;
	switch (field->type->flags & _JC_TYPE_MASK) {
	case _JC_TYPE_BOOLEAN:
		PUSHI(*(jboolean *)data);
		break;
	case _JC_TYPE_BYTE:
		PUSHI(*(jbyte *)data);
		break;
	case _JC_TYPE_CHAR:
		PUSHI(*(jchar *)data);
		break;
	case _JC_TYPE_SHORT:
		PUSHI(*(jshort *)data);
		break;
	case _JC_TYPE_INT:
		PUSHI(*(jint *)data);
		break;
	case _JC_TYPE_FLOAT:
		PUSHF(*(jfloat *)data);
		break;
	case _JC_TYPE_LONG:
		PUSHJ(*(jlong *)data);
		break;
	case _JC_TYPE_DOUBLE:
		PUSHD(*(jdouble *)data);
		break;
	case _JC_TYPE_REFERENCE:
		PUSHL(*(_jc_object **)data);
		break;
	default:
		_JC_ASSERT(JNI_FALSE);
		break;
	}
	NEXT();
    }
do_getstatic:
    {
	_jc_field *const field = INFO(field);
	void *data;

	/* Initialize field's class */
	if (!_JC_FLG_TEST(field->class, INITIALIZED)) {
		if (_jc_initialize_type(env, field->class) != JNI_OK)
			goto exception;
	}

	/* Get field */
	data = (char *)field->class->u.nonarray.class_fields + field->offset;
	switch (field->type->flags & _JC_TYPE_MASK) {
	case _JC_TYPE_BOOLEAN:
		PUSHI(*(jboolean *)data);
		break;
	case _JC_TYPE_BYTE:
		PUSHI(*(jbyte *)data);
		break;
	case _JC_TYPE_CHAR:
		PUSHI(*(jchar *)data);
		break;
	case _JC_TYPE_SHORT:
		PUSHI(*(jshort *)data);
		break;
	case _JC_TYPE_INT:
		PUSHI(*(jint *)data);
		break;
	case _JC_TYPE_FLOAT:
		PUSHF(*(jfloat *)data);
		break;
	case _JC_TYPE_LONG:
		PUSHJ(*(jlong *)data);
		break;
	case _JC_TYPE_DOUBLE:
		PUSHD(*(jdouble *)data);
		break;
	case _JC_TYPE_REFERENCE:
		PUSHL(*(_jc_object **)data);
		break;
	default:
		_JC_ASSERT(JNI_FALSE);
		break;
	}
	NEXT();
    }
do_goto:
	JUMP(INFO(target));
do_i2b:
	POP(1);
	PUSHI((jbyte)STACKI(0));
	NEXT();
do_i2c:
	POP(1);
	PUSHI((jchar)STACKI(0));
	NEXT();
do_i2d:
	POP(1);
	PUSHD(STACKI(0));
	NEXT();
do_i2f:
	POP(1);
	PUSHF(STACKI(0));
	NEXT();
do_i2l:
	POP(1);
	PUSHJ(STACKI(0));
	NEXT();
do_i2s:
	POP(1);
	PUSHI((jshort)STACKI(0));
	NEXT();
do_iadd:
	POP(2);
	PUSHI(STACKI(0) + STACKI(1));
	NEXT();
do_iaload:
    {
	_jc_int_array *array;
	jint index;

	POP(2);
	array = (_jc_int_array *)STACKL(0);
	index = STACKI(1);
	ARRAYCHECK(array, index);
	PUSHI(array->elems[index]);
	NEXT();
    }
do_iand:
	POP(2);
	PUSHI(STACKI(0) & STACKI(1));
	NEXT();
do_iastore:
    {
	_jc_int_array *array;
	jint index;

	POP(3);
	array = (_jc_int_array *)STACKL(0);
	index = STACKI(1);
	ARRAYCHECK(array, index);
	array->elems[index] = STACKI(2);
	NEXT();
    }
do_idiv:
	POP(2);
	if (STACKI(1) == 0)
		goto arithmetic_exception;
	PUSHI(STACKI(0) / STACKI(1));
	NEXT();
do_if_acmpeq:
	POP(2);
	JUMP(STACKL(0) == STACKL(1) ? INFO(target) : pc + 1);
do_if_acmpne:
	POP(2);
	JUMP(STACKL(0) != STACKL(1) ? INFO(target) : pc + 1);
do_if_icmpeq:
	POP(2);
	JUMP(STACKI(0) == STACKI(1) ? INFO(target) : pc + 1);
do_if_icmpne:
	POP(2);
	JUMP(STACKI(0) != STACKI(1) ? INFO(target) : pc + 1);
do_if_icmplt:
	POP(2);
	JUMP(STACKI(0) < STACKI(1) ? INFO(target) : pc + 1);
do_if_icmpge:
	POP(2);
	JUMP(STACKI(0) >= STACKI(1) ? INFO(target) : pc + 1);
do_if_icmpgt:
	POP(2);
	JUMP(STACKI(0) > STACKI(1) ? INFO(target) : pc + 1);
do_if_icmple:
	POP(2);
	JUMP(STACKI(0) <= STACKI(1) ? INFO(target) : pc + 1);
do_ifeq:
	POP(1);
	JUMP(STACKI(0) == 0 ? INFO(target) : pc + 1);
do_ifne:
	POP(1);
	JUMP(STACKI(0) != 0 ? INFO(target) : pc + 1);
do_iflt:
	POP(1);
	JUMP(STACKI(0) < 0 ? INFO(target) : pc + 1);
do_ifge:
	POP(1);
	JUMP(STACKI(0) >= 0 ? INFO(target) : pc + 1);
do_ifgt:
	POP(1);
	JUMP(STACKI(0) > 0 ? INFO(target) : pc + 1);
do_ifle:
	POP(1);
	JUMP(STACKI(0) <= 0 ? INFO(target) : pc + 1);
do_ifnonnull:
	POP(1);
	JUMP(STACKL(0) != NULL ? INFO(target) : pc + 1);
do_ifnull:
	POP(1);
	JUMP(STACKL(0) == NULL ? INFO(target) : pc + 1);
do_iinc:
	LOCALI(INFO(iinc).index) += INFO(iinc).value;
	NEXT();
do_iload:
	PUSHI(LOCALI(INFO(local)));
	NEXT();
do_imul:
	POP(2);
	PUSHI(STACKI(0) * STACKI(1));
	NEXT();
do_ineg:
	POP(1);
	PUSHI(-STACKI(0));
	NEXT();
do_instanceof:
	POP(1);
	switch (_jc_instance_of(env, STACKL(0), INFO(type))) {
	case 1:
		PUSHI(1);
		break;
	case 0:
		PUSHI(0);
		break;
	case -1:
		goto exception;
	default:
		_JC_ASSERT(JNI_FALSE);
	}
	NEXT();
do_invokespecial:
do_invokevirtual:
do_invokestatic:
do_invokeinterface:
    {
	const _jc_invoke *const invoke = &INFO(invoke);
	_jc_method *imethod = invoke->method;
	_jc_word *params;
	_jc_object *obj;
	jint status;

	/* Pop the stack and check for null */
	POP(invoke->pop);
	if (code->opcodes[pc] == _JC_invokestatic)
		obj = NULL;
	else {
		if ((obj = STACKL(0)) == NULL)
			goto null_pointer_exception;
	}

	/* Sanity check */
	_JC_ASSERT((code->opcodes[pc] == _JC_invokeinterface)
	    == _JC_ACC_TEST(imethod->class, INTERFACE));

	/* Do method lookup */
	switch (code->opcodes[pc]) {
	case _JC_invokeinterface:
	    {
		const jlong sig_hash = imethod->signature_hash;
		_jc_method *const *methodp;
		int bucket;

		/* Sanity check */
		_JC_ASSERT(_JC_ACC_TEST(imethod->class, INTERFACE));

		/* Verify object implements the interface */
		switch (_jc_instance_of(env, obj, imethod->class)) {
		case 0:
			_jc_post_exception_msg(env,
			    _JC_IncompatibleClassChangeError,
			    "`%s' does not implement interface `%s'",
			    obj->type->name, imethod->class->name);
			goto exception;
		case 1:
			break;
		case -1:
			goto exception;
		}

		/* Lookup interface method entry point */
		_JC_ASSERT(obj->type->imethod_quick_table != NULL);
		_JC_ASSERT(obj->type->imethod_hash_table != NULL);
		bucket = (int)sig_hash & (_JC_IMETHOD_HASHSIZE - 1);
		methodp = obj->type->imethod_hash_table[bucket];
		if (methodp == NULL)
			goto not_found;
		while (*methodp != NULL) {
			if ((*methodp)->signature_hash == sig_hash) {
				imethod = *methodp;
				break;
			}
			methodp++;
		}
		if (*methodp == NULL) {
not_found:		_jc_post_exception_msg(env, _JC_AbstractMethodError,
			    "%s.%s%s invoked from %s.%s%s on a %s",
			    imethod->class->name, imethod->name,
			    imethod->signature, method->class->name,
			    method->name, method->signature, obj->type->name);
			goto exception;
		}

		/* Verify method is public */
		if (!_JC_ACC_TEST(imethod, PUBLIC)) {
			_jc_post_exception_msg(env, _JC_IllegalAccessError,
			    "%s.%s%s invoked from %s.%s%s on a %s",
			    imethod->class->name, imethod->name,
			    imethod->signature, method->class->name,
			    method->name, method->signature, obj->type->name);
			goto exception;
		}
		break;
	    }
	case _JC_invokevirtual:
	    {
		_jc_type *vtype;

		vtype = _JC_LW_TEST(obj->lockword, ARRAY) ?
		    env->vm->boot.types.Object : obj->type;
		imethod = vtype->u.nonarray.mtable[imethod->vtable_index];
		break;
	    }
	case _JC_invokespecial:
		break;
	case _JC_invokestatic:
		if (!_JC_FLG_TEST(imethod->class, INITIALIZED)
		    && _jc_initialize_type(env, imethod->class) != JNI_OK)
			goto exception;
		break;
	default:
		_JC_ASSERT(JNI_FALSE);
		break;
	}

	/* Get pointer to parameters */
	params = sp + (code->opcodes[pc] != _JC_invokestatic);

	/* Invoke the method */
	if (_JC_ACC_TEST(imethod, NATIVE))
		status = _jc_invoke_native_method(env, imethod, JNI_TRUE, sp);
	else if (_JC_ACC_TEST(imethod, INTERP))
		status = _jc_interp(env, imethod, obj, params);
	else {
		status = _jc_invoke_jcni_a(env,
		    imethod, imethod->function, obj, params);
	}

	/* Did method throw an exception? */
	if (status != JNI_OK)
		goto exception;

	/* Push return value, if any */
	switch (imethod->param_ptypes[imethod->num_parameters]) {
	case _JC_TYPE_BOOLEAN:
		PUSHI(env->retval.z);
		break;
	case _JC_TYPE_BYTE:
		PUSHI(env->retval.b);
		break;
	case _JC_TYPE_CHAR:
		PUSHI(env->retval.c);
		break;
	case _JC_TYPE_SHORT:
		PUSHI(env->retval.s);
		break;
	case _JC_TYPE_INT:
		PUSHI(env->retval.i);
		break;
	case _JC_TYPE_FLOAT:
		PUSHF(env->retval.f);
		break;
	case _JC_TYPE_REFERENCE:
		PUSHL(env->retval.l);
		break;
	case _JC_TYPE_LONG:
		PUSHJ(env->retval.j);
		break;
	case _JC_TYPE_DOUBLE:
		PUSHD(env->retval.d);
		break;
	case _JC_TYPE_VOID:
		break;
	default:
		_JC_ASSERT(JNI_FALSE);
	}
	NEXT();
    }
do_ior:
	POP(2);
	PUSHI(STACKI(0) | STACKI(1));
	NEXT();
do_irem:
	POP(2);
	if (STACKI(1) == 0)
		goto arithmetic_exception;
	PUSHI(STACKI(0) % STACKI(1));
	NEXT();
do_ireturn:
	POP(1);
	env->retval.i = STACKI(0);
	goto done;
do_ishl:
	POP(2);
	PUSHI(STACKI(0) << (STACKI(1) & 0x1f));
	NEXT();
do_ishr:
	POP(2);
	PUSHI(_JC_ISHR(STACKI(0), STACKI(1) & 0x1f));
	NEXT();
do_istore:
	POP(1);
	LOCALI(INFO(local)) = STACKI(0);
	NEXT();
do_isub:
	POP(2);
	PUSHI(STACKI(0) - STACKI(1));
	NEXT();
do_iushr:
	POP(2);
	PUSHI(_JC_IUSHR(STACKI(0), STACKI(1) & 0x1f));
	NEXT();
do_ixor:
	POP(2);
	PUSHI(STACKI(0) ^ STACKI(1));
	NEXT();
do_jsr:
	PUSHI(pc + 1);
	JUMP(INFO(target));
do_l2d:
	POP2(1);
	PUSHD(STACKJ(0));
	NEXT();
do_l2f:
	POP2(1);
	PUSHF(STACKJ(0));
	NEXT();
do_l2i:
	POP2(1);
	PUSHI(STACKJ(0));
	NEXT();
do_ladd:
	POP2(2);
	PUSHJ(STACKJ(0) + STACKJ(2));
	NEXT();
do_laload:
    {
	_jc_long_array *array;
	jint index;

	POP(2);
	array = (_jc_long_array *)STACKL(0);
	index = STACKI(1);
	ARRAYCHECK(array, index);
	PUSHJ(array->elems[index]);
	NEXT();
    }
do_land:
	POP2(2);
	PUSHJ(STACKJ(0) & STACKJ(2));
	NEXT();
do_lastore:
    {
	_jc_long_array *array;
	jint index;

	POP(4);
	array = (_jc_long_array *)STACKL(0);
	index = STACKI(1);
	ARRAYCHECK(array, index);
	array->elems[index] = STACKJ(2);
	NEXT();
    }
do_lcmp:
	POP2(2);
	PUSHI(_JC_LCMP(STACKJ(0), STACKJ(2)));
	NEXT();
do_ldc:
	memcpy(sp, &INFO(constant), sizeof(jint));
	POP(-1);
	NEXT();
do_ldc_string:
    {
	_jc_resolve_info rinfo;
	_jc_object *string;

	/* Create intern'd string */
	if ((string = _jc_new_intern_string(env,
	    INFO(utf8), strlen(INFO(utf8)))) == NULL)
		goto exception;

	/* Create reference list with one reference */
	memset(&rinfo, 0, sizeof(rinfo));
	rinfo.loader = method->class->loader;
	rinfo.implicit_refs = &string;
	rinfo.num_implicit_refs = 1;

	/* Add implicit reference to string from class loader */
	if (_jc_merge_implicit_refs(env, &rinfo) != JNI_OK) {
		_jc_post_exception_info(env);
		goto exception;
	}

	/* Update instruction */
	INFO(constant).l = string;
	code->opcodes[pc] = _JC_ldc;

	/* Now execute it again */
	JUMP(pc);
    }
do_ldc2_w:
	memcpy(sp, &INFO(constant), 2 * sizeof(jint));
	POP(-2);
	NEXT();
do_ldiv:
	POP2(2);
	if (STACKJ(2) == 0)
		goto arithmetic_exception;
	PUSHJ(STACKJ(0) / STACKJ(2));
	NEXT();
do_lload:
	PUSHJ(LOCALJ(INFO(local)));
	NEXT();
do_lmul:
	POP2(2);
	PUSHJ(STACKJ(0) * STACKJ(2));
	NEXT();
do_lneg:
	POP2(1);
	PUSHJ(-STACKJ(0));
	NEXT();
do_lookupswitch:
    {
	_jc_lookupswitch *const lsw = INFO(lookupswitch);
	_jc_lookup *entry;
	_jc_lookup key;

	POP(1);
	key.match = STACKI(0);
	entry = bsearch(&key, lsw->pairs, lsw->num_pairs,
	    sizeof(*lsw->pairs), _jc_lookup_compare);
	JUMP(entry != NULL ? entry->target : lsw->default_target);
    }
do_lor:
	POP2(2);
	PUSHJ(STACKJ(0) | STACKJ(2));
	NEXT();
do_lrem:
	POP2(2);
	if (STACKJ(2) == 0)
		goto arithmetic_exception;
	PUSHJ(STACKJ(0) % STACKJ(2));
	NEXT();
do_lreturn:
	POP2(1);
	env->retval.j = STACKJ(0);
	goto done;
do_lshl:
	POP(3);
	PUSHJ(STACKJ(0) << (STACKI(2) & 0x3f));
	NEXT();
do_lshr:
	POP(3);
	PUSHJ(_JC_LSHR(STACKJ(0), STACKI(2)));
	NEXT();
do_lstore:
	POP2(1);
	LOCALJ(INFO(local)) = STACKJ(0);
	NEXT();
do_lsub:
	POP2(2);
	PUSHJ(STACKJ(0) - STACKJ(2));
	NEXT();
do_lushr:
	POP(3);
	PUSHJ(_JC_LUSHR(STACKJ(0), STACKI(2)));
	NEXT();
do_lxor:
	POP2(2);
	PUSHJ(STACKJ(0) ^ STACKJ(2));
	NEXT();
do_monitorenter:
	POP(1);
	if (STACKL(0) == NULL)
		goto null_pointer_exception;
	if (_jc_lock_object(env, STACKL(0)) != JNI_OK)
		goto exception;
	NEXT();
do_monitorexit:
	POP(1);
	if (STACKL(0) == NULL)
		goto null_pointer_exception;
	if (_jc_unlock_object(env, STACKL(0)) != JNI_OK)
		goto exception;
	NEXT();
do_multianewarray:
    {
	_jc_multianewarray *const info = &INFO(multianewarray);
	_jc_array *array;
	jint *sizes;
	int i;

	POP(info->dims);
	sizes = &STACKI(0);			/* overwrite popped stack */
	for (i = 0; i < info->dims; i++)
		sizes[i] = STACKI(i);
	if ((array = _jc_new_multiarray(env,
	    info->type, info->dims, sizes)) == NULL)
		goto exception;
	PUSHL((_jc_object *)array);
	NEXT();
    }
do_new:
    {
	_jc_object *obj;

	if ((obj = _jc_new_object(env, INFO(type))) == NULL)
		goto exception;
	PUSHL(obj);
	NEXT();
    }
do_newarray:
    {
	_jc_array *array;

	POP(1);
	if ((array = _jc_new_array(env, INFO(type), STACKI(0))) == NULL)
		goto exception;
	PUSHL((_jc_object *)array);
	NEXT();
    }
do_nop:
	NEXT();
do_pop:
	POP(1);
	NEXT();
do_pop2:
	POP2(1);
	NEXT();
do_putfield:
    {
	_jc_field *const field = INFO(field);
	_jc_type *const ftype = field->type;
	const void *data;

	/* Pop the stack */
	if (_jc_dword_type[ftype->flags & _JC_TYPE_MASK])
		POP2(1);
	else
		POP(1);
	POP(1);

	/* Check for null instance */
	if (STACKL(0) == NULL)
		goto null_pointer_exception;

	/* Set the field */
	data = (char *)STACKL(0) + field->offset;
	switch (ftype->flags & _JC_TYPE_MASK) {
	case _JC_TYPE_BOOLEAN:
		*(jboolean *)data = STACKI(1) & 0x01;
		break;
	case _JC_TYPE_BYTE:
		*(jbyte *)data = STACKI(1);
		break;
	case _JC_TYPE_CHAR:
		*(jchar *)data = STACKI(1);
		break;
	case _JC_TYPE_SHORT:
		*(jshort *)data = STACKI(1);
		break;
	case _JC_TYPE_INT:
		*(jint *)data = STACKI(1);
		break;
	case _JC_TYPE_FLOAT:
		*(jfloat *)data = STACKF(1);
		break;
	case _JC_TYPE_LONG:
		*(jlong *)data = STACKJ(1);
		break;
	case _JC_TYPE_DOUBLE:
		*(jdouble *)data = STACKD(1);
		break;
	case _JC_TYPE_REFERENCE:
		*(_jc_object **)data = STACKL(1);
		break;
	default:
		_JC_ASSERT(JNI_FALSE);
		break;
	}
	NEXT();
    }
do_putstatic:
    {
	_jc_field *const field = INFO(field);
	const u_char ptype = _jc_sig_types[(u_char)*field->signature];
	const void *data;

	/*
	 * Pop the stack. We can't look directly at the field's type
	 * here because the field's class may not be resolved yet.
	 */
	if (_jc_dword_type[ptype])
		POP2(1);
	else
		POP(1);

	/* Initialize field's class */
	if (!_JC_FLG_TEST(field->class, INITIALIZED)) {
		if (_jc_initialize_type(env, field->class) != JNI_OK)
			goto exception;
	}

	/* Set the field */
	data = (char *)field->class->u.nonarray.class_fields + field->offset;
	switch (ptype) {
	case _JC_TYPE_BOOLEAN:
		*(jboolean *)data = STACKI(0) & 0x01;
		break;
	case _JC_TYPE_BYTE:
		*(jbyte *)data = STACKI(0);
		break;
	case _JC_TYPE_CHAR:
		*(jchar *)data = STACKI(0);
		break;
	case _JC_TYPE_SHORT:
		*(jshort *)data = STACKI(0);
		break;
	case _JC_TYPE_INT:
		*(jint *)data = STACKI(0);
		break;
	case _JC_TYPE_FLOAT:
		*(jfloat *)data = STACKF(0);
		break;
	case _JC_TYPE_LONG:
		*(jlong *)data = STACKJ(0);
		break;
	case _JC_TYPE_DOUBLE:
		*(jdouble *)data = STACKD(0);
		break;
	case _JC_TYPE_REFERENCE:
		*(_jc_object **)data = STACKL(0);
		break;
	default:
		_JC_ASSERT(JNI_FALSE);
		break;
	}
	NEXT();
    }
do_ret:
	JUMP(LOCALI(INFO(local)));
do_return:
	goto done;
do_saload:
    {
	_jc_short_array *array;
	jint index;

	POP(2);
	array = (_jc_short_array *)STACKL(0);
	index = STACKI(1);
	ARRAYCHECK(array, index);
	PUSHI(array->elems[index]);
	NEXT();
    }
do_sastore:
    {
	_jc_short_array *array;
	jint index;

	POP(3);
	array = (_jc_short_array *)STACKL(0);
	index = STACKI(1);
	ARRAYCHECK(array, index);
	array->elems[index] = STACKI(2);
	NEXT();
    }
do_swap:
    {
	jint temp;

	temp = STACKI(-2);
	STACKI(-2) = STACKI(-1);
	STACKI(-1) = temp;
	NEXT();
    }
do_tableswitch:
    {
	_jc_tableswitch *const tsw = INFO(tableswitch);
	jint key;

	POP(1);
	key = STACKI(0);
	JUMP((key >= tsw->low && key <= tsw->high) ?
	    tsw->targets[key - tsw->low] : tsw->default_target);
    }

periodic_check:
	if (_jc_thread_check(env) != JNI_OK)
		goto exception;
	ticker = PERIODIC_CHECK_TICKS;
	JUMP(pc);

null_pointer_exception:
	_jc_post_exception(env, _JC_NullPointerException);
	goto exception;

arithmetic_exception:
	_jc_post_exception(env, _JC_ArithmeticException);
	goto exception;

exception:
    {
	jint status;
	int i;

	/* Sanity check */
	_JC_ASSERT(env->head.pending != NULL);

	/* Check this method for a matching trap */
	for (i = 0; i < code->num_traps; i++) {
		_jc_jvm *const vm = env->vm;
		_jc_interp_trap *const trap = &code->traps[i];
		_jc_object *e;

		/* See if trap matches */
		if (pc < trap->start || pc >= trap->end)
			continue;
		if ((e = _jc_retrieve_exception(env, trap->type)) == NULL)
			continue;

		/* Verbosity for caught exception */
		if ((env->vm->verbose_flags
		    & (1 << _JC_VERBOSE_EXCEPTIONS)) != 0) {
			_jc_printf(vm, "[verbose %s: caught via trap"
			    " %d (%d-%d) in %s.%s%s in thread %p: ",
			    _jc_verbose_names[_JC_VERBOSE_EXCEPTIONS],
			    i, trap->start, trap->end,
			    method->class->name, method->name,
			    method->signature, env);
			_jc_fprint_exception_headline(env, stdout, e);
			_jc_printf(vm, "]\n");
		}

		/* Push exception and proceed with handler */
		state.locals[code->max_locals] = (_jc_word)e;
		sp = &state.locals[code->max_locals + 1];
		JUMP(trap->target);
	}

	/* Exception not caught */
	status = JNI_ERR;
	goto exit;

done:
	status = JNI_OK;

exit:
	/* Sanity check */
	_JC_ASSERT(status == JNI_OK || env->head.pending != NULL);

	/* De-synchronize if necessary */
	if (lock != NULL) {
		_jc_value retval;

		/* Temporarily save return value */
		retval = env->retval;

		/* Unlock monitor */
		if (_jc_unlock_object(env, lock) != JNI_OK)
			status = JNI_ERR;

		/* Restore return value */
		env->retval = retval;
	}

	/* Pop Java stack frame */
	env->java_stack = state.jstack.next;

	/* Done */
	return status;
    }
}

/*
 * Comparison function for binary search of lookupswitch tables.
 */
static int
_jc_lookup_compare(const void *v1, const void *v2)
{
	const _jc_lookup *const l1 = v1;
	const _jc_lookup *const l2 = v2;

	return (l1->match > l2->match) - (l1->match < l2->match);
}

/*
 * Look up a Java source file line number.
 */
int
_jc_interp_pc_to_jline(_jc_method *method, int index)
{
	_jc_method_code *const code = &method->u.code;
	_jc_linemap *base;
	int span;

	/* Sanity check */
	_JC_ASSERT(_JC_ACC_TEST(method, INTERP));
	_JC_ASSERT(_JC_FLG_TEST(method->class, RESOLVED));
	_JC_ASSERT(index >= 0 && index < code->num_insns);

	/* Binary search for line number */
	for (base = code->linemaps, span = code->num_linemaps;
	    span != 0; span >>= 1) {
		_jc_linemap *const sample = &base[span >> 1];

		if (index <= sample->index)
			continue;
		if (index > (sample + 1)->index) {
			base = sample + 1;
			span--;
			continue;
		}
		return sample->line;
	}

	/* Not found */
	return 0;
}

/*
 * Type-specific gateway functions into _jc_vinterp()
 */
#define _JC_INTERP_ENTRY(_letter, _name, _type, _rtn)			\
_type									\
_jc_ ## _name ## _ ## _letter(_jc_env *env, ...)			\
{									\
	va_list args;							\
									\
	va_start(args, env);						\
	_jc_v ## _name(env, args);					\
	va_end(args);							\
	return _rtn;							\
}
_JC_INTERP_ENTRY(z, interp, jboolean, env->retval.z)
_JC_INTERP_ENTRY(b, interp, jbyte, env->retval.b)
_JC_INTERP_ENTRY(c, interp, jchar, env->retval.c)
_JC_INTERP_ENTRY(s, interp, jshort, env->retval.s)
_JC_INTERP_ENTRY(i, interp, jint, env->retval.i)
_JC_INTERP_ENTRY(j, interp, jlong, env->retval.j)
_JC_INTERP_ENTRY(f, interp, jfloat, env->retval.f)
_JC_INTERP_ENTRY(d, interp, jdouble, env->retval.d)
_JC_INTERP_ENTRY(l, interp, _jc_object *, env->retval.l)
_JC_INTERP_ENTRY(v, interp, void, )
_JC_INTERP_ENTRY(z, interp_native, jboolean, env->retval.z)
_JC_INTERP_ENTRY(b, interp_native, jbyte, env->retval.b)
_JC_INTERP_ENTRY(c, interp_native, jchar, env->retval.c)
_JC_INTERP_ENTRY(s, interp_native, jshort, env->retval.s)
_JC_INTERP_ENTRY(i, interp_native, jint, env->retval.i)
_JC_INTERP_ENTRY(j, interp_native, jlong, env->retval.j)
_JC_INTERP_ENTRY(f, interp_native, jfloat, env->retval.f)
_JC_INTERP_ENTRY(d, interp_native, jdouble, env->retval.d)
_JC_INTERP_ENTRY(l, interp_native, _jc_object *, env->retval.l)
_JC_INTERP_ENTRY(v, interp_native, void, )

/*
 * Entry point for interpreted methods when invoked from ELF methods.
 * We get here by way of a trampoline set up by _jc_build_trampoline(),
 * which also sets env->interp to the method we are going to interpret.
 */
static void
_jc_vinterp(_jc_env *env, va_list args)
{
	_jc_method *const method = env->interp;
	jboolean clipped_stack;
	_jc_word *params;
	_jc_object *this;
	jint status;
	int pnum;
	int i;

	/* Sanity check */
	_JC_ASSERT(_JC_ACC_TEST(method, INTERP));
	_JC_ASSERT(!_JC_ACC_TEST(method, NATIVE));
	_JC_ASSERT(_JC_FLG_TEST(method->class, RESOLVED));

	/* Allocate space for parameter array */
	if ((params = _JC_STACK_ALLOC(env,
	    method->u.code.num_params2)) == NULL) {
		_jc_post_exception_info(env);
		_jc_throw_exception(env);
	}

	/* Get 'this' if method is non-static */
	this = !_JC_ACC_TEST(method, STATIC) ?
	    va_arg(args, _jc_object *) : NULL;

	/* Get method parameters, occupying two slots for long/double */
	for (pnum = i = 0; i < method->num_parameters; i++) {
		switch (method->param_ptypes[i]) {
		case _JC_TYPE_BOOLEAN:
			params[pnum++] = (jint)(jboolean)va_arg(args, jint);
			break;
		case _JC_TYPE_BYTE:
			params[pnum++] = (jint)(jbyte)va_arg(args, jint);
			break;
		case _JC_TYPE_CHAR:
			params[pnum++] = (jint)(jchar)va_arg(args, jint);
			break;
		case _JC_TYPE_SHORT:
			params[pnum++] = (jint)(jshort)va_arg(args, jint);
			break;
		case _JC_TYPE_INT:
			params[pnum++] = va_arg(args, jint);
			break;
		case _JC_TYPE_FLOAT:
		    {
			jfloat param = (jfloat)va_arg(args, jdouble);

			memcpy(params + pnum, &param, sizeof(param));
			pnum++;
			break;
		    }
		case _JC_TYPE_LONG:
		    {
			jlong param = va_arg(args, jlong);

			memcpy(params + pnum, &param, sizeof(param));
			pnum += 2;
			break;
		    }
		case _JC_TYPE_DOUBLE:
		    {
			jdouble param = va_arg(args, jdouble);

			memcpy(params + pnum, &param, sizeof(param));
			pnum += 2;
			break;
		    }
		case _JC_TYPE_REFERENCE:
			params[pnum++] = (_jc_word)va_arg(args, _jc_object *);
			break;
		default:
			_JC_ASSERT(JNI_FALSE);
			break;
		}
	}
	_JC_ASSERT(pnum == method->u.code.num_params2);

	/* Clip the current top of the Java stack */
	clipped_stack = !env->java_stack->interp && _jc_stack_clip(env);

	/* Invoke method */
	status = _jc_interp(env, method, this, params);

	/* Unclip the current top of the Java stack */
	if (clipped_stack)
		_jc_stack_unclip(env);

	/* Throw exception if any */
	if (status != JNI_OK)
		_jc_throw_exception(env);
}

/*
 * Same thing as _jc_vinterp() but for native methods.
 */
static void
_jc_vinterp_native(_jc_env *env, va_list args)
{
	_jc_method *const method = env->interp;

	/* Sanity check */
	_JC_ASSERT(_JC_ACC_TEST(method, INTERP));
	_JC_ASSERT(_JC_ACC_TEST(method, NATIVE));
	_JC_ASSERT(_JC_FLG_TEST(method->class, RESOLVED));

	/* Invoke method */
	if (_jc_invoke_native_method(env, method, JNI_FALSE, args) != JNI_OK)
		_jc_throw_exception(env);
}


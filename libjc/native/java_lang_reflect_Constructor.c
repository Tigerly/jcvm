
/*
 * @COPYRIGHT@
 *
 * $Id: java_lang_reflect_Constructor.c,v 1.7 2005/05/15 21:41:01 archiecobbs Exp $
 */

#include "libjc.h"
#include "java_lang_reflect_Constructor.h"

/* Internal functions */
static _jc_method	*_jc_check_constructor(_jc_env *env, _jc_object *this);

/*
 * private final native Object constructNative(Object[], Class, int)
 *      throws InstantiationException, IllegalAccessException,
 *		InvocationTargetException
 */
_jc_object * _JC_JCNI_ATTR
JCNI_java_lang_reflect_Constructor_constructNative(_jc_env *env,
	_jc_object *this, _jc_object_array *params, _jc_object *cl, jint slot)
{
	_jc_jvm *const vm = env->vm;
	_jc_type *calling_class;
	_jc_method *method;

	/* Get constructor */
	method = _jc_check_constructor(env, this);

	/* Check accessibility */
	if (_jc_invoke_virtual(env,
	    vm->boot.methods.AccessibleObject.isAccessible, this) != JNI_OK)
		_jc_throw_exception(env);
	if (env->retval.z)
		goto accessible;

	/* Check access */
	switch (_jc_reflect_accessible(env, method->class,
	    method->access_flags, &calling_class)) {
	case -1:
		_jc_throw_exception(env);
	case 1:
		break;
	case 0:
		_jc_post_exception_msg(env, _JC_IllegalAccessException,
		    "`%s.%s%s' is not accessible from `%s'",
		    method->class->name, method->name, method->signature,
		    calling_class->name);
		_jc_throw_exception(env);
	}

accessible:
	/* Invoke constructor */
	if (_jc_reflect_invoke(env, method, NULL, params) != JNI_OK)
		_jc_throw_exception(env);
	_JC_ASSERT(env->retval.l != NULL);

	/* Return created object */
	return env->retval.l;
}

/*
 * public final native Class[] getExceptionTypes()
 */
_jc_object_array * _JC_JCNI_ATTR
JCNI_java_lang_reflect_Constructor_getExceptionTypes(_jc_env *env,
	_jc_object *this)
{
	_jc_object_array *array;
	_jc_method *method;

	/* Get constructor */
	method = _jc_check_constructor(env, this);

	/* Get exception types */
	if ((array = _jc_get_exception_types(env, method)) == NULL)
		_jc_throw_exception(env);

	/* Return array */
	return array;
}

/*
 * public final native int getModifiers()
 */
jint _JC_JCNI_ATTR
JCNI_java_lang_reflect_Constructor_getModifiers(_jc_env *env, _jc_object *this)
{
	_jc_method *method;

	/* Get constructor */
	method = _jc_check_constructor(env, this);

	/* Return flags */
	return method->access_flags & _JC_ACC_MASK;
}

/*
 * public final native Class[] getParameterTypes()
 */
_jc_object_array * _JC_JCNI_ATTR
JCNI_java_lang_reflect_Constructor_getParameterTypes(_jc_env *env,
	_jc_object *this)
{
	_jc_object_array *array;
	_jc_method *method;

	/* Get constructor */
	method = _jc_check_constructor(env, this);

	/* Get parameter types */
	if ((array = _jc_get_parameter_types(env, method)) == NULL)
		_jc_throw_exception(env);

	/* Return array */
	return array;
}

/*
 * Find the _jc_method structure corresponding to a Constructor object.
 */
static _jc_method *
_jc_check_constructor(_jc_env *env, _jc_object *this)
{
	/* Check for null */
	if (this == NULL) {
		_jc_post_exception(env, _JC_NullPointerException);
		_jc_throw_exception(env);
	}

	/* Return constructor */
	return _jc_get_constructor(env, this);
}


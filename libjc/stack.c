
/*
 * @COPYRIGHT@
 *
 * $Id: stack.c,v 1.19 2005/04/04 14:04:05 archiecobbs Exp $
 */

#include "libjc.h"

/*
 * Clip the current top of the Java stack. A thread's Java stack is
 * a sequence of disconnected chunks of contiguous C stack frames
 * intersperced with non-crawlable stuff like native stack frames
 * and signal frames. Each chunk also contains a set of saved registers
 * which are scanned conservatively during garbage collection.
 *
 * This sets the top frame to be the calling function's calling function.
 * See also _jc_invoke_jcni_a().
 *
 * Returns JNI_TRUE if the stack was clipped, otherwise JNI_FALSE
 * (i.e., the stack was already clipped so we didn't clip it again).
 *
 * NOTE: If this function returns JNI_TRUE, the function calling this
 * function must call _jc_stack_unclip() before returning or else throw
 * (not just post) an exception.
 */
jboolean
_jc_stack_clip(_jc_env *env)
{
	_jc_java_stack *const stack = env->java_stack;
	_jc_exec_stack *estack;

	/* Sanity check */
	_JC_ASSERT(env == _jc_get_current_env());

	/* If no Java call stack yet, nothing to do */
	if (stack == NULL)
		return JNI_FALSE;

	/* If interpreting, just set the flag */
	if (stack->interp) {
		_jc_interp_stack *const istack = (_jc_interp_stack *)stack;

		if (!istack->clipped) {
			istack->clipped = JNI_TRUE;
			return JNI_TRUE;
		}
		return JNI_FALSE;
	}

	/* We are running executable Java */
	estack = (_jc_exec_stack *)stack;

	/* Is the Java stack already clipped? */
	if (estack->pc != NULL) {
		_JC_ASSERT(_jc_stack_frame_valid(estack->frame));
		return JNI_FALSE;
	}

	/* Grab the current context and clip the Java stack with it */
#if HAVE_GETCONTEXT
    {
	ucontext_t ctx;

	getcontext(&ctx);
	_jc_stack_clip_ctx(env, &ctx.uc_mcontext);
    }
#else
    {
	mcontext_t ctx;

	/* Sanity check */
	_JC_ASSERT(env->ctx == NULL);

	/* Get current machine context */
#if defined(__FreeBSD__) && defined(__i386__)
	/* Work around for FreeBSD bug threads/75374 */
	{
	    jmp_buf buf;

	    setjmp(buf);
	    memset(&ctx, 0, sizeof(ctx));
	    ctx.mc_eip = buf[0]._jb[0];
	    ctx.mc_ebx = buf[0]._jb[1];
	    ctx.mc_esp = buf[0]._jb[2];
	    ctx.mc_ebp = buf[0]._jb[3];
	    ctx.mc_esi = buf[0]._jb[4];
	    ctx.mc_edi = buf[0]._jb[5];
	}
#else
	env->ctx = &ctx;
	pthread_kill(pthread_self(), SIGSEGV);
	env->ctx = NULL;
#endif

	/* Use it to clip Java stack */
	_jc_stack_clip_ctx(env, &ctx);
    }
#endif

	/* Roll back two frames to calling function's calling function */
	_jc_stack_frame_next(&estack->frame, &estack->pc);
	_jc_stack_frame_next(&estack->frame, &estack->pc);

	/* Done */
	return JNI_TRUE;
}

/*
 * Clip the top of the Java stack using the supplied machine context,
 * which must be created either from getcontext() or by a signal.
 * The stack must not already be clipped.
 *
 * This sets the top of the Java stack to be the function in which
 * the context was created.
 *
 * Returns JNI_TRUE if the stack was clipped, otherwise JNI_FALSE
 */
jboolean
_jc_stack_clip_ctx(_jc_env *env, const mcontext_t *ctx)
{
	_jc_java_stack *const stack = env->java_stack;
	_jc_exec_stack *estack;

	/* Sanity check */
	_JC_ASSERT(env == _jc_get_current_env());

	/* If there's no Java call stack yet, nothing to do */
	if (stack == NULL)
		return JNI_FALSE;

	/* Sanity check */
	_JC_ASSERT(!stack->interp);
	estack = (_jc_exec_stack *)stack;

	/* Sanity check */
	_JC_ASSERT(estack->pc == NULL);
	_JC_ASSERT(!_jc_stack_frame_valid(estack->frame));

	/* Save machine registers */
	estack->regs = *ctx;

	/* Extract frame info from saved context */
	estack->frame = _jc_mcontext_frame(&estack->regs);
	estack->pc = _jc_mcontext_pc(&estack->regs);

	/* Done */
	return JNI_TRUE;
}

/*
 * Un-do the effects of _jc_stack_clip().
 */
void
_jc_stack_unclip(_jc_env *env)
{
	_jc_java_stack *const stack = env->java_stack;
	_jc_exec_stack *estack;

	/* Sanity check */
	_JC_ASSERT(stack != NULL);
	_JC_ASSERT(env == _jc_get_current_env());

	/* If interpreting, just unset the flag */
	if (stack->interp) {
		_jc_interp_stack *const istack = (_jc_interp_stack *)stack;

		/* Sanity check: the stack is clipped */
		_JC_ASSERT(istack->clipped);

		/* Mark it unclipped */
		istack->clipped = JNI_FALSE;
		return;
	}

	/* We are running executable Java */
	estack = (_jc_exec_stack *)stack;

	/* Sanity check: the stack is clipped */
	_JC_ASSERT(estack->pc != NULL);

	/* Invalidate Java stack top, allowing it to "float" again */
	estack->pc = NULL;
#ifndef NDEBUG
	_jc_stack_frame_init(&estack->frame);
#endif
}

/*
 * Print out the exception stack trace associated with the
 * currently posted exception, which must not be NULL.
 */
void
_jc_print_stack_trace(_jc_env *env, FILE *fp)
{
	_jc_jvm *const vm = env->vm;
	_jc_saved_frame *frames;
	_jc_object *vmThrowable;
	_jc_byte_array *bytes;
	_jc_object *cause;
	int num_frames;
	_jc_object *e;

	/* Get exception */
	e = env->head.pending;

again:
	/* Sanity check */
	_JC_ASSERT(_jc_subclass_of(e, vm->boot.types.Throwable));

	/* Print exception headling */
	_jc_fprint_exception_headline(env, fp, e);
	_jc_fprintf(vm, fp, "\n");

	/* Get associated VMThrowable */
	if ((vmThrowable = *_JC_VMFIELD(vm, e,
	    Throwable, vmState, _jc_object *)) == NULL)
		goto no_trace;

	/* Get saved frames from 'vmdata' byte[] array */
	if ((bytes = *_JC_VMFIELD(vm, vmThrowable,
	    VMThrowable, vmdata, _jc_byte_array *)) == NULL)
		goto no_trace;

	/* Print stack frames */
	frames = (_jc_saved_frame *)_JC_ROUNDUP2(
	    (_jc_word)bytes->elems, _JC_FULL_ALIGNMENT);
	num_frames = (bytes->length -
	    ((_jc_word)frames - (_jc_word)bytes->elems)) / sizeof(*frames);
	_jc_print_stack_frames(env, fp, num_frames, frames);

no_trace:
	/* Print causing exception, if any */
	if ((cause = *_JC_VMFIELD(vm, e,
	      Throwable, cause, _jc_object *)) != NULL
	    && cause != e) {
		_jc_fprintf(vm, fp, "Caused by ");
		e = cause;
		goto again;
	}
}

/*
 * Print out stack frames from an array of _jc_saved_frame's.
 */
void
_jc_print_stack_frames(_jc_env *env, FILE *fp,
	int num_frames, _jc_saved_frame *frames)
{
	_jc_jvm *const vm = env->vm;
	int i;

	/* Print out stack trace */
	for (i = 0; i < num_frames; i++) {
		_jc_saved_frame *const frame = &frames[i];
		_jc_method *method;
		_jc_type *class;

		/* Get method and class */
		method = frame->method;
		class = method->class;

		/* Print out stack frame */
		_jc_fprintf(vm, fp, "\tat ");
		_jc_fprint_noslash(vm, fp, class->name);
		_jc_fprintf(vm, fp, ".%s(", method->name);
		if (_JC_ACC_TEST(method, NATIVE))
			_jc_fprintf(vm, fp, "Native Method");
		else if (class->u.nonarray.source_file == NULL)
			_jc_fprintf(vm, fp, "Unknown");
		else {
			int jline;

			/* Print source file */
			_jc_fprintf(vm, fp, "%s",
			    class->u.nonarray.source_file);

			/* Print Java line number if known */
			jline = _JC_ACC_TEST(method, INTERP) ?
			    _jc_interp_pc_to_jline(method, frame->u.ipc) :
			    _jc_exec_pc_to_jline(method, frame->u.pc);
			if (jline != 0)
				_jc_fprintf(vm, fp, ":%d", jline);
		}
		_jc_fprintf(vm, fp, ")\n");
	}
}

/*
 * Save the current Java stack trace as an array of _jc_saved_frame's.
 *
 * If 'thread' doesn't correspond to the current thread, then the
 * target thread must be running in non-java mode or halted
 * (so that it's top Java stack frame is already clipped).
 *
 * The array is stored in the area pointed to by 'frames' (if not NULL).
 * At most 'max' frames are stored.
 *
 * Returns the total number of frames if successful, otherwise -1.
 */
int
_jc_save_stack_frames(_jc_env *env, _jc_env *thread,
	int max, _jc_saved_frame *frames)
{
	_jc_jvm *const vm = env->vm;
	_jc_stack_crawl crawl;
	int i;

	/* Lock VM */
	_JC_MUTEX_LOCK(env, vm->mutex);

	/* Crawl the stack */
	for (i = 0, _jc_stack_crawl_first(thread, &crawl);
	    crawl.method != NULL; _jc_stack_crawl_next(vm, &crawl)) {
		_jc_saved_frame *const frame = &frames[i];

		/* Skip non-Java functions */
		if (crawl.method->class == NULL)
			continue;

		/* Save this one */
		if (i < max) {
			frame->method = crawl.method;
			if (_JC_ACC_TEST(crawl.method, INTERP)) {
				_jc_interp_stack *const istack
				    = (_jc_interp_stack *)crawl.stack;

				frame->u.ipc = *istack->pcp;
			} else
				frame->u.pc = crawl.pc;
		}
		i++;
	}

	/* Unlock VM */
	_JC_MUTEX_UNLOCK(env, vm->mutex);

	/* Done */
	return i;
}

/*
 * Initialize a Java stack crawl.
 *
 * If 'thread' doesn't correspond to the current thread, then the
 * target thread must be running in non-java mode or halted
 * (so that it's top Java stack frame is already clipped).
 *
 * NOTE: caller must acquire the VM lock before calling this function.
 */
void
_jc_stack_crawl_first(_jc_env *thread, _jc_stack_crawl *crawl)
{
	_jc_java_stack *const stack = thread->java_stack;
	_jc_exec_stack *const estack = (_jc_exec_stack *)stack;
	_jc_interp_stack *const istack = (_jc_interp_stack *)stack;

	/* Initialize */
	memset(crawl, 0, sizeof(*crawl));

	/* If there's no Java call stack yet, return end of stack */
	if (stack == NULL)
		return;

	/* Sanity check */
	_JC_ASSERT(thread == _jc_get_current_env()
	    || (((!stack->interp && estack->pc != NULL)
	       || (stack->interp && istack->clipped))
	      && (thread->status == _JC_THRDSTAT_HALTING_NONJAVA
	       || thread->status == _JC_THRDSTAT_HALTED)));

	/* Start with top of stack */
	crawl->stack = stack;

	/* Interpreted case is easy */
	if (stack->interp) {
		crawl->method = istack->method;
		return;
	}

	/*
	 * Executable case.
	 *
	 * If the stack top already clipped, use it; otherwise start here.
	 * In the latter case, this must be the current thread.
	 */
	if ((crawl->pc = estack->pc) != NULL)
		crawl->frame = estack->frame;
	else {
		_JC_ASSERT(!_jc_stack_frame_valid(estack->frame));
		_jc_stack_frame_current(&crawl->frame);
		_jc_stack_frame_next(&crawl->frame, &crawl->pc);
	}

	/* Skip over non-Java C stack frames */
	_jc_stack_crawl_skip(thread->vm, crawl);
}

/*
 * Get the next registered Java method (or _jc_invoke_jcni_a()) in a stack
 * crawl. The 'thread' parameter does not have to be the current thread.
 *
 * If the bottom of the stack is reached, crawl->method is set to NULL.
 *
 * NOTE: caller must acquire the VM lock before calling this function.
 */
void
_jc_stack_crawl_next(_jc_jvm *vm, _jc_stack_crawl *crawl)
{
	_jc_java_stack *stack = crawl->stack;

	/* Sanity check */
	_JC_ASSERT(crawl->method != NULL);

	/*
	 * For executable stack, just scan C stack frames until we
	 * reach _jc_invoke_jcni_a() (we always will eventually).
	 */
	if (!stack->interp && crawl->method != &vm->invoke_method) {
		_jc_stack_frame_next(&crawl->frame, &crawl->pc);
		_jc_stack_crawl_skip(vm, crawl);
		return;
	}

	/* Advance to the next deeper stack chunk */
	crawl->stack = crawl->stack->next;

	/* Did we reach the end of the Java stack? */
	if (crawl->stack == NULL) {
		memset(crawl, 0, sizeof(*crawl));
		return;
	}

	/* Initialize new stack chunk */
	if (crawl->stack->interp) {
		_jc_interp_stack *const istack
		    = (_jc_interp_stack *)crawl->stack;

		crawl->method = istack->method;
	} else {
		_jc_exec_stack *const estack = (_jc_exec_stack *)crawl->stack;

		crawl->pc = estack->pc;
		crawl->frame = estack->frame;
		_JC_ASSERT(_jc_stack_frame_valid(crawl->frame));
		_jc_stack_crawl_skip(vm, crawl);
	}
}

/*
 * March backwards through consecutive stack frames
 * until we hit one that matches a registered method.
 */
void
_jc_stack_crawl_skip(_jc_jvm *vm, _jc_stack_crawl *crawl)
{
	/* Look for a Java function (or _jc_invoke_jcni_a()) */
	while (JNI_TRUE) {
		_jc_method key;

		/* See if stack frame function is a registered method */
		key.function = key.u.exec.function_end = (void *)crawl->pc;
		if ((crawl->method = _jc_splay_find(
		    &vm->method_tree, &key)) != NULL) {
			_JC_ASSERT(!_JC_ACC_TEST(crawl->method, INTERP));
			break;
		}

		/* Advance to next C stack frame */
		_jc_stack_frame_next(&crawl->frame, &crawl->pc);
	}
}


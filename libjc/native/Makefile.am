## $Id: Makefile.am,v 1.8 2005/05/15 21:41:01 archiecobbs Exp $

jcbindir=	$(bindir)
jcdir=		$(datadir)/jc
cpdir=		$(CLASSPATH_HOME)/share/classpath

EXTRA_DIST=	gnu_classpath_VMStackWalker.c \
		gnu_classpath_VMStackWalker.h \
		gnu_classpath_VMSystemProperties.c \
		gnu_classpath_VMSystemProperties.h \
		java_io_VMObjectStreamClass.c \
		java_io_VMObjectStreamClass.h \
		java_lang_Thread.h \
		java_lang_VMClass.c \
		java_lang_VMClass.h \
		java_lang_VMClassLoader.c \
		java_lang_VMClassLoader.h \
		java_lang_VMCompiler.c \
		java_lang_VMCompiler.h \
		java_lang_VMObject.c \
		java_lang_VMObject.h \
		java_lang_VMRuntime.c \
		java_lang_VMRuntime.h \
		java_lang_VMSystem.c \
		java_lang_VMSystem.h \
		java_lang_VMThread.c \
		java_lang_VMThread.h \
		java_lang_VMThrowable.c \
		java_lang_VMThrowable.h \
		java_lang_reflect_Constructor.c \
		java_lang_reflect_Constructor.h \
		java_lang_reflect_Field.c \
		java_lang_reflect_Field.h \
		java_lang_reflect_Method.c \
		java_lang_reflect_Method.h \
		org_dellroad_jc_JCFinder.c \
		org_dellroad_jc_JCFinder.h \
		org_dellroad_jc_cgen_SootCodeGenerator.c \
		org_dellroad_jc_cgen_SootCodeGenerator.h \
		org_dellroad_jc_vm_DebugThread.c \
		org_dellroad_jc_vm_DebugThread.h \
		org_dellroad_jc_vm_FinalizerThread.c \
		org_dellroad_jc_vm_FinalizerThread.h

##
## Generate JCNI header files automatically using jcjavah
##

JAVAH=		$(jcbindir)/jcjavah
JAVAH_CMD=	$(JAVAH) -classpath $(jcdir)/jc.zip:$(cpdir)/glibj.zip \
		    -d $(srcdir)

HFILES=		gnu_classpath_VMStackWalker.h \
		gnu_classpath_VMSystemProperties.h \
		java_io_VMObjectStreamClass.h \
		java_lang_Thread.h \
		java_lang_VMClass.h \
		java_lang_VMClassLoader.h \
		java_lang_VMCompiler.h \
		java_lang_VMObject.h \
		java_lang_VMRuntime.h \
		java_lang_VMSystem.h \
		java_lang_VMThread.h \
		java_lang_VMThrowable.h \
		java_lang_reflect_Constructor.h \
		java_lang_reflect_Field.h \
		java_lang_reflect_Method.h \
		org_dellroad_jc_JCFinder.h \
		org_dellroad_jc_cgen_SootCodeGenerator.h \
		org_dellroad_jc_vm_DebugThread.h \
		org_dellroad_jc_vm_FinalizerThread.h

hfiles:			${HFILES}

gnu_classpath_VMStackWalker.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
gnu_classpath_VMSystemProperties.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
java_io_VMObjectStreamClass.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
java_lang_Thread.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
java_lang_VMClass.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
java_lang_VMClassLoader.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
java_lang_VMCompiler.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
java_lang_VMObject.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
java_lang_VMRuntime.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
java_lang_VMSystem.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
java_lang_VMThread.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
java_lang_VMThrowable.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
java_lang_reflect_Constructor.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
java_lang_reflect_Field.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
java_lang_reflect_Method.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
org_dellroad_jc_JCFinder.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
org_dellroad_jc_cgen_SootCodeGenerator.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
org_dellroad_jc_vm_DebugThread.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`
org_dellroad_jc_vm_FinalizerThread.h:
	$(JAVAH_CMD) `echo $@ | sed -e 's/\.h$$//g' -e 's/_/./g'`


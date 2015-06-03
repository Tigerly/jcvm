
//
// @COPYRIGHT@
//
// $Id: JarLoader.java,v 1.2 2006/01/21 01:50:26 archiecobbs Exp $
//

package org.dellroad.jc.vm;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.jar.Attributes;
import java.util.jar.JarFile;
import java.util.jar.Manifest;

/**
 * Helper class used to implement the "--jar" command line flag.
 */
public class JarLoader {

	private JarLoader() {
	}

	/**
	 * Invoke the main class from a JAR file.
	 *
	 * @param args command line parameters with the JAR file
	 *	name prepended as the first element of the array.
	 */
	public static void main(String[] args) throws Throwable {

		// Open JAR file
		File jarfile = new File(args[0]);
		JarFile jar;
		try {
			jar = new JarFile(jarfile);
		} catch (IOException e) {
			throw new RuntimeException("can't open ``"
			    + jarfile + "'': " + e.toString());
		}

		// Find manifest and read main class attribute
		Manifest manifest;
		try {
			manifest = jar.getManifest();
		} catch (IOException e) {
			throw new RuntimeException("can't read manifest"
			    + " in ``" + jarfile + "'': " + e.toString());
		}
		if (manifest == null) {
			throw new RuntimeException("no manifest found in ``"
			    + jarfile + "''");
		}
		String main = manifest.getMainAttributes().getValue(
		    Attributes.Name.MAIN_CLASS);
		if (main == null) {
			throw new RuntimeException("no ``"
			    + Attributes.Name.MAIN_CLASS + "'' attribute"
			    + " found in manifest of ``" + jarfile + "''");
		}

		// Find the main class
		Class cl;
		try {
			cl = ClassLoader.getSystemClassLoader().loadClass(main);
		} catch (ClassNotFoundException e) {
			throw new RuntimeException("class ``" + main + "'' not"
			    + " found in JAR file ``" + jarfile + "''");
		}

		// Find the main() method
		Method method;
		try {
			method = cl.getDeclaredMethod("main",
			    new Class[] { String[].class });
		} catch (NoSuchMethodException e) {
			throw new RuntimeException("no main() method found"
			    + " in class ``" + main + "''");
		}

		// Invoke main() method
		String[] mainArgs = new String[args.length - 1];
		System.arraycopy(args, 1, mainArgs, 0, mainArgs.length);
		try {
			method.invoke(null, new Object[] { mainArgs });
		} catch (IllegalAccessException e) {
			throw new RuntimeException("can't invoke main() method"
			    + " in class ``" + main + "'': " + e.toString());
		} catch (InvocationTargetException e) {
			throw e.getTargetException();
		}
	}
}



//
// @COPYRIGHT@
//
// $Id: MethodOptimizer.java,v 1.5 2004/12/19 21:01:03 archiecobbs Exp $
//

package org.dellroad.jc.cgen;

import java.util.List;
import java.util.Set;
import soot.SootMethod;
import soot.jimple.StmtBody;
import soot.toolkits.graph.CompleteUnitGraph;

/**
 * Interface for optimizing the Jimple code associated with a method.
 * An implementation of this interface is assocated with each
 * {@link SootCodeGenerator SootCodeGenerator}.
 */
public interface MethodOptimizer {

	/**
	 * Optimize the given method's body.
	 *
	 * <p>
	 * Depedencies should be added
	 * to <code>deps</code> when the class may not be explicitly
	 * referenced in the new body. For example, if a call to
	 * <code>Arrays.sort()</code> were inlined, then <code>Arrays</code>
	 * would need to be added as an explicit dependency, because the
	 * inlined code might not explicitly refer to the <code>Arrays</code>
	 * class.
	 *
	 * @param method method to optimize
	 * @param deps set to which to add any class file dependencies
	 *	which may not be reflected in the byte code. To add a
	 *	dependency, add the class' {@link soot.SootClass SootClass}
	 *	object.
	 * @return control flow graph of optimized method body
	 */
	public CompleteUnitGraph optimize(SootMethod method, Set deps);
}



//
// @COPYRIGHT@
//
// $Id: ActiveUseTagger.java,v 1.2 2004/12/24 21:55:56 archiecobbs Exp $
//

package org.dellroad.jc.cgen.analysis;

import java.util.Iterator;
import org.dellroad.jc.cgen.Util;
import org.dellroad.jc.cgen.ActiveUseCheckStmt;
import soot.Body;
import soot.SootClass;
import soot.SootField;
import soot.Value;
import soot.ValueBox;
import soot.jimple.*;
import soot.Unit;
import soot.toolkits.graph.UnitGraph;

/**
 * Tags static field and method references which are known to not
 * be the first ``active use'' of the associated class, and so for which
 * it is not necessary to check if the class needs to be initialized.
 */
public class ActiveUseTagger {

	private final ActiveUseAnalysis analysis;

	public ActiveUseTagger(Body body) {
		analysis = new ActiveUseAnalysis(body);
		tag(body);
	}

	public ActiveUseTagger(UnitGraph graph) {
		analysis = new ActiveUseAnalysis(graph);
		tag(graph.getBody());
	}

	public ActiveUseAnalysis getAnalysis() {
		return analysis;
	}

	private void tag(Body body) {
		for (Iterator i = body.getUnits().iterator(); i.hasNext(); ) {
			Stmt stmt = (Stmt)i.next();
			if (stmt.containsInvokeExpr())
				tagInvoke(stmt, stmt.getInvokeExprBox());
			if (stmt.containsFieldRef())
				tagField(stmt, stmt.getFieldRefBox());
		}
	}

	private void tagInvoke(Stmt stmt, ValueBox box) {
		Value value = box.getValue();
		if (!(value instanceof StaticInvokeExpr))
			return;
		StaticInvokeExpr expr = (StaticInvokeExpr)value;
		SootClass cl = expr.getMethod().getDeclaringClass();
		if (ActiveUseCheckStmt.isActiveUseCheck(stmt))
			cl = ActiveUseCheckStmt.getSootClass(stmt);
		if (!analysis.isCheckNeeded(stmt, cl))
			box.addTag(ActiveUseTag.v(false));
	}

	private void tagField(Stmt stmt, ValueBox box) {
		Value value = box.getValue();
		if (!(value instanceof FieldRef))
			return;
		SootField field = ((FieldRef)value).getField();
		if (field.isStatic()
		    && !analysis.isCheckNeeded(stmt, field.getDeclaringClass()))
			box.addTag(ActiveUseTag.v(false));
	}
}


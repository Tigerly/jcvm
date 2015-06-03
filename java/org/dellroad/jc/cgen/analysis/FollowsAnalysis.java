
//
// @COPYRIGHT@
//
// $Id: FollowsAnalysis.java,v 1.2 2004/12/19 21:01:04 archiecobbs Exp $
//

package org.dellroad.jc.cgen.analysis;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import soot.Body;
import soot.Unit;
import soot.toolkits.graph.CompleteUnitGraph;
import soot.toolkits.scalar.ArraySparseSet;
import soot.toolkits.scalar.FlowSet;
import soot.toolkits.scalar.ForwardFlowAnalysis;

/**
 * Analysis that determines which statements can be executed
 * after which other statements.
 */
public class FollowsAnalysis extends ForwardFlowAnalysis {

	public FollowsAnalysis(Body body) {
		super(new CompleteUnitGraph(body));
		doAnalysis();
	}

	public FollowsAnalysis(CompleteUnitGraph graph) {
		super(graph);
		doAnalysis();
	}

	/**
	 * Determine if <code>after</code> can execute after
	 * <code>before</code>.
	 */
	public boolean canFollow(Unit before, Unit after) {
		FlowSet inSet = (FlowSet)unitToBeforeFlow.get(after);
		return inSet.contains(before);
	}

	protected Object entryInitialFlow() {
		return new ArraySparseSet();
	}

	protected Object newInitialFlow() {
		return new ArraySparseSet();
	}

	protected void flowThrough(Object in, Object stmt, Object out) {
		FlowSet inFlow = (FlowSet)in;
		FlowSet outFlow = (FlowSet)out;
		inFlow.copy(outFlow);
		outFlow.add(stmt);
	}

	protected void merge(Object in1, Object in2, Object out) {
		FlowSet inSet1 = (FlowSet)in1;
		FlowSet inSet2 = (FlowSet)in2;
		FlowSet outSet = (FlowSet)out;
		inSet1.union(inSet2, outSet);
	}

	protected void copy(Object src, Object dst) {
		FlowSet srcSet = (FlowSet)src;
		FlowSet dstSet = (FlowSet)dst;
		srcSet.copy(dstSet);
	}

}


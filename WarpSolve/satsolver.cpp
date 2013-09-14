
/****************************************************************************************************

(c) 2013 Todd Bandrowsky  
All Rights Reserved

****************************************************************************************************/

#include "satsolver.h"
#include <iostream>

namespace satsolver {

	sat3solver::sat3solver() : verbose( true )
	{
		;
	}

	warpsolve::solution sat3solver::solve( sat3::cnfExpression& _expression )
	{
		warpsolve::solution sat3solution;
		warpsolve::solver sat3solver( _expression.clauses.size(), _expression.getMaxVariable() + 1 );

		if (verbose)
			std::cout << "Mark Alones" << std::endl;

		// step 0, set the isalones on all the clauses
		_expression.markAlones();

		if (verbose)
			std::cout << "Convert to Tables" << std::endl;

		// step 1, convert all of our clauses into tables
		for (auto ic = _expression.clauses.begin(); ic != _expression.clauses.end(); ic++) 
		{
			// grab the clause and get the number of distinct variables
			auto cl = *ic;	
			std::map< int, std::list<int> > distincts;
			std::list<int> variableNames;
			warpsolve::table solveTable( 3, 8 ); // probably ok with 8			

			for (int i = 0; i < 3; i++) {
				int nm = cl.bools[ i ].name ;
				solveTable.setColumnName( i, nm );
				if (distincts.count( nm ) == 0) {
					variableNames.push_back( nm );
				}
				distincts[ nm ].push_back( i );
			}

			// given the distinct variables, build up a table for this clause that has only rows of variables whose assignments satisfy this clause
			warpsolve::solutionIterator siter = warpsolve::solutionIterator::begin( variableNames, 1 );
			
			// for debugging
			//std::cout << std::endl << "Table" << std::endl;

			do 
			{
				warpsolve::solution s = siter.value();
				if (cl.check( s )) { // meaning that, this solution satisfies this clause
					std::vector<int> values;
					values.resize(3);
					bool doAdd = true;
					for (auto si : s.variablesAndValues) {
						for (auto sk = distincts[ si.first ].begin(); sk != distincts[ si.first ].end(); sk++) {
							// mark the variables that will be added to the row of the table... 
							// but, only if the free variables, that is, those with no dependency, would satisfy the clause on their own.
							sat3::boolInst& bi = cl.bools[ *sk ];
							if (bi.isAlone && (( bi.notted && si.second == true ) ||( !bi.notted && !si.second )))
								doAdd = false;
							values[ *sk ] = si.second;
						}
					}
					if (doAdd) {
				//		std::cout << s.toString() << std::endl;
						solveTable.addRow( values );
					}
				}
			} 
			while (siter.next());

			// and, add that table to our satsolver. 

			sat3solver.addTable( solveTable );
		}

		// step 2, solve them
		sat3solver.verbose = verbose;
		sat3solution = sat3solver.solve();

		if (sat3solution.valid) {
			sat3solution.works = _expression.check( sat3solution );
		}

		return sat3solution;
	}

}


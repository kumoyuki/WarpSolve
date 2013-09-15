
/****************************************************************************************************

(c) 2013 Todd Bandrowsky  
All Rights Reserved

****************************************************************************************************/

#if defined(WINVER)
#include "conio.h"
#else
#define _stricmp strcasecmp
#define __debugbreak() do{}while(false)
#endif

#include <iostream>
#include "solution.h"
#include "satsolver.h"
#include <cstring>

void solutionIterationTest();
void cnfClauseTest();
void cnfSolvingTestSingleTerm();
void cnfSolvingTestTripleTerm();
void cnfSolvingRealTests();
void cnfFindFailure();

int main( int argc, char **argv)
{
	for (int i = 0; i < argc; i++) {
		if (_stricmp( argv[i], "-itertest" ) == 0) {
			solutionIterationTest();
		} else if (_stricmp( argv[i], "-cnfclause" ) == 0) {
			cnfClauseTest();
		} else if (_stricmp( argv[i], "-cnfoneclause" ) == 0) {
			cnfSolvingTestSingleTerm();
		} else if (_stricmp( argv[i], "-cnfthreeclause" ) == 0) {
			cnfSolvingTestTripleTerm();
		} else if (_stricmp( argv[i], "-cnfrealtest" ) == 0) {
			cnfSolvingRealTests();
		} else if (_stricmp( argv[i], "-cnffindfailure" ) == 0) {
			cnfFindFailure();
		} 
	}
}

void solutionIterationTest()
{
	std::cout << "Solution Iteration Test" << std::endl;

	std::list<int> variables;
	variables.push_back( 1 );
	variables.push_back( 5 );
	variables.push_back( 7 );

	auto x = warpsolve::solutionIterator::begin( variables, 1 ); 

	do {
		auto y = x.value();
		std::cout << y.toString() << std::endl;
	} while (x.next());
}

void cnfClauseTest()
{
	std::cout << "CNF Clause Test 1" << std::endl;

	sat3::cnfEnumerator cnfe1( 3, 1 );

	bool icnfe1 = cnfe1.first();

	while (icnfe1) {
		auto expr = cnfe1.getExpression();
		std::cout << expr.toString() << std::endl;
		icnfe1 = cnfe1.next();
	}
}

void cnfSolvingTestSingleTerm()
{
	std::cout << "Solving Test 1 Term" << std::endl;

	sat3::cnfEnumerator cnfe( 3, 1 );

	bool icnfe = cnfe.first();
	satsolver::sat3solver solver;

	while (icnfe) {
		auto expr = cnfe.getExpression();
		std::cout << expr.toString();
		auto sol = solver.solve( expr );
		if (!sol.valid) {
			std::cout << "no solution" << std::endl;
		} else {
			std::cout << " -> ";
			std::cout << sol.toString() << std::endl;
		}

		icnfe = cnfe.next();
	}
}

void cnfSolvingTestTripleTerm()
{
	std::cout << "Solving Test 3 Term" << std::endl;

	sat3::cnfEnumerator cnfe2( 4, 3 );

	bool icnfe2 = cnfe2.first();
	satsolver::sat3solver solver2;

	while (icnfe2) {
		auto expr = cnfe2.getExpression();
		auto exprText = expr.toString();
		std::cout << exprText;

		if (exprText == "( !A || B || C)  && ( A || A || B)  && ( A || A || A) ") 
		 __debugbreak();
		
		auto sol = solver2.solve( expr );
		if (!sol.valid) {
			std::cout << "no solution" << std::endl;
		} else {
			std::cout << " -> ";
			std::cout << sol.toString() << std::endl;
		}

		icnfe2 = cnfe2.next();
	}
}

void cnfFindFailure()
{
	std::cout << "Solving Test Find Failure" << std::endl;

	satsolver::sat3solver solver3;

	int nClauses = 10;
	int nVariables = 4;
	int j = 0;
	bool fail = false;

	while (!fail) {

 		std::cout << "Test " << j << " For " << nClauses << " Clauses, " << nVariables << " Variables" << std::endl;
		
		auto expr = sat3::cnfExpression::getRandomExpression(nClauses,nVariables);
		auto exprText = expr.toString();

		std::cout << "Expression:" << std::endl;
		std::cout << exprText << std::endl;

//			if (exprText == "( !A || B || C)  && ( A || A || B)  && ( A || A || A) ") 
//				 __debugbreak();

		auto sol = solver3.solve( expr );

		std::cout << "Solution:" << std::endl;

		if (!sol.works) {
			std::cout << "***sat solver failed.  the following clauses do not comply with the incorrect solution below ***" << std::endl;
			for (auto s : sol.failingClauses) {
				std::cout << s << " ";
			}
			std::cout << std::endl;
		}

		if (!sol.valid) {
			std::cout << "no solution" << std::endl;
		} else {
			std::cout << sol.toString() << std::endl;
		}

		std::cout << std::endl;

	}

}

void cnfSolvingRealTests()
{
	std::cout << "Solving Test Real Test Term" << std::endl;

	satsolver::sat3solver solver3;

	int nClauses = 100;
	int nVariables = 10;

	for (int i = 0; i < 4; i++) {

		for (int j = 0; j < 3; j++) {

 			std::cout << "Test " << j << " For " << nClauses << " Clauses, " << nVariables << " Variables" << std::endl;
		
			auto expr = sat3::cnfExpression::getRandomExpression(nClauses,nVariables);
			auto exprText = expr.toString();

			std::cout << "Expression:" << std::endl;
			std::cout << exprText << std::endl;

//			if (exprText == "( !A || B || C)  && ( A || A || B)  && ( A || A || A) ") 
//				 __debugbreak();

			auto sol = solver3.solve( expr );

			std::cout << "Solution:" << std::endl;

			if (!sol.works) {
				std::cout << "***sat solver failed.  the following clauses do not comply with the incorrect solution below ***" << std::endl;
				for (auto s : sol.failingClauses) {
					std::cout << s << " ";
				}
				std::cout << std::endl;
			}

			if (!sol.valid) {
				std::cout << "no solution" << std::endl;
			} else {
				std::cout << sol.toString() << std::endl;
			}

			std::cout << std::endl;

		}

		nClauses *= 10;
		nVariables *= 10;
	}
}

// Local Variables:
// tab-width: 4
// End:

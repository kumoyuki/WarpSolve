
/****************************************************************************************************

(c) 2013 Todd Bandrowsky  
All Rights Reserved

****************************************************************************************************/

#include "sat3.h"

namespace sat3 {

	std::string boolInst::toString()
	{
		std::string retVal = notted ? "!" : "";
		retVal += warpsolve::variableConvert::toString( name );
		return retVal;
	}

	cnfClause::cnfClause()
	{
		for (int i = 0; i < 3; i++) {
			bools[i].notted = false;
			bools[i].name = 0;
			bools[i].isAlone = false;
		}
	}

	cnfClause::cnfClause( bool nottedA, int nameA, bool nottedB, int nameB, bool nottedC, int nameC)
	{
		bools[0].notted = nottedA;
		bools[0].name = nameA;
		bools[0].isAlone = true;
		bools[1].notted = nottedB;
		bools[1].name = nameB;
		bools[1].isAlone = true;
		bools[2].notted = nottedC;
		bools[2].name = nameC;
		bools[2].isAlone = true;
	}

	cnfClause::cnfClause(const cnfClause& src)
	{
		for (int i = 0; i < 3; i++) {
			bools[i] = src.bools[i];
		}
	}

	int cnfClause::getMaxVariable()
	{
		int maxVid = 0;
		for (int i = 0; i < 3; i++)  {
			if (bools[ i ].name > maxVid ) 
				maxVid = bools[ i ].name;
		}
		return maxVid;
	}

	bool cnfClause::check( warpsolve::solution& s )
	{
		bool success = false;
		for (int i = 0; i < 3 && !success; i++) {
			auto& b = bools[i];
			if (b.notted && !s.variablesAndValues[b.name])
				success = true;
			else if (!b.notted && s.variablesAndValues[b.name])
				success = true;
		}
		return success;
	}

	std::string cnfClause::toString()
	{
		std::string result;
		result = "( ";
		result += bools[0].toString();
		result += " || ";
		result += bools[1].toString();
		result += " || ";
		result += bools[2].toString();
		result += ") ";

		return result;
	}

	cnfClauseEnumerator::cnfClauseEnumerator() : maxVariables( 0 ),	counter(0), maxCounter(0)
	{
		;
	}

	cnfClauseEnumerator::cnfClauseEnumerator(  int _maxVariables )
	: 	maxVariables( _maxVariables ),
		counter(0)
	{

		int i = 0;
		maxCounter = maxVariables * 2;

		for (i = 1; i < 3; i++) {
			maxCounter *= (maxVariables * 2);
		}
	}

	cnfClauseEnumerator::cnfClauseEnumerator(const cnfClauseEnumerator& src)
	{
		maxVariables = src.maxVariables;
		maxCounter = src.maxCounter;
		counter = src.counter;
	}

	void cnfClauseEnumerator::first()
	{
		counter = 0;
	}

	bool cnfClauseEnumerator::next()
	{

		if (done())
			return false;

		counter++;

		return true;
	}

	bool cnfClauseEnumerator::done()
	{
		return counter >= maxCounter;
	}
	
	cnfClause cnfClauseEnumerator::getClause(  )
	{
		int variables[3];
		int nots[3];
		int i = 0;
		long lcounter = counter;
		long vtemplate = maxVariables * 2;

		while (i < 3) {
			long temp = lcounter % vtemplate;
			nots[ i ] =(temp % 2);
			temp /= 2;
			variables[ i ] = temp;
			lcounter /= vtemplate;
			i++;
		}

		cnfClause ret( nots[2], variables[2], nots[1], variables[1], nots[0], variables[0] );
		return ret;
	}

	cnfExpression::cnfExpression()
	{
		;
	}

	int cnfExpression::getMaxVariable()
	{
		int maxVid = 0;
		for (auto i = clauses.begin(); i != clauses.end(); i++) {
			int mvcandidate = i->getMaxVariable();
			if (mvcandidate > maxVid) 
				maxVid = mvcandidate;
		}
		return maxVid;
	}

	bool cnfExpression::check( warpsolve::solution& s )
	{
		bool success = true;
		for (auto& iter : clauses) {
			success = iter.check( s );
			if (!success) {
				s.failingClauses.push_back( iter.toString() );
			}
		}
		return success;
	}

	void cnfExpression::markAlones(  )
	{
		std::map<int,int> useCounts;

		// get counts of all the variables in all of the clauses
		for (auto& clause : clauses) {

			// the same variable might be in the same clause twice
			std::map<int,int> vars;

			for (int i = 0; i < 3; i++) {
				int name = clause.bools[i].name;
				vars[ name ] = true;
			}

			for (auto variable : vars ) {
				if (useCounts.count( variable.first ) == 0) {
					useCounts[ variable.first ] = 1;
				} else {
					useCounts[ variable.first ]++;
				}
			}
		}

		// now, go back through the clauses and mark the variables
		for (auto& clause : clauses) {
			for (int i = 0; i < 3; i++) {
				int name = clause.bools[i].name;
				clause.bools[ i ].isAlone = (useCounts[ name ] == 1);
			}
		}
	}

	cnfExpression cnfExpression::getRandomExpression( int _clauses, int _variableRange )
	{
		cnfExpression expr;
		int randMax = _variableRange;
		int a,b,c;

		// then get the rest
		for (int clausei = 0; clausei < _clauses; clausei++) {
			a = rand() % randMax;

			do {
				b = rand() % randMax;
			}
			while(b == a);

			do {
				c = rand() % randMax;
			}
			while(c == a || c == b);

			cnfClause clause( rand() % 2, a, rand() % 2, b, rand() % 2, c );
			expr.clauses.push_back( clause );
		}

		expr.clauses.sort( [](const cnfClause& a, const cnfClause& b)
            {
				if (a.bools[0].name < b.bools[0].name)
					return true;
				if (a.bools[0].name > b.bools[0].name)
					return false;
				if (a.bools[1].name < b.bools[1].name)
					return true;
				if (a.bools[1].name > b.bools[1].name)
					return false;
				return false;
            } );

		return expr;
	}
	
	std::string cnfExpression::toString()
	{
		std::string result = "";
		bool addAnd = false;

		for (auto iter = clauses.begin(); iter != clauses.end(); iter++) {
			if (addAnd) {
				result += " && ";
			} else {
				addAnd = true;
			}
			result += iter->toString();
		}
		return result;
	}

	cnfEnumerator::cnfEnumerator( int _maxVariables, int _maxClauses )
		: maxVariables( _maxVariables ),
		  maxClauses( _maxClauses )
	{
		enumerators.resize( _maxClauses );

		cnfClauseEnumerator clauseEnumerator( maxVariables );

		for (int i = 0; i < _maxClauses; i++) {
			 enumerators[ i ] = clauseEnumerator;
		}
	}

	bool cnfEnumerator::first()
	{
		for (int i = 0; i < maxClauses; i++) {
			 enumerators[ i ].first();
		}
		return true;
	}

	bool cnfEnumerator::next()
	{
		bool carry = true;
		bool carried = true;

		int i = 0;

		while (i < maxClauses && enumerators[ i ].done()) {
			i++;
		}

		if (i == maxClauses) {
			return false;
		} else {
			enumerators[ i ].next();
			i--;
			while (i >= 0) {
				enumerators[ i ].first();
				i--;
			}
		}

		return true;

	}

	cnfExpression cnfEnumerator::getExpression()
	{
		cnfExpression expression;
		cnfClause clause;

		for (int i = 0; i < maxClauses; i++) {
			clause = enumerators[ i ].getClause();
			expression.clauses.push_back( clause );
		}

		return expression;
	}

}

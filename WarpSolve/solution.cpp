
/****************************************************************************************************

(c) 2013 Todd Bandrowsky  
All Rights Reserved

****************************************************************************************************/

#include "solution.h"

namespace warpsolve 
{

	std::string variableConvert::toString( int _identifierId )
	{
		char identifier[2];
		identifier[1] = 0;
		char buff[256];
		std::string ret;
		int var = _identifierId % 26;
		identifier[0] = var + 'A';
		int id = _identifierId / 26;
		if (id > 0) {
			sprintf( buff, "%s%d", identifier, id );
		} else {
			sprintf( buff, "%s", identifier, id );
		}
		ret = buff;
		return ret;
	}
	
	std::string solution::toString()
	{
		std::string result = "";
		for (auto it=variablesAndValues.begin(); it!=variablesAndValues.end(); ++it) {
			char buff[256];
			sprintf(buff, "%d", it->second );
			result += variableConvert::toString( it->first );
			result += " = ";
			result += buff;
			result += ". ";
		}
		return result;
	}

	solutionIterator::solutionIterator( std::list<int>& _variableNames, int _maxValue )  
	{
		maxVariable = _variableNames.size();
		maxValues.resize( maxVariable );
		variables.resize( maxVariable );
		variableNames.resize( maxVariable );
		int idx = 0;
		for (auto i = _variableNames.begin(); i != _variableNames.end(); i++) {
			variableNames[ idx ] = *i;
			maxValues[ idx ] = _maxValue;
			variables[ idx ] = 0;
			idx++;
		}
	}

	solutionIterator::solutionIterator( std::list< std::pair<int, int> >& _variablesAndMaxValues )
	{
		maxVariable = _variablesAndMaxValues.size();;
		maxValues.resize( maxVariable );
		variables.resize( maxVariable );
		variableNames.resize( maxVariable );
		int idx = 0;
		for (auto i = _variablesAndMaxValues.begin(); i != _variablesAndMaxValues.end(); i++) {
			variableNames[ idx ] = i->first;
			maxValues[ idx ] = i->second;
			idx++;
		}
	}

	solutionIterator solutionIterator::begin( std::list<int>& _variableNames, int _maxValue )
	{
		solutionIterator iter( _variableNames, _maxValue );
		return iter;
	}

	solutionIterator solutionIterator::begin( std::list< std::pair<int, int> >& _variablesAndMaxValues )
	{
		solutionIterator iter( _variablesAndMaxValues );
		return iter;
	}

	bool solutionIterator::next()
	{
		bool carry = true;
		bool carried = true;

		int i = 0;

		while (i < maxVariable && variables[ i ] == maxValues[ i ]) {
			i++;
		}

		if (i == maxVariable) {
			return false;
		} else {
			variables[ i ]++;
			i--;
			while (i >= 0) {
				variables[ i ] = 0;
				i--;
			}
		}

		return true;
	}

	solution solutionIterator::value()
	{
		solution s;
		s.valid = true;
		for (int i = 0; i < maxVariable; i++) {
			s.variablesAndValues[ variableNames[ i ] ] = variables[ i ];
		}
		return s;
	}

};

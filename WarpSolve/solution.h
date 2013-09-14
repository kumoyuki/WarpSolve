
/****************************************************************************************************

(c) 2013 Todd Bandrowsky  
All Rights Reserved

****************************************************************************************************/

#ifndef SOLUTION_H
#define SOLUTION_H

#include <vector>
#include <map>
#include <list>
#include <string>

namespace warpsolve 
{

	class variableConvert
	{
	public:
		static std::string toString( int _identifierId );
	};

	class solution
	{
	public:
		bool valid;
		bool works;
		std::list<std::string> failingClauses;
		std::map< int, int > variablesAndValues;
		std::string toString();
	};

	class solutionIterator
	{
		int maxVariable;
		std::vector<int> maxValues;
		std::vector<int> variables;
		std::vector<int> variableNames;
		solutionIterator( std::list<int>& _variableNames, int _maxValue );
		solutionIterator( std::list< std::pair<int, int> >& _variablesAndMaxValues );

	public:

		static solutionIterator begin( std::list<int>& _variableNames, int _maxValue );
		static solutionIterator begin( std::list< std::pair<int, int> >& _variablesAndMaxValues );
		bool next();
		solution value();

	};

}

#endif
// Local Variables:
// tab-width: 4
// End:

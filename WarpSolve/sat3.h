
/****************************************************************************************************

(c) 2013 Todd Bandrowsky  
All Rights Reserved

****************************************************************************************************/

#ifndef satsolverH
#define satsolverH

#include <string>
#include <list>
#include <vector>
#include <algorithm>

#include "solution.h"

namespace sat3 {

	class boolInst
	{
	public:

		bool notted;
		int name;
		bool isAlone;

		std::string toString();
	};

	class cnfClause
	{
	public:
		boolInst bools[3];

		cnfClause();
		cnfClause( bool nottedA, int nameA, bool nottedB, int nameB, bool nottedC, int nameC);
		cnfClause(const cnfClause& src);

		int getMaxVariable();
		std::string toString();

		bool check( warpsolve::solution& s );
	};

	class cnfClauseEnumerator
	{
		long counter;
		long maxCounter;
		int maxVariables;

	public:

		cnfClauseEnumerator();
		cnfClauseEnumerator(const cnfClauseEnumerator& src);
		cnfClauseEnumerator(  int _maxVariables );

		void first();
		bool next();
		bool done();
		cnfClause getClause(  );

	};

	class cnfExpression
	{
	public:

		std::list<cnfClause> clauses;

		cnfExpression();
		int getMaxVariable();

		static cnfExpression getRandomExpression( int _clauses, int _variableRange );
		std::string toString();

		bool check( warpsolve::solution& s );
		void markAlones();

	};

	class cnfEnumerator
	{
		std::vector<cnfClauseEnumerator> enumerators;
		int maxVariables;
		int maxClauses;

	public:

		cnfEnumerator( int _maxVariables, int _maxClauses );
		bool first();
		bool next();

		cnfExpression getExpression();
	};

}

//---------------------------------------------------------------------------
#endif


// Local Variables:
// tab-width: 4
// End:


/****************************************************************************************************

(c) 2013 Todd Bandrowsky  
All Rights Reserved

****************************************************************************************************/

#ifndef WARPSOLVE_H
#define WARPSOLVE_H

#include <vector>
#include <list>
#include <map>

#include "solution.h"

namespace warpsolve 
{

	// represents a set of variable assignments that satisfies an associated clause

	class table 
	{

		int tableIdx;
		int columns;
		int rows;
		int maxrows;
		std::vector<int> data;
		std::vector<int> columnNames;
		std::vector<int> matches;
		bool isAlone;
		std::list<int> matchedTables;

	public:

		table();
		table( int _columns, int _maxrows );

		int setTableIndex( int _index );

		inline int getTableIdx() const
		{
			return tableIdx;
		}

		void setColumnNames( std::vector<int>& _names );

		inline int setColumnName( int _column, int _value )
		{
			columnNames[ _column ] = _value;
			return _value;
		}

		inline int getColumnName( int _column ) const
		{
			return columnNames[ _column ];
		}

		inline int getColumnCount() const
		{
			return columns;
		}

		inline int getRowCount() const
		{
			return rows;
		}

		inline int getMatchedTableCount() const
		{
			return matchedTables.size();
		}

		inline std::list<int> getMatchedTables() const
		{
			return matchedTables;
		}

		inline int set( int _row, int _col, int _value )
		{
			data[ _row * columns + _col ] = _value;
			return _value;
		}

		inline int get( int _row, int _col ) const
		{
			int value = data[ _row * columns + _col ];
			return value;
		}

		inline int getByColumnName( int _row, int _colName ) const
		{
			int col = columnNames[ _colName ];
			int value = data[ _row * columns + col ];
			return value;
		}

		inline int setByColumnName( int _row, int _colName, int _value )
		{
			int col = columnNames[ _colName ];
			data[ _row * columns + col ] = _value;
			return _value;
		}

		inline bool getAlone() const
		{
			return isAlone;
		}

		inline bool setAlone( bool _alone )
		{
			isAlone = _alone;
			return isAlone;
		}

		bool addRow( std::vector<int>& _items );

		int getFirstSolutionRow() const;
		std::list<int> getSolutionRows() const;
		bool join( table& _dest );
		std::list< std::pair<int,int> > getMatchingColumns( table& _dest ) const;

	};

	// a solver does the work of creating a solution for a set of tables
	
	class solver 
	{
		int tableCount;
		std::vector< table > tables;
		std::vector< std::map<int, bool> > tableIdsByVariableId;

		void joinToSolution( int _tableIdx, int _rowId, solution& _solution, std::map<int, bool>& _tablesUsed );
		solution createEmptySolution();
		solution createSolutionForSingleTable();
		solution createSolutionForMultipleTables();

	public:

		bool verbose;
		solver( int _numberOfTables, int _maxVariables );
		void addTable( table& _table );
		solution solve();
		
	};

}

#endif

// Local Variables:
// tab-width: 4
// End:

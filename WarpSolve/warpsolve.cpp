
/****************************************************************************************************

(c) 2013 Todd Bandrowsky  
All Rights Reserved

****************************************************************************************************/


#include "warpsolve.h"
#include <iostream>

namespace warpsolve {


	table::table() : columns(0), maxrows(0), rows(0), isAlone( false ), matchedTables(0)
	{
		;
	}

	table::table( int _columns, int _maxrows ) : columns( _columns ), maxrows( _maxrows ), rows(0), matchedTables(0)
	{
		data.resize( maxrows * columns );
		matches.resize( maxrows );
		columnNames.resize( columns );
		for (int i = 0; i < maxrows; i++)
			matches[ i ] = 0;
	}

	int table::setTableIndex( int _index )
	{
		tableIdx = _index;
		return tableIdx;
	}

	void table::setColumnNames( std::vector<int>& _names )
	{
		columnNames = _names;
	}

	bool table::addRow( std::vector<int>& _items )
	{
		if (rows == maxrows)
			return false;

		_items.resize( columns );
		int start = columns * rows;
		int end = start + columns;
		int i = 0;
		while (start < end) {
			data[ start ] = _items[ i ];
			i++;
			start++;
		}
		rows++;

		return true;
	}

	int table::getFirstSolutionRow() const
	{
		int rc = rows;
		int sz = matchedTables.size();
		for (int i = 0; i < rc; i++) {
			if (matches[ i ] == sz)
				return i;
		}
		return -1;
	}

	std::list<int> table::getSolutionRows() const
	{
		std::list<int> solutionRows;
		int rc = rows;
		int sz = matchedTables.size();
		for (int i = 0; i < rc; i++) {
			if (matches[ i ] == sz) {
				solutionRows.push_back( i );
			}
		}
		return solutionRows;
	}

	std::list< std::pair<int,int> > table::getMatchingColumns( table& _dest ) const
	{
		std::list< std::pair<int, int> > matchingColumns;

		for (int colMeIdx = 0; colMeIdx < columns; colMeIdx++) {
			int columnMe = getColumnName( colMeIdx );
			for (int colThemIdx = 0; colThemIdx < _dest.columns; colThemIdx++) {
				int columnThem = _dest.getColumnName( colThemIdx );
				if (columnThem == columnMe) {
					matchingColumns.push_back( std::pair<int, int> ( colMeIdx, colThemIdx ) );
				}
			}
		}

		return matchingColumns;
	}

	bool table::join( table& _dest )
	{
		// in the future we could do a hash join, but this nested loops should get the job done for the moment
		// first, find matching columns

		std::list< std::pair<int, int> >& matchingColumns = getMatchingColumns( _dest );

		if (matchingColumns.size() == 0) {
			return true;
		}

		// then, do the join... this isn't a real join per se because we are only tallying matching rows

		matchedTables.push_back( _dest.getTableIdx() );
		_dest.matchedTables.push_back( getTableIdx() );

		bool everMatched = false;

		std::map<int,bool> matched;

		for (int i = 0; i < this->rows; i++) {
			bool rowMatched = false;
			for (int j = 0; j < _dest.rows; j++) {
				if (matched.count(j)==0) {
					bool allMatches = true;
					for (auto colmatch = matchingColumns.begin(); colmatch != matchingColumns.end(); colmatch++) {
						if ( get( i, colmatch->first ) != _dest.get( j, colmatch->second ) ) {
							allMatches = false;
							break;
						}
					}

					if (allMatches) {
						rowMatched = true;
						matched[j] = true;
						_dest.matches[ j ]++;
						everMatched = true;
					}
				}
			}
			if (rowMatched) {
				matches[ i ]++;
			}
		}

		return everMatched;
	}


	solution solver::createEmptySolution()
	{
		solution s;

		s.valid = true;
		s.works = true;

		return s;
	}

	solution solver::createSolutionForSingleTable()
	{
		// here, just grab the first row of a table and shove it in there
		solution s;

		s.valid = true;
		s.works = true;

		auto src = tables[ 0 ];

		int r = src.getFirstSolutionRow();

		if (r < 0) {
			s.valid = false;
			return s;
		}

		int c = src.getColumnCount();

		for (int ic = 0; ic < c; ic++) {
			int name = src.getColumnName( ic );
			int value = src.get( r, ic );
			if (s.variablesAndValues.count( name ) == 0) {
				s.variablesAndValues[ name ] = value;
			} else if (s.variablesAndValues[ name ] != value) {
				s.valid = false;
			}
		}

		return s;
	}

	void solver::joinToSolution( int _tableIdx, int _rowId, solution& _solution, std::map<int, bool>& _tablesUsed )
	{
		auto &src = tables[ _tableIdx ];
		_tablesUsed[ src.getTableIdx() ] = true;
		int colCount = src.getColumnCount();

		for (int idx = 0; idx < colCount; idx++) 
		{
			int name = src.getColumnName( idx );
			int value = src.get( _rowId, idx );
			_solution.variablesAndValues[ name ] = value;
		}

		std::list<int> tableList = src.getMatchedTables();

		for (auto destTableIdx : tableList) {

			if (_tablesUsed.count( destTableIdx ) > 0) 
				continue;

			auto &dest = tables[ destTableIdx ];
			auto destSolutionRows = dest.getSolutionRows();
			auto matchingColumns = src.getMatchingColumns( dest );

			for (auto destRowId : destSolutionRows) {
				bool allMatched = true;
				for (auto matchingColumnPair : matchingColumns) {
					if (src.get( _rowId, matchingColumnPair.first ) != dest.get( destRowId, matchingColumnPair.second)) {
						allMatched = false;
						break;
					}
				}

				// hey hey, we have a solution
				if (allMatched) {
					joinToSolution( destTableIdx, destRowId, _solution, _tablesUsed );
					break;
				}
			}
		}
	}
	
	solution solver::createSolutionForMultipleTables()
	{
		solution s;

		s.valid = true;
		s.works = true;

		// pass 1, create virtual joins on tables

		std::map<int,int> tablesUsed;

		if (verbose) {
			std::cout << "Virtual Join.";
		}

		int nTables = tableCount;
		for (int i = 0; i < nTables; i++) {
			auto& src = tables[ i ];

			if (verbose && i % 10 == 0) {
				std::cout << ".";
			}

			int colcnt = src.getColumnCount();
			for (int colidx = 0; colidx < colcnt; colidx++) {
				int colname = src.getColumnName( colidx );
				auto& tableList = tableIdsByVariableId[ colname ];
				for (auto tableIter = tableList.begin(); tableIter != tableList.end(); tableIter++) {
					int tableId = tableIter->first;

					if (tableId == i)
						continue;

					int tableIdA = i;
					int tableIdB = tableId;
					int tableKey = 0;

					if (i > tableId) {
						int temp = tableIdB;
						tableIdB = i;
						tableIdA = temp;
					}

					tableKey = tableCount * tableIdA + tableIdB;
						
					if (tablesUsed.count( tableKey ) == 0) {
						auto& dest = tables[ tableId ];
						bool success = src.join( dest );
						tablesUsed[ tableKey ] = 1;
						if (!success) {
							s.valid = false;
							return s;
						}
					}
				}
			}
		}

		if (verbose) {
			std::cout << std::endl;
			std::cout << "Resolve Joins.";
		}

		// pass 2, resolve virtual joins into a solution

		// joinToSolution( int _tableIdx, int _rowId, solution& _solution, std::map<int, bool>& _tablesUsed )

		std::map<int, bool> localTablesUsed;

		for (int i = 0; i < nTables; i++) {

			auto& src = tables[ i ];
			int idx = src.getTableIdx(); 

			if (localTablesUsed.count( idx ) == 0) 
			{
				int r = src.getFirstSolutionRow();

				if (r < 0) {

					if (verbose) {
						std::cout << std::endl;
					}

					s.valid = false;
					return s;
				}

				joinToSolution( idx, r, s, localTablesUsed );
 
				if (verbose) {
					std::cout << localTablesUsed.size() << ". ";
				}

			}			
		}

		if (verbose) {
			std::cout << std::endl << "complete" << std::endl;
		}

		return s;
	}

	solver::solver( int _numberOfTables, int _maxVariables )
	{
		tableCount = 0;
		tables.resize( _numberOfTables );
		tableIdsByVariableId.resize( _maxVariables );
	}

	void solver::addTable( table& _table )
	{
		tables[ tableCount ] = _table;
		int tableId = tableCount;
		tableCount++;
		tables[ tableId ].setTableIndex( tableId );
		int c = _table.getColumnCount();
		for (int i = 0; i < c; i++) {
			int col = _table.getColumnName(i);
			tableIdsByVariableId[ col ][ tableId ] = true;
		}
	}

	solution solver::solve()
	{
		solution s;

		// if there is only one table, that tables contents are the solution;

		int c = tableCount;

		if (c == 0) {
			s = createEmptySolution();
		} else if (c == 1) {
			s = createSolutionForSingleTable();
		} else {
			s = createSolutionForMultipleTables();
		}

		return s;
	}
		





















}
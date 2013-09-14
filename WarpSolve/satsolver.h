
/****************************************************************************************************

(c) 2013 Todd Bandrowsky  
All Rights Reserved

****************************************************************************************************/


#include "sat3.h"
#include "warpsolve.h"

namespace satsolver {

	class sat3solver {
	public:

		bool verbose;

		sat3solver();
		warpsolve::solution solve( sat3::cnfExpression& _expression );

	};
	
}
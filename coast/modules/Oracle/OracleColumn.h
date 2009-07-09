/*
 * OracleColumn.h
 *
 *  Created on: Jul 8, 2009
 *      Author: hum
 */

#ifndef ORACLECOLUMN_H_
#define ORACLECOLUMN_H_

struct OracleColumn {
	enum EColumnDesc {
		// dont use 0 since it may be considered to be a char*
		eColumnName = 1,
		eColumnType = 2,
		eDataLength = 3,
		eRawBuf = 4,
		eDefHandle = 5,
		eIndicator = 6,
		eScale = 7,
		eEffectiveLength = 8,
		eInOutType = 9
	};
};

#endif /* ORACLECOLUMN_H_ */

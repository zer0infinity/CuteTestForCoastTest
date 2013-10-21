// Scheduler.cpp: implementation of the Scheduler class.
//
//////////////////////////////////////////////////////////////////////

#include "Scheduler.h"
#include "SystemBase.h"
#include "ITOString.h"

using namespace coast;

Scheduler::Scheduler() {
}

Scheduler::~Scheduler() {
}

String Scheduler::currTimeInDayHourMin() {
	// convert the timeDate from C-Format into DayHourMin
	// C-Format		"WED JUL 07 17:27:04 1999"
	//               ---        -- --
	//
	// DayHourMin	"WED 17:27"
	time_t now;
	time(&now);
	struct tm time;
	String timeDate;
	system::LocalTime(&now, &time);
	system::AscTime(&time, timeDate);

	String result = timeDate.SubString(0, 3); // Day of week
	result.Append(" ");
	result.Append(timeDate.SubString(11, 2)); // Hour
	result.Append(":");
	result.Append(timeDate.SubString(14, 2)); // Min
	result.ToUpper();
	return (result);
}

String Scheduler::convertFromDayHourMinInDecimal(String timeDate) {
	// convert the timeDate from DayHourMin-Format into Decimal-Format
	// DAY	:	MON = 1, TUE = 2, WED = 3, THU = 4, FRI = 5, SAT = 6, SUN = 6
	// HOUR	:	00..23
	// MIN	:	00..60
	// WED 10:27 -----> 31027

	bool statusOk = true;
	String value;

	// Analyze the day
	if (timeDate.CompareN("MON", 3, 0) == 0) {
		value.Append("1");
	} else if (timeDate.CompareN("TUE", 3, 0) == 0) {
		value.Append("2");
	} else if (timeDate.CompareN("WED", 3, 0) == 0) {
		value.Append("3");
	} else if (timeDate.CompareN("THU", 3, 0) == 0) {
		value.Append("4");
	} else if (timeDate.CompareN("FRI", 3, 0) == 0) {
		value.Append("5");
	} else if (timeDate.CompareN("SAT", 3, 0) == 0) {
		value.Append("6");
	} else if (timeDate.CompareN("SUN", 3, 0) == 0) {
		value.Append("7");
	} else {
		statusOk = false;
	}

	// Analyze the hour
	if (statusOk) {
		char td4 = timeDate.At(4), td5 = timeDate.At(5);
		if (isdigit(td4) && isdigit(td5)) {
			long val = (td4 - '0') * 10 + (td5 - '0');
			if (val < 24) {
				value.Append(timeDate.SubString(4, 2));
			} else {
				statusOk = false;
			}
		} else {
			statusOk = false;
		}
	}

	// Analyze the minutes
	if (statusOk) {
		char td7 = timeDate.At(7), td8 = timeDate.At(8);
		if (isdigit(td7) && isdigit(td8)) {
			long val = (td7 - '0') * 10 + (td8 - '0');
			if (val < 60) {
				value.Append(timeDate.SubString(7, 2));
			} else {
				statusOk = false;
			}
		} else {
			statusOk = false;
		}
	}

	if (statusOk) {
		return value;
	} else {
		return "";
	}
}

String Scheduler::CurrTimeDateInDecimal() {
	// CTimeDate = "Wed Apr 07 17:27:04 1999
	// decimal	 = "199904071727"
	time_t now;
	time(&now);
	struct tm result;
	String cTimeDate;
	system::LocalTime(&now, &result);
	system::AscTime(&result, cTimeDate);

	bool statusOk = true;
	String value;

	// Analyze the year
	char td20 = cTimeDate.At(20), td21 = cTimeDate.At(21), td22 = cTimeDate.At(22), td23 = cTimeDate.At(23);

	if (isdigit(td20) && isdigit(td21) && isdigit(td22) && isdigit(td23)) {
		value.Append(cTimeDate.SubString(20, 4));
	} else {
		statusOk = false;
	}

	// Analyze the month
	if (statusOk) {
		cTimeDate.ToUpper();
		;
		if (cTimeDate.CompareN("JAN", 3, 4) == 0) {
			value.Append("01");
		} else if (cTimeDate.CompareN("FEB", 3, 4) == 0) {
			value.Append("02");
		} else if (cTimeDate.CompareN("MAR", 3, 4) == 0) {
			value.Append("03");
		} else if (cTimeDate.CompareN("APR", 3, 4) == 0) {
			value.Append("04");
		} else if (cTimeDate.CompareN("MAY", 3, 4) == 0) {
			value.Append("05");
		} else if (cTimeDate.CompareN("JUN", 3, 4) == 0) {
			value.Append("06");
		} else if (cTimeDate.CompareN("JUL", 3, 4) == 0) {
			value.Append("07");
		} else if (cTimeDate.CompareN("AUG", 3, 4) == 0) {
			value.Append("08");
		} else if (cTimeDate.CompareN("SEP", 3, 4) == 0) {
			value.Append("09");
		} else if (cTimeDate.CompareN("OCT", 3, 4) == 0) {
			value.Append("10");
		} else if (cTimeDate.CompareN("NOV", 3, 4) == 0) {
			value.Append("11");
		} else if (cTimeDate.CompareN("DEC", 3, 4) == 0) {
			value.Append("12");
		} else {
			statusOk = false;
		}
	}

	// Analyze the day
	if (statusOk) {
		char td8 = cTimeDate.At(8), td9 = cTimeDate.At(9);
		if (isdigit(td8) && isdigit(td9)) {
			long val = (td8 - '0') * 10 + (td9 - '0');
			if (val <= 31) {
				value.Append(cTimeDate.SubString(8, 2));
			} else {
				statusOk = false;
			}
		} else {
			statusOk = false;
		}
	}

	// Analyze the hour
	if (statusOk) {
		char td11 = cTimeDate.At(11), td12 = cTimeDate.At(12);
		if (isdigit(td11) && isdigit(td12)) {
			long val = (td11 - '0') * 10 + (td12 - '0');
			if (val < 24) {
				value.Append(cTimeDate.SubString(11, 2));
			} else {
				statusOk = false;
			}
		} else {
			statusOk = false;
		}
	}

	// Analyze the min
	if (statusOk) {
		char td14 = cTimeDate.At(14), td15 = cTimeDate.At(15);
		if (isdigit(td14) && isdigit(td15)) {
			long val = (td14 - '0') * 10 + (td15 - '0');
			if (val < 60) {
				value.Append(cTimeDate.SubString(14, 2));
			} else {
				statusOk = false;
			}
		} else {
			statusOk = false;
		}
	}

	if (statusOk) {
		return value;
	} else {
		return "";
	}
}

String Scheduler::ConvertFromSingleTimeInDecimal(String singleTime) {
	// SingleTime = "1999-04-08 14:30"
	// decimal	  = 199904081439
	String result = singleTime.SubString(0, 4); // Year
	result.Append(singleTime.SubString(5, 2)); // Month
	result.Append(singleTime.SubString(8, 2)); // Day
	result.Append(singleTime.SubString(11, 2)); // Hour
	result.Append(singleTime.SubString(14, 2)); // Min

	// Check if the result contains only digits
	long len = result.Length();
	for (long i = 0; i < len; i++) {
		if (!isdigit(result[i])) {
			result = "";
			break;
		}
	}

	return result;
}

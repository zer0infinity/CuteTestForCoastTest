// Scheduler.h: interface for the Scheduler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCHEDULER_H__7F6C16A2_D25E_11D3_9CBC_00C026FFE74C__INCLUDED_)
#define AFX_SCHEDULER_H__7F6C16A2_D25E_11D3_9CBC_00C026FFE74C__INCLUDED_


class String;

class Scheduler
{
public:
	virtual ~Scheduler();

	static String currTimeInDayHourMin();
	static String convertFromDayHourMinInDecimal( String timeDate );
	static String CurrTimeDateInDecimal();
	static String ConvertFromSingleTimeInDecimal( String singleTime );

private:
	// static only, not instantiated
	Scheduler();
};

#endif // !defined(AFX_SCHEDULER_H__7F6C16A2_D25E_11D3_9CBC_00C026FFE74C__INCLUDED_)

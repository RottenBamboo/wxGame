#pragma once
#include "stdafx.h"
class wxTimer
{
public:
	wxTimer():mSecondsPerCount(0.0),mDeltaTime(0),mStopTimeStamp(0),\
		mPausedTime(0), mBaseTime(0), mCurrTimeStamp(clock()), mPrevTimeStamp(clock())
	{
		long_int64 clockPerSecond = CLOCKS_PER_SEC;
		mSecondsPerCount = 1.0f / (double)clockPerSecond;
	}
	void Tick();
	void Reset();
	void Stop();
	void Start();
	double DeltaTime();
	~wxTimer();
private:
	double mSecondsPerCount;
	double mDeltaTime;
	time_t mBaseTime;
	time_t mPausedTime;
	time_t mStopTimeStamp;
	time_t mPrevTimeStamp;
	time_t mCurrTimeStamp;
	bool mStopped;
};


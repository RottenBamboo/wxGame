#pragma once
#include "stdafx.h"
class wxTimer
{
public:
	wxTimer():mSecondsPerCount(0.0),mDeltaTime(0),mStopTimeStamp(0),\
		mPausedTime(0), mBaseTime(0), mCurrTimeStamp(0), mPrevTimeStamp(0)
	{}
	void Tick();
	void Reset();
	void Stop();
	void Start();
	time_t DeltaTime();
	~wxTimer();
private:
	double mSecondsPerCount;
	time_t mDeltaTime;
	time_t mBaseTime;
	time_t mPausedTime;
	time_t mStopTimeStamp;
	time_t mPrevTimeStamp;
	time_t mCurrTimeStamp;
	bool mStopped;
};


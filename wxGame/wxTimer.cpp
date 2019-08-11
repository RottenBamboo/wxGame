#include "wxTimer.h"

void wxTimer::Tick()
{
	if (mStopped)
	{
		mDeltaTime = 0.0f;
		return;
	}
	tm temp = tm();
	gmtime_s(&temp, &mCurrTimeStamp);
	mDeltaTime = mCurrTimeStamp - mPrevTimeStamp;
	mPrevTimeStamp = mCurrTimeStamp;
	if (mDeltaTime < 0.0f)
	{
		mDeltaTime = 0.0f;
	}
}

void wxTimer::Reset()
{
	tm temp = tm();
	gmtime_s(&temp, &mCurrTimeStamp);

	mBaseTime = mCurrTimeStamp;
	mPrevTimeStamp = mCurrTimeStamp;
	mStopTimeStamp = 0;
	mStopped = false;
}

void wxTimer::Stop()
{
	if (!mStopped)
	{
		tm temp = tm();
		gmtime_s(&temp, &mCurrTimeStamp);

		mStopTimeStamp = mCurrTimeStamp;
		mStopped = true;
	}
}

void wxTimer::Start()
{
	tm temp = tm(); 
	time_t startTime;
	gmtime_s(&temp, &startTime);  

	if (mStopped)
	{
		mPausedTime += (startTime - mStopTimeStamp);

		mPrevTimeStamp = startTime;
		mStopTimeStamp = 0;
		mStopped = false;
	}
}

time_t wxTimer::DeltaTime()
{
	return mDeltaTime;
}

wxTimer::~wxTimer()
{
}

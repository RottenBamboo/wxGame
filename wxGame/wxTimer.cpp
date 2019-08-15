#include "wxTimer.h"

void wxTimer::Tick()
{
	if (mStopped)
	{
		mDeltaTime = 0.0f;
		return;
	}
	mCurrTimeStamp = clock();
	mDeltaTime = (mCurrTimeStamp - mPrevTimeStamp) * mSecondsPerCount;
	mPrevTimeStamp = mCurrTimeStamp;
	if (mDeltaTime < 0.0f)
	{
		mDeltaTime = 0.0f;
	}
}

void wxTimer::Reset()
{
	mCurrTimeStamp = clock();

	mBaseTime = mCurrTimeStamp;
	mPrevTimeStamp = mCurrTimeStamp;
	mStopTimeStamp = 0;
	mStopped = false;
}

void wxTimer::Stop()
{
	if (!mStopped)
	{
		mCurrTimeStamp = clock();

		mStopTimeStamp = mCurrTimeStamp;
		mStopped = true;
	}
}

void wxTimer::Start()
{
	time_t startTime;
	startTime = clock();

	if (mStopped)
	{
		mPausedTime += (startTime - mStopTimeStamp);

		mPrevTimeStamp = startTime;
		mStopTimeStamp = 0;
		mStopped = false;
	}
}

double wxTimer::DeltaTime()
{
	return mDeltaTime;
}

wxTimer::~wxTimer()
{
}

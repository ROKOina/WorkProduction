#pragma once

#include <windows.h>

class HighResolutionTimer
{
public:
	HighResolutionTimer() : deltaTime_(-1.0), pausedTime_(0), isStopped_(false)
	{
		LONGLONG counts_per_sec;
		QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&counts_per_sec));
		secondsPerCount_ = 1.0 / static_cast<double>(counts_per_sec);

		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&thisTime_));
		baseTime_ = thisTime_;
		lastTime_ = thisTime_;
	}

	// Returns the total time elapsed since Reset() was called, NOT counting any
	// time when the clock is isStopped_.
	float TimeStamp() const  // in seconds
	{
		// If we are isStopped_, do not count the time that has passed since we isStopped_.
		// Moreover, if we previously already had a pause, the distance 
		// stopTime_ - baseTime_ includes paused time, which we do not want to count.
		// To correct this, we can subtract the paused time from mStopTime:  
		//
		//                     |<--pausedTime_-->|
		// ----*---------------*-----------------*------------*------------*------> time
		//  baseTime_       stopTime_        start_time     stopTime_    thisTime_

		if (isStopped_)
		{
			return static_cast<float>(((stopTime_ - pausedTime_) - baseTime_)*secondsPerCount_);
		}

		// The distance thisTime_ - mBaseTime includes paused time,
		// which we do not want to count.  To correct this, we can subtract 
		// the paused time from thisTime_:  
		//
		//  (thisTime_ - pausedTime_) - baseTime_ 
		//
		//                     |<--pausedTime_-->|
		// ----*---------------*-----------------*------------*------> time
		//  baseTime_       stopTime_        start_time     thisTime_
		else
		{
			return static_cast<float>(((thisTime_ - pausedTime_) - baseTime_)*secondsPerCount_);
		}
	}

	float TimeInterval() const  // in seconds
	{
		return static_cast<float>(deltaTime_);
	}

	void Reset() // Call before message loop.
	{
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&thisTime_));
		baseTime_ = thisTime_;
		lastTime_ = thisTime_;

		stopTime_ = 0;
		isStopped_ = false;
	}

	void Start() // Call when unpaused.
	{
		LONGLONG start_time;
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&start_time));

		// Accumulate the time elapsed between stop and start pairs.
		//
		//                     |<-------d------->|
		// ----*---------------*-----------------*------------> time
		//  baseTime_       stopTime_        start_time     
		if (isStopped_)
		{
			pausedTime_ += (start_time - stopTime_);
			lastTime_ = start_time;
			stopTime_ = 0;
			isStopped_ = false;
		}
	}

	void Stop() // Call when paused.
	{
		if (!isStopped_)
		{
			QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&stopTime_));
			isStopped_ = true;
		}
	}

	void Tick() // Call every frame.
	{
		if (isStopped_)
		{
			deltaTime_ = 0.0;
			return;
		}

		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&thisTime_));
		// Time difference between this frame and the previous.
		deltaTime_ = (thisTime_ - lastTime_)*secondsPerCount_;

		// Prepare for next frame.
		lastTime_ = thisTime_;

		// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
		// processor goes into a power save mode or we get shuffled to another
		// processor, then mDeltaTime can be negative.
		if (deltaTime_ < 0.0)
		{
			deltaTime_ = 0.0;
		}
	}

private:
	double secondsPerCount_;
	double deltaTime_;

	LONGLONG baseTime_;
	LONGLONG pausedTime_;
	LONGLONG stopTime_;
	LONGLONG lastTime_;
	LONGLONG thisTime_;

	bool isStopped_;
};

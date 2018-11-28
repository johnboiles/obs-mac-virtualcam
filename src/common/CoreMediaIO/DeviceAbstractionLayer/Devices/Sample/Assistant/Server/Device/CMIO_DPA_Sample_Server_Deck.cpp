/*
	    File: CMIO_DPA_Sample_Server_Deck.cpp
	Abstract: n/a
	 Version: 1.2
	
	Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
	Inc. ("Apple") in consideration of your agreement to the following
	terms, and your use, installation, modification or redistribution of
	this Apple software constitutes acceptance of these terms.  If you do
	not agree with these terms, please do not use, install, modify or
	redistribute this Apple software.
	
	In consideration of your agreement to abide by the following terms, and
	subject to these terms, Apple grants you a personal, non-exclusive
	license, under Apple's copyrights in this original Apple software (the
	"Apple Software"), to use, reproduce, modify and redistribute the Apple
	Software, with or without modifications, in source and/or binary forms;
	provided that if you redistribute the Apple Software in its entirety and
	without modifications, you must retain this notice and the following
	text and disclaimers in all such redistributions of the Apple Software.
	Neither the name, trademarks, service marks or logos of Apple Inc. may
	be used to endorse or promote products derived from the Apple Software
	without specific prior written permission from Apple.  Except as
	expressly stated in this notice, no other rights or licenses, express or
	implied, are granted by Apple herein, including but not limited to any
	patent rights that may be infringed by your derivative works or by other
	works in which the Apple Software may be incorporated.
	
	The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
	MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
	THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
	FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
	OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
	
	IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
	MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
	AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
	STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
	
	Copyright (C) 2012 Apple Inc. All Rights Reserved.
	
*/

// Self Includes
#include "CMIO_DPA_Sample_Server_Deck.h"

// Internal Includes
#include "CMIO_DPA_Sample_Server_Stream.h"

// CA Public Utility Includes
#include "CACFArray.h"
#include "CACFDictionary.h"
#include "CACFNumber.h"
#include "CAException.h"
#include "CAHostTimeBase.h"

namespace CMIO { namespace DPA { namespace Sample { namespace Server
{
	#pragma mark -
	#pragma mark Deck
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Deck()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Deck::Deck(Stream& stream) :
		mStateMutex("Deck state mutex"),
		mOwningStream(stream),
		mDeckThreadRunning(false),
		mStreamDeck(),
		mTimecode(0),
		mCueState(1),
		mDirection(true),
		mCurrentDeckMode(kCMIODeckStateStop),
		mNewDeckMode(kCMIODeckStateStop),
		mSimulationThread(reinterpret_cast<CAPThread::ThreadRoutine>(SimulationEntry), this),
		mSimulationGuard("deck simulation thread guard"),
		mStopSimulationThread(false),
		mTimecodeUpdateThread(reinterpret_cast<CAPThread::ThreadRoutine>(TimecodeUpdateEntry), this),
		mTimecodeUpdateGuard("timecode thread guard"),
		mStopTimecodeUpdateThread(false),
		mCueingThread(reinterpret_cast<CAPThread::ThreadRoutine>(CueingEntry), this),
		mCueingGuard("cueing thread guard"),
		mStopCueingThread(false),
		mCueing(false),
		mRequestedCuePoint(0),
		mStepping(false),
		mStepCount(0)
	{
		StartSimulationThread();
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ~Deck()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Deck::~Deck()
	{
		// Grab the mutex for the deck's state
		CAMutex::Locker locker(mStateMutex);

		StopThreads();
		StopSimulationThread();
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Play()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Deck::Play()
	{
		SetDeckMode(kCMIODeckStatePlay);
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Stop()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Deck::Stop()
	{
		SetDeckMode(kCMIODeckStateStop);
	}


	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Jog()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Deck::Jog(SInt32 speed)
	{
		if (0 == speed)
		{
			SetDeckMode((UInt32)kCMIODeckStatePause);
		}
		else if (speed > 0)
		{
			if (speed < 3)
				SetDeckMode((UInt32)kCMIODeckStatePlaySlow);
			else
				SetDeckMode((UInt32)kCMIODeckStateFastForward);
		}
		else
		{
			if (speed > -3)
				SetDeckMode((UInt32)kCMIODeckStateReverseSlow);
			else
				SetDeckMode((UInt32)kCMIODeckStateFastRewind);
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CueTo()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Deck::CueTo(Float64 requestedTimecode, Boolean	playOnCue)
	{
		// Grab the mutex for the deck's state
		CAMutex::Locker locker(mStateMutex);

		if (not mCueing)
		{
			mRequestedCuePoint = requestedTimecode;
			mCueing = true;
			mPlayOnCue = playOnCue;

			// Notify the cueing thread that a cueing command has been submitted
			mCueingGuard.NotifyAll();
		}
		else
		{
			DebugMessage("Trying to Cue when in Cueing state");
		}
		
	}
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StartDeckSimulationThread()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Deck::StartSimulationThread()
	{
		// Grab the mutex for the deck's state
		CAMutex::Locker locker(mStateMutex);

		{
			// Grab the simulation thread guard
			CAGuard::Locker simulationGuard(mSimulationGuard);
			
			// Inform the simulation thread that it should continue its work loop
			mStopSimulationThread = false;
			
			// Spawn the simulation thread
			mSimulationThread.Start();
			
			// Wait for the simulation thread to indicate it has started
			simulationGuard.Wait();
		}
	}
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StopDeckSimulationThread()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Deck::StopSimulationThread()
	{
		// Grab the mutex for the deck's state
		CAMutex::Locker locker(mStateMutex);

		if (mSimulationThread.IsRunning())
		{
			// Grab the simulation guard
			CAGuard::Locker	simulationGuard(mSimulationGuard);
			
			// Inform the simulation thread that it should stop its work loop
			mStopSimulationThread = true;
			
			// Wait for the simulation thread to indicate it has stopped
			simulationGuard.NotifyAll();
			simulationGuard.Wait();
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StartThreads()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Deck::StartThreads()
	{		
		DebugMessage("Deck::StartThreads");
		// Don't need to do anything if the threads are already running
		if (mDeckThreadRunning)
			return;

		// Grab the mutex for the deck's state
//		CAMutex::Locker locker(mStateMutex);

		{
			// Grab the timecode update thread guard
			CAGuard::Locker timecodeUpdateGuard(mTimecodeUpdateGuard);
			
			// Inform the timecode update thread that it should continue its work loop
			mStopTimecodeUpdateThread = false;

			// Spawn the timecode update thread
			mTimecodeUpdateThread.Start();
			
			// Wait for the timecode update thread to indicate it has started
			timecodeUpdateGuard.Wait();
		}
		
		{
			// Grab the cueing thread guard
			CAGuard::Locker cueingGuard(mCueingGuard);
			
			// Inform the cueing thread that it should continue its work loop
			mStopCueingThread = false;

			// Spawn the cueing thread
			mCueingThread.Start();
			
			// Wait for the cueing thread to indicate it has started
			cueingGuard.Wait();
		}
		// Remember that the threads are running now
		mDeckThreadRunning = true;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StopThreads()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Deck::StopThreads()
	{
//		DebugMessage("Deck::StopThreads");

		// Don't need to do anything they threads are not running
		if (not mDeckThreadRunning)
			return;

		// Remember that the deck threads are stopped
		mDeckThreadRunning = false;

		// Grab the mutex for the deck's state
//		CAMutex::Locker locker(mStateMutex);

		if (mCueingThread.IsRunning())
		{
			// Grab the cueing guard
			CAGuard::Locker	cueingGuard(mCueingGuard);
			
			// Inform the cueing thread that it should stop its work loop
			mStopCueingThread = true;
			
			// Wait for the cueing thread to indicate it has stopped
			cueingGuard.NotifyAll();
			cueingGuard.Wait();
		}
		
		if (mTimecodeUpdateThread.IsRunning())
		{
			// Grab the timecode update guard
			CAGuard::Locker	timecodeUpdateGuard(mTimecodeUpdateGuard);
			
			// Inform the timecode update thread that it should stop its work loop
			mStopTimecodeUpdateThread = true;
			
			// Wait for the timecode update thread to indicate it has stopped
			timecodeUpdateGuard.NotifyAll();
			timecodeUpdateGuard.Wait();
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetDeckMode()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Deck::SetDeckMode(UInt32 mode)
	{
		// Grab the mutex for the deck's state
		CAMutex::Locker locker(mStateMutex);

		if (mode != mCurrentDeckMode)
		{
			switch (mode)
			{
				case kCMIODeckStateStop:
					{
						if (mCueing)
						{
							mCueing = false;
							mCueState = -1;
							mOwningStream.DeckCueingChanged();
						}
						
						mNewDeckMode = kCMIODeckStateStop; 
					}
					break;
					
				case kCMIODeckStatePlay:
					{
						if (mCueing)
						{
							mCueing = false;
							mCueState = -1;
							mOwningStream.DeckCueingChanged();
						}
						
						mNewDeckMode = kCMIODeckStatePlay; 
					}
					break;
					
				case kCMIODeckStatePlaySlow:
					mNewDeckMode = kCMIODeckStatePlaySlow; 
					break;
				case kCMIODeckStateReverseSlow:
					mNewDeckMode = kCMIODeckStateReverseSlow; 
					break;
					
				case kCMIODeckStatePause:
					mNewDeckMode = kCMIODeckStatePause;
					break;
					
				case kCMIODeckStatePlayReverse:
					mNewDeckMode = kCMIODeckStatePlayReverse; 
					break;
					
				case kCMIODeckStateFastForward:
					mNewDeckMode = kCMIODeckStateFastForward; 
					break;
					
				case kCMIODeckStateFastRewind:
					mNewDeckMode = kCMIODeckStateFastRewind; 
					break;
			}
			
			// Notify the simulation guard that mode has changed
			mSimulationGuard.NotifyAll();
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FrameArrived()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Deck::FrameArrived()
	{
		mTimecodeUpdateGuard.NotifyAll();
	}
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CueingEntry()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void* Deck::CueingEntry(Deck& deck)
	{
		{
			// Grab the cueing guard
			CAGuard::Locker	cueingGuard(deck.mCueingGuard);
			
			// Signal that the cueing thread is running
			cueingGuard.NotifyAll();
		}
		
		while (not deck.mStopCueingThread)
		{
			if (deck.mCueing)
			{
				// Grab the mutex for the deck's state
				CAMutex::Locker locker(deck.mStateMutex);
				
				// Get the difference between current and requested
				Float64	difference = deck.mRequestedCuePoint - deck.mTimecode;
				
				// Remember if this is reverse for later
				bool reverse = (difference < 0) ? true : false;
				
				// See if the requested timecode has been reached
				if (0 == difference and deck.mStepping and 0 == deck.mStepCount)
				{
					DebugMessage("Deck::CueingEntry: at exactly the requested timecode");
					deck.mCueing = false;
					deck.SetDeckMode(kCMIODeckStatePause);
					deck.mCueState = 1;
					deck.mOwningStream.DeckCueingChanged();
					continue;
				}

				// Still need to shuttle to the requested timecode
				deck.mStepping = false;		
				difference = fabs(difference);	
				
				// Shuttle in stages depending on how many frames the difference is
				if (0 == difference)
				{
					DebugMessage("Deck::CueingEntry: 0 == difference, begin stepping.");
					deck.mCueing = false;
					deck.mCueState = 1;
					deck.mOwningStream.DeckCueingChanged();
					
					if (deck.mPlayOnCue)
					{
						DebugMessage("Deck::CueingEntry: Calling CueComplete");
						deck.SetDeckMode(kCMIODeckStatePlay);
						continue;
					}
					
					deck.mOwningStream.DeckJog(kCMIODeckShuttlePause);
					continue;
				}
				
				SInt32 speed = kCMIODeckShuttlePause;
				if (difference < (reverse ? 7 : 7))
					speed = kCMIODeckShuttlePlayNextFrame;
				else if (difference < (reverse ? 20 : 20))
					speed = kCMIODeckShuttlePlaySlowest;
				else if (difference < (reverse ? 60 : 60))
					speed = kCMIODeckShuttlePlaySlow2;
				else if (difference < (reverse ? 120 : 120)) 
					speed = kCMIODeckShuttlePlay1x;
				else if (difference < (reverse ? 300 : 300))
					speed = kCMIODeckShuttlePlayFast;
				else if (difference < (reverse ? 3600 : 3600))
					speed = kCMIODeckShuttlePlayFastest;
				else if (difference >= (reverse ? 3600 : 3600))
					speed = kCMIODeckShuttlePlayHighSpeed;

				// Only send the step command once in a while, monitored by difference, send the first one, then increment mStepCount...
				if (difference > 0 and 0 == deck.mStepCount)
					deck.mOwningStream.DeckJog((reverse ? -speed : speed));
				
				if (difference < 3)
					++deck.mStepCount %= 31;
				else if (difference < 7)
					++deck.mStepCount %= 13;
				else
					deck.mStepCount = 0;
			}
			else
			{
				// Wait for the next cueing command
				CAGuard::Locker	cueingGuard(deck.mCueingGuard);
				cueingGuard.Wait();
			}
		}
		
		// Grab the cueing guard
		CAGuard::Locker	cueingGuard(deck.mCueingGuard);
		
		// Signal that the cueing thread is stopping
		cueingGuard.NotifyAll();
		
		return NULL;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SimulationEntry()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void* Deck::SimulationEntry(Deck& deck)
	{
		{
			// Grab the simulation guard
			CAGuard::Locker	simulationGuard(deck.mSimulationGuard);
			
			// Signal that the simulation thread is running
			simulationGuard.NotifyAll();
		}
		
		while (not deck.mStopSimulationThread)
		{
			DebugMessage("Deck::SimulationEntry: in loop");

			if (deck.mCurrentDeckMode != deck.mNewDeckMode)
			{
				// Grab the mutex for the deck's state
				CAMutex::Locker locker(deck.mStateMutex);

				DebugMessage("Deck::SimulationEntry: mCurrentDeckMode != mNewDeckMode");
				deck.mCurrentDeckMode = deck.mNewDeckMode;
				deck.SetStreamDeck(kCMIODeckStatusOpcode, deck.mCurrentDeckMode, 0);
				deck.mOwningStream.StreamDeckChanged();
			}
			else
			{
				// Wait until the next deck mode change
				CAGuard::Locker	simulationGuard(deck.mSimulationGuard);
				simulationGuard.Wait();
			}
		}
		
		// Grab the simulation guard
		CAGuard::Locker	simulationGuard(deck.mSimulationGuard);
		
		// Signal that the simulation thread is stopping
		simulationGuard.NotifyAll();
		
		return NULL;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// TimecodeUpdateEntry()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void* Deck::TimecodeUpdateEntry(Deck& deck)
	{
		{
			// Grab the conversion guard
			CAGuard::Locker	timecodeUpdateGuard(deck.mTimecodeUpdateGuard);
			
			// Signal that the conversion thread is running
			timecodeUpdateGuard.NotifyAll();
		}
		
		while (not deck.mStopTimecodeUpdateThread)
		{
			{
				// Grab the mutex for the deck's state
				CAMutex::Locker locker(deck.mStateMutex);

				switch (deck.mCurrentDeckMode)
				{
					case kCMIODeckStateStop:
					case kCMIODeckStatePause:
						DebugMessage("TimecodeUpdateEntry stop");
						break;
						
					case kCMIODeckStatePlay:
					case kCMIODeckStatePlaySlow:
					{
						DebugMessage("TimecodeUpdateEntry +1");
						deck.mTimecode += 1;
						break;
					}
						
					case kCMIODeckStatePlayReverse:
					case kCMIODeckStateReverseSlow:
					{
						deck.mTimecode -= 1;
						if (deck.mTimecode <= 0)
						{
							deck.mTimecode = 0;
							deck.SetDeckMode(kCMIODeckStateStop);
						}
						break;
					}
					
					case kCMIODeckStateFastForward:
					{
						deck.mTimecode += 10;
						break;
					}

					case kCMIODeckStateFastRewind:
					{
						deck.mTimecode -= 10;
						if (deck.mTimecode <= 0)
						{
							deck.mTimecode = 0;
							deck.SetDeckMode(kCMIODeckStateStop);
						}
						break;
					}
				}
			}
			
			deck.mOwningStream.DeckTimecodeChanged();
			// timecodeUpdateGuard.WaitFor(33000000);

			// Wait until the next timecode  change
			CAGuard::Locker(deck.mTimecodeUpdateGuard).Wait();
		}
		
		// Grab the conversion guard
		CAGuard::Locker	timecodeUpdateGuard(deck.mTimecodeUpdateGuard);
		
		// Signal that the conversion thread is stopping
		timecodeUpdateGuard.NotifyAll();
		
		return NULL;
	}
}}}}
	

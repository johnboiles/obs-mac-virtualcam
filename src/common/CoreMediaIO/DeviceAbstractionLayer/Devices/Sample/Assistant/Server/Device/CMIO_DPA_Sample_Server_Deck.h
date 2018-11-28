/*
	    File: CMIO_DPA_Sample_Server_Deck.h
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

#if !defined(__CMIO_DPA_Sample_Server_Deck_h__)
#define __CMIO_DPA_Sample_Server_Deck_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Internal Includes
#include "CMIO_DPA_Sample_Server_Common.h"
#include "CMIO_DPA_Sample_Shared.h"

// CA Public Utility Includes
#include "CACFString.h"
#include "CAAutoDisposer.h"
#include "CAGuard.h"
#include "CAPThread.h"

namespace CMIO { namespace DPA { namespace Sample { namespace Server
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Types
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Stream;
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Deck
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Deck
	{
	#pragma mark -
	#pragma mark Types in class Deck
	public:
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// Deck::StreamDeck
		//	Deck::StreamDeck extends the CMIOStreamDeck structure to C++ including constructors and other utility operations.
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		struct StreamDeck : public CMIOStreamDeck 
		{
		// Construction/Destruction
		public:
						StreamDeck()											: CMIOStreamDeck() { mStatus = kCMIODeckStatusLocal; mState = 0; mState2 = 0; }
						StreamDeck(UInt32 status, UInt32 state, UInt32 state2)	: CMIOStreamDeck() { mStatus = status; mState = state; mState2 = state2; }
						StreamDeck(const CMIOStreamDeck& streamDeck)			: CMIOStreamDeck(streamDeck) {}
						StreamDeck(const StreamDeck& streamDeck)				: CMIOStreamDeck(streamDeck) {}
			StreamDeck&	operator=(const CMIOStreamDeck& streamDeck)			{ CMIOStreamDeck::operator=(streamDeck); return *this; }
			StreamDeck&	operator=(const StreamDeck& streamDeck)					{ CMIOStreamDeck::operator=(streamDeck); return *this; }
		};

	#pragma mark -
	#pragma mark Deck
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Deck
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Construction/Destruction
	public:
							Deck(Stream& stream);
		virtual				~Deck();
	
	protected:
		CAMutex				mStateMutex;				// Controls access to decks's state
		Stream&				mOwningStream;

	private:
		bool				mDeckThreadRunning;			// True when deck thread is running
		
		
	// Deck Control
	public:
		void				StartThreads();
		void				StopThreads();
		void				Play();
		void				Stop();
		void				Jog(SInt32 speed);
		void				CueTo(Float64 requestedTimecode, Boolean playOnCue);

		CMIOStreamDeck	GetStreamDeck() const { return mStreamDeck; } 
		Float64				GetTimecode() const { return mTimecode; } 
		SInt32				GetCueState() const { return mCueState; } 

		void				FrameArrived();

	private:
		void				SetStreamDeck(UInt32 status, UInt32 state, UInt32 state2) { mStreamDeck.mStatus = status; mStreamDeck.mState = state; mStreamDeck.mState2 = state2; }


		void				StartSimulationThread();
		void				StopSimulationThread();
		void				SetDeckMode(UInt32 mode);

	private:
		StreamDeck			mStreamDeck;
		Float64				mTimecode;
		SInt32				mCueState;
		bool				mDirection;
		
		Float64				mCurrentFrameCount;
		UInt32				mCurrentDeckMode;
		UInt32				mNewDeckMode;
		CAPThread			mSimulationThread;
		CAGuard				mSimulationGuard;
		bool				mStopSimulationThread;
		CAPThread			mTimecodeUpdateThread;
		CAGuard				mTimecodeUpdateGuard;
		bool				mStopTimecodeUpdateThread;
		CAPThread			mCueingThread;
		CAGuard				mCueingGuard;
		bool				mStopCueingThread;
		bool				mCueing;
		Float64				mRequestedCuePoint;
		bool				mStepping;
		UInt32				mStepCount;
		Boolean				mPlayOnCue;
		
	private:
		static void*		SimulationEntry(Deck& deck);
		static void*		TimecodeUpdateEntry(Deck& deck);
		static void*		CueingEntry(Deck& deck);
	};
}}}}

#endif



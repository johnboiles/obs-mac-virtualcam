/*
	    File: CMIO_DP_Property_Deck.h
	Abstract: Implements the kCMIOStreamPropertyCanProcessDeckCommand, kCMIOStreamPropertyDeck, kCMIOStreamPropertyDeckFrameNumber, kCMIOStreamPropertyDeckDropness,
				kCMIOStreamPropertyDeckThreaded, kCMIOStreamPropertyDeckLocal, and kCMIOStreamPropertyDeckCueing properties.
	
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

#if !defined(__CMIO_DP_Property_Deck_h__)
#define __CMIO_DP_Property_Deck_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Super Class Includes
#include "CMIO_DP_Property_Base.h"

namespace CMIO { namespace DP
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Types in the CMIO::DP namespace
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Stream;
}};

namespace CMIO { namespace DP { namespace Property
{
	class Deck : public Base
	{
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Deck::XXOneShot() callback typedefs
	//	If a client has NOT signed up to listen to any of the deck properties, these callback accessors are used to to get the current state of the properties.
	//	If any of the properties ARE being listended too, the properties will be updated by the owning stream.
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	public:
		typedef CMIOStreamDeck (*StreamDeckOneShot)(Stream& stream);
		typedef Float64 (*TimeCodeOneShot)(Stream& stream);
		typedef UInt32 (*DropnessOneShot)(Stream& stream);
		typedef UInt32 (*ThreadedOneShot)(Stream& stream);
		typedef UInt32 (*LocalOneShot)(Stream& stream);
		typedef SInt32 (*CueingOneShot)(Stream& stream);
		
		struct OneShotCallbacks
		{
			StreamDeckOneShot mStreamDeck;
			
		};
		
	#pragma mark -
	#pragma mark Deck
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Deck
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Construction/Destruction
	public:
							Deck(DP::Stream& owningStream, StreamDeckOneShot streamDeckOneShot, TimeCodeOneShot timeCodeOneShot, DropnessOneShot dropnessOneShot,
									ThreadedOneShot threadedOneShot, LocalOneShot localOneShot, CueingOneShot cueingOneShot) : Base(), mOwningStream(owningStream), mUseOneShots(true),
									mStreamDeckOneShot(streamDeckOneShot), mTimecodeOneShot(timeCodeOneShot), mDropnessOneShot(dropnessOneShot), mThreadedOneShot(threadedOneShot),
									mLocalOneShot(localOneShot), mCueingOneShot(cueingOneShot) { Reset();}
		virtual				~Deck() {}
		
	private:
		Stream&				GetOwningStream() const { return mOwningStream; }
		Stream&				mOwningStream;
		
	// Property Operations
	public:
		virtual bool		IsActive(const CMIOObjectPropertyAddress& address) const;
		virtual bool		IsPropertySettable(const CMIOObjectPropertyAddress& address) const;
		virtual UInt32		GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const;
		virtual void		GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const;
		
		virtual UInt32		GetNumberAddressesImplemented() const;
		virtual void		GetImplementedAddressByIndex(UInt32 index, CMIOObjectPropertyAddress& address) const;
		
	// Property Implementation
	public:
		void				Reset()
							{
								mStreamDeck	= mStreamDeckOneShot(GetOwningStream());
								mTimecode	= mTimecodeOneShot(GetOwningStream());
								mDropness	= mDropnessOneShot(GetOwningStream());
								mThreaded	= mThreadedOneShot(GetOwningStream());
								mLocal		= mLocalOneShot(GetOwningStream());
								mCueing		= mCueingOneShot(GetOwningStream());
							}
		
		CMIOStreamDeck	GetStreamDeck() const {return mStreamDeck;}
		void				SetStreamDeck(CMIOStreamDeck deckStatus, bool sendChangeNotifications = true); 
		Float64				GetTimecode() const {return mTimecode;}
		void				SetTimecode(Float64 timecode, bool sendChangeNotifications = true); 
		UInt32				GetDropness() const {return mDropness;}
		void				SetDropness(UInt32 dropness, bool sendChangeNotifications = true); 
		UInt32				GetThreaded() const {return mDropness;}
		void				SetThreaded(UInt32 threaded, bool sendChangeNotifications = true); 
		UInt32				GetLocal() const {return mLocal;}
		void				SetLocal(UInt32 local, bool sendChangeNotifications = true); 
		SInt32				GetCueing() const {return mCueing;}
		void				SetCueing(SInt32 cueing, bool sendChangeNotifications = true) ;
		
		void				SetUseOneShotGetters(bool useOneShots) { mUseOneShots = useOneShots; }
		bool				UseOneShotGetters() { return mUseOneShots; }

	private:
		bool				mUseOneShots;
		StreamDeckOneShot	mStreamDeckOneShot;
		TimeCodeOneShot		mTimecodeOneShot;
		DropnessOneShot		mDropnessOneShot;
		ThreadedOneShot		mThreadedOneShot;
		LocalOneShot		mLocalOneShot;
		CueingOneShot		mCueingOneShot;

		CMIOStreamDeck	mStreamDeck;
		Float64				mTimecode;
		UInt32				mDropness;
		UInt32				mThreaded;
		UInt32				mLocal;
		SInt32				mCueing;

	};
}}}
#endif

/*
	    File: CMIO_DP_Property_SuspendedByUser.h
	Abstract: Implements the kCMIODevicePropertySuspendedByUser property.
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

#if !defined(__CMIO_DP_Property_SuspendedByUser_h__)
#define __CMIO_DP_Property_SuspendedByUser_h__

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
	class Device;
}}

namespace CMIO { namespace DP { namespace Property
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SuspendedByUser
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class SuspendedByUser : public Base
	{
	//	Construction/Destruction
	public:
						SuspendedByUser(DP::Device& owningDevice, bool suspendedByUser) : Base(), mOwningDevice(owningDevice), mSuspendedByUser(suspendedByUser) {}
	virtual				~SuspendedByUser() {}
	
	private:
		DP::Device&		GetOwningDevice() const { return mOwningDevice; }
		DP::Device&		mOwningDevice;
	
	// Property Operations
	public:
		virtual bool	IsPropertySettable(const CMIOObjectPropertyAddress& address) const { return false; }
		virtual UInt32	GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const;
		virtual void	GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const;
	
		virtual UInt32	GetNumberAddressesImplemented() const { return 1; }
		virtual void	GetImplementedAddressByIndex(UInt32 index, CMIOObjectPropertyAddress& address) const;
	
	//	Property Implementation
		void			SetSuspendedByUser(bool suspendedByUser);
		bool			IsSuspendedByUser() const { return mSuspendedByUser; }
	
	private:
		bool			mSuspendedByUser;
	};
}}}
#endif

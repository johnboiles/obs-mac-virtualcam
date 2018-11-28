/*
	    File: CMIO_PropertyAddress.h
	Abstract: A utility class to assist with CMIOObjectPropertyAddress.
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

#if !defined(__CMIO_PropertyAddress_h__)
#define __CMIO_PropertyAddress_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Public Utility Includes
#include "CMIODebugMacros.h"

// System Includes
#include <CoreMediaIO/CMIOHardware.h>

// Standard Library Includes
#include <algorithm>
#include <functional>
#include <vector>

namespace CMIO
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CMIO::PropertyAddress
	//	CMIO::PropertyAddress extends the CMIOObjectPropertyAddress structure to C++ including constructors and other utility operations. Note that there is no defined operator< or
	//	operator== because the presence of wildcards for the fields make comparisons ambiguous without specifying whether or not to take the wildcards into account. Consequently, if you want
	//	to use this struct in an STL data structure, you'll need to specify the approriate function object explicitly in the template declaration.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	struct PropertyAddress : public CMIOObjectPropertyAddress
	{
	// Construction/Destruction
	public:
							PropertyAddress()																										: CMIOObjectPropertyAddress() { mSelector = 0; mScope = kCMIOObjectPropertyScopeGlobal; mElement = kCMIOObjectPropertyElementMaster; }
							PropertyAddress(CMIOObjectPropertySelector selector)																	: CMIOObjectPropertyAddress() { mSelector = selector; mScope = kCMIOObjectPropertyScopeGlobal; mElement = kCMIOObjectPropertyElementMaster; }
							PropertyAddress(CMIOObjectPropertySelector selector, CMIOObjectPropertyScope scope)										: CMIOObjectPropertyAddress() { mSelector = selector; mScope = scope; mElement = kCMIOObjectPropertyElementMaster; }
							PropertyAddress(CMIOObjectPropertySelector selector, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)	: CMIOObjectPropertyAddress() { mSelector = selector; mScope = scope; mElement = element; }
							PropertyAddress(const CMIOObjectPropertyAddress& address)																: CMIOObjectPropertyAddress(address){}
							PropertyAddress(const PropertyAddress& address)																			: CMIOObjectPropertyAddress(address){}
		PropertyAddress&	operator=(const CMIOObjectPropertyAddress& address)																		{ CMIOObjectPropertyAddress::operator=(address); return *this; }
		PropertyAddress&	operator=(const PropertyAddress& address)																				{ CMIOObjectPropertyAddress::operator=(address); return *this; }
		
	// Operations
	public:
		static bool			IsSameAddress(const CMIOObjectPropertyAddress& address1, const CMIOObjectPropertyAddress& address2)						{ return (address1.mScope == address2.mScope) && (address1.mSelector == address2.mSelector) && (address1.mElement == address2.mElement); }
		static bool			IsLessThanAddress(const CMIOObjectPropertyAddress& address1, const CMIOObjectPropertyAddress& address2)					{ bool theAnswer = false; if(address1.mScope != address2.mScope) { theAnswer = address1.mScope < address2.mScope; } else if(address1.mSelector != address2.mSelector) { theAnswer = address1.mSelector < address2.mSelector; } else { theAnswer = address1.mElement < address2.mElement; } return theAnswer; }
		static bool			IsCongruentSelector(CMIOObjectPropertySelector selector1, CMIOObjectPropertySelector selector2)							{ return (selector1 == selector2) || (selector1 == kCMIOObjectPropertySelectorWildcard) || (selector2 == kCMIOObjectPropertySelectorWildcard); }
		static bool			IsCongruentScope(CMIOObjectPropertyScope scope1, CMIOObjectPropertyScope scope2)										{ return (scope1 == scope2) || (scope1 == kCMIOObjectPropertyScopeWildcard) || (scope2 == kCMIOObjectPropertyScopeWildcard); }
		static bool			IsCongruentElement(CMIOObjectPropertyElement element1, CMIOObjectPropertyElement element2)								{ return (element1 == element2) || (element1 == kCMIOObjectPropertyElementWildcard) || (element2 == kCMIOObjectPropertyElementWildcard); }
		static bool			IsCongruentAddress(const CMIOObjectPropertyAddress& address1, const CMIOObjectPropertyAddress& address2)				{ return IsCongruentScope(address1.mScope, address2.mScope) && IsCongruentSelector(address1.mSelector, address2.mSelector) && IsCongruentElement(address1.mElement, address2.mElement); }
		static bool			IsCongruentLessThanAddress(const CMIOObjectPropertyAddress& address1, const CMIOObjectPropertyAddress& address2)		{ bool theAnswer = false; if(!IsCongruentScope(address1.mScope, address2.mScope)) { theAnswer = address1.mScope < address2.mScope; } else if(!IsCongruentSelector(address1.mSelector, address2.mSelector)) { theAnswer = address1.mSelector < address2.mSelector; } else if(!IsCongruentElement(address1.mElement, address2.mElement)) { theAnswer = address1.mElement < address2.mElement; } return theAnswer; }

	// STL Helpers
	public:
		struct EqualTo : public std::binary_function<CMIOObjectPropertyAddress, CMIOObjectPropertyAddress, bool>
		{
			bool	operator()(const CMIOObjectPropertyAddress& address1, const CMIOObjectPropertyAddress& address2) const							{ return IsSameAddress(address1, address2); }
		};

		struct LessThan : public std::binary_function<CMIOObjectPropertyAddress, CMIOObjectPropertyAddress, bool>
		{
			bool	operator()(const CMIOObjectPropertyAddress& address1, const CMIOObjectPropertyAddress& address2) const							{ return IsLessThanAddress(address1, address2); }
		};

		struct CongruentEqualTo : public std::binary_function<CMIOObjectPropertyAddress, CMIOObjectPropertyAddress, bool>
		{
			bool	operator()(const CMIOObjectPropertyAddress& address1, const CMIOObjectPropertyAddress& address2) const							{ return IsCongruentAddress(address1, address2); }
		};

		struct CongruentLessThan : public std::binary_function<CMIOObjectPropertyAddress, CMIOObjectPropertyAddress, bool>
		{
			bool	operator()(const CMIOObjectPropertyAddress& address1, const CMIOObjectPropertyAddress& address2) const							{ return IsCongruentLessThanAddress(address1, address2); }
		};
	};

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// PropertyAddressList
	//	An auto-resizing array of PropertyAddress structures.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class PropertyAddressList
	{
	// Construction/Destruction
	public:
												PropertyAddressList()														: mAddressList() {}
												PropertyAddressList(const PropertyAddressList& addressList)					: mAddressList(addressList.mAddressList) {}
		PropertyAddressList&					operator=(const PropertyAddressList& addressList)							{ mAddressList = addressList.mAddressList; return *this; }
												~PropertyAddressList()														{}

	// Operations
	public:
		void*									GetToken() const															{ return mToken; }
		void									SetToken(void* token)														{ mToken = token; }
	
		bool									IsEmpty() const																{ return mAddressList.empty(); }
		size_t									GetNumberItems() const														{ return mAddressList.size(); }
		void									GetItemByIndex(UInt32 index, CMIOObjectPropertyAddress& address) const		{ if (index < mAddressList.size()) { address = mAddressList.at(index); } }
		const CMIOObjectPropertyAddress*		GetItems() const															{ return &(*mAddressList.begin()); }
		
		bool									HasItem(const CMIOObjectPropertyAddress& address) const						{ AddressList::const_iterator theIterator = std::find_if(mAddressList.begin(), mAddressList.end(), std::bind1st(PropertyAddress::CongruentEqualTo(), address)); return theIterator != mAddressList.end(); }
		bool									HasExactItem(const CMIOObjectPropertyAddress& address) const				{ AddressList::const_iterator theIterator = std::find_if(mAddressList.begin(), mAddressList.end(), std::bind1st(PropertyAddress::EqualTo(), address)); return theIterator != mAddressList.end(); }

		void									AppendItem(const CMIOObjectPropertyAddress& address)						{ mAddressList.push_back(address); }
		void									AppendUniqueItem(const CMIOObjectPropertyAddress& address)					{ if (!HasItem(address)) { mAddressList.push_back(address); } }
		void									AppendUniqueExactItem(const CMIOObjectPropertyAddress& address)				{ if (!HasExactItem(address)) { mAddressList.push_back(address); } }
		void									InsertItemAtIndex(UInt32 index, const CMIOObjectPropertyAddress& address)	{ if (index < mAddressList.size()) { AddressList::iterator theIterator = mAddressList.begin(); std::advance(theIterator, index); mAddressList.insert(theIterator, address); } else { mAddressList.push_back(address); } }
		void									EraseExactItem(const CMIOObjectPropertyAddress& address)					{ AddressList::iterator theIterator = std::find_if(mAddressList.begin(), mAddressList.end(), std::bind1st(PropertyAddress::EqualTo(), address)); if (theIterator != mAddressList.end()) { mAddressList.erase(theIterator); } }
		void									EraseItemAtIndex(UInt32 index)												{ if (index < mAddressList.size()) { AddressList::iterator theIterator = mAddressList.begin(); std::advance(theIterator, index); mAddressList.erase(theIterator); } }
		void									EraseAllItems()																{ mAddressList.clear(); }

	// Implementation
	private:
		typedef std::vector<PropertyAddress>	AddressList;
		AddressList								mAddressList;
		void*									mToken;
	};

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//  PropertyAddressListVector
	//
	//  An auto-resizing array of PropertyAddressList objects.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	class	PropertyAddressListVector
	{

	//	Construction/Destruction
	public:
													PropertyAddressListVector()														: mAddressListVector() {}
													PropertyAddressListVector(const PropertyAddressListVector& addressListVector)	: mAddressListVector(addressListVector.mAddressListVector) {}
		PropertyAddressListVector&					operator=(const PropertyAddressListVector& addressListVector)					{ mAddressListVector = addressListVector.mAddressListVector; return *this; }
													~PropertyAddressListVector()													{}

	//	Operations
	public:
		bool										IsEmpty() const																	{ return mAddressListVector.empty(); }
		bool										HasAnyNonEmptyItems() const;
		bool										HasAnyItemsWithAddress(const CMIOObjectPropertyAddress& address) const;
		bool										HasAnyItemsWithExactAddress(const CMIOObjectPropertyAddress& address) const;

		UInt32										GetNumberItems() const															{ return ToUInt32(mAddressListVector.size()); }
		const PropertyAddressList&					GetItemByIndex(UInt32 index) const												{ return mAddressListVector.at(index); }
		PropertyAddressList&						GetItemByIndex(UInt32 index)													{ return mAddressListVector.at(index); }
		const PropertyAddressList*					GetItemByToken(void* token) const;
		PropertyAddressList*						GetItemByToken(void* token);
		
		void										AppendItem(const PropertyAddressList& addressList)								{ mAddressListVector.push_back(addressList); }
		void										EraseAllItems()																	{ mAddressListVector.clear(); }
		
	//  Implementation
	private:
		typedef std::vector<PropertyAddressList>	AddressListVector;

		AddressListVector							mAddressListVector;

	};

	inline bool	PropertyAddressListVector::HasAnyNonEmptyItems() const
	{
		bool theAnswer = false;
		for(AddressListVector::const_iterator theIterator = mAddressListVector.begin(); !theAnswer && (theIterator != mAddressListVector.end()); ++theIterator)
		{
			theAnswer = !theIterator->IsEmpty();
		}
		return theAnswer;
	}

	inline bool	PropertyAddressListVector::HasAnyItemsWithAddress(const CMIOObjectPropertyAddress& address) const
	{
		bool theAnswer = false;
		for(AddressListVector::const_iterator theIterator = mAddressListVector.begin(); !theAnswer && (theIterator != mAddressListVector.end()); ++theIterator)
		{
			theAnswer = theIterator->HasItem(address);
		}
		return theAnswer;
	}

	inline bool	PropertyAddressListVector::HasAnyItemsWithExactAddress(const CMIOObjectPropertyAddress& address) const
	{
		bool theAnswer = false;
		for(AddressListVector::const_iterator theIterator = mAddressListVector.begin(); !theAnswer && (theIterator != mAddressListVector.end()); ++theIterator)
		{
			theAnswer = theIterator->HasExactItem(address);
		}
		return theAnswer;
	}

	inline const PropertyAddressList*	PropertyAddressListVector::GetItemByToken(void* token) const
	{
		const PropertyAddressList* theAnswer = NULL;
		bool wasFound = false;
		for(AddressListVector::const_iterator theIterator = mAddressListVector.begin(); !wasFound && (theIterator != mAddressListVector.end()); ++theIterator)
		{
			if(theIterator->GetToken() == token)
			{
				wasFound = true;
				theAnswer = &(*theIterator);
			}
		}
		return theAnswer;
	}

	inline PropertyAddressList*	PropertyAddressListVector::GetItemByToken(void* token)
	{
		PropertyAddressList* theAnswer = NULL;
		bool wasFound = false;
		for(AddressListVector::iterator theIterator = mAddressListVector.begin(); !wasFound && (theIterator != mAddressListVector.end()); ++theIterator)
		{
			if(theIterator->GetToken() == token)
			{
				wasFound = true;
				theAnswer = &(*theIterator);
			}
		}
		return theAnswer;
	}


}
#endif

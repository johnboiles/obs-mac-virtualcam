/*
	    File: CMIO_DP_DeviceSettings.cpp
	Abstract: A place holder for saving and
	 restoring device settings.
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

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Self Include
#include "CMIO_DP_DeviceSettings.h"

// Internal Includes
#include "CMIO_DP_Device.h"
#include "CMIO_DP_Stream.h"

// Public Utility Includes
#include "CMIO_PropertyAddress.h"

// CA Public Utility Includes
#include "CAAutoDisposer.h"
#include "CACFData.h"
#include "CACFDictionary.h"
#include "CACFPreferences.h"
#include "CACFString.h"
#include "CMIODebugMacros.h"
#include "CAException.h"

#if	CMIO_Debug
//	#define	Log_SaveRestoreFromPrefs	1
#endif

#define	kDALDeviceSettingsFilePath	"~/Library/Preferences/com.apple.cmio.DeviceSettings.plist"

#pragma mark -
#pragma mark Anonymous namespace
namespace 
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeviceSettings_ConstructControlKey()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef DeviceSettings_ConstructControlKey(UInt32 id, UInt32 channel, CMIOObjectPropertyScope scope, bool isStream)
	{
		char idAsCharacter[5];
		*((UInt32*)idAsCharacter) = id;
		idAsCharacter[4] = 0;
		return CFStringCreateWithFormat(NULL, NULL, CFSTR("%s '%s' control on %s channel %ld"), isStream ? "stream" : "device", idAsCharacter, (kCMIODevicePropertyScopeInput == scope) ? "input" : "output", channel);
	}

	CFStringRef DeviceSettings_ConstructFormatKey(UInt32 streamIndex, CMIOObjectPropertyScope scope)
	{
		return CFStringCreateWithFormat(NULL, NULL, CFSTR("physical format for %s stream %ld"), (kCMIODevicePropertyScopeInput == scope) ? "input" : "output", streamIndex);
	}

	void DeviceSettings_ConstructDictionaryFromFormat(CMFormatDescriptionRef format, CACFDictionary& formatDictionary)
	{
	}

	bool DeviceSettings_ConstructFormatFromDictionary(const CACFDictionary& formatDictionary, CMFormatDescriptionRef& format)
	{
		return false;
	}

	void DeviceSettings_SaveControlsFromObject(const CMIO::DP::Object& object, bool isStream, UInt32 numberChannels, CACFDictionary& settings, CMIOObjectPropertyScope scope, const CMIO::DP::DeviceSettings::ControlInfo* controlsToSave, UInt32 numberControlsToSave)
	{
		CMIO::PropertyAddress address(0, scope);
		for(UInt32 controlIndex = 0; controlIndex < numberControlsToSave; ++controlIndex)
		{
			address.mSelector = controlsToSave[controlIndex].mSelector;
			
			// Note that 0 is the master channel, making the actual channels start at 1
			for(address.mElement = 0; address.mElement <= numberChannels; ++address.mElement)
			{
				// Only do something if the property is present
				if (object.HasProperty(address))
				{
					try
					{
						// Create the key
						CACFString controlKey(DeviceSettings_ConstructControlKey(address.mSelector, address.mElement, scope, isStream));
						
						#if Log_SaveRestoreFromPrefs
							char controlKeyString[1024];
							UInt32 controlKeyStringSize = 1024;
							controlKey.GetCString(controlKeyString, controlKeyStringSize);
						#endif
					
						// All control values are the same size
						UInt32 size = 4;
					
						// But not the same type.  Get the value and stuff it in the dictionary.
						UInt32 dataUsed = 0;;
						UInt32 intValue;
						Float32 floatValue;
						switch (controlsToSave[controlIndex].mValueType)
						{
							case CMIO::DP::DeviceSettings::kControlValueTypeBool:
								object.GetPropertyData(address, 0, NULL, size, dataUsed, &intValue);
								settings.AddUInt32(controlKey.GetCFString(), intValue);
								#if Log_SaveRestoreFromPrefs
									DebugMessage("Saving %s: %lu", controlKeyString, intValue);
								#endif
								break;
							
							case CMIO::DP::DeviceSettings::kControlValueTypeFloat:
								object.GetPropertyData(address, 0, NULL, size, dataUsed, &floatValue);
								settings.AddFloat32(controlKey.GetCFString(), floatValue);
								#if Log_SaveRestoreFromPrefs
									DebugMessage("Saving %s: %f", controlKeyString, floatValue);
								#endif
								break;
							
							case CMIO::DP::DeviceSettings::kControlValueType4CC:
								object.GetPropertyData(address, 0, NULL, size, dataUsed, &intValue);
								settings.AddUInt32(controlKey.GetCFString(), intValue);
								#if Log_SaveRestoreFromPrefs
									DebugMessage("Saving %s: %lu", controlKeyString, intValue);
								#endif
								break;
						};
					}
					catch(...)
					{
					}
				}
			}
		}
	}

	void DeviceSettings_SaveDeviceSettings(const CMIO::DP::Device& device, CACFDictionary& settings, CMIOObjectPropertyScope scope, const CMIO::DP::DeviceSettings::ControlInfo* controlsToSave, UInt32 numberControlsToSave)
	{
		DeviceSettings_SaveControlsFromObject(device, false, device.GetTotalNumberChannels(scope), settings, scope, controlsToSave, numberControlsToSave);
	}

	void DeviceSettings_SaveStreamSettings(const CMIO::DP::Device& device, CACFDictionary& settings, CMIOObjectPropertyScope scope, const CMIO::DP::DeviceSettings::ControlInfo* controlsToSave, UInt32 numberControlsToSave)
	{
		// Iterate through the streams in the given direction
		UInt32 numberStreams = device.GetNumberStreams(scope);
		for(UInt32 streamIndex = 0; streamIndex < numberStreams; ++streamIndex)
		{
			// Get the stream
			CMIO::DP::Stream* stream = device.GetStreamByIndex(scope, streamIndex);
			
			// Go through the controls and save the master control
			DeviceSettings_SaveControlsFromObject(*stream, true, 0, settings, scope, controlsToSave, numberControlsToSave);
			
			try
			{
				// Get the stream's  format
				CMFormatDescriptionRef format = stream->GetCurrentFormat();
				
				#if Log_SaveRestoreFromPrefs
					DebugMessage("Saving %s stream %lu format", (kCMIODevicePropertyScopeInput == scope) ? "input" : "output", streamIndex);
				#endif
				
				// Make a dictionary out of it
				CACFDictionary formatDictionary(true);
				DeviceSettings_ConstructDictionaryFromFormat(format, formatDictionary);
				
				// Create the format key
				CACFString formatKey(DeviceSettings_ConstructFormatKey(streamIndex, scope));
				
				// Add it to the settings
				settings.AddDictionary(formatKey.GetCFString(), formatDictionary.GetCFDictionary());
			}
			catch(...)
			{
			}
		}
	}

	#pragma mark -
	void DeviceSettings_RestoreControlsToObject(CMIO::DP::Object& object, bool isStream, UInt32 numberChannels, const CACFDictionary& settings, CMIOObjectPropertyScope scope, const CMIO::DP::DeviceSettings::ControlInfo* controlsToRestore, UInt32 numberControlsToRestore)
	{
		CMIO::PropertyAddress address(0, (kCMIODevicePropertyScopeInput == scope) ? kCMIODevicePropertyScopeInput : kCMIODevicePropertyScopeOutput);
		for(UInt32 controlIndex = 0; controlIndex < numberControlsToRestore; ++controlIndex)
		{
			address.mSelector = controlsToRestore[controlIndex].mSelector;
			
			// Note that 0 is the master channel, making the actual channels start at 1
			for(address.mElement = 0; address.mElement <= numberChannels; ++address.mElement)
			{
				// Only do something if the property is present
				if (object.HasProperty(address))
				{
					try
					{
						// Create the key
						CACFString controlKey(DeviceSettings_ConstructControlKey(address.mSelector, address.mElement, scope, isStream));
					
						#if Log_SaveRestoreFromPrefs
							char controlKeyString[1024];
							UInt32 controlKeyStringSize = 1024;
							controlKey.GetCString(controlKeyString, controlKeyStringSize);
						#endif
					
						// All control values are the same size
						UInt32 size = 4;
					
						// But not the same type.  Get the value and tell the object
						UInt32 intValue;
						Float32 floatValue;
						switch (controlsToRestore[controlIndex].mValueType)
						{
							case CMIO::DP::DeviceSettings::kControlValueTypeBool:
								if (settings.GetUInt32(controlKey.GetCFString(), intValue))
								{
									#if Log_SaveRestoreFromPrefs
										DebugMessage("Restoring %s: %lu", controlKeyString, intValue);
									#endif
									object.SetPropertyData(address, 0, NULL, size, &intValue);
								}
								break;
							
							case CMIO::DP::DeviceSettings::kControlValueTypeFloat:
								if (settings.GetFloat32(controlKey.GetCFString(), floatValue))
								{
									#if Log_SaveRestoreFromPrefs
										DebugMessage("Restoring %s: %f", controlKeyString, floatValue);
									#endif
									object.SetPropertyData(address, 0, NULL, size, &floatValue);
								}
								break;
							
							case CMIO::DP::DeviceSettings::kControlValueType4CC:
								if (settings.GetUInt32(controlKey.GetCFString(), intValue))
								{
									#if Log_SaveRestoreFromPrefs
										DebugMessage("Restoring %s: %lu", controlKeyString, intValue);
									#endif
									object.SetPropertyData(address, 0, NULL, size, &intValue);
								}
								break;
						};
					}
					catch(...)
					{
					}
				}
			}
		}
	}

	void DeviceSettings_RestoreDeviceSettings(CMIO::DP::Device& device, const CACFDictionary& settings, CMIOObjectPropertyScope scope, const CMIO::DP::DeviceSettings::ControlInfo* controlsToRestore, UInt32 numberControlsToRestore)
	{
		DeviceSettings_RestoreControlsToObject(device, false, device.GetTotalNumberChannels(scope), settings, scope, controlsToRestore, numberControlsToRestore);
	}

	void DeviceSettings_RestoreStreamSettings(CMIO::DP::Device& device, const CACFDictionary& settings, CMIOObjectPropertyScope scope, const CMIO::DP::DeviceSettings::ControlInfo* controlsToRestore, UInt32 numberControlsToRestore)
	{
		// Iterate through the streams in the given direction
		UInt32 numberStreams = device.GetNumberStreams(scope);
		for(UInt32 streamIndex = 0; streamIndex < numberStreams; ++streamIndex)
		{
			// Get the stream
			CMIO::DP::Stream* stream = device.GetStreamByIndex(scope, streamIndex);
			
			// Go through the controls and restore the master control
			DeviceSettings_RestoreControlsToObject(*stream, true, 0, settings, scope, controlsToRestore, numberControlsToRestore);
			
			// Create the format key
			CACFString formatKey(DeviceSettings_ConstructFormatKey(streamIndex, scope));

			// Get the stream's physical format dictionary
			CFDictionaryRef rawFormatDictionary;
			if (settings.GetDictionary(formatKey.GetCFString(), rawFormatDictionary))
			{
				// Since this CFObject came directly out of a CFDictionary without an additional CFRetain, we don't have to release it here
				CACFDictionary formatDictionary(rawFormatDictionary, false);
				
				// Attempt to restore the stream format
				CMFormatDescriptionRef format;
				if (DeviceSettings_ConstructFormatFromDictionary(formatDictionary, format))
				{
					#if Log_SaveRestoreFromPrefs
						DebugMessage("Restoring %s stream %lu format", (kCMIODevicePropertyScopeInput == scope) ? "input" : "output", streamIndex);
					#endif
					
					// Once DeviceSettings_ConstructFormatFromDictionary() is extended to do more than just return 'false' the kCMIOStreamPropertyFormatDescription &
					// kCMIOStreamPropertyFrameRate could be restored
					try
					{
					}
					catch(...)
					{
					}
				}
			}
		}
	}
}

namespace CMIO { namespace DP
{
	const DeviceSettings::ControlInfo DeviceSettings::sStandardControlsToSave[] = 
	{
	};

	#pragma mark -
	#pragma mark CMIO::DP namespace 
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SaveToPrefs()
	//	The settings for each device are stored in CFPreferences on a per-user basis. 
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DeviceSettings::SaveToPrefs(const Device& device, const DeviceSettings::ControlInfo* controlsToSave, UInt32 numberControlsToSave)
	{
		// Until further implementation, make this a NOP
		return;
		
		#if	Log_SaveRestoreFromPrefs
			CACFString deviceName(device.CopyDeviceName());
			char name[256];
			UInt32 nameSize = 256;
			deviceName.GetCString(name, nameSize);
			DebugMessage("CMIO::DP::DeviceSettings::SaveToPrefs: %s", name);
		#endif
		
		// Get the current prefs Data
		CFMutableDictionaryRef currentPrefsCFDictionary = NULL;

		// Open the prefs file for reading
		FILE* prefsFile = fopen(kDALDeviceSettingsFilePath, "r");
		if (prefsFile != NULL)
		{
			// Get the length of the file
			fseek(prefsFile, 0, SEEK_END);
			UInt32 fileLength = ftell(prefsFile);
			fseek(prefsFile, 0, SEEK_SET);
			
			if (fileLength > 0)
			{
				// Allocate a block of memory to hold the data in the file
				CAAutoFree<Byte> rawPrefsData(fileLength);
				
				// Read all the data in
				fread(static_cast<Byte*>(rawPrefsData), fileLength, 1, prefsFile);
				
				// Close the file
				fclose(prefsFile);
				
				// Put it into a CFData object
				CACFData rawPrefsCFData(static_cast<Byte*>(rawPrefsData), fileLength);
				
				// Parse the data as a property list
				currentPrefsCFDictionary = (CFMutableDictionaryRef)CFPropertyListCreateFromXMLData(NULL, rawPrefsCFData.GetCFData(), kCFPropertyListMutableContainersAndLeaves, NULL);
			}
			else
			{
				// No data, so close the file
				fclose(prefsFile);
				
				// Create a new, mutable dictionary
				currentPrefsCFDictionary = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
			}
		}
		else
		{
			// No file, or a bad file, so just create a new, mutable dictionary
			currentPrefsCFDictionary = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		}
		
		// Just skip things if we still don't have a current prefs dictionary
		if (currentPrefsCFDictionary != NULL)
		{
			// Make a CACFDictionary for convenience and to be sure it gets released
			CACFDictionary currentPrefsDictionary(currentPrefsCFDictionary, true);
			
			// Make the dictionary key for this device
			CACFString uid(device.CopyDeviceUID());
			CACFString prefsKey(CFStringCreateWithFormat(NULL, NULL, CFSTR("com.apple.cmio.CMIO.DeviceSettings.%@"), uid.GetCFString()));
			
			// Save the device's settings into a dictionary
			CACFDictionary settingsDictionary(SaveToDictionary(device, controlsToSave, numberControlsToSave), true);
			
			// Put the settings into the prefs
			currentPrefsDictionary.AddDictionary(prefsKey.GetCFString(), settingsDictionary.GetCFDictionary());
			
			// Make a CFData that contains the new prefs
			CACFData newRawPrefsCFData(CFPropertyListCreateXMLData(NULL, (CFPropertyListRef)currentPrefsDictionary.GetCFMutableDictionary()), true);
			
			if (newRawPrefsCFData.IsValid())
			{
				// Open the prefs file for writing
				prefsFile = fopen(kDALDeviceSettingsFilePath, "w+");
				if (prefsFile != NULL)
				{
					// Write the data
					fwrite(newRawPrefsCFData.GetDataPtr(), newRawPrefsCFData.GetSize(), 1, prefsFile);
					
					// Close the file
					fclose(prefsFile);
				}
			}
		}
		
		#if	Log_SaveRestoreFromPrefs
			DebugMessage("CMIO::DP::DeviceSettings::SaveToPrefs: %s finished", name);
		#endif
	}

	CFDictionaryRef	DeviceSettings::SaveToDictionary(const Device& device, const DeviceSettings::ControlInfo* controlsToSave, UInt32 numberControlsToSave)
	{
		// Create a dictionary to hold the settings
		CACFDictionary settings(false);
		
		// Save the device settings
		DeviceSettings_SaveDeviceSettings(device, settings, true, controlsToSave, numberControlsToSave);
		DeviceSettings_SaveDeviceSettings(device, settings, false, controlsToSave, numberControlsToSave);
		
		// Save the stream settings
		DeviceSettings_SaveStreamSettings(device, settings, true, controlsToSave, numberControlsToSave);
		DeviceSettings_SaveStreamSettings(device, settings, false, controlsToSave, numberControlsToSave);
		
		// Return the dictionary
		CFDictionaryRef dictionary = settings.GetDict();
		return dictionary;
	}

	#pragma mark -
	void	DeviceSettings::RestoreFromPrefs(CMIO::DP::Device& device, const DeviceSettings::ControlInfo* controlsToRestore, UInt32 numberControlsToRestore)
	{
		// Until further implementation, make this a NOP
		return;
		
		// Take and hold the device's state guard while we do this to prevent notifications from
		// Interupting the full completion of this routine.
		CAMutex::Locker deviceStateMutex(device.GetStateMutex());
		
		#if	Log_SaveRestoreFromPrefs
			CACFString deviceName(device.CopyDeviceName());
			char name[256];
			UInt32 nameSize = 256;
			deviceName.GetCString(name, nameSize);
		#endif

		// Look for this device's settings in the global preferences domain
		CACFString uid(device.CopyDeviceUID());
		CACFString prefsKey(CFStringCreateWithFormat(NULL, NULL, CFSTR("com.apple.cmio.CMIO.DeviceSettings.%@"), uid.GetCFString()));
		CFDictionaryRef prefValue = CACFPreferences::CopyDictionaryValue(prefsKey.GetCFString(), false, true);
		
		// Delete it if we found it, since we aren't using this location any longer
		if (prefValue != NULL)
		{
			#if	Log_SaveRestoreFromPrefs
				DebugMessage("CMIO::DP::DeviceSettings::RestoreFromPrefs: %s (old-skool)", name);
			#endif
			CACFPreferences::DeleteValue(prefsKey.GetCFString(), false, true, true);
			CACFPreferences::Synchronize(false, true, true);
		}
		
		// If we didn't find it, look in the prefs file
		if (prefValue == NULL)
		{
			// Get the current prefs Data
			CFMutableDictionaryRef currentPrefsCFDictionary = NULL;

			// Open the prefs file for reading
			FILE* prefsFile = fopen(kDALDeviceSettingsFilePath, "r");
			if (prefsFile != NULL)
			{
				// Get the length of the file
				fseek(prefsFile, 0, SEEK_END);
				UInt32 fileLength = ftell(prefsFile);
				fseek(prefsFile, 0, SEEK_SET);
				
				if (fileLength > 0)
				{
					// Allocate a block of memory to hold the data in the file
					CAAutoFree<Byte> rawPrefsData(fileLength);
					
					// Read all the data in
					fread(static_cast<Byte*>(rawPrefsData), fileLength, 1, prefsFile);
					
					// Close the file
					fclose(prefsFile);
					
					// Put it into a CFData object
					CACFData rawPrefsCFData(static_cast<Byte*>(rawPrefsData), fileLength);
					
					// Parse the data as a property list
					currentPrefsCFDictionary = (CFMutableDictionaryRef)CFPropertyListCreateFromXMLData(NULL, rawPrefsCFData.GetCFData(), kCFPropertyListMutableContainersAndLeaves, NULL);
				}
				else
				{
					// No data in the file, so just close it
					fclose(prefsFile);
				}
			}
			
			if (currentPrefsCFDictionary != NULL)
			{
				// There are some prefs, so make a CACFDictionary for convenience and to make sure it is released
				CACFDictionary currentPrefsDictionary(currentPrefsCFDictionary, true);
				
				// Look for the settings for this device
				currentPrefsDictionary.GetDictionary(prefsKey.GetCFString(), prefValue);
				
				// If we found it, retain it because we will release it later
				if (prefValue != NULL)
				{
					CFRetain(prefValue);
				}
			}
			
			#if	Log_SaveRestoreFromPrefs
				if (prefValue != NULL)
				{
					DebugMessage("CMIO::DP::DeviceSettings::RestoreFromPrefs: %s (nu-skool)", name);
				}
				else
				{
					DebugMessage("CMIO::DP::DeviceSettings::RestoreFromPrefs: %s (nada)", name);
				}
			#endif
		}
		
		// Restore the settings
		if (prefValue != NULL)
		{
			RestoreFromDictionary(device, prefValue, controlsToRestore, numberControlsToRestore);
			CFRelease(prefValue);
		}
		
		#if	Log_SaveRestoreFromPrefs
			DebugMessage("CMIO::DP::DeviceSettings::RestoreFromPrefs: %s finished", name);
		#endif
	}

	void DeviceSettings::RestoreFromDictionary(Device& device, const CFDictionaryRef dictionary, const ControlInfo* controlsToRestore, UInt32 numberControlsToRestore)
	{

		if (dictionary != NULL)
		{
			// Take and hold the device's state guard while we do this to prevent notifications from Interupting the full completion of this routine.
			CAMutex::Locker deviceStateMutex(device.GetStateMutex());
		
			CACFDictionary settings(dictionary, false);
			
			// Restore the device settings
			DeviceSettings_RestoreDeviceSettings(device, settings, kCMIODevicePropertyScopeInput, controlsToRestore, numberControlsToRestore);
			DeviceSettings_RestoreDeviceSettings(device, settings, kCMIODevicePropertyScopeOutput, controlsToRestore, numberControlsToRestore);
		
			// Restore the stream settings
			DeviceSettings_RestoreStreamSettings(device, settings, kCMIODevicePropertyScopeInput, controlsToRestore, numberControlsToRestore);
			DeviceSettings_RestoreStreamSettings(device, settings, kCMIODevicePropertyScopeOutput, controlsToRestore, numberControlsToRestore);
		}
	}
}}

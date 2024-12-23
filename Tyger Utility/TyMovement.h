#pragma once
#include "TyMemoryValues.h"
#include "TyPositionRotation.h"

using namespace TyMemoryValues;

namespace TyMovement {
	//Ty
	inline float* GetGlideUpDownPtr() { return (float*)(TyBaseAddress + 0x288964); }; //Default 5.5
	inline float* GetGlideSpeedPtr() { return (float*)(TyBaseAddress + 0x288928); }; //Default 7.0
	inline float* GetRunSpeedPtr() { return (float*)(TyBaseAddress + 0x288914); }; //Default 10.0
	inline float* GetGroundJumpHeightPtr() { return (float*)(TyBaseAddress + 0x28893C); }; //Default 18.57417488
	inline float* GetWaterJumpHeightPtr() { return (float*)(TyBaseAddress + 0x288998); }; //Default 10.67707825
	inline float* GetSwimSurfaceSpeedPtr() { return (float*)(TyBaseAddress + 0x28892C); }; //Default 6.0
	inline float* GetAirSpeedPtr() { return (float*)(TyBaseAddress + 0x288920); }; //Default 10.0

	inline float SwimSpeed = 20.0f; //Default 20.0
	//Changes all the required swim speed pointers, which all point to a shared float, to a float that I've created
	inline void EditSwimSpeedPtrs() {
		DWORD oldProtection;
		int* SwimRotationAmount = (int*)(TyBaseAddress + 0x33293);
		int* SwimRotationSensitivity = (int*)(TyBaseAddress + 0x332A6);
		int* MakesTyNotPoofOutOfExistenceIDK = (int*)(TyBaseAddress + 0x33D29);
		int* HardCodedSwimSpeed = (int*)(TyBaseAddress + 0x33DF3);
		//Change the memory access to ReadWrite to be able to change the pointers (this will set the entire page that contains all the pointers that need to be edited)
		VirtualProtect(SwimRotationAmount, 4, PAGE_EXECUTE_READWRITE, &oldProtection);

		//Set everything to Tyger Utility's swim speed float
		*SwimRotationAmount = *SwimRotationSensitivity = *MakesTyNotPoofOutOfExistenceIDK = *HardCodedSwimSpeed = (int)&SwimSpeed;

		//Set it back to the old access protection
		VirtualProtect(SwimRotationAmount, 4, oldProtection, &oldProtection);
	}

	inline float GetTyHorizontalSpeed() {
		//Scale them based on the direction Ty is facing, so when adding them diagonal speeds aren't wrong
		float speed = std::abs(*(float*)(TyBaseAddress + 0x270B98) * std::sin(TyPositionRotation::GetTyYawRot())) + //X
			std::abs(*(float*)(TyBaseAddress + 0x270BA0) * std::cos(TyPositionRotation::GetTyYawRot())); //Z
		return (speed);
	};
	inline float GetTyVerticalSpeed() { return *(float*)(TyBaseAddress + 0x270B9C); };

	//Bull
	inline float* GetBullSpeedPtr() { return (float*)(TyBaseAddress + 0x25462C); }; //Default 35
	inline float* GetBullHardcodedSpeedPtr() { return (float*)(TyBaseAddress + 0x43935); };

	inline void SetHardcodedBullSpeed() {
		DWORD oldProtection;
		//Change the memory access to ReadWrite to be able to change the hardcoded value (usually its read only)
		VirtualProtect(TyMovement::GetBullHardcodedSpeedPtr(), 4, PAGE_EXECUTE_READWRITE, &oldProtection);
		//The game sets the value used for the slider to this hardcoded value every ~2 seconds for some reason
		*TyMovement::GetBullHardcodedSpeedPtr() = *TyMovement::GetBullSpeedPtr();

		//Set it back to the old access protection
		VirtualProtect(TyMovement::GetBullHardcodedSpeedPtr(), 4, oldProtection, &oldProtection);
	}

	inline float GetBullHorizontalSpeed() {
		//Scale them based on the direction Bull is facing, so when adding them diagonal speeds aren't wrong
		float speed = std::abs(*(float*)(TyBaseAddress + 0x254288) * std::sin(TyPositionRotation::GetBullRot())) + //X
			std::abs(*(float*)(TyBaseAddress + 0x254290) * std::cos(TyPositionRotation::GetBullRot())); //Z
		return (speed);
	};
	inline float GetBullVerticalSpeed() { return *(float*)(TyBaseAddress + 0x25428C); };
};
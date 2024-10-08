#pragma once
#include <map>
#include <string>
#include "TyMemoryValues.h"

class TyState {
public:
	static inline std::map<int, std::string> Ty{
		{-1, "Null"},
		{0, "No State"},
		{1, "Biting"},
		{2, "Sneaking"},
		{3, "Walking"},
		{5, "Running"},
		{6, "Skidding to Stop"},
		{7, "Jumping"},
		{8, "Gliding Init"},
		{9, "Gliding"},
		{10, "Ledge Grab"},
		{11, "Bouncing on Mushroom"},
		{12, "In Flower"},
		{13, "Flower Launch"},
		{14, "Doggy Paddle Moving"},
		{15, "Swimming on Surface"},
		{16, "Going Underwater"},
		{17, "Swimming Underwater"},
		{19, "Surfacing Water"},
		{20, "Knockback/Hurt Underwater"},
		{21, "Rising in Water"}, //Rises until you hit surface ?? maybe old version of state 22
		{22, "Entering Shark Cage"},
		{23, "Inside Shark Cage"},
		{24, "Exiting Shark Cage"},
		{25, "Eaten by Big Fish"},
		{26, "Falling"},
		{27, "Long Fall"}, //Includes fall damage
		{28, "Dying"},
		{30, "Respawning"},
		{31, "Pushed Back"},
		{32, "Taking Damage"},
		{33, "Collecting TE/Cog"},
		{34, "Deep Breath"},
		{35, "Idle"},
		{36, "Balancing"},
		{37, "Doggy Paddle Idle"},
		{38, "Swim Surface Idle"},
		{39, "Underwater Idle"},
		{40, "Shaking Dry"},
		{41, "The Rotator"}, //No idea, rotates ty to the left slightly???
		{42, "Swapping Rangs"},
		{43, "Collecting Second Rang"}, //Locks all movement, unescapable
		{44, "On Waterslide"},
		{45, "Ty's View"},
		{46, "Slipping"},
		{47, "Diving"},
		{48, "Bit Ground"},
		{49, "Diving & in Water"},
		{50, "Credits Lock"},
		{51, "Quicksand"},
		{52, "Rex Diving"}
	};

	static inline std::map<int, std::string> Bull{
		{-1, "No State"},
		{0, "Idle"},
		{1, "Walking"},
		{2, "Running"},
		{3, "Falling"},
		{4, "Taking Damage"},
		{5, "Fall Damage"},
		{6, "Bonking"},
		{7, "Dying"},
		{8, "Respawning"},
		{9, "Charging"},
		{10, "Pulling Emu"},
		{11, "Collecting TE/Cog"},
		{12, "Jogging"}
	};

	//Seems to maybe be something that sets Ty's next state
	static int* GetTyStatePtr() { return (int*)(TyMemoryValues::TyBaseAddress + 0x271590); };
	//Slightly different from one, doesn't ever seem to be -1, and doesn't change to -1 at the start of a loading screen
	static int GetTyState() { return *(int*)(TyMemoryValues::TyBaseAddress + 0x26EE4C); };
	static int GetBullState() { return *(int*)(TyMemoryValues::TyBaseAddress + 0x254560); };
};
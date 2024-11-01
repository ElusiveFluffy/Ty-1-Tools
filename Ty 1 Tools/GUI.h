#pragma once
#include "imgui.h"
#include "TyPositionRotation.h"
#include <string>
class GUI
{
public:
	static inline bool init;

	static inline bool EnableLevelSelect;
	static inline bool DisableFallDamage;
	static inline bool AutoSetPosition;
	static inline bool DontAutoUpdatePosition;
	//Mostly used for the count in the UI and to easily set the amount of charge bites
	static inline ImU8 ChargeBiteCount;

	static void Initialize();

	static void DrawUI();
	static void RangsDrawUI();
	static void MovementDrawUI();
	static void PositionDrawUI();
	static void MiscDrawUI();

	static void SetImGuiStyle();
	static bool ImGuiWantCaptureMouse();

	class Overlay {
	public:
		inline static bool ShowOverlay;
		inline static int TextLineCount;
		inline static int LongestLine;
		inline static ImVec2 TextStartPos;
		inline static ImVec2 DropShadowOffset = ImVec2(2, 3);

		static void DrawOverlay();
		static void DrawLabelWithNumbers(ImDrawList* drawList, std::string label, std::string numberText);
		static void DrawDropShadowText(ImDrawList* drawList, const char* text, bool addNewLine = true, ImVec2 positionOffset = ImVec2(0, 0), ImFont* font = GUI::TyFont);

	private:
		//States
		static std::string TyStateText();
		static std::string BullStateText();
	};

private:
	static inline float FontSize = 20;
	static inline ImFont* TyFont;
	static inline ImFont* TyNumberFont;

	static inline int IntStepAmount = 1;
	static inline float FloatStepAmount = 10.0f;

	//Position
	static inline TyPositionRotation::Vector3 TyBullPos;
	static inline bool AnyChanged;

	static inline void AddToolTip(const char* toolTip);
};


#include "GUI.h"
#include "framework.h"
#include "TygerFrameworkAPI.hpp"
#include "TygerUtility.h"
#include "TeleportPositions.h"
#include <string>
#include <format>
#include <regex>

//Memory
#include "TyMemoryValues.h"
#include "TyAttributes.h"
#include "TyState.h"
#include "TyMovement.h"
#include "TyPositionRotation.h"
#include "Camera.h"
#include "Levels.h"
using namespace TyPositionRotation;

//Fonts
#include "Fonts/TyFont.hpp"
#include "Fonts/TyNumberFont.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "imgui_stdlib.h" //For std::string
#include "imgui_internal.h" //For Free Drawing

//WndProc to be able to interact with imgui or block any WndProc from interacting with the Ty window
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	//Teleport back to spawn from the position auto set
	if (msg == WM_KEYDOWN && TyMemoryValues::GetTyGameState() == TyMemoryValues::Gameplay &&
		((!TyState::IsBull() && TyState::GetTyState() != 0) ||
		(TyState::IsBull() && TyState::GetBullState() != -1)))
	{
		switch ((int)wParam)
		{
		case VK_HOME:
			auto position = TeleportPositions::SpawnPositions[Levels::GetCurrentLevelID()];
			AdvancedTeleportPlayer(position);
			GUI::Overlay::SetAndShowSlotText("Returned to Spawn", -1);
			break;
		// [{
		case VK_OEM_4:
			TeleportPositions::CurrentSlot = TeleportPositions::CurrentSlot == 0 ? TeleportPositions::SlotCount - 1 : TeleportPositions::CurrentSlot - 1;
			GUI::Overlay::SetAndShowSlotText("Changed to Slot", TeleportPositions::CurrentSlot);
			break;
		// ]}
		case VK_OEM_6:
			TeleportPositions::CurrentSlot = (TeleportPositions::CurrentSlot + 1) % TeleportPositions::SlotCount;
			GUI::Overlay::SetAndShowSlotText("Changed to Slot", TeleportPositions::CurrentSlot);
			break;
		case VK_F4:
			if (GetKeyState(VK_SHIFT) & 0x8000) {
				auto& positions = TeleportPositions::SavedPositions[Levels::GetCurrentLevelID()];

				if (!TyState::IsBull())
					positions[TeleportPositions::CurrentSlot] = { true, TyPositionRotation::GetTyPos(), TyPositionRotation::GetTyYawRot(), TyState::GetTyState(), Camera::GetCameraPos(), Camera::GetCameraRotYaw(), Camera::GetCameraRotPitch() };
				else
					positions[TeleportPositions::CurrentSlot] = { true, TyPositionRotation::GetBullPos(), TyPositionRotation::GetUnmodifiedBullRot(), TyState::GetBullState(), Camera::GetCameraPos(), Camera::GetCameraRotYaw(), Camera::GetCameraRotPitch() };

				//Save the positions every time its set just in case a crash happens and you lose your unsaved positions
				TeleportPositions::SavePositionsToFile();

				GUI::Overlay::SetAndShowSlotText("Saved slot", TeleportPositions::CurrentSlot);
			}
			else
			{
				auto position = TeleportPositions::SavedPositions[Levels::GetCurrentLevelID()][TeleportPositions::CurrentSlot];
				if (position.ValidSlot)
				{
					AdvancedTeleportPlayer(position);
					GUI::Overlay::SetAndShowSlotText("Loaded slot", TeleportPositions::CurrentSlot);
				}
				else
				{
					GUI::Overlay::SetAndShowSlotText("No position in slot", TeleportPositions::CurrentSlot);
				}
			}
			break;
		}
	}

	if (API::DrawingGUI())
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return true;

	return false;
}

void GUI::Initialize()
{
	//Need to cast this, otherwise TygerFramework won't get the return value
	API::AddPluginWndProc((WndProcFunc)WndProc);

	//Setup ImGui Context
	ImGui::CreateContext();

	//Set the font to be the same as TygerFramework
	API::SetImGuiFont(ImGui::GetIO().Fonts);
	SetImGuiStyle();

	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	if (API::GetTyWindowHandle() == 0) {
		API::LogPluginMessage("Error Getting Ty Window Handle", Error);
		return;
	}
	//Setup backend
	ImGui_ImplWin32_InitForOpenGL(API::GetTyWindowHandle());
	ImGui_ImplOpenGL3_Init();

	API::LogPluginMessage("Initialized ImGui");
	GUI::init = true;
}

void GUI::SetImGuiStyle() {
	ImFontAtlas* fonts = ImGui::GetIO().Fonts;

	ImFontConfig custom_icons{};
	custom_icons.FontDataOwnedByAtlas = false;

	GUI::TyFont = fonts->AddFontFromMemoryCompressedTTF(SfSlapstickComic_compressed_data, SfSlapstickComic_compressed_size, GUI::FontSize);
	GUI::TyNumberFont = fonts->AddFontFromMemoryCompressedTTF(TyNumberFont_compressed_data, TyNumberFont_compressed_size, 28);
	fonts->Build();
}

void GUI::DrawUI()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (Overlay::ShowOverlay)
		Overlay::DrawOverlay();

	if (Overlay::PosTextShowSeconds != 0.0f)
		Overlay::DrawPosSlotOverlay();

	if (API::DrawingGUI())
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(60, 420), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(390, 340), ImGuiCond_FirstUseEver);
		ImGui::Begin(TygerUtility::PluginName.c_str());
		ImGui::Checkbox("Show Debug Overlay", &Overlay::ShowOverlay);
		AddToolTip("Tip: You can drag the overlay around to place it anywhere you want\nThe overlay only shows during gameplay");

		//Game finished initializing
		if (TyMemoryValues::GetTyGameState() > 4)
		{
			//Is only true if the check box changes value
			if (ImGui::Checkbox("Enable Level Select", &EnableLevelSelect))
				TyMemoryValues::SetLevelSelect(EnableLevelSelect);

			ImGui::Spacing();

			if (ImGui::BeginTabBar("Tool Tabs")) {
				if (ImGui::BeginTabItem("Rangs")) {
					RangsDrawUI();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Movement")) {
					MovementDrawUI();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Position")) {
					PositionDrawUI();
					TeleportPositions::TeleportPosDrawUI();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Free Cam")) {
					FreeCamDrawUI();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Misc")) {
					MiscDrawUI();
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}
		else
			ImGui::Text("Game still initializing, please wait");

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::RangsDrawUI()
{
	if (ImGui::Button("Give All Rangs"))
		TyAttributes::SetAllRangs(true);
	ImGui::SameLine();
	if (ImGui::Button("Remove All Rangs"))
		TyAttributes::SetAllRangs(false);

	if (ImGui::Button("Give Element Rangs"))
		TyAttributes::SetElementRangs(true);
	ImGui::SameLine();
	if (ImGui::Button("Give Techno Rangs"))
		TyAttributes::SetTechnoRangs(true);

	if (ImGui::BeginTable("Rangs", 3)) {
		//Row 1
		ImGui::TableNextColumn(); ImGui::Checkbox("2nd Rang", TyAttributes::GetRangState(TyAttributes::Two));
		ImGui::TableNextColumn(); ImGui::Checkbox("Swim", TyAttributes::GetRangState(TyAttributes::Swim));
		ImGui::TableNextColumn(); ImGui::Checkbox("Dive", TyAttributes::GetRangState(TyAttributes::Dive));

		//Row 2
		ImGui::TableNextColumn(); ImGui::Checkbox("Boomerang", TyAttributes::GetRangState(TyAttributes::IronBark));
		ImGui::TableNextColumn(); ImGui::Checkbox("Flamerang", TyAttributes::GetRangState(TyAttributes::Flame));
		ImGui::TableNextColumn(); ImGui::Checkbox("Frostyrang", TyAttributes::GetRangState(TyAttributes::Frosty));

		//Row 3
		ImGui::TableNextColumn(); ImGui::Checkbox("Zappyrang", TyAttributes::GetRangState(TyAttributes::Zappy));
		ImGui::TableNextColumn(); ImGui::Checkbox("Aquarang", TyAttributes::GetRangState(TyAttributes::Aqua));
		ImGui::TableNextColumn(); ImGui::Checkbox("Zoomerang", TyAttributes::GetRangState(TyAttributes::Zoomer));

		//Row 4
		ImGui::TableNextColumn(); ImGui::Checkbox("Multirang", TyAttributes::GetRangState(TyAttributes::Multi));
		ImGui::TableNextColumn(); ImGui::Checkbox("Infrarang", TyAttributes::GetRangState(TyAttributes::Infra));
		ImGui::TableNextColumn(); ImGui::Checkbox("Megarang", TyAttributes::GetRangState(TyAttributes::Mega));

		//Row 5
		ImGui::TableNextColumn(); ImGui::Checkbox("Kaboomarang", TyAttributes::GetRangState(TyAttributes::Kaboom));
		ImGui::TableNextColumn(); ImGui::Checkbox("Chronorang", TyAttributes::GetRangState(TyAttributes::Chrono));
		ImGui::TableNextColumn(); ImGui::Checkbox("Doomarang", TyAttributes::GetRangState(TyAttributes::Doom));
	}
	ImGui::EndTable();
}

void GUI::FloatSliderElement(std::string text, float* valuePtr, float min, float max, float defaultValue) {
	ImGui::Text(text.c_str());
	ImGui::SetNextItemWidth(sliderWidth);
	ImGui::SliderFloat(("##" + text).c_str(), valuePtr, min, max);
	ImGui::SameLine();
	//Needs ## so it has a different internal name from all the others
	if (ImGui::Button(("Reset##Reset " + text).c_str()))
		*valuePtr = defaultValue;
}

void GUI::MovementDrawUI()
{
	if (!TyState::IsBull())
	{
		ImGui::Checkbox("Disable Fall Damage", &DisableFallDamage);
		AddToolTip("Also disables the long fall animation");

		ImGui::Spacing();
		if (ImGui::Button("Give Groundswim"))
			*TyState::GetTyStatePtr() = 39;

		ImGui::Spacing();
		FloatSliderElement("Glide Up/Down", TyMovement::GetGlideUpDownPtr(), 20, -20, 5.5f);

		ImGui::Spacing();
		FloatSliderElement("Glide Speed", TyMovement::GetGlideSpeedPtr(), 2.0f, 100, 7.0f);

		ImGui::Spacing();
		FloatSliderElement("Run Speed", TyMovement::GetRunSpeedPtr(), 1.0f, 100, 10.0f);

		ImGui::Spacing();
		FloatSliderElement("Jump Height", TyMovement::GetGroundJumpHeightPtr(), 5.0f, 100, 18.57417488f);

		ImGui::Spacing();
		FloatSliderElement("Airborne Speed", TyMovement::GetAirSpeedPtr(), 0.25f, 100, 10.0f);

		ImGui::Spacing();
		FloatSliderElement("Swim Speed", &TyMovement::SwimSpeed, 2.5f, 100, 20.0f);

		ImGui::Spacing();
		FloatSliderElement("Swim Surface Speed", TyMovement::GetSwimSurfaceSpeedPtr(), 1.0f, 100, 6.0f);

		ImGui::Spacing();
		FloatSliderElement("Water Jump Height", TyMovement::GetWaterJumpHeightPtr(), 0.25f, 100, 10.67707825f);
	}
	else
	{
		ImGui::Text("Bull Run Speed");
		ImGui::SetNextItemWidth(sliderWidth);
		//Only true when the slider changes
		if (ImGui::SliderFloat("##Bull Run Speed", TyMovement::GetBullSpeedPtr(), 0.25f, 200))
		{
			TyMovement::SetHardcodedBullSpeed();
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset##Reset Bull Run Speed"))
		{
			*TyMovement::GetBullSpeedPtr() = 35.0f;
			TyMovement::SetHardcodedBullSpeed();
		}
	}
}

int GUI::PositionTextBoxFilter(ImGuiInputTextCallbackData* data)
{
	//Only allow stuff that matches any of these characters
	if (strchr("-1234567890,. ", (char)data->EventChar))
		return 0;
	return 1;
}

std::vector<std::string> Split(const std::string str, const std::string regex_str)
{
	std::regex regexz(regex_str);
	std::vector<std::string> list(std::sregex_token_iterator(str.begin(), str.end(), regexz, -1),
		std::sregex_token_iterator());
	return list;
}

void GUI::PositionDrawUI()
{
	ImGui::Checkbox("Auto Teleport", &AutoSetPosition);
	AddToolTip("Automatically set's Ty/Bull's position when they're edited");
	ImGui::SameLine();
	ImGui::Checkbox("Don't Auto Update Position", &DontAutoUpdatePosition);

	ImGui::InputScalar("Step Amount", ImGuiDataType_Float, &FloatStepAmount);
	AddToolTip("Sets the amount the -/+ buttons add or subtract");
	if (!TyState::IsBull())
	{
		//Only auto update it if none have changed
		if (!AnyChanged && !DontAutoUpdatePosition)
		{
			TyBullPos = TyPositionRotation::GetTyPos();
			PositionText = std::format("{:.3f}, {:.3f}, {:.3f}", TyBullPos.X, TyBullPos.Y, TyBullPos.Z);
		}
		ImGui::Text("Ty Position (X, Y, Z):");
		SetPositionElements();
		if (ImGui::Button("Teleport") || (AutoSetPosition && AnyChanged))
		{
			auto posDelta = TyBullPos - TyPositionRotation::GetTyPos();

			Camera::SetCameraPos(Camera::GetCameraPos() + posDelta);
			TyPositionRotation::SetTyPos(TyBullPos);
			AnyChanged = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Update Position"))
			TyBullPos = TyPositionRotation::GetTyPos();
	}
	else
	{
		//Only auto update it if none have changed
		if (!AnyChanged && !DontAutoUpdatePosition)
		{
			TyBullPos = TyPositionRotation::GetBullPos();
			PositionText = std::format("{:.3f}, {:.3f}, {:.3f}", TyBullPos.X, TyBullPos.Y, TyBullPos.Z);
		}
		ImGui::Text("Bull Position (X, Y, Z):");
		SetPositionElements();
		if (ImGui::Button("Teleport") || (AutoSetPosition && AnyChanged))
		{
			auto posDelta = TyBullPos - TyPositionRotation::GetBullPos();

			Camera::SetCameraPos(Camera::GetCameraPos() + posDelta);
			TyPositionRotation::SetBullPos(TyBullPos);
			AnyChanged = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Update Position"))
			TyBullPos = TyPositionRotation::GetBullPos();
	}

}

void GUI::SetPositionElements()
{
	//To be able to copy and paste in the position
	ImGui::InputText("##Position", &PositionText, ImGuiInputTextFlags_CallbackCharFilter, GUI::PositionTextBoxFilter);
	if (ImGui::IsItemDeactivated())
	{
		std::vector<std::string> positions = Split(PositionText, ",");
		if (positions.size() == 3 && PositionText != std::format("{:.3f}, {:.3f}, {:.3f}", TyBullPos.X, TyBullPos.Y, TyBullPos.Z))
		{
			TyBullPos = { std::stof(positions[0]), std::stof(positions[1]), std::stof(positions[2]) };
			AnyChanged = true;
		}
	}
	//ImGui::Spacing();
	//Or just so if any have previously changed it'll keep it true
	//AnyChanged = ImGui::InputScalar("X", ImGuiDataType_Float, &TyBullPos.X, &FloatStepAmount) || AnyChanged;
	//AnyChanged = ImGui::InputScalar("Y", ImGuiDataType_Float, &TyBullPos.Y, &FloatStepAmount) || AnyChanged;
	//AnyChanged = ImGui::InputScalar("Z", ImGuiDataType_Float, &TyBullPos.Z, &FloatStepAmount) || AnyChanged;
}

void GUI::FreeCamDrawUI()
{
	//Only runs when the checkbox state changes
	if (ImGui::Checkbox("Enable Free Cam", &EnableFreeCam)) {
		if (EnableFreeCam)
		{
			Camera::SetCameraState(Camera::FreeCam);
			//Ty states get set in the tick before game event in TygerUtility.cpp
		}
		else
		{
			Camera::SetCameraState(Camera::Default);
			//Just always reset it just incase
			*TyState::GetTyStatePtr() = 35;
			TyState::SetBullState(0);
		}
	}
	//To unlock/lock it while free cam is active
	if (ImGui::Checkbox("Lock Ty's Movement During Free Cam", &LockTyMovement)) {
		if (LockTyMovement && EnableFreeCam)
		{
			*TyState::GetTyStatePtr() = 50;
			TyState::SetBullState(-1);
		}
		else
		{
			*TyState::GetTyStatePtr() = 35;
			TyState::SetBullState(0);
		}
	}
	AddToolTip("Makes Bull invisible as they have no locked state that they can be set to\nthat doesn't turn them invisible and doesn't cause any issues");

	ImGui::Spacing();
	FloatSliderElement("Free Cam Speed", Camera::GetFreeCamSpeedPtr(), 0.1f, 25, 0.6f);
}

void GUI::MiscDrawUI()
{
	if (ImGui::InputScalar("Charge Bite Count", ImGuiDataType_U8, &ChargeBiteCount, &IntStepAmount))
		*TyAttributes::GetChargeBiteOpalCounterPtr() = ChargeBiteCount * 100;

	ImGui::Checkbox("Ty Invincibility", TyState::TyInvincibility());
	AddToolTip("Only affects Ty, doesn't affect Bull");
}

void GUI::AddToolTip(const char* toolTip)
{
	ImGui::SameLine();
	ImGui::Text("(?)");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(toolTip);
}

bool GUI::ImGuiWantCaptureMouse()
{
	return ImGui::GetIO().WantCaptureMouse;
}

std::string GUI::Overlay::AddSpacesBeforeCapitalAndNum(std::string text)
{
	std::string new_str = "";
	bool lastCharWasNumber = false;
	//Iterate through the characters in the string (except the last character)
	for (UINT i = 0; i < (text.length() - 1); i++) {
		new_str += text[i]; // Append the current character to the new string

		bool isNum = isdigit(text[i + 1]);
		//Check if the next character is uppercase or a number, group numbers together
		if (isupper(text[i + 1]) || (!lastCharWasNumber && isNum)) {
			//If the next character is uppercase, insert a space in the new string
			new_str += " ";
			lastCharWasNumber = isNum;
		}
	}
	//Append the last character of the input string to the new string
	new_str += text.back();
	return new_str;
}

std::string GUI::Overlay::TyStateText() {
	//If the map doesn't contain the state just return a blank string
	if (!TyState::Ty.contains(TyState::GetTyState()))
		return "";

	return TyState::Ty[TyState::GetTyState()];
}

std::string GUI::Overlay::BullStateText() {
	//If the map doesn't contain the state just return a blank string
	if (!TyState::Bull.contains(TyState::GetBullState()))
		return "";

	return TyState::Bull[TyState::GetBullState()];
}

std::string GUI::Overlay::CameraStateText()
{
	//If the map doesn't contain the state just return a blank string
	if (!Camera::StateNames.contains(Camera::GetCameraState()))
		return "";

	return Camera::StateNames[Camera::GetCameraState()];
}

void GUI::Overlay::DrawOverlay()
{
	ImGuiIO& io = ImGui::GetIO();

	//Remove all window elements
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::Begin((TygerUtility::PluginName + " Overlay").c_str(), nullptr, ImGuiWindowFlags_NoDecoration);
	//Reset the counts
	TextLineCount = 0;
	LongestLine = 0;

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImDrawList* drawList = window->DrawList;
	//Set the text start pos to the window pos
	TextStartPos = window->Pos;

	//Only draw this overlay elements during gameplay
	if (TyMemoryValues::GetTyGameState() == TyMemoryValues::Gameplay)
	{
		//Only Show the Values for the bull if in outback safari
		if (!TyState::IsBull())
		{
			DrawDropShadowText(drawList, "Ty:");
			Vector3 tyPos = TyPositionRotation::GetTyPos();
			DrawLabelWithNumbers(drawList, "Pos:", std::format("{:.2f}, {:.2f}, {:.2f}", tyPos.X, tyPos.Y, tyPos.Z));
			DrawLabelWithNumbers(drawList, "Rot:", std::format("{:.3f}, {:.3f}", TyPositionRotation::GetTyYawRot(), TyPositionRotation::GetTyPitchRot()));

			DrawDropShadowText(drawList, ("State: (" + std::to_string(TyState::GetTyState()) + ") " + TyStateText()).c_str());
			DrawDropShadowText(drawList, TyState::GetGroundIDName());
			std::string tyAnimText = AddSpacesBeforeCapitalAndNum(std::string(TyState::GetTyAnimationName()));
			DrawDropShadowText(drawList, ("Anim: " + tyAnimText).c_str());
			//DrawLabelWithNumbers(drawList, "Floor ID:", std::to_string(TyMemoryValues::GetTyFloorID()));
			DrawDropShadowText(drawList, ("Camera State: (" + std::to_string(Camera::GetCameraState()) + ") " + CameraStateText()).c_str());

			DrawLabelWithNumbers(drawList, "Horizontal Speed:", std::format("{:.3f}", TyMovement::GetTyHorizontalSpeed()));
			DrawLabelWithNumbers(drawList, "Vertical Speed:", std::format("{:.3f}", TyMovement::GetTyVerticalSpeed()));
		}
		else
		{
			DrawDropShadowText(drawList, "Bull:");
			Vector3 bullPos = TyPositionRotation::GetBullPos();
			DrawLabelWithNumbers(drawList, "Pos:", std::format("{:.2f}, {:.2f}, {:.2f}", bullPos.X, bullPos.Y, bullPos.Z));
			DrawLabelWithNumbers(drawList, "Rot:", std::format("{:.3f}", TyPositionRotation::GetBullRot()));

			DrawDropShadowText(drawList, ("State: (" + std::to_string(TyState::GetBullState()) + ") " + BullStateText()).c_str());
			DrawDropShadowText(drawList, TyState::GetGroundIDName());
			std::string bullAnimText = AddSpacesBeforeCapitalAndNum(std::string(TyState::GetBullAnimationName()));
			DrawDropShadowText(drawList, ("Anim: " + bullAnimText).c_str());
			DrawDropShadowText(drawList, ("Camera State: (" + std::to_string(Camera::GetCameraState()) + ") " + CameraStateText()).c_str());

			DrawLabelWithNumbers(drawList, "Horizontal Speed:", std::format("{:.3f}", TyMovement::GetBullHorizontalSpeed()));
			DrawLabelWithNumbers(drawList, "Vertical Speed:", std::format("{:.3f}", TyMovement::GetBullVerticalSpeed()));
		}
	}

	//Only set the pos on the first time loading the plugin
	ImGui::SetWindowPos(ImVec2(io.DisplaySize.x - 550, 150), ImGuiCond_FirstUseEver);
	//Auto resize to content
	ImGui::SetWindowSize(ImVec2(LongestLine * 13.71f, TextLineCount* (FontSize + 5)), ImGuiCond_Always);

	drawList->PushClipRectFullScreen();
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
}

void GUI::Overlay::DrawLabelWithNumbers(ImDrawList* drawList, std::string label, std::string numberText)
{
	DrawDropShadowText(drawList, label.c_str(), false);
	DrawDropShadowText(drawList, numberText.c_str(), true, ImVec2((label.length() * 13.71f), -5), TyNumberFont);

	//Work out if this line is longer than any of the other ones (needs to be done here too to check the two combined)
	int lineLength = (label.length() + numberText.length());
	if (lineLength > LongestLine)
		LongestLine = lineLength;
}

void GUI::Overlay::DrawDropShadowText(ImDrawList* drawList, const char* text, bool addNewLine, ImVec2 positionOffset, ImFont* font)
{
	//Pos
	float x = TextStartPos.x + positionOffset.x;
	float y = TextStartPos.y + (TextLineCount * (FontSize + 5)) + positionOffset.y;
	//Draw text
	drawList->AddText(font, font->FontSize, ImVec2(x + DropShadowOffset.x, y + DropShadowOffset.y), IM_COL32(10, 10, 10, 255), text); //Drop shadow text
	drawList->AddText(font, font->FontSize, ImVec2(x, y), IM_COL32(255, 255, 255, 255), text); //Normal text
	//Add New Line
	if (addNewLine)
		TextLineCount++;

	//Work out if this line is longer than any of the other ones
	int lineLength = strlen(text);
	if (lineLength > LongestLine)
		LongestLine = lineLength;
}

void GUI::Overlay::DrawPosSlotOverlay()
{
	ImGuiIO& io = ImGui::GetIO();

	//Remove all window elements
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::Begin((TygerUtility::PluginName + " Slot Overlay").c_str(), nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs);

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImDrawList* drawList = window->DrawList;
	//Set the text start pos to the window pos
	TextStartPos = window->Pos;

	//Fade in and out
	ImU32 opacity = 255;
	if (PosTextShowSeconds > 1.6f)
		opacity = ((0.4f - (PosTextShowSeconds - 1.6f)) / 0.4f) * 255;
	else if (PosTextShowSeconds < 0.4f)
		opacity = (PosTextShowSeconds / 0.4f) * 255;

	//Draw text
	const char* charText = PosSlotText.c_str();
	drawList->AddText(TyFont, TyFont->FontSize, ImVec2(TextStartPos.x + DropShadowOffset.x, TextStartPos.y + DropShadowOffset.y), IM_COL32(10, 10, 10, opacity), charText); //Drop shadow text
	drawList->AddText(TyFont, TyFont->FontSize, ImVec2(TextStartPos.x, TextStartPos.y), IM_COL32(255, 255, 255, opacity), charText); //Normal text

	//Get the text size with the Ty font
	ImGui::PushFont(TyFont);
	ImVec2 size = ImGui::CalcTextSize(charText);
	//Reset the current font back to the default font
	ImGui::PopFont();

	//Only set the pos on the first time loading the plugin
	ImGui::SetWindowPos(ImVec2(io.DisplaySize.x - (size.x + 10), io.DisplaySize.y - (size.y + 10)), ImGuiCond_Always);
	//Auto resize to content
	ImGui::SetWindowSize({size.x + DropShadowOffset.x, size.y + DropShadowOffset.y}, ImGuiCond_Always);

	drawList->PushClipRectFullScreen();
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
}

void GUI::Overlay::SetAndShowSlotText(std::string text, int slotNumber)
{
	//-1 used for the spawn teleport
	if (slotNumber != -1)
		GUI::Overlay::PosSlotText = text + " " + std::to_string(slotNumber);
	else
		GUI::Overlay::PosSlotText = text;
	GUI::Overlay::PosTextShowSeconds = GUI::Overlay::ShowTime;
}

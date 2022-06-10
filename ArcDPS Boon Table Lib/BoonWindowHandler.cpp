#include "BoonWindowHandler.h"

#include "Tracker.h"

#include "imgui/imgui.h"

#include <format>

void BoonWindowHandler::Draw() {
	if (ImGui::Begin("Boon Table Test Window")) {
		for (Player& player : Tracker::instance().mPlayers) {
			Boon& boon = player.mBoons[magic_enum::enum_integer(Boons::Might)];
			ImGui::TextUnformatted(std::format("{}: {}", player.mCharacterName, boon.mCurrentStacks).c_str());
		}

		ImGui::End();
	}
}

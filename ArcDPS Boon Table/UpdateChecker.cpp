#include "UpdateChecker.h"

UpdateChecker updateChecker;

void UpdateChecker::Draw() {
	if (update_available && shown) {
        ImGui::Begin("Arcdps Boon Table Update##BoonTableUpdate", &shown);
        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "A new update for the Boon Table plugin is available.");
        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Current version: %.0f.%.0f.%.0f", version.x, version.y, version.z);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "New version: %.0f.%.0f.%.0f", newVersion.x, newVersion.y, newVersion.z);
		if (ImGui::Button("Open download page")) {
            ShellExecuteA(nullptr, nullptr, "https://github.com/knoxfighter/GW2-ArcDPS-Boon-Table/releases/latest", nullptr, nullptr, SW_SHOW);
		}
        ImGui::End();
	}
}

#include "UpdateChecker.h"

#include <string>

#include "Lang.h"

UpdateChecker updateChecker;

void UpdateChecker::Draw() {
	if (update_available && shown) {
        std::string headerName = lang.translate(LangKey::UpdateWindowHeader);
        headerName.append("##BoonTableUpdate");
        ImGui::Begin(headerName.c_str(), &shown);
        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), lang.translate(LangKey::UpdateDesc).c_str());
        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "%s: %.0f.%.0f.%.0f", lang.translate(LangKey::UpdateCurrentVersion).c_str(), version.x, version.y, version.z);
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "%s: %.0f.%.0f.%.0f", lang.translate(LangKey::UpdateNewVersion).c_str(), newVersion.x, newVersion.y, newVersion.z);
		if (ImGui::Button("Open download page")) {
            ShellExecuteA(nullptr, nullptr, "https://github.com/knoxfighter/GW2-ArcDPS-Boon-Table/releases/latest", nullptr, nullptr, SW_SHOW);
		}
        ImGui::End();
	}
}

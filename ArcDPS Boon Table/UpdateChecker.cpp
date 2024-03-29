#include "UpdateChecker.h"

#include <string>
#include <thread>

#include "Helpers.h"
#include "Lang.h"

UpdateChecker updateChecker;

void UpdateChecker::Draw() {
	if (update_status != Status::Unknown && shown) {
		std::string headerName = lang.translate(LangKey::UpdateWindowHeader);
		headerName.append("##BoonTableUpdate");
		ImGui::Begin(headerName.c_str(), &shown, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), lang.translate(LangKey::UpdateDesc).c_str());
		ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "%s: %i.%i.%i", lang.translate(LangKey::UpdateCurrentVersion).c_str(), version[0], version[1],
		                   version[2]);
		ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "%s: %i.%i.%i", lang.translate(LangKey::UpdateNewVersion).c_str(), newVersion[0], newVersion[1],
		                   newVersion[2]);
		if (ImGui::Button(lang.translate(LangKey::UpdateOpenPage).c_str())) {
			std::thread([](){
				ShellExecuteA(nullptr, nullptr, "https://github.com/knoxfighter/GW2-ArcDPS-Boon-Table/releases/latest", nullptr, nullptr, SW_SHOW);
			}).detach();
		}

		switch (update_status) {
			case Status::UpdateAvailable: {
				if (ImGui::Button(lang.translate(LangKey::UpdateAutoButton).c_str())) {
					UpdateAutomatically(self_dll);
				}
				break;
			}
			case Status::UpdatingInProgress: {
				ImGui::TextUnformatted(lang.translate(LangKey::UpdateInProgress).c_str());
				break;
			}
			case Status::RestartPending: {
				ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), lang.translate(LangKey::UpdateRestartPending).c_str());
				break;
			}
			case Status::UpdateError: {
				ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), lang.translate(LangKey::UpdateError).c_str());
			}
		}

		ImGui::End();
	}
}

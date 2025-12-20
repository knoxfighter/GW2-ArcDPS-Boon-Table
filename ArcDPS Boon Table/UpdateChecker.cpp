#include "UpdateChecker.h"

#include <string>
#include <thread>

#include "Helpers.h"
#include "Lang.h"

UpdateChecker updateChecker;

void UpdateChecker::Draw() {
	if (!update_state)
	{
		return;
	}

	std::lock_guard lock(update_state->Lock);

	if (update_state->UpdateStatus != Status::Unknown && update_state->UpdateStatus != Status::Dismissed)
	{
		const Version& version = *update_state->CurrentVersion;
		const Version& newVersion = update_state->NewVersion;

		std::string headerName = lang.translate(LangKey::UpdateWindowHeader);
		headerName.append("##BoonTableUpdate");
		bool shown = true;
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

		switch (update_state->UpdateStatus) {
			case Status::UpdateAvailable: {
				if (ImGui::Button(lang.translate(LangKey::UpdateAutoButton).c_str())) {
					PerformInstallOrUpdate(*update_state);
				}
				break;
			}
			case Status::UpdateInProgress: {
				ImGui::TextUnformatted(lang.translate(LangKey::UpdateInProgress).c_str());
				break;
			}
			case Status::UpdateSuccessful: {
				ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), lang.translate(LangKey::UpdateRestartPending).c_str());
				break;
			}
			case Status::UpdateError: {
				ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), lang.translate(LangKey::UpdateError).c_str());
			}
		}

		ImGui::End();

		if (!shown)
		{
			update_state->UpdateStatus = Status::Dismissed;
		}
	}
}

void UpdateChecker::CheckForUpdate(HMODULE dll, const Version& currentVersion, std::string&& repo, bool allowPreRelease)
{
	ClearFiles(dll);
	update_state = UpdateCheckerBase::CheckForUpdate(dll, currentVersion, std::move(repo), allowPreRelease);
}

void UpdateChecker::FinishPendingTasks()
{
	if (update_state)
	{
		update_state->FinishPendingTasks();
	}
}

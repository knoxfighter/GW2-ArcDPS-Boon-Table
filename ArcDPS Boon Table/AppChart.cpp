#include "AppChart.h"



AppChart::AppChart()
{
}


AppChart::~AppChart()
{
}

void AppChart::Draw(const char* title, bool* p_open = nullptr, Tracker* tracker = nullptr)
{
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiSetCond_FirstUseEver);
	ImGui::Begin(title, p_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);
	ImGui::PushAllowKeyboardFocus(false);

	//menu
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Show..."))
		{
			for (std::list<BoonDef>::iterator boon = tracked_buffs.begin(); boon != tracked_buffs.end(); ++boon)
			{
				ImGui::MenuItem(boon->name.c_str(), NULL, &boon->is_relevant);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	int column_number = 1;
	for (auto current_buff : tracked_buffs)
	{
		if (current_buff.is_relevant) column_number++;
	}
	
	ImGui::Columns(column_number);
	ImGui::Text("Name");

	float current_boon_uptime = 0.0f;

	std::lock_guard<std::mutex> lock(tracker->players_mtx);

	for (auto current_player : tracker->players)
	{
		ImGui::Text(current_player.name.c_str());
	}

	for (auto current_buff : tracked_buffs)
	{
		if (!current_buff.is_relevant)continue;

		ImGui::NextColumn();
		
		ImGui::Text(current_buff.name.c_str());

		for (auto current_player : tracker->players)
		{
			current_boon_uptime = current_player.getBoonUptime(current_buff.id);

			if (current_buff.is_duration_stacking)
			{
				current_boon_uptime = current_boon_uptime > 1 ? 1 : current_boon_uptime;
				ImGui::ProgressBar(current_boon_uptime, ImVec2(-1, ImGui::GetFontSize()));
			}
			else
			{
				current_boon_uptime = current_boon_uptime > 25 ? 25 : current_boon_uptime;
				char label[5];
				sprintf(label, "%.1f", current_boon_uptime);
				current_boon_uptime /= 25;
				ImGui::ProgressBar(current_boon_uptime, ImVec2(-1, ImGui::GetFontSize()), label);
			}
		}
	}
	ImGui::Columns(1);
	
	

	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
}
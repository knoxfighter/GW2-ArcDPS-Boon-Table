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
	ImGui::Begin(title, p_open, ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	int column_number = 1;
	for (auto current_buff : tracked_buffs)
	{
		if (current_buff.is_relevant) column_number++;
	}
	
	ImGui::Columns(column_number);
	ImGui::Text("Name");

	uint16_t index = 0;
	float current_boon_uptime = 0.0f;

	std::lock_guard<std::mutex> lock(tracker->players_mtx);

	for (auto current_player : tracker->players)
	{
		ImGui::Text(current_player.name.c_str());
		index++;
	}

	for (auto current_buff : tracked_buffs)
	{
		if (!current_buff.is_relevant)continue;

		index = 0;
		ImGui::NextColumn();
		
		ImGui::Text(current_buff.name.c_str());

		for (auto current_player : tracker->players)
		{
			current_boon_uptime = current_player.getBoonUptime(current_buff.id);

			if (current_buff.is_duration_stacking)
			{
				current_boon_uptime *= 100.0f;
				current_boon_uptime = current_boon_uptime > 100 ? 100 : current_boon_uptime;
				ImGui::Text("%d%%", (int)current_boon_uptime);
			}
			else
			{
				current_boon_uptime = current_boon_uptime > 25 ? 25 : current_boon_uptime;
				ImGui::Text("%.1f", current_boon_uptime);
			}
			
			index++;
		}
	}
	ImGui::Columns(1);
	
	

	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
}
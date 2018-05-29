#include "AppChart.h"



AppChart::AppChart()
{
}


AppChart::~AppChart()
{
}

void AppChart::Draw(const char* title, bool* p_open = nullptr, Tracker* tracker = nullptr,ImGuiWindowFlags flags)
{
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiSetCond_FirstUseEver);
	ImGui::Begin(title, p_open, flags);
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

	for (std::list<Player>::iterator current_player = tracker->players.begin(); current_player != tracker->players.end(); ++current_player)
	{
		ImGui::Text(current_player->name.c_str());
	}

	for (auto current_subgroup : tracker->subgroups)
	{
		ImGui::Text("Sub %d", current_subgroup);
	}

	if (bShowTotal(tracker)) ImGui::Text("Total");

	for (std::list<BoonDef>::iterator current_buff = tracked_buffs.begin(); current_buff != tracked_buffs.end(); ++current_buff)
	{
		if (!current_buff->is_relevant)continue;

		ImGui::NextColumn();
		
		ImGui::Text(current_buff->name.c_str());

		//players
		for (std::list<Player>::iterator current_player = tracker->players.begin(); current_player != tracker->players.end(); ++current_player)
		{
			current_boon_uptime = current_player->getBoonUptime(&*current_buff);

			buffProgressBar(&*current_buff, current_boon_uptime);
		}
		//subgroups
		for (auto current_subgroup : tracker->subgroups)
		{
			current_boon_uptime = tracker->getSubgroupBoonUptime(&*current_buff, current_subgroup);

			buffProgressBar(&*current_buff, current_boon_uptime);
		}
		//total
		if (bShowTotal(tracker))
		{
			current_boon_uptime = tracker->getAverageBoonUptime(&*current_buff);

			buffProgressBar(&*current_buff, current_boon_uptime);
		}
	}
	ImGui::Columns(1);
	
	

	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
}

void buffProgressBar(BoonDef* current_buff, float current_boon_uptime)
{
	if (current_buff->is_duration_stacking)
	{
		ImGui::ProgressBar(current_boon_uptime, ImVec2(-1, ImGui::GetFontSize()));
	}
	else
	{
		char label[5];
		sprintf(label, "%.1f", current_boon_uptime);
		current_boon_uptime /= 25;
		ImGui::ProgressBar(current_boon_uptime, ImVec2(-1, ImGui::GetFontSize()), label);
	}
}

bool bShowTotal(Tracker* tracker)
{
	return tracker->players.size() > 1;
}

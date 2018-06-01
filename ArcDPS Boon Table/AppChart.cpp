#include "AppChart.h"



AppChart::AppChart()
{
	active_player = -1;
	active_column = -1;
	last_active_player = -1;
	last_active_column = -1;
}


AppChart::~AppChart()
{
}

void AppChart::Draw(const char* title, bool* p_open = nullptr, Tracker* tracker = nullptr,ImGuiWindowFlags flags)
{
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiSetCond_FirstUseEver);
	ImGui::Begin(title, p_open, flags);
	ImGui::PushAllowKeyboardFocus(false);
	last_active_player = active_player;
	last_active_column = active_column;
	active_player = -1;
	active_column = -1;

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
		if (last_active_player == current_player->id)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, active_bar_color);
		}
		ImGui::Text(current_player->name.c_str());
		if (ImGui::IsItemHoveredRect())
		{
			active_player = current_player->id;
			active_column = ImGui::GetColumnIndex();
		}
		if (last_active_player == current_player->id)
		{
			ImGui::PopStyleColor();
		}
	}

	for (auto current_subgroup : tracker->subgroups)
	{
		if (last_active_player == current_subgroup)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, active_bar_color);
		}
		ImGui::Text("Sub %d", current_subgroup);
		if (ImGui::IsItemHoveredRect())
		{
			active_player = current_subgroup;
			active_column = ImGui::GetColumnIndex();
		}
		if (last_active_player == current_subgroup)
		{
			ImGui::PopStyleColor();
		}
	}

	if (bShowTotal(tracker)) ImGui::Text("Total");

	for (std::list<BoonDef>::iterator current_buff = tracked_buffs.begin(); current_buff != tracked_buffs.end(); ++current_buff)
	{
		if (!current_buff->is_relevant)continue;

		ImGui::NextColumn();
		
		if (last_active_column == ImGui::GetColumnIndex())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, active_bar_color);
		}
		
		ImGui::Text(current_buff->name.c_str());
		if (ImGui::IsItemHoveredRect())
		{
			active_player = -1;
			active_column = ImGui::GetColumnIndex();
		}
		//players
		for (std::list<Player>::iterator current_player = tracker->players.begin(); current_player != tracker->players.end(); ++current_player)
		{
			current_boon_uptime = current_player->getBoonUptime(&*current_buff);

			buffProgressBar(&*current_buff, current_boon_uptime, current_player->id);
		}
		//subgroups
		for (auto current_subgroup : tracker->subgroups)
		{
			current_boon_uptime = tracker->getSubgroupBoonUptime(&*current_buff, current_subgroup);

			buffProgressBar(&*current_buff, current_boon_uptime, current_subgroup);
		}
		//total
		if (bShowTotal(tracker))
		{
			current_boon_uptime = tracker->getAverageBoonUptime(&*current_buff);

			buffProgressBar(&*current_buff, current_boon_uptime, 0);
		}
		if (last_active_column == ImGui::GetColumnIndex())
		{
			ImGui::PopStyleColor();
		}
	}
	ImGui::Columns(1);
	
	

	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
}

void AppChart::buffProgressBar(BoonDef* current_buff, float current_boon_uptime, uintptr_t current_player)
{
	if (last_active_player == current_player)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, active_bar_color);
	}
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
	if (last_active_player == current_player)
	{
		ImGui::PopStyleColor();
	}
	if (ImGui::IsItemHoveredRect())
	{
		active_player = current_player;
		active_column = ImGui::GetColumnIndex();
	}
}

bool bShowTotal(Tracker* tracker)
{
	return tracker->players.size() > 1;
}

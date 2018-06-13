#include "AppChart.h"



AppChart::AppChart()
{
	active_player = -1;
	active_column = -1;
	last_active_player = -1;
	last_active_column = -1;
	sorting_collumn = 1;
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

	std::lock_guard<std::mutex> lock(tracker->players_mtx);

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

	int column_number = 2;
	for (auto current_buff : tracked_buffs)
	{
		if (current_buff.is_relevant) column_number++;
	}
	
	ImGui::Columns(column_number,"Players");
	if (ImGui::SmallButton("Name")) tracker->setSortMethod(name);

	float current_boon_uptime = 0.0f;

	for (std::list<Player>::iterator current_player = tracker->players.begin(); current_player != tracker->players.end(); ++current_player)
	{
		highlightedText(current_player->id, current_player->name.c_str());
	}

	//show player subgroup numbers
	ImGui::NextColumn();
	if (last_active_column == ImGui::GetColumnIndex())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, active_bar_color);
	}
	if (ImGui::SmallButton("Subgrp")) tracker->setSortMethod(subgroup);
	for (std::list<Player>::iterator current_player = tracker->players.begin(); current_player != tracker->players.end(); ++current_player)
	{
		highlightedText(current_player->id, "%d", current_player->subgroup);
	}
	
	if (last_active_column == ImGui::GetColumnIndex())
	{
		ImGui::PopStyleColor();
	}

	//show boon uptimes
	for (std::list<BoonDef>::iterator current_buff = tracked_buffs.begin(); current_buff != tracked_buffs.end(); ++current_buff)
	{
		if (!current_buff->is_relevant)continue;

		ImGui::NextColumn();
		
		if (last_active_column == ImGui::GetColumnIndex())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, active_bar_color);
		}
		
		if (ImGui::SmallButton(current_buff->name.c_str())) tracker->setSortMethod(boon, &*current_buff);
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

		if (last_active_column == ImGui::GetColumnIndex())
		{
			ImGui::PopStyleColor();
		}
	}
	ImGui::Columns(1);

	if (bShowSubgroups(tracker))
	{
		ImGui::Separator();

		ImGui::Columns(column_number,"Subgroups");

		for (auto current_subgroup : tracker->subgroups)
		{
			highlightedText(current_subgroup, "Subgroup");
		}
		
		ImGui::NextColumn();

		for (std::list<uint8_t>::iterator current_subgroup = tracker->subgroups.begin(); current_subgroup != tracker->subgroups.end(); ++current_subgroup)
		{
			highlightedText(*current_subgroup, "%d", *current_subgroup);
		}

		for (std::list<BoonDef>::iterator current_buff = tracked_buffs.begin(); current_buff != tracked_buffs.end(); ++current_buff)
		{
			if (!current_buff->is_relevant)continue;

			ImGui::NextColumn();

			for (auto current_subgroup : tracker->subgroups)
			{
				current_boon_uptime = tracker->getSubgroupBoonUptime(&*current_buff, current_subgroup);

				buffProgressBar(&*current_buff, current_boon_uptime, current_subgroup);
			}
		}
		ImGui::Columns(1);
	}
	
	if (bShowTotal(tracker))
	{
		ImGui::Separator();

		ImGui::Columns(column_number, "Total");

		highlightedText(12345, "Total");

		ImGui::NextColumn();
		
		highlightedText(12345, "All");

		for (std::list<BoonDef>::iterator current_buff = tracked_buffs.begin(); current_buff != tracked_buffs.end(); ++current_buff)
		{
			if (!current_buff->is_relevant)continue;

			ImGui::NextColumn();

			current_boon_uptime = tracker->getAverageBoonUptime(&*current_buff);

			buffProgressBar(&*current_buff, current_boon_uptime, 12345);
		}
		ImGui::Columns(1);
	}

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

void AppChart::highlightedText(uintptr_t player_id, const char* fmt, ...)
{
	if (last_active_player == player_id)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, active_bar_color);
	}
	va_list args;
	va_start(args, fmt);
	ImGui::TextV(fmt,args);
	va_end(args);

	if (ImGui::IsItemHoveredRect())
	{
		active_player = player_id;
		active_column = ImGui::GetColumnIndex();
	}
	if (last_active_player == player_id)
	{
		ImGui::PopStyleColor();
	}
}

bool bShowSubgroups(Tracker* tracker)
{
	return tracker->subgroups.size()>1;
}

bool bShowTotal(Tracker* tracker)
{
	return tracker->players.size() > 1;
}

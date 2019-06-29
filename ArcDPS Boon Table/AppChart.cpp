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

void AppChart::Draw(const char* title, bool* p_open = nullptr, Tracker* tracker = nullptr,ImGuiWindowFlags flags = 0)
{
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiSetCond_FirstUseEver);
	ImGui::Begin(title, p_open, flags);
	last_active_player = active_player;
	last_active_column = active_column;
	active_player = INDEX_NONE;
	active_column = INDEX_NONE;
	float current_boon_uptime = 0.0f;
	bool _show_subgroups = bShowSubgroups(tracker);

	if (ImGui::BeginPopupContextItem("Options") || ImGui::BeginPopupContextWindow("Options"))
	{
		drawRtClickMenu(tracker);
		ImGui::EndPopup();
	}
	
	std::lock_guard<std::mutex> lock(tracker->players_mtx);
	std::lock_guard<std::mutex> lock2(boons_mtx);

	int column_number = 0;
	column_number += _show_subgroups;
	for (auto current_buff : tracked_buffs)
	{
		if (current_buff.is_relevant) column_number++;
	}
	float column_spacing = (ImGui::GetWindowContentRegionWidth() - tracker->max_character_name_size) / (column_number);

	//show headers	
	if (highlightedSmallButton(INDEX_SORTING_BUTTON, "Name")) tracker->setSortMethod(SortMethod_name);
	ImGui::SameLine(tracker->max_character_name_size + ImGui::GetStyle().ItemSpacing.x);
	current_column++;

	if (_show_subgroups)
	{
		if (highlightedSmallButton(INDEX_SORTING_BUTTON, "Subgrp")) tracker->setSortMethod(SortMethod_subgroup);
		ImGui::SameLine(0, column_spacing - ImGui::GetItemRectSize().x);
		current_column++;
	}
	
	for (auto current_buff = tracked_buffs.begin(); current_buff != tracked_buffs.end(); ++current_buff)
	{
		if (!current_buff->is_relevant)continue;

		if (highlightedSmallButton(INDEX_SORTING_BUTTON, current_buff->name.c_str())) tracker->setSortMethod(SortMethod_boon, &*current_buff);
		ImGui::SameLine(0, column_spacing - ImGui::GetItemRectSize().x);
		current_column++;
	}

	ImGui::NewLine();
	current_column = 0;

	ImGui::BeginChild("Scrolling");
	//show players
	if (bShowPlayers(tracker))
	{
		ImGui::Separator();

		//show player names
		for (auto current_player = tracker->players.begin(); current_player != tracker->players.end(); ++current_player)
		{
			if (!current_player->is_relevant) continue;
			highlightedText(current_player->id, current_player->name.c_str());
			ImGui::SameLine(tracker->max_character_name_size + ImGui::GetStyle().ItemSpacing.x);
			current_column++;

			if (_show_subgroups)
			{
				highlightedText(current_player->id, "%d", current_player->subgroup);
				ImGui::SameLine(0,column_spacing - ImGui::GetItemRectSize().x);
				current_column++;
			}

			for (auto current_buff = tracked_buffs.begin(); current_buff != tracked_buffs.end(); ++current_buff)
			{
				if (!current_buff->is_relevant)continue;

				current_boon_uptime = getPlayerDisplayValue(tracker, &*current_player, &*current_buff);
				
				buffProgressBar(&*current_buff, current_boon_uptime, &*current_player, current_player->id, column_spacing - ImGui::GetStyle().ItemSpacing.x);
				ImGui::SameLine(0, column_spacing - ImGui::GetItemRectSize().x);
				current_column++;
			}
			ImGui::NewLine();
			current_column = 0;
		}
	}

	if (_show_subgroups)
	{
		ImGui::Separator();

		for (auto current_subgroup = tracker->subgroups.begin(); current_subgroup != tracker->subgroups.end(); ++current_subgroup)
		{
			highlightedText(*current_subgroup, "Subgroup");
			ImGui::SameLine(tracker->max_character_name_size + ImGui::GetStyle().ItemSpacing.x);
			current_column++;

			highlightedText(*current_subgroup, "%d", *current_subgroup);
			ImGui::SameLine(0, column_spacing - ImGui::GetItemRectSize().x);
			current_column++;

			for (auto current_buff = tracked_buffs.begin(); current_buff != tracked_buffs.end(); ++current_buff)
			{
				if (!current_buff->is_relevant)continue;

				current_boon_uptime = tracker->getSubgroupBoonUptime(&*current_buff, *current_subgroup);

				buffProgressBar(&*current_buff, current_boon_uptime, nullptr, *current_subgroup, column_spacing - ImGui::GetStyle().ItemSpacing.x);
				ImGui::SameLine(0, column_spacing - ImGui::GetItemRectSize().x);
				current_column++;
			}
			ImGui::NewLine();
			current_column = 0;
		}
	}
	
	if (bShowTotal(tracker))
	{
		ImGui::Separator();

		highlightedText(INDEX_TOTAL, "Total");
		ImGui::SameLine(tracker->max_character_name_size + ImGui::GetStyle().ItemSpacing.x);
		current_column++;

		if (_show_subgroups)
		{
			highlightedText(INDEX_TOTAL, "All");
			ImGui::SameLine(0, column_spacing - ImGui::GetItemRectSize().x);
			current_column++;
		}

		for (auto current_buff = tracked_buffs.begin(); current_buff != tracked_buffs.end(); ++current_buff)
		{
			if (!current_buff->is_relevant)continue;

			current_boon_uptime = tracker->getAverageBoonUptime(&*current_buff);
			buffProgressBar(&*current_buff, current_boon_uptime, nullptr, INDEX_TOTAL, column_spacing - ImGui::GetStyle().ItemSpacing.x);
			ImGui::SameLine(0, column_spacing - ImGui::GetItemRectSize().x);
			current_column++;
		}
		ImGui::NewLine();
		current_column = 0;
	}

	ImGui::EndChild();

	ImGui::End();
}

void AppChart::drawRtClickMenu(Tracker* tracker)
{
	active_player = INDEX_HIDE_ALL;
	active_column = INDEX_HIDE_ALL;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

	ImGui::Combo("Display mode", &tracker->table_to_display, "Boon Uptime\0Boon Generation",2);

	ImGui::Checkbox("Players", &show_players);
	ImGui::Checkbox("Subgroups", &show_subgroups);
	ImGui::Checkbox("Total", &show_total);
	ImGui::Checkbox("Show value as progress bar", &show_boon_as_progress_bar);

	
	if (ImGui::BeginMenu("Boons"))
	{
		for (auto current_boon = tracked_buffs.begin(); current_boon != tracked_buffs.end(); ++current_boon)
		{
			if (current_boon->category == BoonType_boon) ImGui::Checkbox(current_boon->name.c_str(), &current_boon->is_relevant);
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Traits"))
	{
		for (auto current_boon = tracked_buffs.begin(); current_boon != tracked_buffs.end(); ++current_boon)
		{
			if (current_boon->category == BoonType_trait) ImGui::Checkbox(current_boon->name.c_str(), &current_boon->is_relevant);
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Banners"))
	{
		for (auto current_boon = tracked_buffs.begin(); current_boon != tracked_buffs.end(); ++current_boon)
		{
			if (current_boon->category == BoonType_banner) ImGui::Checkbox(current_boon->name.c_str(), &current_boon->is_relevant);
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Spirits"))
	{
		for (auto current_boon = tracked_buffs.begin(); current_boon != tracked_buffs.end(); ++current_boon)
		{
			if (current_boon->category == BoonType_spirit) ImGui::Checkbox(current_boon->name.c_str(), &current_boon->is_relevant);
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Skills"))
	{
		for (auto current_boon = tracked_buffs.begin(); current_boon != tracked_buffs.end(); ++current_boon)
		{
			if (current_boon->category == BoonType_skill) ImGui::Checkbox(current_boon->name.c_str(), &current_boon->is_relevant);
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Other"))
	{
		for (auto current_boon = tracked_buffs.begin(); current_boon != tracked_buffs.end(); ++current_boon)
		{
			if (current_boon->category == BoonType_other) ImGui::Checkbox(current_boon->name.c_str(), &current_boon->is_relevant);
		}
		ImGui::EndMenu();
	}

	ImGui::PopStyleVar();

	if (ImGui::Button("Clear"))
	{
		tracker->clearPlayers();
	}
	if(ImGui::IsItemHovered()) ImGui::SetTooltip("Requires map reload");
}

void AppChart::buffProgressBar(BoonDef* current_buff, float current_boon_uptime, Player* current_player, uintptr_t current_id, float width)
{
	if (last_active_player == current_id || last_active_column == current_column)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, active_bar_color);
	}
	else if (last_active_player != -1 || last_active_column != -1)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, hidden_bar_color);
	}
	
	if (show_boon_as_progress_bar)
	{
		if (current_buff->stacking_type == StackingType_intensity)
		{
			char label[10];
			sprintf(label, "%.1f", current_boon_uptime);
			current_boon_uptime /= 25;
			ImGui::ProgressBar(current_boon_uptime, ImVec2(width, ImGui::GetFontSize()), label);
		}
		else
		{
			ImGui::ProgressBar(current_boon_uptime, ImVec2(width, ImGui::GetFontSize()));
		}
	}
	else
	{
		if (current_buff->stacking_type == StackingType_intensity)
		{//don't show the % for intensity stacking buffs
			ImGui::Text("%.1f", current_boon_uptime);
		}
		else
		{
			ImGui::Text("%.1f%%", 100*(double)current_boon_uptime);
		}
	}

	if (last_active_player != -1 || last_active_column != -1)
	{
		ImGui::PopStyleColor();
	}
	if (ImGui::IsItemHovered())
	{
		active_player = current_id;
		active_column = current_column;
	}
}

void AppChart::highlightedText(uintptr_t player_id, const char* fmt, ...)
{
	if (last_active_player == player_id || last_active_column == current_column)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, active_bar_color);
	}
	else if (last_active_player != -1 || last_active_column != -1)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, hidden_bar_color);
	}
	va_list args;
	va_start(args, fmt);
	ImGui::TextV(fmt,args);
	va_end(args);

	if (ImGui::IsItemHovered())
	{
		active_player = player_id;
		active_column = current_column;
	}
	if (last_active_player != -1 || last_active_column != -1)
	{
		ImGui::PopStyleColor();
	}
}

bool AppChart::highlightedSmallButton(uintptr_t player_id, const char* fmt)
{
	if (last_active_column == current_column)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, active_bar_color);
	}
	else if (last_active_player != -1 || last_active_column != -1)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, hidden_bar_color);
	}
	bool out = ImGui::SmallButton(fmt);

	if (ImGui::IsItemHovered())
	{
		active_player = player_id;
		active_column = current_column;
	}
	if (last_active_player != -1 || last_active_column != -1)
	{
		ImGui::PopStyleColor();
	}
	return out;
}

float AppChart::getPlayerDisplayValue(Tracker* tracker, Player* new_player, BoonDef* new_boon)
{
	switch (tracker->table_to_display)
	{
	case TableToDisplay_uptime:
		return new_player->getBoonUptime(new_boon);
	case TableToDisplay_generation:
		return new_player->getBoonGeneration(new_boon);
	default:
		return 0.0f;
	}
}

std::string AppChart::getWindowTitle(Tracker * tracker, const char* new_title)
{
	if (!tracker) return std::string(new_title);

	if (tracker->table_to_display == TableToDisplay_uptime)
		return std::string(new_title) + " - Uptime";
	else if (tracker->table_to_display == TableToDisplay_generation)
		return std::string(new_title) + " - Generation";

	return std::string(new_title);
}

void AppChart::setShowPlayers(bool new_show)
{
	show_players = new_show;
}

void AppChart::setShowSubgroups(bool new_show)
{
	show_subgroups = new_show;
}

void AppChart::setShowTotal(bool new_show)
{
	show_total = new_show;
}

void AppChart::setShowBoonAsProgressBar(bool new_show)
{
	show_boon_as_progress_bar = new_show;
}

bool AppChart::bShowPlayers(Tracker * tracker)
{
	return show_players;
}

bool AppChart::bShowSubgroups(Tracker* tracker)
{
	return show_subgroups 
		&& (tracker ? tracker->table_to_display != TableToDisplay_generation : true)
		&& (tracker ? tracker->is_squad : true)
		&& (tracker ? tracker->subgroups.size()>1 : true);
}

bool AppChart::bShowTotal(Tracker* tracker)
{
	return show_total
		&& (tracker ? tracker->table_to_display != TableToDisplay_generation : true)
		&& (
		(tracker ? tracker->relevant_player_count > 1 : true)
		|| (!show_players
			&& (tracker ? tracker->relevant_player_count > 0 : true))
			);
}

bool AppChart::bShowBoonAsProgressBar()
{
	return show_boon_as_progress_bar;
}

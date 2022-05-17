#pragma once

#include "extension/Singleton.h"
#include "extension/Windows/MainWindow.h"

#include <deque>
#include <mutex>

class EventWindow : public MainWindow, public Singleton<EventWindow> {
public:
	bool& GetOpenVar() override;
	bool& GetShowScrollbar() override;

	void AddData(const std::string& pData);

protected:
	void DrawContextMenu() override;
	void DrawContent() override;

	SizingPolicy& getSizingPolicy() override;
	bool& getShowTitleBar() override;
	std::optional<std::string>& getTitle() override;
	const std::string& getTitleDefault() override;
	const std::string& getWindowID() override;
	std::optional<std::string>& getAppearAsInOption() override;
	const std::string& getAppearAsInOptionDefault() override;
	bool& getShowBackground() override;
	std::optional<ImVec2>& getPadding() override;

private:
	std::deque<std::string> mDataToPrint;
	std::mutex mDataToPrintMutex;

	std::optional<ImVec2> mPadding;
	SizingPolicy mSizingPolicy = SizingPolicy::ManualWindowSize;
	bool mShowTitleBar = true;
	std::optional<std::string> mTitle;
	std::string mTitleDefault = "Event Window";
	std::string mWindowId = "Event Window";
	std::optional<std::string> mAppearAsInOption;
	std::string mAppearAsInOptionDefault = "Event Window";
	bool mOpenVar = false;
	bool mGetShowScrollbar = true;
	bool mShowBackground = true;
};


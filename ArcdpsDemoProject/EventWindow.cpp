#include "EventWindow.h"

bool& EventWindow::GetOpenVar() {
	return mOpenVar;
}

bool& EventWindow::GetShowScrollbar() {
	return mGetShowScrollbar;
}

void EventWindow::AddData(const std::string& pData) {
	std::lock_guard guard (mDataToPrintMutex);
	if (mDataToPrint.size() == 100) {
		mDataToPrint.pop_front();
	}
	mDataToPrint.push_back(pData);
}

void EventWindow::DrawContextMenu() {}

void EventWindow::DrawContent() {
	std::lock_guard guard (mDataToPrintMutex);
	for (const auto& dataToPrint : mDataToPrint) {
		if (!dataToPrint.empty()) {
			ImGui::TextUnformatted(dataToPrint.c_str());
		}
	}
}

SizingPolicy& EventWindow::getSizingPolicy() {
	return mSizingPolicy;
}

bool& EventWindow::getShowTitleBar() {
	return mShowTitleBar;
}

std::optional<std::string>& EventWindow::getTitle() {
	return mTitle;
}

const std::string& EventWindow::getTitleDefault() {
	return mTitleDefault;
}

const std::string& EventWindow::getWindowID() {
	return mWindowId;
}

std::optional<std::string>& EventWindow::getAppearAsInOption() {
	return mAppearAsInOption;
}

const std::string& EventWindow::getAppearAsInOptionDefault() {
	return mAppearAsInOptionDefault;
}

bool& EventWindow::getShowBackground() {
	return mShowBackground;
}

std::optional<ImVec2>& EventWindow::getPadding() {
	return mPadding;
}

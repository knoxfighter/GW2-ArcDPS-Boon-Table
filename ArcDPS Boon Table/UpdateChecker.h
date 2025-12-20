#pragma once

#include "extension/UpdateCheckerBase.h"

class UpdateChecker : public UpdateCheckerBase {

public:
	void Draw();
	void CheckForUpdate(HMODULE dll, const Version& currentVersion, std::string&& repo, bool allowPreRelease);
	void FinishPendingTasks();
private:
	std::unique_ptr<UpdateChecker::UpdateState> update_state = nullptr;
};

extern UpdateChecker updateChecker;

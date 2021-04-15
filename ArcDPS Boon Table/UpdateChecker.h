#pragma once

#include "extension/UpdateCheckerBase.h"

class UpdateChecker : public UpdateCheckerBase {

public:
	void Draw() override;
};

extern UpdateChecker updateChecker;

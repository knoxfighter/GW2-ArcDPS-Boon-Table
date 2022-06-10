#pragma once

#include "extension/Singleton.h"

class BoonWindowHandler : public Singleton<BoonWindowHandler> {
public:
	void DrawOptionCheckboxes();
	void Draw();
	void Init();
};

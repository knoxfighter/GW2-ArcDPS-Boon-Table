#pragma once

#include <ArcdpsExtension/Singleton.h>

class BoonWindowHandler : public Singleton<BoonWindowHandler> {
public:
	void DrawOptionCheckboxes();
	void Draw();
	void Init();
};

#pragma once
#include "WString.h"
class xmlstring :
	public String
{
public:
	xmlstring();
	~xmlstring();
	void catTag(const __FlashStringHelper * tag, String value);
	void catTag(const __FlashStringHelper * tag, int value);
};


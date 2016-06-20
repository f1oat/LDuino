#include "xmlstring.h"



xmlstring::xmlstring()
{
}


xmlstring::~xmlstring()
{
}

void xmlstring::catTag(const __FlashStringHelper *tag, String value)
{
	*this += "<";
	*this += tag;
	*this += ">" + value + "</" + tag + ">";
}

void xmlstring::catTag(const __FlashStringHelper *tag, int value)
{
	catTag(tag, String(value));
}
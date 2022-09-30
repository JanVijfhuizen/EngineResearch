#include "pch.h"
#include "Jlb/StringView.h"

namespace je
{
	StringView::StringView(const char* strLit) : _strLit(strLit)
	{

	}

	const char* StringView::GetData() const
	{
		return _strLit;
	}

	size_t StringView::GetLength() const
	{
		size_t counter = 0;
		const char* c = _strLit;
		for (; *c != 0; ++c)
			++counter;
		return counter;
	}

	bool StringView::operator==(const StringView& other) const
	{
		return _strLit == other._strLit;
	}

	bool StringView::operator==(const char* other) const
	{
		return _strLit == other;
	}

	bool StringView::operator!=(const StringView& other) const
	{
		return !operator==(other);
	}

	bool StringView::operator!=(const char* other) const
	{
		return !operator==(other);
	}

	StringView::operator const char* () const
	{
		return _strLit;
	}
}
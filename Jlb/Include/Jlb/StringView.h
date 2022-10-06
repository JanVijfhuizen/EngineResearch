#pragma once

namespace je
{
	// Immutable string for increased performance.
	struct StringView final
	{
		StringView() = default;
		StringView(const char* strLit);

		[[nodiscard]] const char* GetData() const;
		[[nodiscard]] size_t GetLength() const;

		[[nodiscard]] bool operator==(const StringView& other) const;
		[[nodiscard]] bool operator==(const char* other) const;
		[[nodiscard]] bool operator!=(const StringView& other) const;
		[[nodiscard]] bool operator!=(const char* other) const;
		
		[[nodiscard]] operator const char* () const;

	private:
		const char* _strLit = nullptr;
	};
}

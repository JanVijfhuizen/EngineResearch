#pragma once

namespace je
{
	class StringView final
	{
	public:
		StringView() = default;
		// ReSharper disable once CppNonExplicitConvertingConstructor
		StringView(const char* strLit);

		[[nodiscard]] const char* GetData() const;
		[[nodiscard]] size_t GetLength() const;

		bool operator==(const StringView& other) const;
		bool operator==(const char* other) const;
		bool operator!=(const StringView& other) const;
		bool operator!=(const char* other) const;

		// ReSharper disable once CppNonExplicitConversionOperator
		operator const char* () const;

	private:
		const char* _strLit = nullptr;
	};
}

#pragma once

template<class R, class T>
R* AddOffset(T* p, ptrdiff_t offset)
{
	auto p2 = const_cast<typename std::remove_cv<T>::type*>(p);
	auto p3 = static_cast<void*>(p2);
	auto p4 = static_cast<std::uint8_t*>(p3) + offset;
	auto p5 = static_cast<void*>(p4);
	return static_cast<R*>(p5);
}

template<class R, class T>
R ReadAtOffset(T* p, ptrdiff_t offset)
{
	return *AddOffset<R>(p, offset);
}

bool IsFlagSet(DWORD flags, DWORD flagToCheck)
{
	return (flags & flagToCheck) != 0;
}

#define DECLARE_NONCOPYABLE(ClassName) \
  ClassName(const ClassName&) = delete; \
  ClassName& operator=(const ClassName&) = delete

void CheckError(bool condition, const char* message)
{
	if (!condition)
	{
		throw std::runtime_error(message);
	}
}
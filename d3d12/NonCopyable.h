#pragma once

template<class T>
class NonCopyable
{
protected:
	NonCopyable() = default;
	~NonCopyable() = default;

private:
	NonCopyable(const T&) = delete;
	NonCopyable(T&&) = delete;
	T& operator=(const T&) = delete;
};

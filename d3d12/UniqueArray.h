#pragma once
#include <memory>
#include "NonCopyable.h"


// std::unique<T[]> + コピー禁止 + イテレートできる + リサイズできる
template<class T>
class UniqueArray : NonCopyable<UniqueArray<T>>
{
public:
	typedef T* iterator;
	typedef const T* const_iterator;

public:
	UniqueArray() = default;

	UniqueArray(size_t size) {
		Resize(size);
	}

	T& operator[](size_t i) { return items_[i]; }
	const T& operator[](size_t i) const { return items_[i]; }

	iterator begin() { return items_.get(); }
	const_iterator begin() const { return items_.get(); }

	iterator end() { return Size() > 0 ? &items_[size_] : begin(); }
	const_iterator end() const { return Size() > 0 ? &items_[size_] : begin(); }

	size_t Size() { return size_; }

	T& At(size_t i) { return this[i]; }
	const T& At(size_t i) const { return this[i]; }

	void Resize(size_t size) {
		size_ = size;
		items_.reset(new T[size + 1]);
	}

private:
	size_t size_ = 0;
	std::unique_ptr<T[]> items_;
};


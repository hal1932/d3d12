#pragma once
#include <memory>
#include "Model.h"


class ModelArray : NonCopyable<ModelArray>
{
public:
	typedef Model* iterator;
	typedef const Model* const_iterator;

public:
	ModelArray() = default;

	ModelArray(size_t size) {
		Resize(size);
	}

	Model& operator[](size_t i) { return pItems_[i]; }
	const Model& operator[](size_t i) const { return pItems_[i]; }

	iterator begin() { return pItems_.get(); }
	const_iterator begin() const { return pItems_.get(); }

	iterator end() { return &pItems_[size_]; }
	const_iterator end() const { return &pItems_[size_]; }

	size_t Size() { return size_; }

	void Resize(size_t size) {
		size_ = size;
		pItems_.reset(new Model[size + 1]);
	}

private:
	size_t size_;
	std::unique_ptr<Model[]> pItems_;
};


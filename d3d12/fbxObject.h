#pragma once
#include "fbxCommon.h"
#include "Transform.h"
#include "NonCopyable.h"

namespace fbx
{
	template<class TFbxObject>
	class Object : private NonCopyable<Object<TFbxObject>>
	{
	public:
		Object(TFbxObject* pFbxObject) : pObject_(pFbxObject) {}
		virtual ~Object() = default;

		u64 HashCode() { return static_cast<u64>(pObject_->GetUniqueID()); }
		u64 HashCode() const { return static_cast<u64>(pObject_->GetUniqueID()); }

		TFbxObject* NativePtr() { return pObject_; }

	private:
		TFbxObject* pObject_;
	};

	template<class TFbxObject>
	class TransformObject : public Object<TFbxObject>, public Transform
	{
	public:
		TransformObject(TFbxObject* pObject) : Object<TFbxObject>(pObject) {}
		virtual ~TransformObject() = default;
	};

}// namespace

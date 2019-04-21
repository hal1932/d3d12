#include "fbxCommon.h"
#include "common.h"

using namespace fbxsdk;

namespace
{
	FbxManager* spManager = nullptr;
}

namespace fbx
{
	FbxManager* GetManager()
	{
		return spManager;
	}

	void Setup()
	{
		spManager = FbxManager::Create();

		auto pIOS = FbxIOSettings::Create(spManager, "IOSROOT");
		spManager->SetIOSettings(pIOS);

		auto fbxPath = FbxGetApplicationDirectory();
		spManager->LoadPluginsDirectory(fbxPath.Buffer());
	}

	void Shutdown()
	{
		SafeDestroy(&spManager);
	}
}// namespace fbx

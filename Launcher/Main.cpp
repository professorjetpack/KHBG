#define _CRT_SECURE_NO_WARNINGS
#include "File.h"
#include <WinNls.h>
#include <ShObjIdl.h>
#include <objbase.h>
#include <ShlGuid.h>
#include <WinInet.h>
#include <atlbase.h>
void removeDirectory(char * directory) {
	char dirPath[MAX_PATH];
	sprintf_s(dirPath, MAX_PATH, "%s/Documents/%s", getenv("USERPROFILE"), directory);
	std::vector<std::string> files;
	files = getDirectory(dirPath);
	char filePath[MAX_PATH];
	for (int i = 0; i < files.size(); i++) {
		sprintf_s(filePath, MAX_PATH, "%s\\%s", dirPath, files[i].c_str());
		remove(filePath);
	}
	RemoveDirectory(dirPath);
}
int unzipTo(BSTR zipFile, BSTR folder) {
	HRESULT result;
	IShellDispatch * pISD;
	Folder * toFolder = NULL;
	IDispatch * pItem;
	VARIANT vDir, vFile, vOpt;
	CoInitialize(NULL);
	result = CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void**)&pISD);
	if (SUCCEEDED(result)) {
		VariantInit(&vDir);
		vDir.vt = VT_BSTR;
		vDir.bstrVal = folder;
		result = pISD->NameSpace(vDir, &toFolder);
		if (SUCCEEDED(result)) {
			Folder * fromFolder = NULL;
			VariantInit(&vFile);
			vFile.vt = VT_BSTR;
			vFile.bstrVal = zipFile;
			pISD->NameSpace(vFile, &fromFolder);
			FolderItems * items = NULL;
			fromFolder->Items(&items);
			items->QueryInterface(IID_IDispatch, (void**)&pItem);

			VariantInit(&vOpt);
			vOpt.vt = VT_I4;
			vOpt.lVal = FOF_NO_UI;

			VARIANT newV;
			VariantInit(&newV);
			newV.vt = VT_DISPATCH;
			newV.pdispVal = pItem;//items;

			result = toFolder->CopyHere(newV, vOpt);
			fromFolder->Release();
			toFolder->Release();
			CoUninitialize();
			return 0;
		}
		else {
			pISD->Release();
			CoUninitialize();
			return -1;
		}
	}
	else return -2;
}
int main() {
	printf("Thank you for installing King Frederick's Battle Royale by Stephen Verderame \n");
	Sleep(2000);
	printf("I hope it will provide you with at least 10 minutes of fun! \n");
	Sleep(1500);
	printf("Installation start \n");
	struct stat exists;
	char dirPath[MAX_PATH];
	sprintf_s(dirPath, MAX_PATH, "%s\\Documents\\KFBR", getenv("USERPROFILE"));
	if (stat(dirPath, &exists) == 0) {
		//if already downloaded;
		printf("Deleting previous install \n");
		removeDirectory("KFBR/Assets/fonts");
		removeDirectory("KFBR/Assets/skybox");
		removeDirectory("KFBR/Assets/sounds");
		removeDirectory("KFBR/Assets/well");
		removeDirectory("KFBR/Assets");
		removeDirectory("KFBR");
		printf("Removed old data! \n");
	}
	struct stat safety;
	if (stat(dirPath, &safety) == 0) {
		char assetsPath[MAX_PATH];
		sprintf_s(assetsPath, MAX_PATH, "%s\\Assets", dirPath);
		if (stat(assetsPath, &safety) == 0) {
			RemoveDirectory(assetsPath);
		}
		RemoveDirectory(dirPath);
	}
	char url[] = "https://dl.dropboxusercontent.com/s/cutdr5fss2oorme/KFBR.zip?dl=0";
	char downloadPath[MAX_PATH];
	sprintf_s(downloadPath, MAX_PATH, "%s\\Downloads\\Frederick.zip", getenv("USERPROFILE"));
	if (stat(downloadPath, &safety) == 0) {
		remove(downloadPath);
	}
	printf("Downloading files. This may take a while. \n");
	DeleteUrlCacheEntry(url);
	HRESULT check = URLDownloadToFile(NULL, url, downloadPath, 0, NULL);
	if (check == S_OK) printf("Downloaded files! \n");
	else printf("Download failed \n");
	printf("Decompressing files. This may take a while \n");
	char folderPath[MAX_PATH];
	sprintf_s(folderPath, MAX_PATH, "%s\\Documents", getenv("USERPROFILE"));
	CComBSTR file(downloadPath);
	CComBSTR folder(folderPath);
	if (unzipTo(file, folder) == 0) printf("Unzip success! \n");
	else printf("Unzip failed! \n");


	char exePath[MAX_PATH];
	sprintf_s(exePath, MAX_PATH, "%s\\Documents\\KFBR\\Launcher.exe", getenv("USERPROFILE"));
	char lnkPath[MAX_PATH];
	sprintf_s(lnkPath, MAX_PATH, "%s\\Desktop\\KFBR.lnk", getenv("USERPROFILE"));
	char icoPath[MAX_PATH];
	sprintf_s(icoPath, MAX_PATH, "%s\\Documents\\KFBR\\bow.ico", getenv("USERPROFILE"));
	CoInitialize(NULL);
	HRESULT res;
	IShellLink * psl;
	res = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
	if (SUCCEEDED(res)) {
		IPersistFile * ppf;
		psl->SetPath(exePath);
		psl->SetWorkingDirectory(dirPath);
		psl->SetIconLocation(icoPath, 0);
		res = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
		if (SUCCEEDED(res)) {
			WCHAR formattedPath[MAX_PATH];
			MultiByteToWideChar(CP_ACP, 0, lnkPath, -1, formattedPath, MAX_PATH);
			ppf->Save(formattedPath, TRUE);
			ppf->Release();
			psl->Release();
			CoUninitialize();
			printf("Desktop shortcut created \n");
		}
		else {
			printf("Couldn't create shortcut \n");
			psl->Release();
			CoUninitialize();
		}
	}
	else printf("Couldn't create shortcut. Could not instantiate \n");

	printf("Thank you for installing KFBR. Main directory in %s\\Documents\\KFBR \n", getenv("USERPROFILE"));
	printf("If there were any issues with this installation contact me immediatly \n");
	printf("Press any key to quit... \n");
	getchar();
}
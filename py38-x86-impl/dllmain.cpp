// dllmain.cpp : Defines the entry point for the DLL application.
#include "vendor/py38/include/Python.h"
#include <unordered_map>
#include <iostream>
#include <MinHook.h>
#pragma comment(lib, "python38.lib")
#include <sstream>
#include "PythonImpl.h"
#include "Entity.h"


bool init = false;

std::unordered_map<std::string, DWORD> Modules;

PyObject* (*o_Py_CreateModule2)(struct PyModuleDef*, int);

PyObject* _cdecl NewPy_CreateModule2(struct PyModuleDef* module_defs,
	int apiver)
{
	if (module_defs != NULL)
	{
		for (int i = 0;; i++)
		{
			if (module_defs->m_methods[i].ml_name == NULL)
			{
				break;
			}
			std::string module = "" + std::string(module_defs->m_name) + module_defs->m_methods[i].ml_name;
			DWORD module_addr = (DWORD)module_defs->m_methods[i].ml_meth;
			std::cout << module << " " << std::hex << module_addr << std::endl;

			Modules.insert({ module, module_addr });
		}
	}
	return o_Py_CreateModule2(module_defs, apiver);
}

void SendChatPacket(const char* message, int  type)
{
	DWORD call_address = Modules["netSendChatPacket"];
	if (call_address)
	{
		auto args = PyTuple_New(2);
		PyTuple_SetItem(args, 0, PyUnicode_FromString(message));
		PyTuple_SetItem(args, 1, PyLong_FromUnsignedLong(type));
		using func = PyObject * (PyObject*, PyObject*);
		auto nfunc = (func*)(call_address);
		nfunc(NULL, args);
		Py_DecRef(args);
	}
}

std::vector<std::string> open_and_read_file(const std::string& filename) {
	std::vector<std::string> result;
	PyObject* pName, * pModule, * pFunc, * pValue, * pFileHandle = nullptr, * pCount;

	pName = PyUnicode_DecodeFSDefault("app");
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if (pModule != nullptr) {
		// Call app.OpenTextFile(filename)
		pFunc = PyObject_GetAttrString(pModule, "OpenTextFile");
		if (PyCallable_Check(pFunc)) {
			pValue = Py_BuildValue("(s)", filename.c_str());
			pFileHandle = PyObject_CallObject(pFunc, pValue);
			Py_DECREF(pValue);
		}

		pFunc = PyObject_GetAttrString(pModule, "GetTextFileLineCount");
		if (PyCallable_Check(pFunc)) {
			pCount = PyObject_CallFunctionObjArgs(pFunc, pFileHandle, NULL);
			int count = PyLong_AsLong(pCount);

			if (count <= 0) {
				return result;
			}

			// Read each line from the file
			pFunc = PyObject_GetAttrString(pModule, "GetTextFileLine");
			for (int i = 0; i < count; ++i) {
				pValue = PyObject_CallFunction(pFunc, "(Oi)", pFileHandle, i);
				if (pValue != nullptr) {
					result.push_back(PyUnicode_AsUTF8(pValue));
				}
			}
		}

		pFunc = PyObject_GetAttrString(pModule, "CloseTextFile");
		if (PyCallable_Check(pFunc)) {
			PyObject_CallFunctionObjArgs(pFunc, pFileHandle, NULL);
		}

		Py_DECREF(pModule);
	}
	else {
	}
	return result;
}

void ShowCommands() {
	std::cout << "------------------------------------------------- \n"
		"Commands: \n"
		"load <filename>      Loads python script.\n"
		"help                 Displays commands.  \n"
		"------------------------------------------------- "
		<< std::endl;
}

DWORD MainThread(HMODULE hMod)
{
	AllocConsole();
	FILE* Dummy;
	freopen_s(&Dummy, "CONOUT$", "w", stdout);
	freopen_s(&Dummy, "CONIN$", "r", stdin);

	//if (MH_Initialize() != MH_OK)
	//{
	//}

	//if (MH_CreateHookApi(L"python38.dll", "PyModule_Create2", &NewPy_CreateModule2, (void**)&o_Py_CreateModule2) != MH_OK)
	//{
	//}

	//MH_EnableHook(MH_ALL_HOOKS);

	if (!init) {
		PythonImpl::init();
		std::cout << "my module :D" << std::endl;
		init = true;
	}

	while (true)
	{

		if (GetAsyncKeyState(VK_LSHIFT) & 1)
		{
			auto poMod = PyImport_ImportModule("xenia");
			if (poMod)
			{
				std::cout << "found module" << std::endl;
			}
			else
			{
				std::cout << "failed" << std::endl;
			}
			Entity::Instance().GetMobs();
		}

		if (GetAsyncKeyState(VK_F10) & 1)
		{
			std::string in;
			std::getline(std::cin, in);

			int spaceFound = in.find(" ", 0);

			std::string cmdName = in.substr(0, spaceFound);
			std::string cmdParams;

			if (spaceFound != -1) {
				cmdParams = in.substr(spaceFound + 1, in.length() - spaceFound);
			}

			if (cmdName == "load") {
				if (!cmdParams.empty()) {
					std::cout << "Loading file " << cmdParams << std::endl;

					std::stringstream stream;
					stream << "exec(compile(open('" << cmdParams.c_str() << "').read(), '"
						<< cmdParams.c_str() << "', 'exec'))";

					PyGILState_STATE state = PyGILState_Ensure();
					int r = PyRun_SimpleStringFlags(stream.str().c_str(), NULL);
					PyGILState_Release(state);
					if (r == 0) {
						std::cout << "Python script loaded!" << std::endl;
					}
					else {
						std::cout << "Error occured while running python script. Check "
							"syserr.txt for more info."
							<< std::endl;
					}
				}
				else {
					std::cout << "File name required!" << std::endl;
				}
			}
			else if (cmdName == "help") {
				std::cout << "TODO..." << std::endl;
			}
			else {
				std::cout << "Unknown command! Use 'help' to display commands."
					<< std::endl;
			}
		}
		Sleep(1);
	}
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		PyEval_InitThreads();

		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0);

		break;
	}

	return TRUE;
}
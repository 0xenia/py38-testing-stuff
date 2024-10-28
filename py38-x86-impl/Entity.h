#pragma once
#include "singleton.hpp"
#include "vendor/py38/include/Python.h"
#include <Windows.h>

class CInstance {
public:
	DWORD* base_class;
	DWORD* abstract_class;
};

class Entity : public CSingleton<Entity>
{
public:
	bool IsDead(DWORD vid);

	static CInstance* _Instance;

	void* GetInstancePtr(DWORD vid);
	inline PyObject* getVIDList() { return pyVIDList; };
	void GetMobs();
	void PickInstance(DWORD vid);
public:
	static PyObject* pyVIDList;
};
 


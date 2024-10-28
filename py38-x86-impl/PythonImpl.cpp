#include "pch.h"
#include "PythonImpl.h"
#include "Entity.h"
#include <iostream>


bool PyTuple_GetInteger(PyObject* poArgs, int pos, int* ret)
{
	if (pos >= PyTuple_Size(poArgs))
		return false;

	PyObject* poItem = PyTuple_GetItem(poArgs, pos);

	if (!poItem)
		return false;

	*ret = PyLong_AsLong(poItem);
	return true;
}

bool PyTuple_GetUnsignedLong(PyObject* poArgs, int pos, unsigned long* ret)
{
	if (pos >= PyTuple_Size(poArgs))
		return false;

	PyObject* poItem = PyTuple_GetItem(poArgs, pos);

	if (!poItem)
		return false;

	*ret = PyLong_AsUnsignedLong(poItem);
	return true;
}

PyObject* Py_BuildNone() {
	Py_RETURN_NONE;
}

PyObject* Py_BuildException(const char* c_pszErr = NULL, ...)
{
	if (!c_pszErr)
		PyErr_Clear();
	else
	{
		char szErrBuf[512 + 1];
		va_list args;
		va_start(args, c_pszErr); 
#pragma warning( push )
#pragma warning( disable : 4996 )
		vsnprintf(szErrBuf, sizeof(szErrBuf), c_pszErr, args);
#pragma warning( pop )
		va_end(args);

		PyErr_SetString(PyExc_RuntimeError, szErrBuf);
	}

	return Py_BuildNone();
	//return NULL;
}

PyObject* pyIsDead(PyObject* poSelf, PyObject* poArgs)
{
	DWORD vid;
	if (!PyTuple_GetUnsignedLong(poArgs, 0, &vid))
	{
		return Py_BuildException();
	}

	auto& emgr = Entity::Instance();

	return Py_BuildValue("i", emgr.IsDead(vid));
}

PyObject* callGetMobs(PyObject* poSelf, PyObject* poArgs) {
	auto& emgr = Entity::Instance();
	emgr.GetMobs();
	Py_RETURN_NONE;
}

PyObject* PickInstance(PyObject* poSelf, PyObject* poArgs)
{
	DWORD vid;
	if (!PyTuple_GetUnsignedLong(poArgs, 0, &vid))
	{
		return Py_BuildException();
	}
	auto& emgr = Entity::Instance();
	emgr.PickInstance(vid);
	Py_RETURN_TRUE;
}

static PyMethodDef s_methods[] = {
	{"IsDead", pyIsDead, METH_VARARGS, "Check if an entity is dead."},
	{"GetMobs", callGetMobs, METH_VARARGS, "Get mobs around player"},
	{"PickInstance", PickInstance, METH_VARARGS, "Pick an instance for attacking"},
	{NULL, NULL, 0, NULL}
};

static PyObject* init_xenia() {
	static struct PyModuleDef xenia_module = {
		PyModuleDef_HEAD_INIT,
		"xenia",
		"",
		-1,
		s_methods
	};
	std::cout << "Initializing Xenia module..." << std::endl;
	return PyModule_Create(&xenia_module);
}


void PythonImpl::init() {
	do {
		Sleep(1000);
	} while (!Py_IsInitialized());
	if (Py_IsInitialized())
	{
		PyImport_AddModule("xenia");
		PyObject* pyModule = init_xenia();
		PyObject* sys_modules = PyImport_GetModuleDict();
		PyModule_AddObject(pyModule, "InstanceList", Entity::Instance().getVIDList());
		PyDict_SetItemString(sys_modules, "xenia", pyModule);
		Py_DECREF(pyModule);
	}
	
}


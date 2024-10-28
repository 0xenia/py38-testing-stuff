#include "pch.h"
#include "Entity.h"
#include "remem.hpp"

CInstance* Entity::_Instance = *reinterpret_cast<CInstance**>(remem::pattern("A1 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC CC A1 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC CC A1 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC CC A1 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC CC A1 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC CC A1 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC CC 83 EC").add(1, true).GetPointer());

DWORD CharacterManager = (remem::pattern("A1 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC CC A1 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC CC A1 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC CC A1 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC CC A1 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC CC A1 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC CC 83 EC").add(1, true).GetPointer());

DWORD IsDeadCallAddress = remem::pattern("81 C1 ? ? ? ? E9 ? ? ? ? CC CC CC CC CC 81 C1 ? ? ? ? E8 ? ? ? ? 0F B6 C0 C3 CC 81 C1 ? ? ? ? E8 ? ? ? ? 0F B6 C0 C3 CC 81 C1").GetPointer();

PyObject* Entity::pyVIDList = PyDict_New();


void* Entity::GetInstancePtr(DWORD vid)
{
	if (_Instance != NULL)
	{
		auto entity = remem::CallFunction<CallingConvention::thiscall_, void*>(_Instance->abstract_class[0x2], (void*)((uintptr_t)_Instance + 0x4), vid);
		if (entity)
			return entity;

		return nullptr;
	}
	return nullptr;
}

bool Entity::IsDead(DWORD vid)
{
	void* entity = GetInstancePtr(vid);
	if (entity)
	{
		if (IsDeadCallAddress)
		{
			bool dead = remem::CallFunction<CallingConvention::thiscall_, bool>(IsDeadCallAddress, entity);
			return dead;
		}
		return true;
	}
	return true;
}

void Entity::PickInstance(DWORD vid)
{
    void* entity = GetInstancePtr(vid);
    if (entity)
    {
        remem::WriteMemory(_Instance, { 0x10 }, entity);
    }
    return;
}

void Entity::GetMobs()
{
    auto mobList = reinterpret_cast<std::map<uint32_t, uint32_t*>*>(
        (uintptr_t)_Instance + 0x20);


    if (mobList->empty()) return;

    for (const auto& mob : *mobList) {
        auto VID = mob.first;             // Mob ID
        auto Instance = mob.second;       // Pointer to the mob instance

        if (!Instance || IsDead(VID)) {
            continue;
        }

        PyObject* pyVID = PyLong_FromUnsignedLong(VID);
        if (!pyVID) {
            continue;
        }

        PyDict_SetItem(pyVIDList, pyVID, pyVID);

        Py_DECREF(pyVID);

    }
}


#include "NamedMutex.hpp"

namespace {
    Mutex _mutex;
}

NamedMutexGuard::MutexMap NamedMutexGuard::_objects;

NamedMutexGuard::NamedMutexGuard(const String& name)
{
    {
        Mutex::Guard guard(_mutex);
        _object = _objects.find(name);
        if (_object == _objects.end())
            _object = _objects.insert(_objects.end(), name);
        ++_object->count;
    }
    _object->mutex.lock();
}

NamedMutexGuard::~NamedMutexGuard()
{
    _object->mutex.unlock();
    {
        Mutex::Guard guard(_mutex);
        if (--_object->count == 0)
            _objects.remove(_object);
    }
}

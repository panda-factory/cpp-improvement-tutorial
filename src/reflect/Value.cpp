
#include "Value.h"

#include "Class.h"

namespace Reflect {

ClassUnknown& Value::getClass() const
{
    return Reflect::getClass<void>();
}

void* Value::valueDataReference()
{
    return nullptr;
}

}

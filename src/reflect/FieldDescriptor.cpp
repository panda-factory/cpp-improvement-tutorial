
#include "FieldDescriptor.h"

namespace Reflect {

std::string fieldDescriptor(const std::string& classDescriptor, const std::string& name)
{
    std::stringstream descriptor;
    if (!classDescriptor.empty())
        descriptor << classDescriptor << "::";
    descriptor << name;
    return descriptor.str();
}

}

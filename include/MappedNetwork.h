#include <iostream>

namespace PyStorm
{
class MappedNetwork
{
public:
    MappedNetwork(std::string name)
    {
        m_name = name;
    }

    std::string getName()
    {
        return m_name;
    }
protected:
    std::string m_name;
};

} // namespace PyStorm

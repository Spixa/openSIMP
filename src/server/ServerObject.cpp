#include "ServerObject.h"
Property::Property()
{
    m_type = Type::NoInit;
}

Property::~Property()
{
	if (m_type == Type::String)
		delete string_data;
}

Property::Property(const Property& property)
{
	m_type = property.m_type;
	switch (m_type)
	{
		case(Type::Float):
		{
			float_data = property.float_data;
			break;
		}
		case(Type::Int):
		{
			int_data = property.int_data;
			break;
		}
		case(Type::Bool):
		{
			bool_data = property.bool_data;
			break;
		}
		case(Type::String):
		{
			string_data = new std::string(*property.string_data);
			break;
		}
		default:break;
	}
}

Property& Property::operator=(const Property& property)
{
	m_type = property.m_type;
	switch (m_type)
	{
		case(Type::Float):
		{
			float_data = property.float_data;
			break;
		}
		case(Type::Int):
		{
			int_data = property.int_data;
			break;
		}
		case(Type::Bool):
		{
			bool_data = property.bool_data;
			break;
		}
		case(Type::String):
		{
			string_data = new std::string(*property.string_data);
			break;
		}
		default:break;
	}
	return *this;
}

Property::Property(Property&& property)
{
	m_type = property.m_type;
	switch (m_type)
	{
	case(Type::Float):
	{
		float_data = property.float_data;
		break;
	}
	case(Type::Int):
	{
		int_data = property.int_data;
		break;
	}
	case(Type::Bool):
	{
		bool_data = property.bool_data;
		break;
	}
	case(Type::String):
	{
		string_data = property.string_data;
		property.string_data = nullptr;
		break;
	}
	default:break;
	}
}

Property& Property::operator=(Property&& property)
{
	m_type = property.m_type;
	switch (m_type)
	{
	case(Type::Float):
	{
		float_data = property.float_data;
		break;
	}
	case(Type::Int):
	{
		int_data = property.int_data;
		break;
	}
	case(Type::Bool):
	{
		bool_data = property.bool_data;
		break;
	}
	case(Type::String):
	{
		string_data = property.string_data;
		property.string_data = nullptr;
		break;
	}
	default:break;
	}
	return *this;
}

Property::Property(bool bool_value)
{
    m_type = Type::Bool;
    bool_data = bool_value;
}

Property::Property(int int_value)
{
    m_type = Type::Int;
    int_data = int_value;
}

Property::Property(const std::string& string_value)
{
    m_type = Type::String;
    string_data = new std::string(string_value);
}

Property::Property(float float_value)
{
    m_type = Type::Float;
    float_data = float_value;
}

bool Property::asBool() const
{
    assert(m_type == Type::Bool);
    return bool_data;
}
int Property::asInt() const
{
    assert(m_type == Type::Int);
    return int_data;
}
float Property::asFloat() const
{
    assert(m_type == Type::Float);
    return float_data;
}
const std::string& Property::asString() const
{
    assert(m_type == Type::String);
    return *string_data;
}
bool Property::isValid() const
{
    return m_type != Type::NoInit;
}

ServerObject::ServerObject()
{
    m_enable = true;
    m_parent = NULL;
}

void ServerObject::setParent(ServerObject* game_object)
{
    m_parent = game_object;
}

ServerObject* ServerObject::getParent() const
{
    return m_parent;
}

void ServerObject::update(Property& property)
{
    if (isEnabled())
    {
        for (auto& obj : m_objects)
            if (!obj->m_started)
            {
                obj->m_started = true;
                obj->start();
            }

        for (auto& obj : m_objects)
            if (obj->isEnabled())
                obj->update(property);
    }
}

void ServerObject::start()
{

}

void ServerObject::setName(const std::string& name)
{
    m_name = name;
}

const std::string&  ServerObject::getName() const
{
    return m_name;
}

void ServerObject::setProperty(const std::string& name, const Property& property)
{
    m_properties[name] = property;
};

Property ServerObject::getProperty(const std::string& name) const
{
    return const_cast<ServerObject*>(this)->m_properties[name];
};

void ServerObject::disable()
{
    m_enable = false;
}

void ServerObject::enable()
{
    m_enable = true;
}

bool ServerObject::isEnabled() const
{
    return m_enable;
}

ServerObject* ServerObject::addObject(ServerObject* object)
{
    m_objects.push_back(object);
    object->setParent(this);
    object->onActivated();
    if (m_started)
    {
        object->m_started = true;
        object->start();
    }
    return object;
}

ServerObject* ServerObject::findObjectByName(const std::string& name)
{
    auto it = std::find_if(m_objects.begin(), m_objects.end(), [this, &name](const ServerObject* obj) -> bool { return obj->getName() == name;  });
    if (it != m_objects.end())
        return *it;
    return nullptr;
}

ServerObject::~ServerObject()
{
    for (auto& obj : m_objects)
        delete obj;
    m_objects.clear();
}


void ServerObject::foreachObject(std::function<void(ServerObject*)> predicate)
{
    for (auto& obj : m_objects)
        predicate(obj);
}
void ServerObject::foreachObject(std::function<void(ServerObject*, bool& )> predicate)
{
    bool need_break = false;
    for (auto& obj : m_objects)
    {
        predicate(obj, need_break);
        if (need_break)
            break;
    }
}

void ServerObject::removeObject(ServerObject* object)
{
    auto action = [this, object]() //remove object later
    {
        auto it = std::find(m_objects.begin(), m_objects.end(), object);
        assert(it != m_objects.end());
        m_objects.erase(it);
        delete object;
    };
    m_preupdate_actions.push_back(action);
}


void ServerObject::moveToBack()
{
    if (getParent())
    {
        auto move_to_back_action = [this]()
        {
            auto list = &(getParent()->m_objects);
            auto it = std::find(list->begin(), list->end(), this);
            assert(*it == this);
            auto tmp = *it;
            it = list->erase(it);
            list->push_front(tmp);
            
        };
        m_preupdate_actions.push_back(move_to_back_action);
    }
}

void ServerObject::moveToFront()
{
    if (getParent())
    {
        auto move_to_front_action = [this]()
        {
            auto list = &(getParent()->m_objects);
            auto it = std::find(list->begin(), list->end(), this);
            assert(*it == this);
            auto tmp = *it;
            it = list->erase(it);
            list->push_back(tmp);
        };

        m_preupdate_actions.push_back(move_to_front_action);
    }
}

void ServerObject::moveUnderTo(ServerObject* obj)
{
    if (getParent())
    {
        auto move_under_action = [this,obj]()
        {
            auto list = &(getParent()->m_objects);
            auto this_obj = std::find(list->begin(), list->end(), this);
            auto other_obj = std::find(list->begin(), list->end(), obj);
            assert(this_obj != list->end() && other_obj != list->end());
            list->erase(this_obj);
            list->insert(other_obj,this);
        };

        m_preupdate_actions.push_back(move_under_action);
    }
}

void ServerObject::clear()
{
    for (auto object : m_objects)
        delete object;
    m_objects.clear();
}

std::vector<std::function<void()>> ServerObject::m_preupdate_actions = std::vector<std::function<void()>>();

void ServerObject::invokePreupdateActions()
{
    for (auto& action : m_preupdate_actions)
        action();
    m_preupdate_actions.clear();
}

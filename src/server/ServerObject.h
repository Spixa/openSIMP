#ifndef SERVEROBJECT_H
#define SERVEROBJECT_H

#include <sstream>
#include <functional>
#include <list>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include "assert.h"

class Property {
public:
    Property();
    Property(bool bool_value);
    Property(int int_value);
    Property(const std::string& string_value);
    Property(float float_value);
    Property(const Property& property);
    Property& operator=(const Property& property);
    Property(Property&& property);
    Property& operator=(Property&& property);
    ~Property();
    bool asBool() const;
    int asInt() const;
    float asFloat() const;
    const std::string& asString() const;
    bool isValid() const;
private:
    union {
        int int_data;
        float float_data;
        std::string* string_data;
        bool bool_data;
    };

    enum class Type { NoInit, Bool, Int, Float, String} m_type;
};

namespace simp {
	enum class updater_void {
		success_return,
		fail_return
	};
};

class ServerObject {
public:
    ServerObject();
    virtual ~ServerObject();
    void setName(const std::string& name);
    const std::string& getName() const;
    void setProperty(const std::string& name, const Property& property);
    Property getProperty(const std::string& name) const;
    void setParent(ServerObject* server_object);
	ServerObject* getParent() const;
	ServerObject* addObject(ServerObject* object);
	ServerObject* findObjectByName(const std::string& name);

    

    void moveToBack();
	void moveToFront();
	void moveUnderTo(ServerObject* obj);
	template <typename T>


	T* findObjectByName(const std::string& name)
	{
		auto it = std::find_if(m_objects.begin(), m_objects.end(), [this, &name](const ServerObject* obj) -> bool { return obj->getName() == name;  });
		if (it != m_objects.end())
			return dynamic_cast<T*>(*it);
		return nullptr;
	}

	template <typename T>
	T* findObjectByType()
	{
		for (auto& obj : m_objects)
			if (dynamic_cast<T*>(obj) != nullptr)
				return (T*)obj;
		return nullptr;
	}
	template <typename T>
	std::vector<T*> findObjectsByType()
	{
		std::vector<T*> objects;
		for (auto& obj : m_objects)
		{
			if (dynamic_cast<T*>(obj) != nullptr)
				objects.push_back((T*)obj);

			auto objects_temp = obj->findObjectsByType<T>();
			if (!objects_temp.empty())
				objects.insert(objects.end(), objects_temp.begin(), objects_temp.end());
		}
		return objects;
	}
	template <typename T>
	T* castTo()
	{
		assert(dynamic_cast<T*>(this));
		return (T*)this;
	}
	template <typename T>
	bool isTypeOf() const
	{
		return (dynamic_cast<const T*>(this) != NULL);
	}
	void foreachObject(std::function<void(ServerObject*)> predicate);
	void foreachObject(std::function<void(ServerObject*, bool& need_break)> predicate);
	void removeObject(ServerObject* obj);
	void clear();
	static void invokePreupdateActions();
	virtual void start();
    virtual void update(Property&);

	void enable();
	void disable();
    bool isEnabled() const;

	void steadySetup() {}

	
	void issueNewServerObjectError(std::string reads) {
		std::cout << "Failure fetching server object \"" << m_name << "\".\n\t" << reads << std::endl;
	}

	void failObject() {}

protected:
    void onActivated() {}
private:

	std::string m_name;
	bool m_started = false;
	static std::vector<std::function<void()>> m_preupdate_actions;
	std::map<std::string, Property> m_properties;
	ServerObject* m_parent;
	std::list<ServerObject*> m_objects;
	bool m_enable;

};


#endif
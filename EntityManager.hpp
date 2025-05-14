#pragma once

#include "Entity.hpp"
#include <map>

using EntityVec = std::vector<std::shared_ptr<Entity>>;

class EntityManager
{
	EntityVec m_entities;
	EntityVec m_entitiesToAdd;
	std::map<std::string, EntityVec> m_entityMap;
	size_t m_totalEntities = 0;

	void removeDeadEntities(EntityVec& vec)
	{
		// remove all dead entities
	}

public:
	EntityManager() = default;

	void update()
	{
		// add entities from m_entitiesToAdd

		removeDeadEntities(m_entities);
		for (auto& [tag, entityVec] : m_entityMap)
		{
			removeDeadEntities(entityVec);
		}
	}
};
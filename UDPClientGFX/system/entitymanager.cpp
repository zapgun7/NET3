#include <system/entitymanager.h>

gdpNamespaceBegin

EntityManager::EntityManager()
{
}

EntityManager::~EntityManager()
{
    for (auto& entity : m_Entities)
    {
        delete entity;
    }
}

EntityManager& EntityManager::GetInstance()
{
    static EntityManager instance; // Singleton instance
    return instance;
}

void EntityManager::GetEntities(std::vector<Entity*>& entities) const
{
    entities = m_Entities;
}

Entity* EntityManager::CreateEntity()
{
    Entity* newEntity = new Entity();
    m_Entities.push_back(newEntity);
    return newEntity;
}

void EntityManager::RemoveEntity(Entity* entity)
{
    auto it = std::find(m_Entities.begin(), m_Entities.end(), entity);
    if (it != m_Entities.end())
    {
        delete* it;
        m_Entities.erase(it);
    }
}


gdpNamespaceEnd
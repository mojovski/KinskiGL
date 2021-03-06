//
//  Object3DComponent.cpp
//  gl
//
//  Created by Fabian on 6/28/13.
//
//

#include "Object3DComponent.h"
#include "gl/Mesh.h"

namespace kinski
{
    Object3DComponent::Object3DComponent():
    m_object_index(RangedProperty<int>::create("index", -1, -1, 256)),
    m_enabled(Property_<bool>::create("enabled", true)),
    m_position_x(Property_<float>::create("position X", 0)),
    m_position_y(Property_<float>::create("position Y", 0)),
    m_position_z(Property_<float>::create("position Z", 0)),
    m_scale(Property_<glm::vec3>::create("scale", glm::vec3(1))),
    m_rotation(Property_<glm::mat3>::create("rotation", glm::mat3()))
    {
        registerProperty(m_object_index);
        registerProperty(m_enabled);
        registerProperty(m_position_x);
        registerProperty(m_position_y);
        registerProperty(m_position_z);
        registerProperty(m_scale);
        registerProperty(m_rotation);
        set_name("3D_Objects");
    }
    
    Object3DComponent::~Object3DComponent(){}
    
    void Object3DComponent::updateProperty(const Property::ConstPtr &theProperty)
    {
        gl::Object3DPtr active_object = m_objects.empty() ? gl::Object3DPtr() : m_objects[*m_object_index];
        if(!active_object)
        {
            LOG_ERROR << "could not update: no component set ...";
            return;
        }
        if(theProperty == m_object_index)
        {
            refresh();
        }
        else if(theProperty == m_enabled)
        {
            active_object->set_enabled(*m_enabled);
        }
        else if(theProperty == m_position_x || theProperty == m_position_y || theProperty == m_position_z)
        {
            active_object->setPosition(glm::vec3(*m_position_x, *m_position_y, *m_position_z));
        }
        else if(theProperty == m_scale)
        {
            active_object->setScale(*m_scale);
        }
        else if(theProperty == m_rotation)
        {
            active_object->setRotation(*m_rotation);
        }
    }
    
    void Object3DComponent::set_objects(const std::vector<gl::Object3DPtr> &the_objects, bool copy_settings)
    {
        m_objects.assign(the_objects.begin(), the_objects.end());
        m_object_index->setRange(0, the_objects.size() - 1);
        
        if(copy_settings)
        {
            observeProperties();
            m_object_index->set(*m_object_index);
        }
    }
    
    void Object3DComponent::set_objects(const std::vector<gl::MeshPtr> &the_objects,
                                        bool copy_settings)
    {
        m_objects.clear();
        for(auto m : the_objects)
        {
            m_objects.push_back(std::dynamic_pointer_cast<gl::Object3D>(m));
        }
        
        m_object_index->setRange(0, m_objects.size() - 1);
        
        if(copy_settings)
        {
            observeProperties();
            m_object_index->set(*m_object_index);
        }
    }
    
    void Object3DComponent::set_index(int index)
    {
        *m_object_index = index;
    }
    
    void Object3DComponent::refresh()
    {
        observeProperties(false);
        
        gl::Object3DPtr active_object = m_objects.empty() ? gl::Object3DPtr() : m_objects[*m_object_index];
        if(!active_object)
        {
            LOG_ERROR << "could not refresh: no component set ...";
            return;
        }
        
        *m_enabled = active_object->enabled();
        *m_position_x = active_object->position().x;
        *m_position_y = active_object->position().y;
        *m_position_z = active_object->position().z;
        *m_scale = active_object->scale();
        *m_rotation = glm::mat3_cast(active_object->rotation());
        
        observeProperties(true);
    }
}

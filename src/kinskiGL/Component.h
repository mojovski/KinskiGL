//
//  COMPONENT.h
//  ATS
//
//  Created by Sebastian Heymann on 8/11/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef __KINSKI_COMPONENT_INCLUDED__
#define __KINSKI_COMPONENT_INCLUDED__

#include <string>
#include <list>
#include <boost/shared_ptr.hpp>

#include <iostream>
#include <exception>
#include "Property.h"
#include "Exception.h"

namespace kinski 
{
    
    class Component 
    {
    public:
        typedef boost::shared_ptr<Component> Ptr;
        
        Component();
        virtual ~Component() = 0;    
        std::list<Property::Ptr> getPropertyList(); 
        Property::Ptr getPropertyByName(const std::string & thePropertyName);
    
    public: 
        virtual void terminate() = 0;
        virtual void tic() = 0;

    protected:        
        std::list<Property::Ptr> m_propertyList;
        Property::Ptr registerProperty(Property * theProperty);
    };

    // Exception definitions. TODO: put those to some neat macros
    class PropertyNotFoundException : public Exception
    {
    public:
        PropertyNotFoundException(std::string thePropertyName): 
        Exception(std::string("Named Property not found: ")+thePropertyName)
        {}
    }; 
    
    class ComponentError: public Exception
    {
    public:
        ComponentError(std::string theErrorString):
        Exception(std::string("ComponentError: ")+theErrorString)
        {}
    }; 
}

#endif // __KINSKI_COMPONENT_INCLUDED__
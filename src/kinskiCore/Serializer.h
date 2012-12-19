//
//  Serializer.h
//  kinskiGL
//
//  Created by Fabian on 11/4/12.
//
//

#ifndef __kinskiGL__Serializer__
#define __kinskiGL__Serializer__

#include "json/json.h"
#include "Component.h"

namespace kinski
{
    /*!
     * Delegate object to handle all known types
     * Can be provided by user to add support for arbitrary data formats
     */
    class PropertyIO
    {
    public:
        
        static const std::string PROPERTY_TYPE;
        static const std::string PROPERTY_VALUE;
        static const std::string PROPERTY_VALUES;
        static const std::string PROPERTY_TYPE_FLOAT;
        static const std::string PROPERTY_TYPE_STRING;
        static const std::string PROPERTY_TYPE_DOUBLE;
        static const std::string PROPERTY_TYPE_BOOLEAN;
        static const std::string PROPERTY_TYPE_INT;
        static const std::string PROPERTY_TYPE_UNKNOWN;
        static const std::string PROPERTY_NAME;
        static const std::string PROPERTIES;
        
        virtual ~PropertyIO(){};
        virtual bool readPropertyValue(const Property::ConstPtr &theProperty,
                                       Json::Value &theJsonValue) const;
        virtual bool writePropertyValue(Property::Ptr &theProperty,
                                        const Json::Value &theJsonValue) const;
    };
    
    class Serializer
    {
    public:
        /*!
         * Save a component´s state to file using json file formatting
         */
        static void saveComponentState(const Component::ConstPtr &theComponent,
                                       const std::string &theFileName,
                                       const PropertyIO &theIO = PropertyIO());
        
        /*!
         * Read a component´s state from a json-file
         */
        static void loadComponentState(Component::Ptr theComponent,
                                       const std::string &theFileName,
                                       const PropertyIO &theIO = PropertyIO());
        
        /*!
         * Serialize a component to a string in json format.
         * Supported Property types are determined by theIO object
         */
        static std::string serializeComponent(const Component::ConstPtr &theComponent,
                                              const PropertyIO &theIO = PropertyIO());
        
        /*!
         * Read a component´s state from a string in json-format
         * Supported Property types are determined by theIO object
         */
        static void applyStateToComponent(const Component::Ptr &theComponent,
                                          const std::string &theState,
                                          const PropertyIO &theIO = PropertyIO());
        
        class ParsingException: public Exception
        {
        public:
            ParsingException(const std::string &theContentString) :
            Exception(std::string("Error while parsing json string: ") + theContentString) {}
        };
    };
    
    /************************ Exceptions ************************/
    
    class FileReadingException: public Exception
    {
    public:
        FileReadingException(const std::string &theFilename) :
        Exception(std::string("Error while reading file: ") + theFilename) {}
    };
    
    class OutputFileException: public Exception
    {
    public:
        OutputFileException(const std::string &theFilename) :
        Exception(std::string("Could not open file for writing configuration: ") + theFilename) {}
    };
}

#endif /* defined(__kinskiGL__Serializer__) */

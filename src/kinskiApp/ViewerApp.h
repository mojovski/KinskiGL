//
//  ViewerApp.h
//  kinskiGL
//
//  Created by Fabian on 3/1/13.
//
//

#ifndef __kinskiGL__ViewerApp__
#define __kinskiGL__ViewerApp__

#include "kinskiApp/GLFW_App.h"
#include "kinskiGL/SerializerGL.h"
#include "kinskiGL/Scene.h"
#include "kinskiGL/Camera.h"
#include "kinskiGL/Mesh.h"

namespace kinski {
    
    template <typename T>
    class MovingAverage
    {
    public:
        explicit MovingAverage(uint32_t sz = 5):m_size(sz){}
        inline void push(const T &theValue)
        {
            m_values.push_back(theValue);
            if(m_values.size() > m_size) m_values.pop_front();
        }
        
        const T filter()
        {
            T ret;
            typename std::list<T>::const_iterator it = m_values.begin();
            for ( ; it != m_values.end(); ++it)
                ret += *it;
            ret /= (float)m_values.size();
            m_values.clear();
            return ret;
        }
        
    private:
        std::list<T> m_values;
        uint32_t m_size;
    };
    
    class ViewerApp : public GLFW_App
    {
    public:
        ViewerApp();
        virtual ~ViewerApp();
        
        void setup();
        void update(const float timeDelta);
        void mousePress(const MouseEvent &e);
        void mouseDrag(const MouseEvent &e);
        void mouseRelease(const MouseEvent &e);
        void mouseWheel(const MouseEvent &e);
        void keyPress(const KeyEvent &e);
        void resize(int w, int h);
        
        // Property observer callback
        void updateProperty(const Property::ConstPtr &theProperty);
        
        bool wireframe() const { return *m_wireFrame; };
        bool normals() const { return *m_drawNormals; };
        const glm::vec3& light_direction(){ return *m_light_direction; };
        const gl::PerspectiveCamera::Ptr& camera() const { return m_camera; };
        gl::MeshPtr selected_mesh(){ return m_selected_mesh; };
        const std::vector<gl::MaterialPtr>& materials() const { return m_materials; };
        std::vector<gl::MaterialPtr>& materials(){ return m_materials; };
        const gl::Scene& scene() const { return m_scene; };
        gl::Scene& scene() { return m_scene; };
        bool precise_selection() const { return m_precise_selection; };
        void set_precise_selection(bool b){ m_precise_selection = b; };
        
    private:
        std::vector<gl::MaterialPtr> m_materials;
        gl::MeshPtr m_selected_mesh;
        gl::PerspectiveCamera::Ptr m_camera;
        gl::Scene m_scene;
        bool m_precise_selection;
        
        RangedProperty<int>::Ptr m_logger_severity;
        Property_<bool>::Ptr m_show_tweakbar;
        Property_<glm::vec2>::Ptr m_window_size;
        RangedProperty<float>::Ptr m_distance;
        Property_<glm::mat3>::Ptr m_rotation;
        RangedProperty<float>::Ptr m_rotationSpeed;
        Property_<bool>::Ptr m_wireFrame;
        Property_<bool>::Ptr m_drawNormals;
        Property_<glm::vec3>::Ptr m_light_direction;
        Property_<glm::vec4>::Ptr m_clear_color;
        
        // mouse rotation control
        glm::vec2 m_clickPos, m_dragPos, m_inertia;
        float m_rotation_damping;
        bool  m_mouse_down;
        glm::mat3 m_lastTransform;
        MovingAverage<glm::vec2> m_avg_filter;
    };
}// namespace


#endif /* defined(__kinskiGL__ViewerApp__) */
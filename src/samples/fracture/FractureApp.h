//
//  FractureApp.h
//  gl
//
//  Created by Fabian on 01/02/15.
//
//

#ifndef __gl__3dViewer__
#define __gl__3dViewer__

#include "app/ViewerApp.h"
#include "app/LightComponent.h"

// module header
#include "physics_context.h"

namespace kinski
{
    class FractureApp : public ViewerApp
    {
    private:
        
        gl::MeshPtr m_mesh;
        
        physics::physics_context m_physics;
        
        LightComponent::Ptr m_light_component;
        
        Property_<std::string>::Ptr m_model_path = Property_<std::string>::create("Model path", "");
        
        
        Property_<bool>::Ptr
        m_physics_running = Property_<bool>::create("physics running", true),
        m_physics_debug_draw = Property_<bool>::create("physics debug draw", true);
        
        Property_<uint32_t>::Ptr
        m_num_fracture_shards = Property_<uint32_t>::create("num fracture shards", 20);
        
        physics::btCollisionShapePtr m_box_shape;
        gl::GeometryPtr m_box_geom;
        
        void shoot_box(const gl::Ray &the_ray, float the_velocity,
                       const glm::vec3 &the_half_extents = glm::vec3(.5f));
        
        void fracture_test(uint32_t num_shards);
        
    public:
        
        void setup() override;
        void update(float timeDelta) override;
        void draw() override;
        void resize(int w ,int h) override;
        void keyPress(const KeyEvent &e) override;
        void keyRelease(const KeyEvent &e) override;
        void mousePress(const MouseEvent &e) override;
        void mouseRelease(const MouseEvent &e) override;
        void mouseMove(const MouseEvent &e) override;
        void mouseDrag(const MouseEvent &e) override;
        void mouseWheel(const MouseEvent &e) override;
        void got_message(const std::vector<uint8_t> &the_message) override;
        void fileDrop(const MouseEvent &e, const std::vector<std::string> &files) override;
        void tearDown() override;
        void updateProperty(const Property::ConstPtr &theProperty) override;
    };
}// namespace kinski

#endif /* defined(__gl__3dViewer__) */

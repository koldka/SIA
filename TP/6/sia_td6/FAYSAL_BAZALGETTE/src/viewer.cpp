#include "viewer.h"
#include "camera.h"

#include "SOIL2.h"

extern int WIDTH;
extern int HEIGHT;

using namespace Eigen;

Viewer::Viewer()
{
}

Viewer::~Viewer()
{
}

////////////////////////////////////////////////////////////////////////////////
// GL stuff

// initialize OpenGL context
void Viewer::init(int w, int h){
    _winWidth = w;
    _winHeight = h;

    // set the background color, i.e., the color used
    // to fill the screen when calling glClear(GL_COLOR_BUFFER_BIT)
    glClearColor(0.f,0.f,0.f,1.f);

    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);

    loadProgram();

    Quad* quad = new Quad();
    quad->init();
    quad->setTransformationMatrix( AngleAxisf((float)M_PI_2, Vector3f(-1,0,0)) * Scaling(20.f,20.f,1.f) * Translation3f(0,0,-0.5));
    _shapes.push_back(quad);
    _specularCoef.push_back(0.f);

    Mesh* mesh = new Mesh();
    mesh = new Mesh();
    mesh->load(DATA_DIR"/models/sphere.off");
    mesh->init();
    mesh->setTransformationMatrix(Translation3f(0,0,2.f) * Scaling(0.5f) );
    _shapes.push_back(mesh);
    _specularCoef.push_back(0.3f);

    _pointLight = Sphere(0.025f);
    _pointLight.init();
    _pointLight.setTransformationMatrix(Affine3f(Translation3f(1.f,0.75f,1.f)));
    _lightColor = Vector3f(1.f,1.f,1.f);

    AlignedBox3f aabb;
    for(uint i=0; i<_shapes.size(); ++i)
        aabb.extend(_shapes[i]->boundingBox());

    _cam.setSceneCenter(aabb.center());
    _cam.setSceneRadius(aabb.sizes().maxCoeff());
    _cam.setSceneDistance(_cam.sceneRadius() * 3.f);
    _cam.setMinNear(0.1f);
    _cam.setNearFarOffsets(-_cam.sceneRadius() * 100.f,
                           _cam.sceneRadius() * 100.f);
    _cam.setScreenViewport(AlignedBox2f(Vector2f(0.0,0.0), Vector2f(w,h)));

    Vector4f tmp = (_pointLight.getTransformationMatrix() * Vector4f(0,0,0,1));
    Vector3f lightPos = Vector3f(tmp[0], tmp[1], tmp[2]);

    _shadow = mesh->computeShadowVolume(lightPos);
}

void Viewer::reshape(int w, int h){
    _winWidth = w;
    _winHeight = h;
    _cam.setScreenViewport(AlignedBox2f(Vector2f(0.0,0.0), Vector2f(w,h)));
    glViewport(0, 0, w, h);
}


/*!
   callback to draw graphic primitives
 */
void Viewer::display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |GL_STENCIL_BUFFER_BIT);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
    _blinnPrg.activate();
     _pointLight.setTransformationMatrix(Affine3f(AngleAxisf(M_PI/50,Vector3f(0.0,1.0,0.0)))*_pointLight.getTransformationMatrix());

    Vector4f lightPos = (_pointLight.getTransformationMatrix() * Vector4f(0,0,0,1));
    glUniformMatrix4fv(_blinnPrg.getUniformLocation("projection_matrix"), 1, GL_FALSE, _cam.computeProjectionMatrix().data());
    glUniformMatrix4fv(_blinnPrg.getUniformLocation("view_matrix"), 1, GL_FALSE, _cam.computeViewMatrix().data());

    glUniform4fv(_blinnPrg.getUniformLocation("light_pos_view"), 1, (_cam.computeViewMatrix() * lightPos).eval().data());
    glUniform3fv(_blinnPrg.getUniformLocation("light_color"), 1, _lightColor.data());

    for(uint i=0; i<_shapes.size(); ++i)
    {
        if(Mesh* sch = dynamic_cast<Mesh*>(_shapes[i])){
            delete _shadow;
            _shadow = sch->computeShadowVolume(lightPos.head(3));
        }
        glUniformMatrix4fv(_blinnPrg.getUniformLocation("model_matrix"), 1, GL_FALSE, _shapes[i]->getTransformationMatrix().data());
        Matrix3f normal_matrix = (_cam.computeViewMatrix() * _shapes[i]->getTransformationMatrix()).linear().inverse().transpose();
        glUniformMatrix3fv(_blinnPrg.getUniformLocation("normal_matrix"), 1, GL_FALSE, normal_matrix.data());
        glUniform1f(_blinnPrg.getUniformLocation("specular_coef"), _specularCoef[i]);

        _shapes[i]->display(&_blinnPrg);

    }

    _blinnPrg.deactivate();
    glEnable(GL_STENCIL_TEST);
    glDepthMask(GL_FALSE);
    glStencilFunc(GL_ALWAYS, 0, 0xff);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);

    // Draw light source
    _simplePrg.activate();
    glUniformMatrix4fv(_simplePrg.getUniformLocation("projection_matrix"), 1, GL_FALSE, _cam.computeProjectionMatrix().data());
    glUniformMatrix4fv(_simplePrg.getUniformLocation("view_matrix"), 1, GL_FALSE, _cam.computeViewMatrix().data());

   // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glUniformMatrix4fv(_simplePrg.getUniformLocation("model_matrix"), 1, GL_FALSE, _shadow->getTransformationMatrix().data());
    _shadow->display(&_simplePrg);
   // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    _simplePrg.deactivate();

    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT );
    glStencilFunc(GL_EQUAL, 0, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    _blinnPrg.activate();
    glUniformMatrix4fv(_blinnPrg.getUniformLocation("projection_matrix"), 1, GL_FALSE, _cam.computeProjectionMatrix().data());
    glUniformMatrix4fv(_blinnPrg.getUniformLocation("view_matrix"), 1, GL_FALSE, _cam.computeViewMatrix().data());

    lightPos = (_pointLight.getTransformationMatrix() * Vector4f(0,0,0,1));
    glUniform4fv(_blinnPrg.getUniformLocation("light_pos_view"), 1, (_cam.computeViewMatrix() * lightPos).eval().data());
    glUniform3fv(_blinnPrg.getUniformLocation("light_color"), 1, _lightColor.data());

    for(uint i=0; i<_shapes.size(); ++i)
    {
        glUniformMatrix4fv(_blinnPrg.getUniformLocation("model_matrix"), 1, GL_FALSE, _shapes[i]->getTransformationMatrix().data());
        Matrix3f normal_matrix = (_cam.computeViewMatrix() * _shapes[i]->getTransformationMatrix()).linear().inverse().transpose();
        glUniformMatrix3fv(_blinnPrg.getUniformLocation("normal_matrix"), 1, GL_FALSE, normal_matrix.data());
        glUniform1f(_blinnPrg.getUniformLocation("specular_coef"), _specularCoef[i]);

        _shapes[i]->display(&_blinnPrg);
    }
    _blinnPrg.deactivate();

    glDisable(GL_STENCIL_TEST);

    _simplePrg.activate();
    glUniformMatrix4fv(_simplePrg.getUniformLocation("projection_matrix"), 1, GL_FALSE, _cam.computeProjectionMatrix().data());
    glUniformMatrix4fv(_simplePrg.getUniformLocation("view_matrix"), 1, GL_FALSE, _cam.computeViewMatrix().data());
    glUniformMatrix4fv(_simplePrg.getUniformLocation("model_matrix"), 1, GL_FALSE, _pointLight.getTransformationMatrix().data());
    glUniform3fv(_simplePrg.getUniformLocation("light_col"), 1, _lightColor.data());
    _pointLight.display(&_simplePrg);
    _simplePrg.deactivate();
}


void Viewer::updateScene()
{
    display();
}

void Viewer::loadProgram()
{
    _blinnPrg.loadFromFiles(DATA_DIR"/shaders/blinn.vert", DATA_DIR"/shaders/blinn.frag");
    _simplePrg.loadFromFiles(DATA_DIR"/shaders/simple.vert", DATA_DIR"/shaders/simple.frag");
    checkError();
}

////////////////////////////////////////////////////////////////////////////////
// Events

/*!
   callback to manage mouse : called when user press or release mouse button
   You can change in this function the way the user
   interact with the system.
 */
void Viewer::mousePressed(GLFWwindow *window, int button, int action)
{
    if(action == GLFW_PRESS) {
        if(button == GLFW_MOUSE_BUTTON_LEFT)
        {
            _cam.startRotation(_lastMousePos);
        }
        else if(button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            _cam.startTranslation(_lastMousePos);
        }
        _button = button;
    }else if(action == GLFW_RELEASE) {
        if(_button == GLFW_MOUSE_BUTTON_LEFT)
        {
            _cam.endRotation();
        }
        else if(_button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            _cam.endTranslation();
        }
        _button = -1;
    }
}


/*!
   callback to manage mouse : called when user move mouse with button pressed
   You can change in this function the way the user
   interact with the system.
 */
void Viewer::mouseMoved(int x, int y)
{
    if(_button == GLFW_MOUSE_BUTTON_LEFT)
    {
        _cam.dragRotate(Vector2f(x,y));
    }
    else if(_button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        _cam.dragTranslate(Vector2f(x,y));
    }
    _lastMousePos = Vector2f(x,y);
}

void Viewer::mouseScroll(double x, double y)
{
    _cam.zoom((y>0)? 1.1: 1./1.1);
}

/*!
   callback to manage keyboard interactions
   You can change in this function the way the user
   interact with the system.
 */
void Viewer::keyPressed(int key, int action, int mods)
{
    if(key == GLFW_KEY_R && action == GLFW_PRESS)
        loadProgram();
}

void Viewer::charPressed(int key)
{
}

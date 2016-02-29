
#include <gloperate/stages/DemoStage.h>

#include <random>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/constants.hpp>

#include <globjects/base/StringTemplate.h>
#include <globjects/base/StaticStringSource.h>
#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/Framebuffer.h>
#include <globjects/globjects.h>

#include <glbinding/gl/gl.h>


static const char * s_vertexShader = R"(
    #version 140
    #extension GL_ARB_explicit_attrib_location : require

    uniform mat4 modelViewProjectionMatrix;

    layout (location = 0) in vec2 a_vertex;
    out vec2 v_uv;

    void main()
    {
        v_uv = a_vertex * 0.5 + 0.5;
        gl_Position = modelViewProjectionMatrix * vec4(a_vertex, 0.0, 1.0);
    }
)";

static const char * s_fragmentShader = R"(
    #version 140
    #extension GL_ARB_explicit_attrib_location : require

    uniform sampler2D source;

    layout (location = 0) out vec4 fragColor;

    in vec2 v_uv;

    void main()
    {
        fragColor = texture(source, v_uv);
    }
)";


namespace gloperate
{


DemoStage::DemoStage(ViewerContext * viewerContext)
: Stage(viewerContext)
, m_timer(viewerContext)
, m_time(0.0f)
, m_angle(0.0f)
{
    m_timer.fired.connect([this] ()
    {
        // Update virtual time
        m_time += m_timeDelta;

        // Redraw
        invalidateOutput();
    });

    m_timer.start(0.0f);
}

DemoStage::~DemoStage()
{
}

void DemoStage::onContextInit(AbstractGLContext *)
{
    globjects::init();

    createAndSetupCamera();
    createAndSetupTexture();
    createAndSetupGeometry();
}

void DemoStage::onContextDeinit(AbstractGLContext *)
{
}

void DemoStage::onProcess(AbstractGLContext *)
{
    // Update viewport
    gl::glViewport(
        m_deviceViewport.x,
        m_deviceViewport.y,
        m_deviceViewport.z,
        m_deviceViewport.w
    );

    // Bind FBO
    globjects::Framebuffer * fbo = globjects::Framebuffer::defaultFBO();
    fbo->bind(gl::GL_FRAMEBUFFER);

    // Update animation
    m_angle = m_time;

    // Clear background
    /*
    gl::glClearColor(
        (float)0.0f,
        (float)0.0f,
        (float)0.5f,
        1.0f);
    gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);
    */

    // Get model matrix
    glm::mat4 model = glm::mat4(1.0);
    model = glm::rotate(model, m_angle, glm::vec3(0.0, 1.0, 0.0));

    // Update model-view-projection matrix
    m_program->setUniform("viewProjectionMatrix",      m_camera->viewProjection());
    m_program->setUniform("modelViewProjectionMatrix", m_camera->viewProjection() * model);

    // Lazy creation of texture
    if (!m_texture) {
        createAndSetupTexture();
    }

    // Bind texture
    if (m_texture) {
        gl::glActiveTexture(gl::GL_TEXTURE0 + 0);
        m_texture->bind();
    }

    // Draw geometry
    m_program->use();
    m_vao->drawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);
    m_program->release();

    // Unbind texture
    if (m_texture) {
        m_texture->unbind();
    }

    // Unbind FBO
    globjects::Framebuffer::unbind(gl::GL_FRAMEBUFFER);
}

void DemoStage::createAndSetupCamera()
{
    m_camera = new Camera();
    m_camera->setEye(glm::vec3(0.0, 0.0, 12.0));
}

void DemoStage::createAndSetupTexture()
{
    // Create procedural texture
    static const int w(256);
    static const int h(256);
    unsigned char data[w * h * 4];

    std::random_device rd;
    std::mt19937 generator(rd());
    std::poisson_distribution<> r(0.2);

    for (int i = 0; i < w * h * 4; ++i) {
        data[i] = static_cast<unsigned char>(255 - static_cast<unsigned char>(r(generator) * 255));
    }

    m_texture = globjects::Texture::createDefault(gl::GL_TEXTURE_2D);
    m_texture->image2D(0, gl::GL_RGBA8, w, h, 0, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, data);
}

void DemoStage::createAndSetupGeometry()
{
    static const std::array<glm::vec2, 4> raw {
        glm::vec2( +1.f, -1.f ),
        glm::vec2( +1.f, +1.f ),
        glm::vec2( -1.f, -1.f ),
        glm::vec2( -1.f, +1.f ) };

    m_vao = new globjects::VertexArray;
    m_buffer = new globjects::Buffer();
    m_buffer->setData(raw, gl::GL_STATIC_DRAW);

    auto binding = m_vao->binding(0);
    binding->setAttribute(0);
    binding->setBuffer(m_buffer, 0, sizeof(glm::vec2));
    binding->setFormat(2, gl::GL_FLOAT, gl::GL_FALSE, 0);
    m_vao->enable(0);

    globjects::StringTemplate * vertexShaderSource   = new globjects::StringTemplate(new globjects::StaticStringSource(s_vertexShader  ));
    globjects::StringTemplate * fragmentShaderSource = new globjects::StringTemplate(new globjects::StaticStringSource(s_fragmentShader));

#ifdef __APPLE__
    vertexShaderSource  ->replace("#version 140", "#version 150");
    fragmentShaderSource->replace("#version 140", "#version 150");
#endif

    m_vertexShader   = new globjects::Shader(gl::GL_VERTEX_SHADER,   vertexShaderSource);
    m_fragmentShader = new globjects::Shader(gl::GL_FRAGMENT_SHADER, fragmentShaderSource);
    m_program = new globjects::Program();
    m_program->attach(m_vertexShader, m_fragmentShader);

    m_program->setUniform("source", 0);
}


} // namespace gloperate
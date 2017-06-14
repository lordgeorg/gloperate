
#include <gloperate/stages/base/RasterizationStage.h>

#include <glbinding/gl/gl.h>

#include <globjects/Framebuffer.h>

#include <gloperate/rendering/AbstractDrawable.h>


namespace gloperate
{


CPPEXPOSE_COMPONENT(RasterizationStage, gloperate::Stage)


RasterizationStage::RasterizationStage(Environment * environment, const std::string & name)
: Stage(environment, "RasterizationStage", name)
, renderInterface(             this)
, rasterize      ("rasterize", this, true)
, drawable       ("drawable",  this)
{
    inputAdded.connect( [this] (gloperate::AbstractSlot * connectedInput) {
        auto renderTargetInput = dynamic_cast<Input<gloperate::RenderTarget *> *>(connectedInput);

        if (renderTargetInput)
        {
            renderInterface.addRenderTargetInput(renderTargetInput);
        }
    });

    outputAdded.connect( [this] (gloperate::AbstractSlot * connectedOutput) {
        auto renderTargetOutput = dynamic_cast<Output<gloperate::RenderTarget *> *>(connectedOutput);

        if (renderTargetOutput)
        {
            renderInterface.addRenderTargetOutput(renderTargetOutput);
        }
    });
}

RasterizationStage::~RasterizationStage()
{
}

void RasterizationStage::onProcess()
{
    if (!renderInterface.allRenderTargetsCompatible())
    {
        cppassist::warning("gloperate") << "Framebuffer attachments not compatible";

        return;
    }

    // Check if rasterization is enabled
    if (*rasterize)
    {
        // Set viewport
        const glm::vec4 & viewport = *renderInterface.viewport;
        gl::glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

        // Configure FBO
        auto fbo = renderInterface.configureFBO(m_fbo.get(), m_defaultFBO.get());

        // Bind FBO
        fbo->bind(gl::GL_FRAMEBUFFER);

        // Render the drawable
        (*drawable)->draw();

        // Unbind FBO
        fbo->unbind();
    }

    // Update outputs
    renderInterface.pairwiseRenderTargetsDo([](Input <RenderTarget *> * input, Output <RenderTarget *> * output) {
        output->setValue(**input);
    });
}


} // namespace gloperate

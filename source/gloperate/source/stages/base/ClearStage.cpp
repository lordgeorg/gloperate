
#include <gloperate/stages/base/ClearStage.h>

#include <glbinding/gl/gl.h>

#include <globjects/Framebuffer.h>
#include <globjects/FramebufferAttachment.h>
#include <globjects/AttachedRenderbuffer.h>
#include <globjects/AttachedTexture.h>

#include <gloperate/rendering/RenderTarget.h>


namespace gloperate
{


CPPEXPOSE_COMPONENT(ClearStage, gloperate::Stage)


ClearStage::ClearStage(Environment * environment, const std::string & name)
: Stage(environment, "ClearStage", name)
, clear("clear",  this)
, renderInterface(this)
{
    inputAdded.connect( [this] (gloperate::AbstractSlot * connectedInput) {
        auto renderTargetInput = dynamic_cast<Input<gloperate::RenderTarget *> *>(connectedInput);
        auto colorValueInput = dynamic_cast<Input<glm::vec4> *>(connectedInput);
        auto depthValueInput = dynamic_cast<Input<float> *>(connectedInput);
        auto depthStencilValueInput = dynamic_cast<Input<std::pair<float, int>> *>(connectedInput);

        if (renderTargetInput)
        {
            renderInterface.addRenderTargetInput(renderTargetInput);
        }

        if (colorValueInput)
        {
            m_colorValueInputs.push_back(colorValueInput);
        }

        if (depthValueInput)
        {
            m_depthValueInputs.push_back(depthValueInput);
        }

        if (depthStencilValueInput)
        {
            m_depthStencilValueInputs.push_back(depthStencilValueInput);
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

ClearStage::~ClearStage()
{
}

void ClearStage::onContextInit(AbstractGLContext *)
{
    m_defaultFBO = globjects::Framebuffer::defaultFBO();
    m_fbo = cppassist::make_unique<globjects::Framebuffer>();
}

void ClearStage::onContextDeinit(AbstractGLContext *)
{
    m_defaultFBO = nullptr;
    m_fbo = nullptr;
}

void ClearStage::onProcess()
{
    if (!*clear)
    {
        // Setup OpenGL state
        gl::glScissor(renderInterface.viewport->x, renderInterface.viewport->y, renderInterface.viewport->z, renderInterface.viewport->w);
        gl::glEnable(gl::GL_SCISSOR_TEST);

        size_t colorAttachmentIndex        = 0;
        size_t depthAttachmentIndex        = 0;
        size_t depthStencilAttachmentIndex = 0;

        renderInterface.pairwiseRenderTargetsDo([this, & colorAttachmentIndex, & depthAttachmentIndex, & depthStencilAttachmentIndex](Input <RenderTarget *> * input, Output <RenderTarget *> * output) {
            if (!output->isRequired() || !**input)
            {
                return;
            }

            switch ((**input)->attachmentType())
            {
            case AttachmentType::Color:
                {
                    if (m_colorValueInputs.size() <= colorAttachmentIndex)
                    {
                        return;
                    }

                    auto fbo = renderInterface.configureFBO(colorAttachmentIndex, **input, m_fbo.get(), m_defaultFBO.get());

                    fbo->clearBuffer((**input)->attachmentBuffer(), (**input)->attachmentDrawBuffer(colorAttachmentIndex), **m_colorValueInputs.at(colorAttachmentIndex));

                    ++colorAttachmentIndex;
                }
                break;
            case AttachmentType::Depth:
                {
                    if (m_depthValueInputs.size() <= depthAttachmentIndex)
                    {
                        return;
                    }

                    auto fbo = renderInterface.configureFBO(depthAttachmentIndex, **input, m_fbo.get(), m_defaultFBO.get());

                    fbo->clearBuffer((**input)->attachmentBuffer(), (**input)->attachmentDrawBuffer(depthAttachmentIndex), **m_depthValueInputs.at(depthAttachmentIndex), 0);

                    ++depthAttachmentIndex;
                }
                break;
            case AttachmentType::DepthStencil:
                {
                    if (m_depthStencilValueInputs.size() <= depthStencilAttachmentIndex)
                    {
                        return;
                    }

                    auto fbo = renderInterface.configureFBO(depthStencilAttachmentIndex, **input, m_fbo.get(), m_defaultFBO.get());

                    fbo->clearBuffer((**input)->attachmentBuffer(), (**input)->attachmentDrawBuffer(depthAttachmentIndex), (**m_depthStencilValueInputs.at(depthAttachmentIndex)).first, (**m_depthStencilValueInputs.at(depthAttachmentIndex)).second);

                    ++depthStencilAttachmentIndex;
                }
                break;
            default:
                break;
            }
        });

        // Reset OpenGL state
        gl::glDisable(gl::GL_SCISSOR_TEST);
    }

    // Update outputs
    renderInterface.pairwiseRenderTargetsDo([](Input <RenderTarget *> * input, Output <RenderTarget *> * output) {
        output->setValue(**input);
    });
}

} // namespace gloperate

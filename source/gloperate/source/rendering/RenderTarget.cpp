
#include <gloperate/rendering/RenderTarget.h>

#include <globjects/Framebuffer.h>
#include <globjects/FramebufferAttachment.h>
#include <globjects/Renderbuffer.h>
#include <globjects/Texture.h>


namespace gloperate
{


RenderTarget::RenderTarget()
: m_type(RenderTargetType::Invalid)
, m_attachment(gl::GL_NONE)
, m_texture(nullptr)
, m_renderbuffer(nullptr)
, m_userDefined(nullptr)
{
}

RenderTarget::~RenderTarget()
{
}

void RenderTarget::releaseTarget()
{
    switch (m_type)
    {
    case RenderTargetType::Texture:
        m_texture = nullptr;
        break;
    case RenderTargetType::Renderbuffer:
        m_renderbuffer = nullptr;
        break;
    case RenderTargetType::DefaultFBOAttachment:
        m_attachment = gl::GL_NONE;
        break;
    case RenderTargetType::UserDefinedFBOAttachment:
        m_userDefined = nullptr;
        break;
    case RenderTargetType::Invalid:
    default:
        m_attachment = gl::GL_NONE;
        break;
    }

    m_type = RenderTargetType::Invalid;
}

AttachmentType RenderTarget::attachmentType() const
{
    return m_attachmentType;
}

gl::GLenum RenderTarget::attachmentGLType() const
{
    switch (m_attachmentType)
    {
    case AttachmentType::Depth:
        return gl::GL_DEPTH;
    case AttachmentType::DepthStencil:
        return gl::GL_DEPTH_STENCIL;
    case AttachmentType::Color:
    default:
        return gl::GL_COLOR;
    }
}

void RenderTarget::setAttachmentType(AttachmentType attachmentType)
{
    m_attachmentType = attachmentType;
}

void RenderTarget::setTarget(globjects::Texture * texture)
{
    releaseTarget();

    m_type = RenderTargetType::Texture;

    m_texture = texture;
}

void RenderTarget::setTarget(globjects::Renderbuffer * renderbuffer)
{
    releaseTarget();

    m_type = RenderTargetType::Renderbuffer;

    m_renderbuffer = renderbuffer;
}

void RenderTarget::setTarget(gl::GLenum attachment)
{
    releaseTarget();

    m_type = RenderTargetType::DefaultFBOAttachment;

    m_attachment = attachment;
}

void RenderTarget::setTarget(globjects::FramebufferAttachment * fboAttachment)
{
    releaseTarget();

    m_type = RenderTargetType::UserDefinedFBOAttachment;

    m_userDefined = fboAttachment;
}

RenderTargetType RenderTarget::type() const
{
    return m_type;
}

gl::GLenum RenderTarget::defaultFramebufferAttachment() const
{
    return m_attachment;
}

globjects::Texture * RenderTarget::textureAttachment() const
{
    return m_texture;
}

globjects::Renderbuffer * RenderTarget::renderbufferAttachment() const
{
    return m_renderbuffer;
}

globjects::FramebufferAttachment * RenderTarget::framebufferAttachment() const
{
    return m_userDefined;
}

bool RenderTarget::attachmentRequiresUserDefinedFramebuffer() const
{
    return m_type == RenderTargetType::Texture
        || m_type == RenderTargetType::Renderbuffer
        || m_type == RenderTargetType::UserDefinedFBOAttachment;
}

gl::GLenum RenderTarget::attachmentBuffer() const
{
    return attachmentRequiresUserDefinedFramebuffer()
        ? attachmentGLType()
        : m_attachment;
}

gl::GLint RenderTarget::attachmentDrawBuffer(gl::GLint index) const
{
    return attachmentRequiresUserDefinedFramebuffer()
        ? (m_attachmentType == AttachmentType::Color ? index : 0)
        : 0;
}


} // namespace gloperate

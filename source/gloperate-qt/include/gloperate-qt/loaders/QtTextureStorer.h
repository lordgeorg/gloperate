
#pragma once


#include <cppexpose/plugin/plugin_api.h>

#include <gloperate/gloperate-version.h>
#include <gloperate/base/Storer.h>

#include <gloperate-qt/gloperate-qt_api.h>


namespace globjects
{
    class Texture;
}


namespace gloperate_qt
{


/**
*  @brief
*    Texture storer based on Qt
*
*  Supported options:
*    none
*/
class GLOPERATE_QT_API QtTextureStorer : public gloperate::Storer<globjects::Texture>
{
public:
    CPPEXPOSE_DECLARE_COMPONENT(
        QtTextureStorer, gloperate::AbstractStorer
      , "" // Tags
      , "" // Icon
      , "" // Annotations
      , "Store textures using the Qt image functionality"
      , GLOPERATE_AUTHOR_ORGANIZATION
      , "v1.0.0"
    )


public:
    /**
    *  @brief
    *    Constructor
    *
    *  @param[in] environment
    *    Environment to which the storer belongs (must NOT be null!)
    */
    QtTextureStorer(gloperate::Environment * environment);

    /**
    *  @brief
    *    Destructor
    */
    virtual ~QtTextureStorer();

    // Virtual gloperate::AbstractStorer functions
    virtual bool canStore(const std::string & ext) const override;
    virtual std::vector<std::string> storingTypes() const override;
    virtual std::string allStoringTypes() const override;

    // Virtual gloperate::Storer<globjects::Texture> functions
    virtual bool store(const std::string & filename, const globjects::Texture * texture, const cppexpose::Variant & options, std::function<void(int, int)> progress) const override;


protected:
    std::vector<std::string> m_extensions; ///< List of supported file extensions (e.g., ".bmp")
    std::vector<std::string> m_types;      ///< List of supported file types (e.g., "bmp image (*.bmp)")
};


} // namespace gloperate_qt

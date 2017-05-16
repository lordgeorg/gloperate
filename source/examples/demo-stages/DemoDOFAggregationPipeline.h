
#pragma once


#include <cppexpose/plugin/plugin_api.h>

#include <gloperate/gloperate-version.h>
#include <gloperate/pipeline/Pipeline.h>
#include <gloperate/pipeline/Input.h>
#include <gloperate/stages/interfaces/RenderInterface.h>


namespace gloperate_glkernel {

class MultiFrameAggregationPipeline;

}


class DemoDOFRenderingPipeline;


/**
*  @brief
*    Demo pipeline showing multiframe aggregation
*/
class DemoDOFAggregationPipeline : public gloperate::Pipeline
{
public:
    CPPEXPOSE_DECLARE_COMPONENT(
        DemoDOFAggregationPipeline, gloperate::Stage
      , "RenderStage Demo Multiframe" // Tags
      , ""                            // Icon
      , ""                            // Annotations
      , "Demo pipeline showing multiframe aggregation"
      , GLOPERATE_AUTHOR_ORGANIZATION
      , "v0.1.0"
    )


public:
    // Interfaces
    gloperate::RenderInterface renderInterface; ///< Interface for rendering into a viewer

    // Inputs
    Input<int>                 multiFrameCount; ///< Number of frames to aggregate


public:
    /**
    *  @brief
    *    Constructor
    *
    *  @param[in] environment
    *    Environment to which the pipeline belongs (must NOT be null!)
    *  @param[in] name
    *    Pipeline name
    */
    DemoDOFAggregationPipeline(gloperate::Environment * environment, const std::string & name = "DemoDOFAggregationPipeline");

    /**
    *  @brief
    *    Destructor
    */
    virtual ~DemoDOFAggregationPipeline();


protected:
    // Stages
    std::unique_ptr<gloperate_glkernel::MultiFrameAggregationPipeline> m_multiFramePipeline; ///< Aggregation Pipeline
    std::unique_ptr<DemoDOFRenderingPipeline>                                   m_dofPipeline;        ///< Pipeline generating frames to aggregate
};

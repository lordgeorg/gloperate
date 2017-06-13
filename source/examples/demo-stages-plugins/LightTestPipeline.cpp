
#include "LightTestPipeline.h"

#include <gloperate/stages/lights/LightCreationStage.h>
#include <gloperate/stages/lights/LightBufferTextureStage.h>

#include "LightTestStage.h"
#include "TimerStage.h"


CPPEXPOSE_COMPONENT(LightTestPipeline, gloperate::Stage)


LightTestPipeline::LightTestPipeline(gloperate::Environment * environment, const std::string & name)
: Pipeline(environment, "LightTestPipeline", name)
, renderInterface(this)
, glossiness("glossiness", this)
, lightType1("lightType1", this)
, lightType2("lightType2", this)
, lightType3("lightType3", this)
, lightColor1("lightColor1", this)
, lightColor2("lightColor2", this)
, lightColor3("lightColor3", this)
, lightPos1("lightPos1", this)
, lightPos2("lightPos2", this)
, lightPos3("lightPos3", this)
, lightAttenuation1("lightAttenuation1", this)
, lightAttenuation2("lightAttenuation2", this)
, lightAttenuation3("lightAttenuation3", this)
, m_lightDefStage1(cppassist::make_unique<gloperate::LightCreationStage>(environment))
, m_lightDefStage2(cppassist::make_unique<gloperate::LightCreationStage>(environment))
, m_lightDefStage3(cppassist::make_unique<gloperate::LightCreationStage>(environment))
, m_lightAccumulationStage(cppassist::make_unique<gloperate::LightBufferTextureStage>(environment))
, m_timerStage(cppassist::make_unique<TimerStage>(environment))
, m_renderStage(cppassist::make_unique<LightTestStage>(environment))
{
    glossiness.setOptions({
        {"minimum", 0.0f},
        {"maximum", 1.0f}
    });

    addStage(m_lightDefStage1.get());
    m_lightDefStage1->color << lightColor1;
    m_lightDefStage1->position << lightPos1;
    m_lightDefStage1->type << lightType1;
    m_lightDefStage1->attenuationCoefficients << lightAttenuation1;
    addStage(m_lightDefStage2.get());
    m_lightDefStage2->color << lightColor2;
    m_lightDefStage2->position << lightPos2;
    m_lightDefStage2->type << lightType2;
    m_lightDefStage2->attenuationCoefficients << lightAttenuation2;
    addStage(m_lightDefStage3.get());
    m_lightDefStage3->color << lightColor3;
    m_lightDefStage3->position << lightPos3;
    m_lightDefStage3->type << lightType3;
    m_lightDefStage3->attenuationCoefficients << lightAttenuation3;

    lightType1.setValue(0);
    lightColor1.setValue(glm::vec3(0.5, 0, 0));
    lightPos1.setValue(glm::vec3(-2, 0, -2));
    lightAttenuation1.setValue(glm::vec3(1, 0.2f, 0.01f));

    lightType2.setValue(1);
    lightColor2.setValue(glm::vec3(0.125, 0.125, 0.125));
    lightPos2.setValue(glm::vec3(0, 0, 0));
    lightAttenuation2.setValue(glm::vec3(1, 0.2f, 0.01f));

    lightType3.setValue(4);
    lightColor3.setValue(glm::vec3(1, 1, 1));
    lightPos3.setValue(glm::vec3(1.5, -1.3, -1.5));
    lightAttenuation3.setValue(glm::vec3(1, 0.2f, 0.01f));

    addStage(m_lightAccumulationStage.get());
    *(m_lightAccumulationStage->createLightInput()) << m_lightDefStage1->light;
    *(m_lightAccumulationStage->createLightInput()) << m_lightDefStage2->light;
    *(m_lightAccumulationStage->createLightInput()) << m_lightDefStage3->light;

    addStage(m_timerStage.get());
    m_timerStage->timeDelta << renderInterface.timeDelta;

    addStage(m_renderStage.get());
    m_renderStage->renderInterface.deviceViewport << renderInterface.deviceViewport;
    m_renderStage->renderInterface.virtualViewport << renderInterface.virtualViewport;
    m_renderStage->renderInterface.backgroundColor << renderInterface.backgroundColor;
    m_renderStage->renderInterface.frameCounter << renderInterface.frameCounter;
    m_renderStage->renderInterface.timeDelta << renderInterface.timeDelta;
    m_renderStage->renderInterface.targetFBO << renderInterface.targetFBO;
    m_renderStage->lightColorTypeData << m_lightAccumulationStage->colorTypeData;
    m_renderStage->lightPositionData << m_lightAccumulationStage->positionData;
    m_renderStage->lightAttenuationData << m_lightAccumulationStage->attenuationData;
    m_renderStage->glossiness << glossiness;
    m_renderStage->totalTime << m_timerStage->virtualTime;

    renderInterface.rendered << m_renderStage->renderInterface.rendered;
}

LightTestPipeline::~LightTestPipeline()
{
}

#include "RenderManager.hpp"

namespace nary {

ResourceManager::ResourceManager(RenderManager& renderManager)
: pRenderManager(&renderManager) {}

UID ResourceManager::loadModel(const std::string& filename) const {
    return pRenderManager->m_RenderResource->addMesh(naModel::createModelFromFile(*pRenderManager->m_Device, filename));
}

UID ResourceManager::loadImage(const std::string& filename) const {
    return pRenderManager->m_RenderResource->addTexture(std::make_unique<naImage>(naImage::loadImageFromFile(*pRenderManager->m_Device, filename)));
}

}
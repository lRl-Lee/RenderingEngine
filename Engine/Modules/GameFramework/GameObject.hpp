﻿#pragma once

#include "../Rendering/Vulkan/Model.hpp"
#include "../Rendering/Vulkan/SwapChain.hpp"
#include "../Rendering/Vulkan/Texture.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace RenderingEngine {

struct TransformComponent {
  glm::vec3 translation{};
  glm::vec3 scale{1.f, 1.f, 1.f};
  glm::vec3 rotation{};

  // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
  // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
  // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
  glm::mat4 mat4();

  glm::mat3 normalMatrix();
};

struct PointLightComponent {
  float lightIntensity = 1.0f;
};

struct GameObjectBufferData {
  glm::mat4 modelMatrix{1.f};
  glm::mat4 normalMatrix{1.f};
};

class GameObjectManager;  // forward declare game object manager class

class GameObject {
 public:
  using id_t = unsigned int;
  using Map = std::unordered_map<id_t, GameObject>;

  GameObject(GameObject &&) = default;
  GameObject(const GameObject &) = delete;
  GameObject &operator=(const GameObject &) = delete;
  GameObject &operator=(GameObject &&) = delete;

  id_t getId() { return id; }

  VkDescriptorBufferInfo getBufferInfo(int frameIndex);

  glm::vec3 color{};
  TransformComponent transform{};

  // Optional pointer components
  std::shared_ptr<LveModel> model{};

  // Rendering components
  // textures
  std::shared_ptr<LveTexture> diffuseMap = nullptr;
  std::shared_ptr<LveTexture> normalMap = nullptr;
  std::shared_ptr<LveTexture> metallicMap = nullptr;
  std::shared_ptr<LveTexture> roughnessMap = nullptr;

  std::shared_ptr<LveTexture> envMap = nullptr;

  std::unique_ptr<PointLightComponent> pointLight = nullptr;

 private:
  GameObject(id_t objId, const GameObjectManager &manager);

  id_t id;
  const GameObjectManager &gameObjectManger;

  friend class GameObjectManager;
};

class GameObjectManager {
 public:
  static constexpr int MAX_GAME_OBJECTS = 1000;

  GameObjectManager(LveDevice &device);
  GameObjectManager(const GameObjectManager &) = delete;
  GameObjectManager &operator=(const GameObjectManager &) = delete;
  GameObjectManager(GameObjectManager &&) = delete;
  GameObjectManager &operator=(GameObjectManager &&) = delete;

  GameObject &createGameObject() {
    assert(currentId < MAX_GAME_OBJECTS && "Max game object count exceeded!");
    auto gameObject = GameObject{currentId++, *this};
    auto gameObjectId = gameObject.getId();

    gameObject.diffuseMap = textureDefault;
    gameObject.normalMap = textureDefault;
    gameObject.metallicMap = textureDefault;
    gameObject.roughnessMap = textureDefault;
    gameObject.envMap = textureDefault;

    gameObjects.emplace(gameObjectId, std::move(gameObject));
    return gameObjects.at(gameObjectId);
  }

  GameObject &makePointLight(
      float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

  VkDescriptorBufferInfo getBufferInfoForGameObject(
      int frameIndex, GameObject::id_t gameObjectId) const {
    return uboBuffers[frameIndex]->descriptorInfoForIndex(gameObjectId);
  }

  void updateBuffer(int frameIndex);

  GameObject::Map gameObjects{};
  std::vector<std::unique_ptr<LveBuffer>> uboBuffers{LveSwapChain::MAX_FRAMES_IN_FLIGHT};

 private:
  GameObject::id_t currentId = 0;
  std::shared_ptr<LveTexture> textureDefault;
};


} 
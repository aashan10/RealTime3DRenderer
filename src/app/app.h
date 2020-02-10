#pragma once

#include "display.h"
#include "input.h"
#include "types.h"

namespace ecs {
class Coordinator;
}
namespace render_system {
class Camera;
class RenderSystem;
} // namespace render_system
namespace world_system {
class WorldSystem;
}
namespace app {
/**
 * Represents the renderer application, handels and manages WINDOW(Context), ECS
 * & UI
 */
class App : NonCopyable {
public:
  // lazy init instance
  static App &getInstance(int argc, char **argv) {
    static App instance(argc, argv);
    return instance;
  }
  void run();

private:
  App(int argc, char **argv);
  ~App();

  void processInput(float dt);

private:
  Display display;
  Input input;
  ecs::Coordinator &coordinator;
  render_system::RenderSystem *renderSystem;
  world_system::WorldSystem *worldSystem;
  render_system::Camera *camera;
};
} // namespace app

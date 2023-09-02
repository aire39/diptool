#include <iostream>
#include <string_view>

#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <spdlog/spdlog.h>

#include <CLI/CLI.hpp>

int main(int argc, char*argv[])
{
  constexpr std::string_view window_name = "DipTool";

  // setup command line arguments
  CLI::App app{"digital image processing tool"};

  uint32_t window_width = 800;
  app.add_option("--iw", window_width, "set window width");

  uint32_t window_height = 600;
  app.add_option("--ih", window_height, "set window height");

  CLI11_PARSE(app, argc, argv);

  // setup sfml window
  spdlog::info("initializing window...");

  sf::RenderWindow window(sf::VideoMode({window_width, window_height}), window_name.data());

  // setup imgui
  spdlog::info("initializing ui...");
  
  bool imgui_initialized = ImGui::SFML::Init(window);
  if (!imgui_initialized)
  {
    spdlog::error("failed to initialize ImGui!");
    return 0;
  }

  // application loop
  spdlog::info("starting application loop...");

  sf::Clock delta_clock;

  while (window.isOpen())
  {
    for (sf::Event event; window.pollEvent(event);)
    {
      ImGui::SFML::ProcessEvent(window, event);

      if (event.type == sf::Event::Closed)
      {
        window.close();
      }
    }

    ImGui::SFML::Update(window, delta_clock.restart());

    ImGui::Begin("Hello, world!");
    ImGui::Button("Press Button!");
    ImGui::End();

    window.clear();

    ImGui::SFML::Render(window);

    window.display();
  }

  ImGui::SFML::Shutdown();

  spdlog::info("application done!");

  return 0;
}

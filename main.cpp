#include <string_view>
#include <thread>

#include <imgui.h>
#include <imgui-SFML.h>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>

#include "CheckerBoardImage.h"
#include "Menu.h"
#include "DownsampleMenu.h"

#define USE_ON_RESIZING true

void RenderTask(sf::RenderWindow & window
               ,std::string & image_file_path
               ,sf::RectangleShape & bg_image_plane
               ,sf::Image & loaded_image
               ,sf::Texture & loaded_texture
               ,sf::Sprite & loaded_image_plane
               ,bool & app_is_running
               );

void onResizing(const sf::Event& event
               ,sf::RenderWindow & window
               ,sf::RectangleShape & bg_image_plane
               ,CheckerBoardImage& checker_image
               ,sf::Image& image
               ,sf::Texture& tex
               );

int main(int argc, char*argv[])
{
  constexpr std::string_view window_name = "DipTool";

  // setup command line arguments
  CLI::App app{"digital bg_image processing tool"};

  uint32_t window_width = 800;
  app.add_option("--iw", window_width, "set window width");

  uint32_t window_height = 600;
  app.add_option("--ih", window_height, "set window height");

  uint32_t bg_repeat_tiles = 32;
  app.add_option("--tr", bg_repeat_tiles, "set background checker tile repeat");

  std::string image_file_path;
  app.add_option("-i,--bg_image", image_file_path, "load bg_image to process");

  CLI11_PARSE(app, argc, argv)

  // setup logger
  constexpr uint32_t number_of_backtrace_logs = 32;
  spdlog::enable_backtrace(number_of_backtrace_logs);

  // load image file and checkerboard if image is not found
  CheckerBoardImage checker_image(window_width, window_height, bg_repeat_tiles);
  sf::Image loaded_image;

  constexpr uint32_t loaded_image_margin = 16;

  if (!image_file_path.empty())
  {
    spdlog::info("Trying to load: {}...", image_file_path);

    bool found_image = loaded_image.loadFromFile(image_file_path);
    if (!found_image)
    {
      spdlog::warn("Unable to find image file: {}", image_file_path);
    }
    else
    {
      // make the window twice as large of the original image and create pixels (margin) for some space
      // between the original and processed image
      window_width = (loaded_image.getSize().x * 2) + loaded_image_margin;
      window_height = loaded_image.getSize().y + loaded_image_margin;

      checker_image.Generate(window_width, window_height, bg_repeat_tiles);

      spdlog::info("loaded!");
    }
  }

  // setup sfml window
  spdlog::info("initializing window...");

  sf::RenderWindow window(sf::VideoMode({window_width, window_height})
                         ,window_name.data());

  // setup sprite
  sf::Image bg_image;
  bg_image.create(window_width, window_height, checker_image.GetImage().data());

  sf::Texture bg_texture;
  bg_texture.loadFromImage(bg_image);

  sf::RectangleShape bg_image_plane(sf::Vector2f((float)window_width, (float)window_height));
  bg_image_plane.setTexture(&bg_texture);

  sf::Texture loaded_texture;
  sf::Sprite loaded_image_plane;

  if (loaded_image.getSize() != sf::Vector2u(0.0f,0.0f))
  {
    spdlog::info("setup loaded image sprite...");
    loaded_texture.loadFromImage(loaded_image);
    loaded_image_plane.setTexture(loaded_texture);
    loaded_image_plane.setPosition(loaded_image_margin / 2.0f, loaded_image_margin / 2.0f);
  }

  // setup imgui
  spdlog::info("initializing ui...");
  
  bool imgui_initialized = ImGui::SFML::Init(window);
  if (!imgui_initialized)
  {
    spdlog::critical("failed to initialize ImGui!");
    abort();
  }

  // application loop
  spdlog::info("starting application loop...");

  bool app_is_running = true;
  std::thread render_thread(&RenderTask
                           ,std::ref(window)
                           ,std::ref(image_file_path)
                           ,std::ref(bg_image_plane)
                           ,std::ref(loaded_image)
                           ,std::ref(loaded_texture)
                           ,std::ref(loaded_image_plane)
                           ,std::ref(app_is_running)
                           );

  window.setActive(false);

  #ifdef USE_SFML_ONRESIZING_EVENT
  #if USE_ON_RESIZING
  bool is_on_size_set = window.setOnSize([&](const sf::Event& event) {
    onResizing(event, window, bg_image_plane, checker_image, bg_image, bg_texture);
  });

  if (is_on_size_set)
  {
    spdlog::info("custom window on resize (setOnSize) callback set!");
  }
  #endif
  #endif

  while (window.isOpen())
  {
    for (sf::Event event{}; window.pollEvent(event);)
    {
      ImGui::SFML::ProcessEvent(window, event);

      if (event.type == sf::Event::Closed)
      {
        // wait for thread to join since window.close() (sf::Window::close()) destroys the opengl context
        // leading to an annoying warning/error as the application closes
        app_is_running = false;
        if (render_thread.joinable())
        {
          render_thread.join();
        }

        window.setActive();
        window.close();
      }

      // catch the resize events
      if (event.type == sf::Event::Resized)
      {
        // update the view to the new size of the window
        onResizing(event, window, bg_image_plane, checker_image, bg_image, bg_texture);
      }
    }
  }

  ImGui::SFML::Shutdown();

  spdlog::info("application done!");

  return 0;
}

void RenderTask(sf::RenderWindow & window
               ,std::string & image_file_path
               ,sf::RectangleShape & bg_image_plane
               ,sf::Image & loaded_image
               ,sf::Texture & loaded_texture
               ,sf::Sprite & loaded_image_plane
               ,bool & app_is_running
               )
{
  window.setActive();
  sf::Clock delta_clock;

  DownsampleMenu downsample_menu;
  Menu menu;
  menu.SetImagePath(image_file_path);

  while(app_is_running)
  {
    ImGui::SFML::Update(window, delta_clock.restart());

    // GUI
    menu.RenderMenu(loaded_image, loaded_texture, loaded_image_plane);
    if(menu.IsDownSampleSet())
    {
      downsample_menu.RenderMenu();
    }

    // Render
    window.clear(sf::Color::Black);
    window.draw(bg_image_plane);

    if (loaded_image.getSize() != sf::Vector2u(0.0f,0.0f))
    {
      window.draw(loaded_image_plane);
    }

    ImGui::SFML::Render(window);

    window.display();
  }

  window.setActive(false);
}

void onResizing(const sf::Event& event
               ,sf::RenderWindow & window
               ,sf::RectangleShape & bg_image_plane
               ,CheckerBoardImage& checker_image
               ,sf::Image& image
               ,sf::Texture& tex
               )
{
  sf::FloatRect visibleArea(0.0f
                           ,0.0f
                           ,static_cast<float>(event.size.width)
                           ,static_cast<float>(event.size.height)
                           );

  window.setView(sf::View(visibleArea));

  checker_image.Generate(event.size.width, event.size.height, 32);
  image.create(event.size.width, event.size.height, checker_image.GetImage().data());
  tex.loadFromImage(image);

  bg_image_plane.setSize(sf::Vector2f(static_cast<float>(event.size.width)
                                     ,static_cast<float>(event.size.height)
  ));

  window.setSize(sf::Vector2u(event.size.width, event.size.height));
}

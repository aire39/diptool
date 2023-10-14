#include <string_view>
#include <thread>
#include <fstream>

#include <imgui.h>
#include <imgui-SFML.h>
#include <implot/implot.h>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>

#include "CheckerBoardImage.h"
#include "Menu.h"

#include "DownsampleMenu.h"
#include "operations/DownsampleOp.h"
#include "menus/UpsampleMenu.h"
#include "operations/UpsampleOp.h"
#include "menus/VaryBitsMenu.h"
#include "operations/VaryBitsOp.h"
#include "menus/HistogramEqualizationMenu.h"
#include "operations/HistogramEqualizationOp.h"
#include "menus/SpatialFilterMenu.h"
#include "operations/SpatialFilterOp.h"

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

  if (checker_image.IsImageGenerated())
  {
    bg_image.create(window_width, window_height, checker_image.GetImage().data());
  }
  else
  {
    bg_image.create(window_width, window_height, sf::Color::Black);
  }

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
  ImPlot::CreateContext();

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

  ImPlot::DestroyContext();
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

  DownsampleOp downsample_op;
  DownsampleMenu downsample_menu;
  int32_t current_downsample_level = -1;

  UpsampleOp upsample_op;
  UpsampleMenu upsample_menu;
  int32_t current_upsample_level = -1;

  VaryBitsOp varyingbits_op;
  VaryBitsMenu varyingbits_menu;
  int32_t current_bit_level = -1;

  HistogramEqualizationOp histogrameq_op;
  HistogramEqualizationMenu histogrameq_menu;

  SpatialFilterMenu spatial_filter_menu;
  SpatialFilterOp spatial_op;

  Menu menu;
  menu.SetImagePath(image_file_path);

  sf::Image processed_image;
  sf::Texture processed_texture;
  sf::Sprite processed_sprite;

  sf::Image processed_image_copy;
  sf::Texture processed_texture_copy;
  sf::Sprite processed_sprite_copy;

  while(app_is_running)
  {
    ImGui::SFML::Update(window, delta_clock.restart());

    // GUI and operations
    menu.RenderMenu(loaded_image, loaded_texture, loaded_image_plane);
    if(menu.IsDownSampleSet())
    {
      downsample_menu.RenderMenu();

      if (current_downsample_level != downsample_menu.DownsampleIterations())
      {
        current_downsample_level = downsample_menu.DownsampleIterations();

        std::vector<uint8_t> source_pixels (loaded_image.getPixelsPtr(), (loaded_image.getPixelsPtr()+(loaded_image.getSize().x * loaded_image.getSize().y * 4)));
        downsample_op.ProcessImage(downsample_menu.CurrentOperation(), source_pixels, loaded_image.getSize().x, loaded_image.getSize().y, 4, downsample_menu.DownsampleIterations());

        const auto & result_image = downsample_op.GetImage();
        processed_image.create(downsample_op.GetWidth(), downsample_op.GetHeight(), result_image.data());
        processed_texture.loadFromImage(processed_image);
        processed_sprite = sf::Sprite(processed_texture);

        if (menu.IsOutputAsSourceSet())
        {
          processed_image_copy.create(processed_image.getSize().x, processed_image.getSize().y, processed_image.getPixelsPtr());
          processed_texture_copy.loadFromImage(processed_image_copy);
          processed_sprite_copy.setTexture(processed_texture_copy);
        }
      }

      if (downsample_menu.ProcessBegin())
      {
        if (processed_image.saveToFile("output.png"))
        {
          spdlog::info("output buffer was written!");
        }
        else
        {
          spdlog::warn("output buffer was empty!");
        }
      }
    }

    if (menu.IsUpSampleSet())
    {
      upsample_menu.RenderMenu();

      if (current_upsample_level != upsample_menu.UpsampleIterations())
      {
        current_upsample_level = upsample_menu.UpsampleIterations();

        std::vector<uint8_t> source_pixels;

        if (menu.IsOutputAsSourceSet())
        {
          source_pixels = std::vector<uint8_t>(processed_image_copy.getPixelsPtr(), (processed_image_copy.getPixelsPtr() + (processed_image_copy.getSize().x * processed_image_copy.getSize().y * 4)));
          upsample_op.ProcessImage(upsample_menu.CurrentOperation(), source_pixels, processed_image_copy.getSize().x, processed_image_copy.getSize().y, 4, upsample_menu.UpsampleIterations());
        }
        else
        {
          source_pixels = std::vector<uint8_t>(loaded_image.getPixelsPtr(), (loaded_image.getPixelsPtr() + (loaded_image.getSize().x * loaded_image.getSize().y * 4)));
          upsample_op.ProcessImage(upsample_menu.CurrentOperation(), source_pixels, loaded_image.getSize().x, loaded_image.getSize().y, 4, upsample_menu.UpsampleIterations());
        }

        const auto & result_image = upsample_op.GetImage();
        processed_image.create(upsample_op.GetWidth(), upsample_op.GetHeight(), result_image.data());
        processed_texture.loadFromImage(processed_image);
        processed_sprite = sf::Sprite(processed_texture);
      }

      if (upsample_menu.ProcessBegin())
      {
        if (processed_image.saveToFile("output.png"))
        {
          spdlog::info("output buffer was written!");
        }
        else
        {
          spdlog::warn("output buffer was empty!");
        }
      }
    }

    if (menu.IsVaryingBitsSet())
    {
      varyingbits_menu.RenderMenu();

      if ((current_bit_level != varyingbits_menu.BitScale()))
      {
        current_bit_level = varyingbits_menu.BitScale();

        std::vector<uint8_t> source_pixels (loaded_image.getPixelsPtr(), (loaded_image.getPixelsPtr()+(loaded_image.getSize().x * loaded_image.getSize().y * 4)));
        varyingbits_op.ProcessImage(varyingbits_menu.BitScale(), varyingbits_menu.ShiftBitsForContrast(), source_pixels, loaded_image.getSize().x, loaded_image.getSize().y, 4);

        const auto & result_image = varyingbits_op.GetImage();
        processed_image.create(varyingbits_op.GetWidth(), varyingbits_op.GetHeight(), result_image.data());
        processed_texture.loadFromImage(processed_image);
        processed_sprite = sf::Sprite(processed_texture);
      }

      if (varyingbits_menu.ProcessBegin())
      {
        if (processed_image.saveToFile("output.png"))
        {
          spdlog::info("output buffer was written!");
        }
        else
        {
          spdlog::warn("output buffer was empty!");
        }
      }
    }

    if (menu.IsHistogramEqualizationSet())
    {
      histogrameq_menu.RenderMenu();
      histogrameq_menu.SetSizeOfImage(static_cast<int32_t>(loaded_image.getSize().x), static_cast<int32_t>(loaded_image.getSize().y));

      if (histogrameq_menu.ProcessBegin())
      {
        histogrameq_menu.ClearData();

        if (histogrameq_menu.IsHistogramColorTypeGray())
        {
          histogrameq_op.SetHistogramColorType(MenuOp_HistogramColor::GRAY);
        }

        if (histogrameq_menu.IsHistogramColorTypeRGBA())
        {
          histogrameq_op.SetHistogramColorType(MenuOp_HistogramColor::RGBA);
        }

        std::vector<uint8_t> source_pixels (loaded_image.getPixelsPtr(), (loaded_image.getPixelsPtr()+(loaded_image.getSize().x * loaded_image.getSize().y * 4)));

        std::chrono::high_resolution_clock::time_point process_time_begin;
        std::chrono::high_resolution_clock::time_point process_time_end;

        if (histogrameq_menu.IsGlobalMethodType())
        {
          process_time_begin = std::chrono::high_resolution_clock::now();
          histogrameq_op.ProcessImage(MenuOp_HistogramMethod::GLOBAL
                                     ,source_pixels
                                     ,loaded_image.getSize().x
                                     ,loaded_image.getSize().y
                                     ,4
                                     ,0);
          process_time_end = std::chrono::high_resolution_clock::now();
        }

        if (histogrameq_menu.IsLocalizeMethodType())
        {
          histogrameq_op.SetLocalizeKernelSize(histogrameq_menu.GetKernelX(), histogrameq_menu.GetKernelY());
          process_time_begin = std::chrono::high_resolution_clock::now();
          histogrameq_op.ProcessImage(MenuOp_HistogramMethod::LOCALIZE
                                     ,source_pixels
                                     ,loaded_image.getSize().x
                                     ,loaded_image.getSize().y
                                     ,4
                                     ,0);
          process_time_end = std::chrono::high_resolution_clock::now();
        }

        if (histogrameq_menu.IsLocalizeEnchancementMethodType())
        {
          histogrameq_op.SetLocalizeKernelSize(histogrameq_menu.GetKernelX(), histogrameq_menu.GetKernelY());
          histogrameq_op.SetLocalizeKernelConstants(histogrameq_menu.GetKernelK0(), histogrameq_menu.GetKernelK1(), histogrameq_menu.GetKernelK2(), histogrameq_menu.GetKernelK3(), histogrameq_menu.GetKernelEnhanceConst());
          process_time_begin = std::chrono::high_resolution_clock::now();
          histogrameq_op.ProcessImage(MenuOp_HistogramMethod::LOCALIZE_ENCHANCEMENT
                                     ,source_pixels
                                     ,loaded_image.getSize().x
                                     ,loaded_image.getSize().y
                                     ,4
                                     ,0);
          process_time_end = std::chrono::high_resolution_clock::now();
        }

        float process_time_secs = static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(process_time_end - process_time_begin).count()) / 1e6f;
        histogrameq_menu.SetProcessTime(process_time_secs);

        std::vector<std::map<int32_t, float>> histograms_source;
        std::vector<std::map<int32_t, float>> histograms_remap;

        if (histogrameq_op.HistogramColorType() == MenuOp_HistogramColor::GRAY)
        {
          histograms_source.emplace_back(histogrameq_op.GetHistogram());
          histograms_remap.emplace_back(histogrameq_op.GetHistogramRemap());
        }
        else // MenuOp_HistogramColor::RGBA
        {
          histograms_source.emplace_back(histogrameq_op.GetHistogramRed());
          histograms_remap.emplace_back(histogrameq_op.GetHistogramRemapRed());
          histograms_source.emplace_back(histogrameq_op.GetHistogramGreen());
          histograms_remap.emplace_back(histogrameq_op.GetHistogramRemapGreen());
          histograms_source.emplace_back(histogrameq_op.GetHistogramBlue());
          histograms_remap.emplace_back(histogrameq_op.GetHistogramRemapBlue());
        }

        histogrameq_menu.SetHistogramData(histograms_source);
        histogrameq_menu.SetHistogramRemapData(histograms_remap);

        const auto & result_image = histogrameq_op.GetImage();
        processed_image.create(histogrameq_op.GetWidth(), histogrameq_op.GetHeight(), result_image.data());
        processed_texture.loadFromImage(processed_image);
        processed_sprite = sf::Sprite(processed_texture);
      }
    }

    if (menu.IsSpatialFiltering())
    {
      spatial_filter_menu.RenderMenu();

      if (spatial_filter_menu.ProcessBegin())
      {
        std::vector<uint8_t> source_pixels (loaded_image.getPixelsPtr(), (loaded_image.getPixelsPtr()+(loaded_image.getSize().x * loaded_image.getSize().y * 4)));

        spatial_op.SetKernelSize(spatial_filter_menu.GetKernelX(), spatial_filter_menu.GetKernelY());

        if (spatial_filter_menu.CurrentOperation() == MenuOp_SpatialFilter::SHARPENING)
        {
          spatial_op.SetSharpenConstant(spatial_filter_menu.GetSharpenConstant());
          spatial_op.SetSharpenUseFullKernel(spatial_filter_menu.IsSharpenFullUse());
          spatial_op.ShowSharpenFilter(spatial_filter_menu.ShowSharpenFilter());
          spatial_op.ShowSharpenFilterScaling(spatial_filter_menu.ShowSharpenFilterScaling());
        }

        spatial_op.ProcessImage(spatial_filter_menu.CurrentOperation()
                               ,source_pixels
                               ,loaded_image.getSize().x
                               ,loaded_image.getSize().y
                               ,4
                               ,0);

        const auto & result_image = spatial_op.GetImage();
        processed_image.create(spatial_op.GetWidth(), spatial_op.GetHeight(), result_image.data());
        processed_texture.loadFromImage(processed_image);
        processed_sprite = sf::Sprite(processed_texture);
      }
    }

    // Render
    window.clear(sf::Color::Black);
    window.draw(bg_image_plane);

    if (loaded_image.getSize() != sf::Vector2u(0,0))
    {
      window.draw(loaded_image_plane);
    }

    if (processed_image.getSize() != sf::Vector2u(0, 0))
    {
      float shift_x = static_cast<float>(loaded_image.getSize().x - processed_image.getSize().x) / 2.0f;
      float shift_y = static_cast<float>(loaded_image.getSize().y - processed_image.getSize().y) / 2.0f;
      float output_position = static_cast<float>(window.getSize().x) / 2.0f;

      processed_sprite.setPosition(output_position + shift_x + 8.0f, shift_y + 8.0f);
      window.draw(processed_sprite);
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

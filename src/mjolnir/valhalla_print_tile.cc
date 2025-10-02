#include "argparse_utils.h"
#include "baldr/graphreader.h"
#include "baldr/graphtile.h"
#include "baldr/directededge.h"
#include "baldr/nodeinfo.h"
#include "midgard/logging.h"

#include <boost/property_tree/ptree.hpp>
#include <cxxopts.hpp>

#include <bitset>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

using namespace valhalla::baldr;
using namespace valhalla::midgard;

void print_tile_info(const GraphTile* tile) {
  if (!tile) {
    std::cout << "Tile not found or empty" << std::endl;
    return;
  }

  const auto* header = tile->header();
  std::cout << "=== Tile Header ===" << std::endl;
  std::cout << "GraphId: 0x" << std::hex << tile->id().value << std::dec << std::endl;
  std::cout << "Version: " << header->version() << std::endl;
  std::cout << "Date: " << header->date_created() << std::endl;
  std::cout << "Node count: " << header->nodecount() << std::endl;
  std::cout << "Edge count: " << header->directededgecount() << std::endl;
  std::cout << "End offset: " << header->end_offset() << std::endl;
  std::cout << std::endl;

  std::cout << "=== Nodes ===" << std::endl;
  for (uint32_t i = 0; i < header->nodecount(); i++) {
    const auto* node = tile->node(i);
    std::cout << "Node " << i << ":" << std::endl;
    std::cout << "  Edge index: " << node->edge_index() << std::endl;
    std::cout << "  Edge count: " << node->edge_count() << std::endl;
    std::cout << "  Admin index: " << node->admin_index() << std::endl;
    std::cout << "  Timezone: " << node->timezone() << std::endl;
    auto ll = node->latlng(header->base_ll());
    std::cout << "  LL: (" << ll.lat() << ", " << ll.lng() << ")" << std::endl;
    std::cout << std::endl;
  }

  std::cout << "=== Directed Edges ===" << std::endl;
  for (uint32_t i = 0; i < header->directededgecount(); i++) {
    const auto* edge = tile->directededge(i);
    std::cout << "Edge " << i << ":" << std::endl;

    // Print raw 64-bit words for debugging
    const auto* words = reinterpret_cast<const uint64_t*>(edge);
    std::cout << "  Raw words: ";
    for (int w = 0; w < 6; w++) {
      std::cout << "0x" << std::hex << std::setw(16) << std::setfill('0') << words[w] << " ";
    }
    std::cout << std::dec << std::endl;

    std::cout << edge->debug_string() << std::endl;
  }
}

int main(int argc, char** argv) {
  const auto program = std::filesystem::path(__FILE__).stem().string();

  try {
    cxxopts::Options options(
        program,
        program + " " + VALHALLA_PRINT_VERSION + "\n\n"
        "Print structured information about a Valhalla tile\n");

    options.add_options()
      ("h,help", "Print this help message.")
      ("v,version", "Print the version of this software.")
      ("c,config", "Path to the configuration file", cxxopts::value<std::string>())
      ("t,tile", "Tile ID (in GraphId format, e.g., 0x1187D218 or decimal)", cxxopts::value<std::string>())
      ("f,file", "Direct path to tile file (.gph)", cxxopts::value<std::string>());

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
      std::cout << options.help() << std::endl;
      return EXIT_SUCCESS;
    }

    if (result.count("version")) {
      std::cout << program << " " << VALHALLA_PRINT_VERSION << std::endl;
      return EXIT_SUCCESS;
    }

    boost::property_tree::ptree config;

    if (result.count("file")) {
      // Read tile directly from file
      std::string tile_file = result["file"].as<std::string>();
      // For direct file reading, use a dummy GraphId and load the file
      std::ifstream ifs(tile_file, std::ios::binary);
      if (!ifs) {
        throw std::runtime_error("Cannot open file: " + tile_file);
      }
      ifs.seekg(0, std::ios::end);
      size_t size = ifs.tellg();
      ifs.seekg(0, std::ios::beg);
      std::vector<char> data(size);
      ifs.read(data.data(), size);
      auto tile = GraphTile::Create(GraphId(), std::move(data));
      print_tile_info(tile.get());
    } else if (result.count("config") && result.count("tile")) {
      // Read tile via GraphReader
      if (!parse_common_args(program, options, result, &config, "mjolnir.logging"))
        return EXIT_SUCCESS;

      std::string tile_str = result["tile"].as<std::string>();
      uint64_t tile_id_val;
      if (tile_str.substr(0, 2) == "0x" || tile_str.substr(0, 2) == "0X") {
        tile_id_val = std::stoull(tile_str, nullptr, 16);
      } else {
        tile_id_val = std::stoull(tile_str);
      }

      GraphId tile_id(tile_id_val);
      GraphReader reader(config.get_child("mjolnir"));
      auto tile = reader.GetGraphTile(tile_id);
      print_tile_info(tile.get());
    } else {
      std::cerr << "Either --file or both --config and --tile must be specified\n\n";
      std::cout << options.help() << std::endl;
      return EXIT_FAILURE;
    }

  } catch (cxxopts::exceptions::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

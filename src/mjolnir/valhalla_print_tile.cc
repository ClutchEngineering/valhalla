#include "argparse_utils.h"
#include "baldr/graphreader.h"
#include "baldr/graphtile.h"
#include "baldr/directededge.h"
#include "baldr/nodeinfo.h"
#include "baldr/nodetransition.h"
#include "baldr/accessrestriction.h"
#include "baldr/admin.h"
#include "baldr/sign.h"
#include "baldr/turnlanes.h"
#include "baldr/laneconnectivity.h"
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
  std::cout << header->debug_string() << std::endl;
  std::cout << std::endl;

   // Print nodes
   std::cout << "=== Nodes (" << header->nodecount() << ") ===" << std::endl;
   for (uint32_t i = 0; i < header->nodecount(); i++) {
     const auto* node = tile->node(i);
     std::cout << "\nNode " << i << ":" << std::endl;
     std::cout << node->debug_string() << std::endl;
   }
   std::cout << std::endl;

   // Print directed edges with raw words and edgeinfo
   std::cout << "=== Directed Edges (" << header->directededgecount() << ") ===" << std::endl;
   for (uint32_t i = 0; i < header->directededgecount(); i++) {
     const auto* edge = tile->directededge(i);
     std::cout << "\nEdge " << i << ":" << std::endl;

     // Print raw 64-bit words for debugging
     const auto* words = reinterpret_cast<const uint64_t*>(edge);
     std::cout << "  Raw words: ";
     for (int w = 0; w < 6; w++) {
       std::cout << "0x" << std::hex << std::setw(16) << std::setfill('0') << words[w] << " ";
     }
     std::cout << std::dec << std::endl;

     std::cout << edge->debug_string() << std::endl;

     // Print edgeinfo
     auto edgeinfo = tile->edgeinfo(edge);
     std::cout << "  EdgeInfo:" << std::endl;
     std::cout << "    wayid: " << edgeinfo.wayid() << std::endl;
     std::cout << "    name_count: " << edgeinfo.name_count() << std::endl;
     auto names = edgeinfo.GetNames();
     for (size_t n = 0; n < names.size(); n++) {
       std::cout << "    name[" << n << "]: " << names[n] << std::endl;
     }
     std::cout << "    encoded_shape_size: " << edgeinfo.encoded_shape_size() << std::endl;
     auto encoded_shape = edgeinfo.encoded_shape();
     std::cout << "    encoded_shape_hex: ";
     for (char c : encoded_shape) {
       std::cout << std::hex << std::setw(2) << std::setfill('0') << (static_cast<unsigned int>(static_cast<unsigned char>(c))) << " ";
     }
     std::cout << std::dec << std::endl;
     std::cout << "    mean_elevation: " << edgeinfo.mean_elevation() << std::endl;
     std::cout << "    speed_limit: " << edgeinfo.speed_limit() << std::endl;
   }
   std::cout << std::endl;

  // Print directed edge extensions (disabled - crashes)
  // if (header->has_ext_directededge()) {
  //   std::cout << "=== Directed Edge Extensions ===" << std::endl;
  // }

  // Print node transitions
  if (header->transitioncount() > 0) {
    std::cout << "=== Node Transitions (" << header->transitioncount() << ") ===" << std::endl;
    for (uint32_t i = 0; i < header->transitioncount(); i++) {
      const auto* trans = tile->transition(i);
      std::cout << "\nTransition " << i << ":" << std::endl;
      std::cout << trans->debug_string() << std::endl;
    }
    std::cout << std::endl;
  }

  // Print access restrictions
  if (header->access_restriction_count() > 0) {
    std::cout << "=== Access Restrictions (" << header->access_restriction_count() << ") ===" << std::endl;
    for (uint32_t i = 0; i < header->directededgecount(); i++) {
      auto restrictions = tile->GetAccessRestrictions(i, kAllAccess);
      if (!restrictions.empty()) {
        std::cout << "\nEdge " << i << " restrictions:" << std::endl;
        for (const auto& r : restrictions) {
          std::cout << "  type: " << static_cast<int>(r.type()) << ", value: " << r.value() << ", modes: " << r.modes() << std::endl;
        }
      }
    }
    std::cout << std::endl;
  }

  // Print admins
  if (header->admincount() > 0) {
    std::cout << "=== Admins (" << header->admincount() << ") ===" << std::endl;
    for (size_t i = 0; i < header->admincount(); i++) {
      const auto* admin = tile->admin(i);
      if (admin) {
        std::cout << "\nAdmin " << i << ":" << std::endl;
        std::cout << admin->debug_string() << std::endl;
      }
    }
    std::cout << std::endl;
  }

  // Print signs
  if (header->signcount() > 0) {
    std::cout << "=== Signs (" << header->signcount() << ") ===" << std::endl;
    for (uint32_t i = 0; i < header->directededgecount(); i++) {
      const auto* edge = tile->directededge(i);
      if (edge->sign()) {
        auto signs = tile->GetSigns(i);
        if (!signs.empty()) {
          std::cout << "\nEdge " << i << " signs:" << std::endl;
          for (const auto& s : signs) {
            std::cout << "  type: " << static_cast<int>(s.type()) << ", text: " << s.text() << std::endl;
          }
        }
      }
    }
    std::cout << std::endl;
  }

  // Print turn lanes
  if (header->turnlane_count() > 0) {
    std::cout << "=== Turn Lanes (" << header->turnlane_count() << ") ===" << std::endl;
    for (uint32_t i = 0; i < header->directededgecount(); i++) {
      auto lanes = tile->turnlanes(i);
      if (!lanes.empty()) {
        std::cout << "\nEdge " << i << " turn lanes: ";
        for (auto mask : lanes) {
          std::cout << mask << " ";
        }
        std::cout << std::endl;
      }
    }
    std::cout << std::endl;
  }

  // Print signs
  if (header->signcount() > 0) {
    std::cout << "=== Signs (" << header->signcount() << ") ===" << std::endl;
    std::cout << "Sign data present" << std::endl;
    std::cout << std::endl;
  }

  // Print complex restrictions (use offsets as indicators)
  if (header->complex_restriction_forward_offset() > 0 || header->complex_restriction_reverse_offset() > 0) {
    std::cout << "=== Complex Restrictions ===" << std::endl;
    std::cout << "Forward offset: " << header->complex_restriction_forward_offset() << std::endl;
    std::cout << "Reverse offset: " << header->complex_restriction_reverse_offset() << std::endl;
    std::cout << std::endl;
  }

  // Print lane connectivity
  if (header->lane_connectivity_offset() > 0) {
    std::cout << "=== Lane Connectivity ===" << std::endl;
    for (uint32_t i = 0; i < header->directededgecount(); i++) {
      auto connectivity = tile->GetLaneConnectivity(i);
      if (!connectivity.empty()) {
        std::cout << "\nEdge " << i << " lane connectivity:" << std::endl;
        for (const auto& lc : connectivity) {
          std::cout << "  from: " << lc.from() << ", to: " << lc.to() << ", to_lanes: " << lc.to_lanes() << ", from_lanes: " << lc.from_lanes() << std::endl;
        }
      }
    }
    std::cout << std::endl;
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

#include "baldr/graphtileheader.h"
#include "config.h"

#include <algorithm>
#include <sstream>
#include <string>

using namespace valhalla::baldr;

namespace valhalla {
namespace baldr {

// Default constructor.
GraphTileHeader::GraphTileHeader()
    : // initialization of bitfields done here in c++20 can be done in the class definition
      graphid_(0), density_(0), name_quality_(0), speed_quality_(0), exit_quality_(0),
      has_elevation_(0), has_ext_directededge_(0), nodecount_(0), directededgecount_(0),
      predictedspeeds_count_(0), spare1_(0), transitioncount_(0), spare3_(0), turnlane_count_(0),
      spare4_(0), transfercount_(0), spare2_(0), departurecount_(0), stopcount_(0), spare5_(0),
      routecount_(0), schedulecount_(0), signcount_(0), spare6_(0), access_restriction_count_(0),
      admincount_(0), spare7_(0) {
  set_version(PACKAGE_VERSION);
}

// Set the version string.
void GraphTileHeader::set_version(const std::string& version) {
  // reinitializing the version array before copying
  version_ = {};
  std::copy(version.begin(), version.begin() + std::min(kMaxVersionSize, version.size()),
            version_.data());
  version_[kMaxVersionSize - 1] = 0;
}

// Sets the number of transit departures in this tile.
void GraphTileHeader::set_departurecount(const uint32_t departures) {
  // Check against limit
  if (departures > kMaxTransitDepartures) {
    throw std::runtime_error("Exceeding maximum number of transit departures per tile");
  }
  departurecount_ = departures;
}

// Sets the number of transit stops in this tile.
void GraphTileHeader::set_stopcount(const uint32_t stops) {
  // Check against limit
  if (stops > kMaxTransitStops) {
    throw std::runtime_error("Exceeding maximum number of transit stops per tile");
  }
  stopcount_ = stops;
}

// Sets the number of transit routes in this tile.
void GraphTileHeader::set_routecount(const uint32_t routes) {
  // Check against limit
  if (routes > kMaxTransitRoutes) {
    throw std::runtime_error("Exceeding maximum number of transit routes per tile");
  }
  routecount_ = routes;
}

// Sets the number of transit schedules in this tile.
void GraphTileHeader::set_schedulecount(const uint32_t schedules) {
  // Check against limit
  if (schedules > kMaxTransitSchedules) {
    throw std::runtime_error("Exceeding maximum number of transit schedule entries per tile");
  }
  schedulecount_ = schedules;
}

// Sets the number of transit transfers in this tile.
void GraphTileHeader::set_transfercount(const uint32_t transfers) {
  // Check against limit
  if (transfers > kMaxTransfers) {
    throw std::runtime_error("Exceeding maximum number of transit transfer entries per tile");
  }
  transfercount_ = transfers;
}

// Sets the edge bin offsets
void GraphTileHeader::set_edge_bin_offsets(const uint32_t (&offsets)[kBinCount]) {
  std::copy(std::begin(offsets), std::end(offsets), bin_offsets_.data());
}

// Get the offsets to the given bin in the 5x5 grid.
std::pair<uint32_t, uint32_t> GraphTileHeader::bin_offset(size_t index) const {
  if (index < kBinCount) {
    return std::make_pair(index == 0 ? 0 : bin_offsets_[index - 1], bin_offsets_[index]);
  }
  throw std::runtime_error("Bin out of bounds");
}

std::string GraphTileHeader::debug_string() const {
  std::ostringstream os;
  os.setf(std::ios::boolalpha);

  // Bitfields (graphid/quality/flags)
  os << "graphid_: " << static_cast<unsigned long long>(graphid_) << "\n";
  os << "density_: " << static_cast<unsigned long long>(density_) << "\n";
  os << "name_quality_: " << static_cast<unsigned long long>(name_quality_) << "\n";
  os << "speed_quality_: " << static_cast<unsigned long long>(speed_quality_) << "\n";
  os << "exit_quality_: " << static_cast<unsigned long long>(exit_quality_) << "\n";
  os << "has_elevation_: " << static_cast<bool>(has_elevation_) << "\n";
  os << "has_ext_directededge_: " << static_cast<bool>(has_ext_directededge_) << "\n";

  // Base LL
  os << "base_ll_.first (lng): " << base_ll_.first << "\n";
  os << "base_ll_.second (lat): " << base_ll_.second << "\n";

  // Version and dataset id
  os << "version_: " << version_.data() << "\n";
  os << "dataset_id_: " << static_cast<unsigned long long>(dataset_id_) << "\n";

  // Record counts (bitfields)
  os << "nodecount_: " << static_cast<unsigned long long>(nodecount_) << "\n";
  os << "directededgecount_: " << static_cast<unsigned long long>(directededgecount_) << "\n";
  os << "predictedspeeds_count_: " << static_cast<unsigned long long>(predictedspeeds_count_) << "\n";
  os << "spare1_: " << static_cast<unsigned long long>(spare1_) << "\n";

  // Mixed-width counts & spares
  os << "transitioncount_: " << static_cast<unsigned long long>(transitioncount_) << "\n";
  os << "spare3_: " << static_cast<unsigned long long>(spare3_) << "\n";
  os << "turnlane_count_: " << static_cast<unsigned long long>(turnlane_count_) << "\n";
  os << "spare4_: " << static_cast<unsigned long long>(spare4_) << "\n";
  os << "transfercount_: " << static_cast<unsigned long long>(transfercount_) << "\n";
  os << "spare2_: " << static_cast<unsigned long long>(spare2_) << "\n";

  // Transit counts
  os << "departurecount_: " << static_cast<unsigned long long>(departurecount_) << "\n";
  os << "stopcount_: " << static_cast<unsigned long long>(stopcount_) << "\n";
  os << "spare5_: " << static_cast<unsigned long long>(spare5_) << "\n";
  os << "routecount_: " << static_cast<unsigned long long>(routecount_) << "\n";
  os << "schedulecount_: " << static_cast<unsigned long long>(schedulecount_) << "\n";

  // More counts
  os << "signcount_: " << static_cast<unsigned long long>(signcount_) << "\n";
  os << "spare6_: " << static_cast<unsigned long long>(spare6_) << "\n";
  os << "access_restriction_count_: " << static_cast<unsigned long long>(access_restriction_count_) << "\n";
  os << "admincount_: " << static_cast<unsigned long long>(admincount_) << "\n";
  os << "spare7_: " << static_cast<unsigned long long>(spare7_) << "\n";

  // Spare words
  os << "spareword0_: " << static_cast<unsigned long long>(spareword0_) << "\n";
  os << "spareword1_: " << static_cast<unsigned long long>(spareword1_) << "\n";

  // Offsets (variable-size sections)
  os << "complex_restriction_forward_offset_: " << complex_restriction_forward_offset_ << "\n";
  os << "complex_restriction_reverse_offset_: " << complex_restriction_reverse_offset_ << "\n";
  os << "edgeinfo_offset_: " << edgeinfo_offset_ << "\n";
  os << "textlist_offset_: " << textlist_offset_ << "\n";

  // Date created
  os << "date_created_: " << date_created_ << "\n";

  // Bin offsets
  os << "bin_offsets_: [";
  for (size_t i = 0; i < bin_offsets_.size(); ++i) {
    os << bin_offsets_[i];
    if (i + 1 < bin_offsets_.size()) os << ", ";
  }
  os << "]\n";

  // Lane connectivity and predicted speeds offsets
  os << "lane_connectivity_offset_: " << lane_connectivity_offset_ << "\n";
  os << "predictedspeeds_offset_: " << predictedspeeds_offset_ << "\n";

  // Tile size
  os << "tile_size_: " << tile_size_ << "\n";

  // Empty slots
  os << "empty_slots_: [";
  for (size_t i = 0; i < empty_slots_.size(); ++i) {
    os << empty_slots_[i];
    if (i + 1 < empty_slots_.size()) os << ", ";
  }
  os << "]\n";

  return os.str();
}

} // namespace baldr
} // namespace valhalla

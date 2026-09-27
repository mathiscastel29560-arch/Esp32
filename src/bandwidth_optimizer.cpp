#include "bandwidth_optimizer.h"
#include "async_logger.h"

std::vector<uint8_t> BandwidthOptimizer::compressPayload(const uint8_t* data, uint16_t length, CompressionMethod method) {
  if (!data || length == 0) return {};

  originalSize = length;
  std::vector<uint8_t> compressed;

  switch (method) {
    case CompressionMethod::RLE:
      compressed = rleCompress(data, length);
      break;
    case CompressionMethod::DEFLATE:
    case CompressionMethod::HUFFMAN:
      // Placeholder for advanced compression
      compressed.assign(data, data + length);
      break;
    case CompressionMethod::NONE:
      compressed.assign(data, data + length);
      break;
  }

  bytesCompressed = compressed.size();
  compressionRatio = (float)compressed.size() / length;

  LOG_I("Compressed %u bytes to %u bytes (ratio: %.2f)", length, compressed.size(), compressionRatio);
  return compressed;
}

std::vector<uint8_t> BandwidthOptimizer::decompressPayload(const std::vector<uint8_t>& compressed, CompressionMethod method) {
  switch (method) {
    case CompressionMethod::RLE:
      return rleDecompress(compressed);
    default:
      return compressed;
  }
}

std::vector<uint8_t> BandwidthOptimizer::rleCompress(const uint8_t* data, uint16_t length) {
  std::vector<uint8_t> compressed;
  compressed.reserve(length);

  uint16_t i = 0;
  while (i < length) {
    uint8_t current = data[i];
    uint8_t count = 1;

    while (i + count < length && data[i + count] == current && count < 255) {
      count++;
    }

    // Store: count, value
    if (count > 2 || current == 255) {
      compressed.push_back(255);  // Marker
      compressed.push_back(count);
      compressed.push_back(current);
    } else {
      for (uint8_t j = 0; j < count; j++) {
        compressed.push_back(current);
      }
    }

    i += count;
  }

  return compressed;
}

std::vector<uint8_t> BandwidthOptimizer::rleDecompress(const std::vector<uint8_t>& data) {
  std::vector<uint8_t> decompressed;
  decompressed.reserve(data.size() * 2);

  for (size_t i = 0; i < data.size(); i++) {
    if (data[i] == 255 && i + 2 < data.size()) {
      uint8_t count = data[++i];
      uint8_t value = data[++i];
      for (uint8_t j = 0; j < count; j++) {
        decompressed.push_back(value);
      }
    } else {
      decompressed.push_back(data[i]);
    }
  }

  return decompressed;
}

uint32_t BandwidthOptimizer::getTotalBytesSaved() const {
  if (originalSize == 0) return 0;
  return originalSize - bytesCompressed;
}

uint8_t BandwidthOptimizer::selectOptimalChannel(const uint8_t* rssiMap, uint8_t mapSize) {
  uint8_t bestChannel = 0;
  uint8_t bestRSSI = 0;

  for (uint8_t i = 0; i < mapSize; i++) {
    if (rssiMap[i] > bestRSSI) {
      bestRSSI = rssiMap[i];
      bestChannel = i + 1;  // Channels start at 1
    }
  }

  LOG_I("Optimal channel selected: %u (RSSI: %u)", bestChannel, bestRSSI);
  return bestChannel;
}

uint8_t BandwidthOptimizer::selectLowNoiseChannel() {
  LOG_I("Selecting low noise channel");
  return 6;  // Placeholder
}

void BandwidthOptimizer::printCompressionStats() {
  Serial.printf("\n=== COMPRESSION STATISTICS ===\n");
  Serial.printf("Compression Ratio: %.2f%%\n", compressionRatio * 100);
  Serial.printf("Bytes Compressed: %u\n", bytesCompressed);
  Serial.printf("Original Size: %u\n", originalSize);
  Serial.printf("Bytes Saved: %u\n", getTotalBytesSaved());
  Serial.printf("Total Savings: %u KB\n", getTotalBytesSaved() / 1024);
  Serial.println("===============================\n");
}

void BandwidthOptimizer::printChannelAnalysis() {
  Serial.printf("\n=== CHANNEL ANALYSIS ===\n");
  Serial.println("Channel efficiency analysis available");
  Serial.println("========================\n");
}

void ChannelEfficiency::recordChannelPerformance(uint8_t channel, uint16_t packetsTransmitted,
                                                 uint16_t packetsLost, uint32_t latency) {
  for (auto& metric : channelMetrics) {
    if (metric.channel == channel) {
      metric.packetsTransmitted += packetsTransmitted;
      metric.packetsLost += packetsLost;
      metric.totalLatency += latency;
      metric.measurements++;
      return;
    }
  }

  ChannelMetric newMetric = {channel, packetsTransmitted, packetsLost, latency, 1};
  channelMetrics.push_back(newMetric);

  LOG_I("Channel %u: packets=%u, lost=%u, latency=%ums",
    channel, packetsTransmitted, packetsLost, latency);
}

uint8_t ChannelEfficiency::getEfficiencyScore(uint8_t channel) const {
  for (const auto& metric : channelMetrics) {
    if (metric.channel == channel) {
      float lossRate = (float)metric.packetsLost / metric.packetsTransmitted;
      uint8_t score = 100 - (lossRate * 100);
      return (score > 100) ? 100 : score;
    }
  }
  return 0;
}

uint8_t ChannelEfficiency::selectBestChannel() {
  if (channelMetrics.empty()) return 1;

  uint8_t bestChannel = channelMetrics[0].channel;
  uint8_t bestScore = getEfficiencyScore(bestChannel);

  for (const auto& metric : channelMetrics) {
    uint8_t score = getEfficiencyScore(metric.channel);
    if (score > bestScore) {
      bestScore = score;
      bestChannel = metric.channel;
    }
  }

  return bestChannel;
}

void ChannelEfficiency::printChannelStats() {
  Serial.printf("\n=== CHANNEL EFFICIENCY STATS ===\n");
  for (const auto& metric : channelMetrics) {
    uint8_t score = getEfficiencyScore(metric.channel);
    float avgLatency = (float)metric.totalLatency / metric.measurements;
    Serial.printf("Channel %u: Score=%u%%, Packets=%u, Lost=%u, Avg Latency=%.0fms\n",
      metric.channel, score, metric.packetsTransmitted, metric.packetsLost, avgLatency);
  }
  Serial.println("==================================\n");
}

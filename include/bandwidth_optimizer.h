#ifndef BANDWIDTH_OPTIMIZER_H
#define BANDWIDTH_OPTIMIZER_H

#include <Arduino.h>
#include <vector>

// ============= BANDWIDTH OPTIMIZER =============

enum class CompressionMethod {
  NONE = 0,
  RLE = 1,          // Run-Length Encoding
  DEFLATE = 2,      // LZ77-based compression
  HUFFMAN = 3       // Huffman coding
};

class BandwidthOptimizer {
public:
  static BandwidthOptimizer& getInstance() {
    static BandwidthOptimizer instance;
    return instance;
  }

  // Payload compression
  std::vector<uint8_t> compressPayload(const uint8_t* data, uint16_t length, CompressionMethod method);
  std::vector<uint8_t> decompressPayload(const std::vector<uint8_t>& compressed, CompressionMethod method);

  // Compression statistics
  float getCompressionRatio() const { return compressionRatio; }
  uint32_t getBytesCompressed() const { return bytesCompressed; }
  uint32_t getTotalBytesSaved() const;

  // Channel selection
  uint8_t selectOptimalChannel(const uint8_t* rssiMap, uint8_t mapSize);
  uint8_t selectLowNoiseChannel();

  void printCompressionStats();
  void printChannelAnalysis();

private:
  BandwidthOptimizer() : compressionRatio(0.0f), bytesCompressed(0), originalSize(0) {}

  float compressionRatio;
  uint32_t bytesCompressed;
  uint32_t originalSize;
  std::vector<uint8_t> rleCompress(const uint8_t* data, uint16_t length);
  std::vector<uint8_t> rleDecompress(const std::vector<uint8_t>& data);
};

// ============= CHANNEL EFFICIENCY =============

class ChannelEfficiency {
public:
  static ChannelEfficiency& getInstance() {
    static ChannelEfficiency instance;
    return instance;
  }

  void recordChannelPerformance(uint8_t channel, uint16_t packetsTransmitted,
                               uint16_t packetsLost, uint32_t latency);
  uint8_t getEfficiencyScore(uint8_t channel) const;
  uint8_t selectBestChannel();

  void printChannelStats();

private:
  ChannelEfficiency() {}

  struct ChannelMetric {
    uint8_t channel;
    uint16_t packetsTransmitted;
    uint16_t packetsLost;
    uint32_t totalLatency;
    uint16_t measurements;
  };

  std::vector<ChannelMetric> channelMetrics;
};

#endif

import SwiftUI

// MARK: - Statistics Charts

struct AttackSuccessRateChart: View {
    let statistics: AuditStatistics

    var successPercentage: Double {
        guard statistics.totalAttacks > 0 else { return 0 }
        return (Double(statistics.successfulAttacks) / Double(statistics.totalAttacks)) * 100
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("Taux de Réussite")
                .font(.system(size: 16, weight: .bold))
                .foregroundColor(.white)

            HStack(spacing: 20) {
                // Circular Progress
                ZStack {
                    Circle()
                        .stroke(
                            Color(red: 0.20, green: 0.24, blue: 0.31),
                            lineWidth: 12
                        )

                    Circle()
                        .trim(from: 0, to: successPercentage / 100)
                        .stroke(
                            Color(red: 0.96, green: 0.60, blue: 0.07),
                            style: StrokeStyle(lineWidth: 12, lineCap: .round)
                        )
                        .rotationEffect(.degrees(-90))

                    VStack(spacing: 4) {
                        Text(String(format: "%.1f", successPercentage))
                            .font(.system(size: 24, weight: .bold))
                            .foregroundColor(Color(red: 0.96, green: 0.60, blue: 0.07))

                        Text("%")
                            .font(.system(size: 12, weight: .regular))
                            .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                    }
                }
                .frame(width: 120, height: 120)

                // Statistics Details
                VStack(alignment: .leading, spacing: 12) {
                    StatChip(
                        label: "Totales",
                        value: "\(statistics.totalAttacks)",
                        icon: "⚡",
                        color: Color(red: 0.96, green: 0.60, blue: 0.07)
                    )

                    StatChip(
                        label: "Réussies",
                        value: "\(statistics.successfulAttacks)",
                        icon: "✅",
                        color: Color.green
                    )

                    StatChip(
                        label: "Échouées",
                        value: "\(statistics.failedAttacks)",
                        icon: "❌",
                        color: Color.red
                    )
                }

                Spacer()
            }
        }
        .padding(12)
        .background(Color(red: 0.10, green: 0.10, blue: 0.18))
        .cornerRadius(10)
    }
}

struct AttacksByTypeChart: View {
    let statistics: AuditStatistics

    var sortedAttacks: [(String, Int)] {
        statistics.attacksByType.sorted { $0.value > $1.value }
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("Attaques par Type")
                .font(.system(size: 16, weight: .bold))
                .foregroundColor(.white)

            if sortedAttacks.isEmpty {
                Text("Aucune attaque")
                    .font(.system(size: 13, weight: .regular))
                    .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                    .padding(12)
            } else {
                VStack(spacing: 12) {
                    ForEach(sortedAttacks, id: \.0) { type, count in
                        AttackTypeBarChart(type: type, count: count, totalAttacks: statistics.totalAttacks)
                    }
                }
            }
        }
        .padding(12)
        .background(Color(red: 0.10, green: 0.10, blue: 0.18))
        .cornerRadius(10)
    }
}

struct AttackTypeBarChart: View {
    let type: String
    let count: Int
    let totalAttacks: Int

    var percentage: Double {
        guard totalAttacks > 0 else { return 0 }
        return (Double(count) / Double(totalAttacks)) * 100
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            HStack {
                Text(type)
                    .font(.system(size: 12, weight: .semibold))
                    .foregroundColor(Color(red: 0.96, green: 0.60, blue: 0.07))

                Spacer()

                Text("\(count)")
                    .font(.system(size: 12, weight: .bold))
                    .foregroundColor(.white)
            }

            GeometryReader { geometry in
                ZStack(alignment: .leading) {
                    // Background bar
                    RoundedRectangle(cornerRadius: 4)
                        .fill(Color(red: 0.06, green: 0.07, blue: 0.11))

                    // Filled bar
                    RoundedRectangle(cornerRadius: 4)
                        .fill(
                            LinearGradient(
                                gradient: Gradient(colors: [
                                    Color(red: 0.96, green: 0.60, blue: 0.07),
                                    Color(red: 0.96, green: 0.60, blue: 0.07).opacity(0.7)
                                ]),
                                startPoint: .leading,
                                endPoint: .trailing
                            )
                        )
                        .frame(width: geometry.size.width * percentage / 100)
                }
            }
            .frame(height: 24)

            Text(String(format: "%.1f%%", percentage))
                .font(.system(size: 10, weight: .regular))
                .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
        }
    }
}

struct DurationTrendChart: View {
    let results: [AttackResult]

    var averagesByType: [(String, Int)] {
        guard !results.isEmpty else { return [] }

        var byType: [String: [Int]] = [:]

        for result in results {
            byType[result.attackType, default: []].append(result.duration)
        }

        return byType.map { type, durations in
            let average = durations.reduce(0, +) / durations.count
            return (type, average)
        }.sorted { $0.1 > $1.1 }
    }

    var maxDuration: Int {
        averagesByType.map { $0.1 }.max() ?? 0
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("Durée Moyenne par Type")
                .font(.system(size: 16, weight: .bold))
                .foregroundColor(.white)

            if averagesByType.isEmpty {
                Text("Aucun résultat")
                    .font(.system(size: 13, weight: .regular))
                    .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                    .padding(12)
            } else {
                VStack(spacing: 12) {
                    ForEach(averagesByType, id: \.0) { type, duration in
                        DurationBarChart(type: type, duration: duration, maxDuration: maxDuration)
                    }
                }
            }
        }
        .padding(12)
        .background(Color(red: 0.10, green: 0.10, blue: 0.18))
        .cornerRadius(10)
    }
}

struct DurationBarChart: View {
    let type: String
    let duration: Int
    let maxDuration: Int

    var percentage: Double {
        guard maxDuration > 0 else { return 0 }
        return (Double(duration) / Double(maxDuration)) * 100
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            HStack {
                Text(type)
                    .font(.system(size: 12, weight: .semibold))
                    .foregroundColor(Color(red: 0.96, green: 0.60, blue: 0.07))

                Spacer()

                Text("\(duration)s")
                    .font(.system(size: 12, weight: .bold))
                    .foregroundColor(.white)
            }

            GeometryReader { geometry in
                ZStack(alignment: .leading) {
                    RoundedRectangle(cornerRadius: 4)
                        .fill(Color(red: 0.06, green: 0.07, blue: 0.11))

                    RoundedRectangle(cornerRadius: 4)
                        .fill(
                            LinearGradient(
                                gradient: Gradient(colors: [
                                    Color(red: 0.46, green: 0.80, blue: 0.44),
                                    Color(red: 0.46, green: 0.80, blue: 0.44).opacity(0.7)
                                ]),
                                startPoint: .leading,
                                endPoint: .trailing
                            )
                        )
                        .frame(width: geometry.size.width * percentage / 100)
                }
            }
            .frame(height: 24)
        }
    }
}

// MARK: - Supporting Components

struct StatChip: View {
    let label: String
    let value: String
    let icon: String
    let color: Color

    var body: some View {
        HStack(spacing: 6) {
            Text(icon)
                .font(.system(size: 14))

            VStack(alignment: .leading, spacing: 2) {
                Text(label)
                    .font(.system(size: 10, weight: .regular))
                    .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))

                Text(value)
                    .font(.system(size: 14, weight: .bold))
                    .foregroundColor(color)
            }

            Spacer()
        }
        .padding(8)
        .background(Color(red: 0.06, green: 0.07, blue: 0.11))
        .cornerRadius(6)
    }
}

struct TimelineChart: View {
    let results: [AttackResult]

    var recentResults: [AttackResult] {
        results.sorted { $0.timestamp > $1.timestamp }.prefix(10).map { $0 }
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("Chronologie Récente")
                .font(.system(size: 16, weight: .bold))
                .foregroundColor(.white)

            if recentResults.isEmpty {
                Text("Aucun résultat")
                    .font(.system(size: 13, weight: .regular))
                    .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                    .padding(12)
            } else {
                VStack(alignment: .leading, spacing: 8) {
                    ForEach(recentResults, id: \.id) { result in
                        TimelineItem(result: result)
                    }
                }
            }
        }
        .padding(12)
        .background(Color(red: 0.10, green: 0.10, blue: 0.18))
        .cornerRadius(10)
    }
}

struct TimelineItem: View {
    let result: AttackResult

    var timeAgo: String {
        let formatter = RelativeDateTimeFormatter()
        formatter.unitsStyle = .abbreviated
        return formatter.localizedString(for: result.timestamp, relativeTo: Date())
    }

    var body: some View {
        HStack(spacing: 12) {
            // Timeline dot
            ZStack {
                Circle()
                    .fill(
                        LinearGradient(
                            gradient: Gradient(colors: [
                                Color(red: 0.96, green: 0.60, blue: 0.07),
                                Color(red: 0.96, green: 0.60, blue: 0.07).opacity(0.5)
                            ]),
                            startPoint: .topLeading,
                            endPoint: .bottomTrailing
                        )
                    )
                    .frame(width: 12, height: 12)

                Circle()
                    .stroke(Color(red: 0.10, green: 0.10, blue: 0.18), lineWidth: 2)
            }

            // Content
            VStack(alignment: .leading, spacing: 2) {
                HStack {
                    Text(result.attackType)
                        .font(.system(size: 12, weight: .semibold))
                        .foregroundColor(.white)

                    Spacer()

                    Text(result.success ? "✅" : "❌")
                        .font(.system(size: 12))
                }

                HStack {
                    Text(result.target)
                        .font(.system(size: 10, weight: .regular))
                        .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                        .lineLimit(1)

                    Spacer()

                    Text(timeAgo)
                        .font(.system(size: 10, weight: .regular))
                        .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                }
            }
        }
    }
}

#Preview {
    VStack(spacing: 20) {
        AttackSuccessRateChart(statistics: AuditStatistics(
            totalAttacks: 20,
            successfulAttacks: 16,
            failedAttacks: 4,
            totalDuration: 600,
            attacksByType: ["Déauth": 10, "Beacon": 5, "BLE": 5]
        ))

        AttacksByTypeChart(statistics: AuditStatistics(
            totalAttacks: 20,
            successfulAttacks: 16,
            failedAttacks: 4,
            totalDuration: 600,
            attacksByType: ["Déauth": 10, "Beacon": 5, "BLE": 5]
        ))
    }
    .padding()
    .background(Color(red: 0.06, green: 0.07, blue: 0.11))
    .preferredColorScheme(.dark)
}

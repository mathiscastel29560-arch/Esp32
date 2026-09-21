import SwiftUI

struct DashboardView: View {
    @ObservedObject var viewModel: AppViewModel
    @State private var refreshing = false

    var body: some View {
        NavigationStack {
            ScrollView {
                VStack(alignment: .leading, spacing: 16) {
                    // Header
                    HStack {
                        VStack(alignment: .leading, spacing: 4) {
                            Text("📊 " + TranslationsFR.translate("tab_dashboard"))
                                .font(.system(size: 28, weight: .bold))
                                .foregroundColor(.white)
                            Text(TranslationsFR.translate("label_metrics"))
                                .font(.system(size: 14, weight: .semibold))
                                .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                        }
                        Spacer()

                        // Status Indicator
                        HStack(spacing: 8) {
                            Circle()
                                .fill(viewModel.esp32Connection.isConnected ? Color.green : Color.red)
                                .frame(width: 12, height: 12)
                            Text(viewModel.esp32Connection.isConnected ?
                                 TranslationsFR.translate("status_connected") :
                                 TranslationsFR.translate("status_disconnected"))
                                .font(.system(size: 12, weight: .semibold))
                                .foregroundColor(.white)
                        }
                        .padding(.horizontal, 12)
                        .padding(.vertical, 8)
                        .background(Color(red: 0.21, green: 0.24, blue: 0.31))
                        .cornerRadius(6)
                    }
                    .padding(.horizontal)

                    // Statistics Cards
                    VStack(spacing: 12) {
                        StatisticCard(
                            icon: "⚡",
                            label: TranslationsFR.translate("label_total_attacks"),
                            value: "\(viewModel.statistics.totalAttacks)"
                        )

                        StatisticCard(
                            icon: "✅",
                            label: TranslationsFR.translate("label_successful"),
                            value: "\(viewModel.statistics.successfulAttacks)"
                        )

                        StatisticCard(
                            icon: "❌",
                            label: TranslationsFR.translate("label_failed"),
                            value: "\(viewModel.statistics.failedAttacks)"
                        )

                        StatisticCard(
                            icon: "⏱️",
                            label: "Durée Moyenne",
                            value: "\(viewModel.statistics.averageDuration)s"
                        )
                    }
                    .padding(.horizontal)

                    // Recent Audits
                    VStack(alignment: .leading, spacing: 12) {
                        Text("Audits Récents")
                            .font(.system(size: 16, weight: .bold))
                            .foregroundColor(.white)
                            .padding(.horizontal)

                        if viewModel.audits.isEmpty {
                            Text("Aucun audit disponible")
                                .font(.system(size: 14, weight: .regular))
                                .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                                .padding()
                                .frame(maxWidth: .infinity)
                                .background(Color(red: 0.10, green: 0.10, blue: 0.18))
                                .cornerRadius(8)
                                .padding(.horizontal)
                        } else {
                            VStack(spacing: 8) {
                                ForEach(viewModel.audits.prefix(3)) { audit in
                                    AuditRowView(audit: audit)
                                }
                            }
                            .padding(.horizontal)
                        }
                    }

                    // Recent Results
                    VStack(alignment: .leading, spacing: 12) {
                        Text("Résultats Récents")
                            .font(.system(size: 16, weight: .bold))
                            .foregroundColor(.white)
                            .padding(.horizontal)

                        if viewModel.results.isEmpty {
                            Text("Aucun résultat disponible")
                                .font(.system(size: 14, weight: .regular))
                                .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                                .padding()
                                .frame(maxWidth: .infinity)
                                .background(Color(red: 0.10, green: 0.10, blue: 0.18))
                                .cornerRadius(8)
                                .padding(.horizontal)
                        } else {
                            VStack(spacing: 8) {
                                ForEach(viewModel.results.suffix(3)) { result in
                                    ResultRowView(result: result)
                                }
                            }
                            .padding(.horizontal)
                        }
                    }

                    Spacer(minLength: 20)
                }
                .padding(.vertical)
            }
            .navigationTitle("")
            .navigationBarTitleDisplayMode(.inline)
        }
        .background(Color(red: 0.06, green: 0.07, blue: 0.11))
    }
}

// MARK: - Subviews

struct StatisticCard: View {
    let icon: String
    let label: String
    let value: String

    var body: some View {
        HStack(spacing: 12) {
            Text(icon)
                .font(.system(size: 24))

            VStack(alignment: .leading, spacing: 4) {
                Text(label)
                    .font(.system(size: 12, weight: .regular))
                    .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))

                Text(value)
                    .font(.system(size: 18, weight: .bold))
                    .foregroundColor(.white)
            }

            Spacer()
        }
        .padding(12)
        .background(Color(red: 0.10, green: 0.10, blue: 0.18))
        .cornerRadius(8)
    }
}

struct AuditRowView: View {
    let audit: Audit

    var body: some View {
        HStack(spacing: 12) {
            VStack(alignment: .leading, spacing: 4) {
                Text(audit.name)
                    .font(.system(size: 14, weight: .semibold))
                    .foregroundColor(.white)

                HStack(spacing: 8) {
                    Text(audit.type.rawValue)
                        .font(.system(size: 12, weight: .regular))
                        .foregroundColor(Color(red: 0.96, green: 0.60, blue: 0.07))

                    Text(audit.status.rawValue)
                        .font(.system(size: 12, weight: .regular))
                        .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                }
            }

            Spacer()

            Text(audit.results.count > 0 ? "\(audit.results.count) résultats" : "Nouveau")
                .font(.system(size: 12, weight: .regular))
                .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
        }
        .padding(12)
        .background(Color(red: 0.10, green: 0.10, blue: 0.18))
        .cornerRadius(8)
    }
}

struct ResultRowView: View {
    let result: AttackResult

    var body: some View {
        HStack(spacing: 12) {
            Text(result.statusIcon)
                .font(.system(size: 16))

            VStack(alignment: .leading, spacing: 2) {
                Text(result.attackType)
                    .font(.system(size: 12, weight: .semibold))
                    .foregroundColor(.white)

                Text(result.target)
                    .font(.system(size: 11, weight: .regular))
                    .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
            }

            Spacer()

            VStack(alignment: .trailing, spacing: 2) {
                Text(result.success ? "Réussi" : "Échoué")
                    .font(.system(size: 11, weight: .semibold))
                    .foregroundColor(result.success ? Color.green : Color.red)

                Text("\(result.duration)s")
                    .font(.system(size: 10, weight: .regular))
                    .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
            }
        }
        .padding(10)
        .background(Color(red: 0.10, green: 0.10, blue: 0.18))
        .cornerRadius(8)
    }
}

#Preview {
    DashboardView(viewModel: AppViewModel())
        .preferredColorScheme(.dark)
}

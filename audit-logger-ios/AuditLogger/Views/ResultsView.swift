import SwiftUI

struct ResultsView: View {
    @ObservedObject var viewModel: AppViewModel
    @State private var sortBy = "recent"
    @State private var filterStatus = "all"

    var filteredResults: [AttackResult] {
        var results = viewModel.results

        // Filter by status
        if filterStatus == "success" {
            results = results.filter { $0.success }
        } else if filterStatus == "failed" {
            results = results.filter { !$0.success }
        }

        // Sort
        if sortBy == "recent" {
            results.sort { $0.timestamp > $1.timestamp }
        } else if sortBy == "oldest" {
            results.sort { $0.timestamp < $1.timestamp }
        } else if sortBy == "duration" {
            results.sort { $0.duration > $1.duration }
        }

        return results
    }

    var body: some View {
        NavigationStack {
            ZStack {
                Color(red: 0.06, green: 0.07, blue: 0.11)
                    .ignoresSafeArea()

                VStack(spacing: 0) {
                    // Header
                    HStack {
                        VStack(alignment: .leading, spacing: 4) {
                            Text("📈 " + TranslationsFR.translate("tab_results"))
                                .font(.system(size: 28, weight: .bold))
                                .foregroundColor(.white)
                            Text("Analysez les résultats des attaques")
                                .font(.system(size: 14, weight: .semibold))
                                .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                        }
                        Spacer()
                    }
                    .padding(.horizontal)
                    .padding(.vertical, 12)

                    Divider()
                        .background(Color(red: 0.20, green: 0.24, blue: 0.31))

                    // Filters
                    HStack(spacing: 12) {
                        Menu {
                            Button("Plus récent") { sortBy = "recent" }
                            Button("Plus ancien") { sortBy = "oldest" }
                            Button("Par durée") { sortBy = "duration" }
                        } label: {
                            HStack(spacing: 4) {
                                Image(systemName: "arrow.up.arrow.down")
                                Text("Trier")
                            }
                            .font(.system(size: 13, weight: .semibold))
                            .foregroundColor(.white)
                            .padding(.horizontal, 10)
                            .padding(.vertical, 6)
                            .background(Color(red: 0.10, green: 0.10, blue: 0.18))
                            .cornerRadius(6)
                        }

                        Menu {
                            Button("Tous") { filterStatus = "all" }
                            Button("Réussis") { filterStatus = "success" }
                            Button("Échoués") { filterStatus = "failed" }
                        } label: {
                            HStack(spacing: 4) {
                                Image(systemName: "line.3.horizontal.decrease.circle")
                                Text("Filtrer")
                            }
                            .font(.system(size: 13, weight: .semibold))
                            .foregroundColor(.white)
                            .padding(.horizontal, 10)
                            .padding(.vertical, 6)
                            .background(Color(red: 0.10, green: 0.10, blue: 0.18))
                            .cornerRadius(6)
                        }

                        Spacer()

                        Text("\(filteredResults.count)")
                            .font(.system(size: 13, weight: .semibold))
                            .foregroundColor(Color(red: 0.96, green: 0.60, blue: 0.07))
                            .padding(.horizontal, 10)
                            .padding(.vertical, 6)
                            .background(Color(red: 0.10, green: 0.10, blue: 0.18))
                            .cornerRadius(6)
                    }
                    .padding()

                    Divider()
                        .background(Color(red: 0.20, green: 0.24, blue: 0.31))

                    // Results List
                    if filteredResults.isEmpty {
                        VStack(spacing: 20) {
                            Image(systemName: "chart.bar")
                                .font(.system(size: 48))
                                .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))

                            VStack(spacing: 8) {
                                Text("Aucun résultat")
                                    .font(.system(size: 18, weight: .bold))
                                    .foregroundColor(.white)

                                Text("Les résultats des attaques apparaîtront ici")
                                    .font(.system(size: 14, weight: .regular))
                                    .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                                    .multilineTextAlignment(.center)
                            }
                        }
                        .frame(maxWidth: .infinity, maxHeight: .infinity)
                    } else {
                        ScrollView {
                            VStack(spacing: 12) {
                                ForEach(filteredResults) { result in
                                    ResultDetailCard(result: result, viewModel: viewModel)
                                }
                            }
                            .padding()
                        }
                    }
                }
            }
        }
    }
}

struct ResultDetailCard: View {
    let result: AttackResult
    @ObservedObject var viewModel: AppViewModel
    @State private var showDetails = false

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            // Header
            HStack(spacing: 10) {
                Text(result.statusIcon)
                    .font(.system(size: 20))

                VStack(alignment: .leading, spacing: 2) {
                    Text(result.attackType)
                        .font(.system(size: 14, weight: .bold))
                        .foregroundColor(.white)

                    Text(result.target)
                        .font(.system(size: 12, weight: .regular))
                        .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                }

                Spacer()

                VStack(alignment: .trailing, spacing: 2) {
                    Text(result.success ? "Réussi ✓" : "Échoué ✗")
                        .font(.system(size: 12, weight: .bold))
                        .foregroundColor(result.success ? Color.green : Color.red)

                    Text(formatDate(result.timestamp))
                        .font(.system(size: 10, weight: .regular))
                        .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                }
            }

            Divider()
                .background(Color(red: 0.20, green: 0.24, blue: 0.31))

            // Details
            VStack(spacing: 6) {
                DetailRowCompact(label: "Durée", value: "\(result.duration)s")
                DetailRowCompact(label: "ID", value: result.id.uuidString.prefix(8).uppercased())

                if let packets = result.details["Paquets"] {
                    DetailRowCompact(label: "Paquets", value: packets)
                }
            }

            // Actions
            HStack(spacing: 8) {
                Button(action: { showDetails = true }) {
                    HStack(spacing: 4) {
                        Image(systemName: "arrow.up.right")
                        Text("Détails")
                    }
                    .font(.system(size: 12, weight: .semibold))
                    .foregroundColor(.white)
                    .padding(.horizontal, 8)
                    .padding(.vertical, 6)
                    .background(Color(red: 0.96, green: 0.60, blue: 0.07))
                    .cornerRadius(4)
                }

                Button(action: { UIPasteboard.general.string = result.id.uuidString }) {
                    HStack(spacing: 4) {
                        Image(systemName: "doc.on.doc")
                        Text("Copier")
                    }
                    .font(.system(size: 12, weight: .semibold))
                    .foregroundColor(Color(red: 0.96, green: 0.60, blue: 0.07))
                    .padding(.horizontal, 8)
                    .padding(.vertical, 6)
                    .background(Color(red: 0.10, green: 0.10, blue: 0.18))
                    .cornerRadius(4)
                }

                Spacer()

                Button(role: .destructive, action: { viewModel.deleteResult(result) }) {
                    Image(systemName: "trash")
                        .font(.system(size: 12, weight: .semibold))
                }
            }
        }
        .padding(12)
        .background(Color(red: 0.10, green: 0.10, blue: 0.18))
        .cornerRadius(10)
        .sheet(isPresented: $showDetails) {
            ResultDetailsSheet(result: result)
        }
    }

    func formatDate(_ date: Date) -> String {
        let formatter = DateFormatter()
        formatter.timeStyle = .short
        formatter.dateStyle = .short
        return formatter.string(from: date)
    }
}

struct DetailRowCompact: View {
    let label: String
    let value: String

    var body: some View {
        HStack {
            Text(label)
                .font(.system(size: 12, weight: .regular))
                .foregroundColor(Color(red: 0.96, green: 0.60, blue: 0.07))

            Spacer()

            Text(value)
                .font(.system(size: 12, weight: .semibold))
                .foregroundColor(.white)
                .lineLimit(1)
        }
    }
}

struct ResultDetailsSheet: View {
    let result: AttackResult
    @Environment(\.dismiss) var dismiss

    var body: some View {
        NavigationStack {
            ZStack {
                Color(red: 0.06, green: 0.07, blue: 0.11)
                    .ignoresSafeArea()

                ScrollView {
                    VStack(alignment: .leading, spacing: 16) {
                        // Status
                        HStack(spacing: 12) {
                            Text(result.statusIcon)
                                .font(.system(size: 32))

                            VStack(alignment: .leading, spacing: 4) {
                                Text(result.attackType)
                                    .font(.system(size: 20, weight: .bold))
                                    .foregroundColor(.white)

                                Text(result.success ? "Opération réussie" : "Opération échouée")
                                    .font(.system(size: 14, weight: .regular))
                                    .foregroundColor(result.success ? Color.green : Color.red)
                            }
                        }
                        .padding()
                        .background(Color(red: 0.10, green: 0.10, blue: 0.18))
                        .cornerRadius(10)

                        // Information
                        VStack(alignment: .leading, spacing: 12) {
                            Text("Information")
                                .font(.system(size: 16, weight: .bold))
                                .foregroundColor(.white)

                            VStack(spacing: 8) {
                                DetailRow(label: "Type d'Attaque", value: result.attackType)
                                DetailRow(label: "Cible", value: result.target)
                                DetailRow(label: "Durée", value: "\(result.duration)s")
                                DetailRow(label: "Message", value: result.message)
                            }
                        }
                        .padding()
                        .background(Color(red: 0.10, green: 0.10, blue: 0.18))
                        .cornerRadius(10)

                        // Details
                        if !result.details.isEmpty {
                            VStack(alignment: .leading, spacing: 12) {
                                Text("Détails")
                                    .font(.system(size: 16, weight: .bold))
                                    .foregroundColor(.white)

                                VStack(spacing: 8) {
                                    ForEach(result.details.sorted(by: { $0.key < $1.key }), id: \.key) { key, value in
                                        DetailRow(label: key, value: value)
                                    }
                                }
                            }
                            .padding()
                            .background(Color(red: 0.10, green: 0.10, blue: 0.18))
                            .cornerRadius(10)
                        }
                    }
                    .padding()
                }
            }
            .navigationTitle("Détails du Résultat")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .topBarLeading) {
                    Button("Fermer") {
                        dismiss()
                    }
                    .foregroundColor(Color(red: 0.96, green: 0.60, blue: 0.07))
                }
            }
        }
    }
}

struct DetailRow: View {
    let label: String
    let value: String

    var body: some View {
        HStack {
            Text(label)
                .font(.system(size: 13, weight: .regular))
                .foregroundColor(Color(red: 0.96, green: 0.60, blue: 0.07))

            Spacer()

            Text(value)
                .font(.system(size: 13, weight: .semibold))
                .foregroundColor(.white)
                .multilineTextAlignment(.trailing)
        }
    }
}

#Preview {
    ResultsView(viewModel: AppViewModel())
        .preferredColorScheme(.dark)
}

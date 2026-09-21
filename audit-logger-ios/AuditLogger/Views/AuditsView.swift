import SwiftUI

struct AuditsView: View {
    @ObservedObject var viewModel: AppViewModel
    @State private var showNewAudit = false
    @State private var auditName = ""
    @State private var selectedType: Audit.AuditType = .wifi
    @State private var auditTarget = ""
    @State private var auditLocation = ""
    @State private var auditOperator = ""

    var body: some View {
        NavigationStack {
            ZStack {
                Color(red: 0.06, green: 0.07, blue: 0.11)
                    .ignoresSafeArea()

                VStack(spacing: 0) {
                    // Header
                    HStack {
                        VStack(alignment: .leading, spacing: 4) {
                            Text("📋 " + TranslationsFR.translate("tab_audits"))
                                .font(.system(size: 28, weight: .bold))
                                .foregroundColor(.white)
                            Text("Gérez vos audits de sécurité")
                                .font(.system(size: 14, weight: .semibold))
                                .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                        }
                        Spacer()

                        Button(action: { showNewAudit = true }) {
                            HStack(spacing: 6) {
                                Image(systemName: "plus.circle.fill")
                                Text("Nouveau")
                            }
                            .font(.system(size: 14, weight: .semibold))
                            .foregroundColor(.white)
                            .padding(.horizontal, 12)
                            .padding(.vertical, 8)
                            .background(Color(red: 0.96, green: 0.60, blue: 0.07))
                            .cornerRadius(6)
                        }
                    }
                    .padding(.horizontal)
                    .padding(.vertical, 12)

                    Divider()
                        .background(Color(red: 0.20, green: 0.24, blue: 0.31))

                    // Audit List
                    if viewModel.audits.isEmpty {
                        VStack(spacing: 20) {
                            Image(systemName: "doc.text.magnifyingglass")
                                .font(.system(size: 48))
                                .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))

                            VStack(spacing: 8) {
                                Text("Aucun audit")
                                    .font(.system(size: 18, weight: .bold))
                                    .foregroundColor(.white)

                                Text("Créez votre premier audit pour commencer")
                                    .font(.system(size: 14, weight: .regular))
                                    .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                                    .multilineTextAlignment(.center)
                            }
                        }
                        .frame(maxWidth: .infinity, maxHeight: .infinity)
                        .background(Color(red: 0.06, green: 0.07, blue: 0.11))
                    } else {
                        ScrollView {
                            VStack(spacing: 12) {
                                ForEach(viewModel.audits) { audit in
                                    AuditDetailCard(audit: audit, viewModel: viewModel)
                                }
                            }
                            .padding()
                        }
                    }
                }
            }
            .sheet(isPresented: $showNewAudit) {
                NewAuditSheet(
                    isPresented: $showNewAudit,
                    viewModel: viewModel,
                    auditName: $auditName,
                    selectedType: $selectedType,
                    auditTarget: $auditTarget,
                    auditLocation: $auditLocation,
                    auditOperator: $auditOperator
                )
            }
        }
    }
}

struct AuditDetailCard: View {
    let audit: Audit
    @ObservedObject var viewModel: AppViewModel
    @State private var showDeleteConfirmation = false

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            // Header
            HStack {
                VStack(alignment: .leading, spacing: 4) {
                    Text(audit.name)
                        .font(.system(size: 16, weight: .bold))
                        .foregroundColor(.white)

                    HStack(spacing: 8) {
                        Badge(text: audit.type.rawValue, color: Color(red: 0.96, green: 0.60, blue: 0.07))
                        Badge(text: audit.status.rawValue, color: statusColor(audit.status))
                    }
                }

                Spacer()

                Menu {
                    Button(action: { viewModel.currentAudit = audit }) {
                        Label("Ouvrir", systemImage: "folder")
                    }

                    Divider()

                    Button(role: .destructive, action: { showDeleteConfirmation = true }) {
                        Label("Supprimer", systemImage: "trash")
                    }
                } label: {
                    Image(systemName: "ellipsis.circle")
                        .font(.system(size: 18, weight: .semibold))
                        .foregroundColor(Color(red: 0.96, green: 0.60, blue: 0.07))
                }
            }

            Divider()
                .background(Color(red: 0.20, green: 0.24, blue: 0.31))

            // Details
            VStack(spacing: 8) {
                DetailRow(label: "Cible", value: audit.target)
                DetailRow(label: "Localisation", value: audit.location)
                DetailRow(label: "Opérateur", value: audit.operatorName)
                DetailRow(label: "Résultats", value: "\(audit.results.count)")
            }
            .font(.system(size: 13, weight: .regular))
        }
        .padding(12)
        .background(Color(red: 0.10, green: 0.10, blue: 0.18))
        .cornerRadius(10)
        .alert("Confirmer la suppression", isPresented: $showDeleteConfirmation) {
            Button("Supprimer", role: .destructive) {
                viewModel.deleteAudit(audit)
            }
            Button("Annuler", role: .cancel) {}
        } message: {
            Text("Êtes-vous sûr de vouloir supprimer cet audit?")
        }
    }

    func statusColor(_ status: Audit.AuditStatus) -> Color {
        switch status {
        case .pending:
            return Color(red: 0.96, green: 0.60, blue: 0.07)
        case .inProgress:
            return Color.blue
        case .completed:
            return Color.green
        case .paused:
            return Color.orange
        }
    }
}

struct NewAuditSheet: View {
    @Binding var isPresented: Bool
    @ObservedObject var viewModel: AppViewModel
    @Binding var auditName: String
    @Binding var selectedType: Audit.AuditType
    @Binding var auditTarget: String
    @Binding var auditLocation: String
    @Binding var auditOperator: String

    var body: some View {
        NavigationStack {
            ZStack {
                Color(red: 0.06, green: 0.07, blue: 0.11)
                    .ignoresSafeArea()

                ScrollView {
                    VStack(spacing: 16) {
                        VStack(alignment: .leading, spacing: 12) {
                            FormField(label: "Nom de l'Audit", value: $auditName)

                            VStack(alignment: .leading, spacing: 8) {
                                Text("Type d'Audit")
                                    .font(.system(size: 14, weight: .semibold))
                                    .foregroundColor(.white)

                                Picker("Type", selection: $selectedType) {
                                    ForEach(Audit.AuditType.allCases, id: \.self) { type in
                                        Text(type.rawValue).tag(type)
                                    }
                                }
                                .pickerStyle(.segmented)
                            }

                            FormField(label: "Cible", value: $auditTarget)
                            FormField(label: "Localisation", value: $auditLocation)
                            FormField(label: "Opérateur", value: $auditOperator)
                        }
                        .padding()

                        Button(action: {
                            viewModel.createAudit(
                                name: auditName,
                                type: selectedType,
                                target: auditTarget,
                                location: auditLocation,
                                operator: auditOperator
                            )
                            isPresented = false
                        }) {
                            Text("Créer l'Audit")
                                .font(.system(size: 16, weight: .bold))
                                .foregroundColor(.white)
                                .frame(maxWidth: .infinity)
                                .padding(12)
                                .background(Color(red: 0.96, green: 0.60, blue: 0.07))
                                .cornerRadius(8)
                        }
                        .disabled(auditName.isEmpty || auditTarget.isEmpty)
                        .padding()
                    }
                }
            }
            .navigationTitle("Nouvel Audit")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .topBarLeading) {
                    Button("Annuler") {
                        isPresented = false
                    }
                    .foregroundColor(Color(red: 0.96, green: 0.60, blue: 0.07))
                }
            }
        }
    }
}

struct FormField: View {
    let label: String
    @Binding var value: String

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text(label)
                .font(.system(size: 14, weight: .semibold))
                .foregroundColor(.white)

            TextField("", text: $value)
                .padding(10)
                .background(Color(red: 0.10, green: 0.10, blue: 0.18))
                .foregroundColor(.white)
                .cornerRadius(6)
        }
    }
}

struct DetailRow: View {
    let label: String
    let value: String

    var body: some View {
        HStack {
            Text(label)
                .foregroundColor(Color(red: 0.96, green: 0.60, blue: 0.07))
            Spacer()
            Text(value)
                .foregroundColor(Color(red: 0.92, green: 0.94, blue: 0.95))
        }
    }
}

struct Badge: View {
    let text: String
    let color: Color

    var body: some View {
        Text(text)
            .font(.system(size: 11, weight: .semibold))
            .foregroundColor(.white)
            .padding(.horizontal, 8)
            .padding(.vertical, 4)
            .background(color)
            .cornerRadius(4)
    }
}

#Preview {
    AuditsView(viewModel: AppViewModel())
        .preferredColorScheme(.dark)
}

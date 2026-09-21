import SwiftUI

struct SettingsView: View {
    @ObservedObject var viewModel: AppViewModel
    @State private var esp32IP = "192.168.1.100"
    @State private var esp32Port = "8080"
    @State private var mockMode = true
    @State private var autoRefresh = true
    @State private var refreshInterval = 30
    @State private var showAbout = false

    var body: some View {
        NavigationStack {
            ZStack {
                Color(red: 0.06, green: 0.07, blue: 0.11)
                    .ignoresSafeArea()

                VStack(spacing: 0) {
                    // Header
                    HStack {
                        VStack(alignment: .leading, spacing: 4) {
                            Text("⚙️ " + TranslationsFR.translate("tab_settings"))
                                .font(.system(size: 28, weight: .bold))
                                .foregroundColor(.white)
                            Text("Configurez l'application")
                                .font(.system(size: 14, weight: .semibold))
                                .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                        }
                        Spacer()
                    }
                    .padding(.horizontal)
                    .padding(.vertical, 12)

                    Divider()
                        .background(Color(red: 0.20, green: 0.24, blue: 0.31))

                    // Settings
                    ScrollView {
                        VStack(spacing: 20) {
                            // ESP32 Connection
                            SettingsSectionView(title: "Connexion ESP32") {
                                VStack(spacing: 12) {
                                    SettingsTextField(label: "Adresse IP", value: $esp32IP)
                                    SettingsTextField(label: "Port", value: $esp32Port)

                                    HStack {
                                        Circle()
                                            .fill(viewModel.esp32Connection.isConnected ? Color.green : Color.red)
                                            .frame(width: 10, height: 10)

                                        Text(viewModel.esp32Connection.isConnected ? "Connecté" : "Déconnecté")
                                            .font(.system(size: 14, weight: .semibold))
                                            .foregroundColor(.white)

                                        Spacer()

                                        if viewModel.esp32Connection.isConnected {
                                            Button(action: { viewModel.disconnectFromESP32() }) {
                                                Text("Déconnecter")
                                                    .font(.system(size: 12, weight: .semibold))
                                                    .foregroundColor(.white)
                                                    .padding(.horizontal, 10)
                                                    .padding(.vertical, 6)
                                                    .background(Color.red)
                                                    .cornerRadius(4)
                                            }
                                        } else {
                                            Button(action: {
                                                if let port = Int(esp32Port) {
                                                    viewModel.connectToESP32(ipAddress: esp32IP, port: port)
                                                }
                                            }) {
                                                Text("Connecter")
                                                    .font(.system(size: 12, weight: .semibold))
                                                    .foregroundColor(.white)
                                                    .padding(.horizontal, 10)
                                                    .padding(.vertical, 6)
                                                    .background(Color(red: 0.96, green: 0.60, blue: 0.07))
                                                    .cornerRadius(4)
                                            }
                                        }
                                    }
                                }
                            }

                            // Application Settings
                            SettingsSectionView(title: "Application") {
                                VStack(spacing: 12) {
                                    SettingsToggle(
                                        label: "Mode de Test (Mock)",
                                        value: $mockMode,
                                        description: "Simule les attaques sans ESP32"
                                    )

                                    SettingsToggle(
                                        label: "Actualisation Automatique",
                                        value: $autoRefresh,
                                        description: "Actualise les données automatiquement"
                                    )

                                    if autoRefresh {
                                        HStack {
                                            Text("Intervalle (sec)")
                                                .font(.system(size: 14, weight: .regular))
                                                .foregroundColor(.white)

                                            Spacer()

                                            Stepper("", value: $refreshInterval, in: 5...300, step: 5)
                                                .labelsHidden()

                                            Text("\(refreshInterval)s")
                                                .font(.system(size: 14, weight: .semibold))
                                                .foregroundColor(Color(red: 0.96, green: 0.60, blue: 0.07))
                                                .frame(width: 50)
                                        }
                                    }
                                }
                            }

                            // Information
                            SettingsSectionView(title: "Information") {
                                VStack(spacing: 12) {
                                    SettingsInfoRow(label: "Version", value: "1.0.0")
                                    SettingsInfoRow(label: "Langue", value: "Français")
                                    SettingsInfoRow(label: "Thème", value: "Sombre (Dracula)")

                                    Button(action: { showAbout = true }) {
                                        HStack {
                                            Text("À propos")
                                                .font(.system(size: 14, weight: .regular))
                                                .foregroundColor(.white)

                                            Spacer()

                                            Image(systemName: "chevron.right")
                                                .font(.system(size: 14, weight: .semibold))
                                                .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                                        }
                                        .padding(.vertical, 4)
                                    }
                                }
                            }

                            // Statistics
                            SettingsSectionView(title: "Statistiques") {
                                VStack(spacing: 8) {
                                    SettingsInfoRow(label: "Audits Totaux", value: "\(viewModel.audits.count)")
                                    SettingsInfoRow(label: "Résultats", value: "\(viewModel.results.count)")
                                    SettingsInfoRow(label: "Taux de Réussite", value: String(format: "%.1f%%", viewModel.statistics.successRate * 100))
                                }
                            }

                            // Danger Zone
                            VStack(spacing: 12) {
                                Text("Zone Dangereuse")
                                    .font(.system(size: 14, weight: .bold))
                                    .foregroundColor(Color.red)

                                Button(role: .destructive, action: {
                                    // Reset logic here
                                }) {
                                    HStack {
                                        Image(systemName: "trash.fill")
                                        Text("Réinitialiser l'Application")
                                    }
                                    .font(.system(size: 14, weight: .semibold))
                                    .foregroundColor(.white)
                                    .frame(maxWidth: .infinity)
                                    .padding(10)
                                    .background(Color.red.opacity(0.2))
                                    .border(Color.red, width: 1)
                                    .cornerRadius(6)
                                }
                            }
                            .padding()
                            .background(Color(red: 0.20, 0.10, 0.10))
                            .cornerRadius(10)
                            .padding()
                        }
                        .padding(.vertical)
                    }
                }
            }
            .sheet(isPresented: $showAbout) {
                AboutSheet()
            }
        }
    }
}

struct SettingsSectionView<Content: View>: View {
    let title: String
    let content: Content

    init(title: String, @ViewBuilder content: () -> Content) {
        self.title = title
        self.content = content()
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text(title)
                .font(.system(size: 14, weight: .bold))
                .foregroundColor(.white)
                .padding(.horizontal)

            VStack(spacing: 12) {
                content
            }
            .padding()
            .background(Color(red: 0.10, green: 0.10, blue: 0.18))
            .cornerRadius(10)
            .padding(.horizontal)
        }
    }
}

struct SettingsTextField: View {
    let label: String
    @Binding var value: String

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text(label)
                .font(.system(size: 12, weight: .semibold))
                .foregroundColor(Color(red: 0.96, green: 0.60, blue: 0.07))

            TextField("", text: $value)
                .font(.system(size: 14))
                .padding(8)
                .background(Color(red: 0.06, green: 0.07, blue: 0.11))
                .foregroundColor(.white)
                .cornerRadius(6)
        }
    }
}

struct SettingsToggle: View {
    let label: String
    @Binding var value: Bool
    let description: String

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            HStack {
                Text(label)
                    .font(.system(size: 14, weight: .semibold))
                    .foregroundColor(.white)

                Spacer()

                Toggle("", isOn: $value)
                    .labelsHidden()
            }

            Text(description)
                .font(.system(size: 12, weight: .regular))
                .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
        }
    }
}

struct SettingsInfoRow: View {
    let label: String
    let value: String

    var body: some View {
        HStack {
            Text(label)
                .font(.system(size: 14, weight: .regular))
                .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))

            Spacer()

            Text(value)
                .font(.system(size: 14, weight: .semibold))
                .foregroundColor(.white)
        }
    }
}

struct AboutSheet: View {
    @Environment(\.dismiss) var dismiss

    var body: some View {
        NavigationStack {
            ZStack {
                Color(red: 0.06, green: 0.07, blue: 0.11)
                    .ignoresSafeArea()

                ScrollView {
                    VStack(spacing: 20) {
                        // Logo/Icon placeholder
                        VStack(spacing: 12) {
                            Text("🦈")
                                .font(.system(size: 64))

                            Text("Audit Logger")
                                .font(.system(size: 24, weight: .bold))
                                .foregroundColor(.white)

                            Text("v1.0.0")
                                .font(.system(size: 14, weight: .regular))
                                .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                        }
                        .frame(maxWidth: .infinity)
                        .padding(20)
                        .background(Color(red: 0.10, green: 0.10, blue: 0.18))
                        .cornerRadius(10)
                        .padding()

                        // Description
                        VStack(alignment: .leading, spacing: 12) {
                            Text("À propos")
                                .font(.system(size: 16, weight: .bold))
                                .foregroundColor(.white)

                            Text("Audit Logger est une application de test de sécurité sans fil pour iPhone 15 et plus. Elle permet de conduire des audits de pénétration sur WiFi, BLE, Sub-GHz et NFC.")
                                .font(.system(size: 14, weight: .regular))
                                .foregroundColor(Color(red: 0.92, green: 0.94, blue: 0.95))
                                .lineSpacing(2)
                        }
                        .padding()
                        .background(Color(red: 0.10, green: 0.10, blue: 0.18))
                        .cornerRadius(10)
                        .padding()

                        // Features
                        VStack(alignment: .leading, spacing: 12) {
                            Text("Fonctionnalités")
                                .font(.system(size: 16, weight: .bold))
                                .foregroundColor(.white)

                            VStack(alignment: .leading, spacing: 8) {
                                FeatureRow(icon: "🚀", text: "Tests de pénétration multiples")
                                FeatureRow(icon: "📊", text: "Tableau de bord en temps réel")
                                FeatureRow(icon: "🇫🇷", text: "Interface entièrement en français")
                                FeatureRow(icon: "⚙️", text: "Configuration avancée")
                                FeatureRow(icon: "📈", text: "Analyses détaillées des résultats")
                            }
                        }
                        .padding()
                        .background(Color(red: 0.10, green: 0.10, blue: 0.18))
                        .cornerRadius(10)
                        .padding()
                    }
                    .padding(.vertical)
                }
            }
            .navigationTitle("À propos")
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

struct FeatureRow: View {
    let icon: String
    let text: String

    var body: some View {
        HStack(spacing: 10) {
            Text(icon)
                .font(.system(size: 18))

            Text(text)
                .font(.system(size: 13, weight: .regular))
                .foregroundColor(Color(red: 0.92, green: 0.94, blue: 0.95))
        }
    }
}

#Preview {
    SettingsView(viewModel: AppViewModel())
        .preferredColorScheme(.dark)
}

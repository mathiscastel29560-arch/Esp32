import SwiftUI

struct AttacksView: View {
    @ObservedObject var viewModel: AppViewModel
    @State private var showLaunchAttack = false
    @State private var selectedAttackType = "deauth"
    @State private var attackTarget = ""
    @State private var attackDuration = 30
    @State private var attackIntensity = 80.0

    let availableAttacks = [
        ("deauth", "🚀 Déauthentification", "WiFi"),
        ("beacon", "📡 Inondation de Balises", "WiFi"),
        ("ble_pin", "🔐 Brute Force PIN BLE", "BLE"),
        ("ble_gatt", "📝 Lecture/Écriture GATT", "BLE"),
        ("subghz", "📻 Rejeu Sub-GHz", "Sub-GHz"),
        ("nfc", "🏷️ Clonage NFC", "NFC"),
    ]

    var body: some View {
        NavigationStack {
            ZStack {
                Color(red: 0.06, green: 0.07, blue: 0.11)
                    .ignoresSafeArea()

                VStack(spacing: 0) {
                    // Header
                    HStack {
                        VStack(alignment: .leading, spacing: 4) {
                            Text("⚡ " + TranslationsFR.translate("tab_attacks"))
                                .font(.system(size: 28, weight: .bold))
                                .foregroundColor(.white)
                            Text("Lancez les tests de pénétration")
                                .font(.system(size: 14, weight: .semibold))
                                .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                        }
                        Spacer()
                    }
                    .padding(.horizontal)
                    .padding(.vertical, 12)

                    Divider()
                        .background(Color(red: 0.20, green: 0.24, blue: 0.31))

                    // Attack List
                    ScrollView {
                        VStack(spacing: 12) {
                            ForEach(availableAttacks, id: \.0) { id, name, category in
                                AttackCardView(
                                    id: id,
                                    name: name,
                                    category: category,
                                    onTap: {
                                        selectedAttackType = id
                                        showLaunchAttack = true
                                    }
                                )
                            }
                        }
                        .padding()
                    }
                }
            }
            .sheet(isPresented: $showLaunchAttack) {
                AttackParametersSheet(
                    isPresented: $showLaunchAttack,
                    viewModel: viewModel,
                    attackType: selectedAttackType,
                    target: $attackTarget,
                    duration: $attackDuration,
                    intensity: $attackIntensity
                )
            }
        }
    }
}

struct AttackCardView: View {
    let id: String
    let name: String
    let category: String
    let onTap: () -> Void

    var categoryColor: Color {
        switch category {
        case "WiFi":
            return Color.blue
        case "BLE":
            return Color.purple
        case "Sub-GHz":
            return Color.orange
        case "NFC":
            return Color.green
        default:
            return Color.gray
        }
    }

    var body: some View {
        Button(action: onTap) {
            VStack(alignment: .leading, spacing: 12) {
                HStack {
                    VStack(alignment: .leading, spacing: 4) {
                        Text(name)
                            .font(.system(size: 16, weight: .bold))
                            .foregroundColor(.white)

                        HStack(spacing: 6) {
                            Circle()
                                .fill(categoryColor)
                                .frame(width: 6, height: 6)

                            Text(category)
                                .font(.system(size: 12, weight: .regular))
                                .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                        }
                    }

                    Spacer()

                    Image(systemName: "chevron.right")
                        .font(.system(size: 14, weight: .semibold))
                        .foregroundColor(Color(red: 0.96, green: 0.60, blue: 0.07))
                }

                Divider()
                    .background(Color(red: 0.20, green: 0.24, blue: 0.31))

                Text("Appuyez pour configurer et lancer")
                    .font(.system(size: 12, weight: .regular))
                    .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
            }
            .padding(12)
            .background(Color(red: 0.10, green: 0.10, blue: 0.18))
            .cornerRadius(10)
        }
    }
}

struct AttackParametersSheet: View {
    @Binding var isPresented: Bool
    @ObservedObject var viewModel: AppViewModel
    let attackType: String
    @Binding var target: String
    @Binding var duration: Int
    @Binding var intensity: Double
    @State private var isLaunching = false

    var attackName: String {
        switch attackType {
        case "deauth":
            return "Déauthentification"
        case "beacon":
            return "Inondation de Balises"
        case "ble_pin":
            return "Brute Force PIN BLE"
        case "ble_gatt":
            return "Lecture/Écriture GATT"
        case "subghz":
            return "Rejeu Sub-GHz"
        case "nfc":
            return "Clonage NFC"
        default:
            return "Attaque"
        }
    }

    var body: some View {
        NavigationStack {
            ZStack {
                Color(red: 0.06, green: 0.07, blue: 0.11)
                    .ignoresSafeArea()

                ScrollView {
                    VStack(spacing: 16) {
                        // Warning Banner
                        HStack(spacing: 12) {
                            Image(systemName: "exclamationmark.triangle.fill")
                                .font(.system(size: 18))
                                .foregroundColor(.orange)

                            VStack(alignment: .leading, spacing: 4) {
                                Text("⚠️ Utilisation Autorisée")
                                    .font(.system(size: 14, weight: .bold))
                                    .foregroundColor(.white)

                                Text("À n'utiliser que sur vos propres réseaux et appareils")
                                    .font(.system(size: 12, weight: .regular))
                                    .foregroundColor(Color(red: 0.58, green: 0.65, blue: 0.65))
                            }
                        }
                        .padding(12)
                        .background(Color(red: 0.20, 0.15, 0.07))
                        .cornerRadius(8)
                        .padding()

                        // Parameters
                        VStack(spacing: 16) {
                            FormField(label: "Cible (MAC/IP)", value: $target)

                            VStack(alignment: .leading, spacing: 8) {
                                Text("Durée (secondes): \(duration)s")
                                    .font(.system(size: 14, weight: .semibold))
                                    .foregroundColor(.white)

                                Slider(value: Double(duration), in: 1...300, step: 1)
                                    .accentColor(Color(red: 0.96, green: 0.60, blue: 0.07))
                                    .onChange(of: Double(duration)) { newValue in
                                        duration = Int(newValue)
                                    }
                            }

                            VStack(alignment: .leading, spacing: 8) {
                                Text("Intensité: \(Int(intensity))%")
                                    .font(.system(size: 14, weight: .semibold))
                                    .foregroundColor(.white)

                                Slider(value: $intensity, in: 1...100, step: 1)
                                    .accentColor(Color(red: 0.96, green: 0.60, blue: 0.07))
                            }
                        }
                        .padding()

                        // Launch Button
                        Button(action: launchAttack) {
                            HStack(spacing: 8) {
                                if isLaunching {
                                    ProgressView()
                                        .progressViewStyle(.circular)
                                        .tint(.white)
                                } else {
                                    Image(systemName: "bolt.fill")
                                }
                                Text(isLaunching ? "Lancement..." : "🚀 Lancer l'Attaque")
                                    .font(.system(size: 16, weight: .bold))
                            }
                            .foregroundColor(.white)
                            .frame(maxWidth: .infinity)
                            .padding(12)
                            .background(Color(red: 0.96, green: 0.60, blue: 0.07))
                            .cornerRadius(8)
                        }
                        .disabled(target.isEmpty || isLaunching)
                        .padding()
                    }
                }
            }
            .navigationTitle(attackName)
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

    func launchAttack() {
        isLaunching = true

        let result = AttackResult(
            id: UUID(),
            attackType: attackName,
            target: target,
            duration: duration,
            timestamp: Date(),
            success: Bool.random(),
            message: "Attaque \(attackName) lancée avec succès",
            details: [
                "Type": attackType,
                "Intensité": "\(Int(intensity))%",
                "Paquets": String(Int.random(in: 1000...5000))
            ]
        )

        DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) {
            viewModel.addAttackResult(result)
            isPresented = false
        }
    }
}

#Preview {
    AttacksView(viewModel: AppViewModel())
        .preferredColorScheme(.dark)
}

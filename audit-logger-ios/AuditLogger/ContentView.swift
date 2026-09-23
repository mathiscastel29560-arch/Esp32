import SwiftUI

struct ContentView: View {
    @StateObject private var viewModel = AppViewModel()
    @State private var selectedTab = 0

    var body: some View {
        ZStack {
            Color(red: 0.06, green: 0.07, blue: 0.11)
                .ignoresSafeArea()

            TabView(selection: $selectedTab) {
                // Dashboard Tab
                DashboardView(viewModel: viewModel)
                    .tabItem {
                        Label("Tableau de Bord", systemImage: "chart.bar.fill")
                    }
                    .tag(0)

                // Audits Tab
                AuditsView(viewModel: viewModel)
                    .tabItem {
                        Label("Audits", systemImage: "doc.text.fill")
                    }
                    .tag(1)

                // Attacks Tab
                AttacksView(viewModel: viewModel)
                    .tabItem {
                        Label("Attaques", systemImage: "bolt.fill")
                    }
                    .tag(2)

                // Results Tab
                ResultsView(viewModel: viewModel)
                    .tabItem {
                        Label("Résultats", systemImage: "chart.line.uptrend.xyaxis")
                    }
                    .tag(3)

                // Settings Tab
                SettingsView(viewModel: viewModel)
                    .tabItem {
                        Label("Paramètres", systemImage: "gear")
                    }
                    .tag(4)
            }
            .preferredColorScheme(.dark)
            .accentColor(Color(red: 0.96, green: 0.60, blue: 0.07))

            // Toast Notifications
            if let message = viewModel.successMessage {
                VStack {
                    HStack(spacing: 12) {
                        Image(systemName: "checkmark.circle.fill")
                            .foregroundColor(.green)

                        Text(message)
                            .font(.system(size: 14, weight: .semibold))
                            .foregroundColor(.white)

                        Spacer()
                    }
                    .padding(12)
                    .background(Color.green.opacity(0.2))
                    .border(Color.green, width: 1)
                    .cornerRadius(8)
                    .padding()

                    Spacer()
                }
                .transition(.move(edge: .top).combined(with: .opacity))
                .onAppear {
                    DispatchQueue.main.asyncAfter(deadline: .now() + 3) {
                        withAnimation {
                            viewModel.successMessage = nil
                        }
                    }
                }
            }

            if let message = viewModel.errorMessage {
                VStack {
                    HStack(spacing: 12) {
                        Image(systemName: "exclamationmark.circle.fill")
                            .foregroundColor(.red)

                        Text(message)
                            .font(.system(size: 14, weight: .semibold))
                            .foregroundColor(.white)

                        Spacer()
                    }
                    .padding(12)
                    .background(Color.red.opacity(0.2))
                    .border(Color.red, width: 1)
                    .cornerRadius(8)
                    .padding()

                    Spacer()
                }
                .transition(.move(edge: .top).combined(with: .opacity))
                .onAppear {
                    DispatchQueue.main.asyncAfter(deadline: .now() + 3) {
                        withAnimation {
                            viewModel.errorMessage = nil
                        }
                    }
                }
            }
        }
    }
}

#Preview {
    ContentView()
}

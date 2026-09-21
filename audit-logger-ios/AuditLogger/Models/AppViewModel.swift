import Foundation
import Combine

class AppViewModel: ObservableObject {
    @Published var audits: [Audit] = []
    @Published var currentAudit: Audit?
    @Published var results: [AttackResult] = []
    @Published var activeWorkflows: [AttackWorkflow] = []

    @Published var esp32Connection = ESP32Connection()
    @Published var statistics = AuditStatistics()

    @Published var selectedTab: Int = 0
    @Published var showNewAuditSheet = false
    @Published var showSettingsSheet = false

    @Published var errorMessage: String?
    @Published var successMessage: String?

    private let userDefaults = UserDefaults.standard
    private let auditsKey = "savedAudits"
    private let resultsKey = "savedResults"

    private var webSocketService = ESP32WebSocketService()
    private var cancellables = Set<AnyCancellable>()

    init() {
        loadAudits()
        loadResults()
        loadStatistics()
        setupConnections()
        setupWebSocketObservers()
    }

    private func setupWebSocketObservers() {
        // Listen to WebSocket connection status changes
        webSocketService.$isConnected
            .sink { [weak self] isConnected in
                self?.esp32Connection.isConnected = isConnected
                if isConnected {
                    self?.esp32Connection.connectionStatus = .connected
                    self?.successMessage = TranslationsFR.translate("status_connected")
                } else {
                    self?.esp32Connection.connectionStatus = .disconnected
                }
            }
            .store(in: &cancellables)

        // Listen to connection status changes
        webSocketService.$connectionStatus
            .sink { [weak self] status in
                switch status {
                case .connected:
                    self?.esp32Connection.connectionStatus = .connected
                case .connecting:
                    self?.esp32Connection.connectionStatus = .connecting
                case .disconnected:
                    self?.esp32Connection.connectionStatus = .disconnected
                case .error(let message):
                    self?.esp32Connection.connectionStatus = .error
                    self?.errorMessage = message
                }
            }
            .store(in: &cancellables)

        // Listen to received messages (attack results from WebSocket)
        NotificationCenter.default.publisher(for: NSNotification.Name("AttackResultReceived"))
            .sink { [weak self] notification in
                if let message = notification.object as? [String: Any] {
                    self?.processWebSocketAttackResult(message)
                }
            }
            .store(in: &cancellables)
    }

    private func processWebSocketAttackResult(_ message: [String: Any]) {
        guard let attackType = message["attackType"] as? String,
              let target = message["target"] as? String,
              let duration = message["duration"] as? Int,
              let success = message["success"] as? Bool else {
            return
        }

        let result = AttackResult(
            id: UUID(),
            attackType: attackType,
            target: target,
            duration: duration,
            timestamp: Date(),
            success: success,
            message: message["message"] as? String ?? "",
            details: (message["details"] as? [String: String]) ?? [:]
        )

        addAttackResult(result)
    }

    // MARK: - Audit Management

    func createAudit(name: String, type: Audit.AuditType, target: String, location: String, operator: String) {
        let newAudit = Audit(name: name, type: type, target: target, location: location, operatorName: `operator`)
        audits.append(newAudit)
        currentAudit = newAudit
        saveAudits()
        successMessage = TranslationsFR.translate("dialog_audit_created")
    }

    func deleteAudit(_ audit: Audit) {
        audits.removeAll { $0.id == audit.id }
        if currentAudit?.id == audit.id {
            currentAudit = nil
        }
        saveAudits()
        successMessage = TranslationsFR.translate("dialog_audit_deleted")
    }

    func updateAuditStatus(_ audit: Audit, status: Audit.AuditStatus) {
        if let index = audits.firstIndex(where: { $0.id == audit.id }) {
            audits[index].status = status
            if currentAudit?.id == audit.id {
                currentAudit?.status = status
            }
            saveAudits()
        }
    }

    // MARK: - Attack Results

    func addAttackResult(_ result: AttackResult) {
        results.append(result)

        if var audit = currentAudit {
            audit.results.append(result)
            currentAudit = audit
        }

        updateStatistics(with: result)
        saveResults()
        successMessage = TranslationsFR.translate("success_attack_launched")
    }

    func deleteResult(_ result: AttackResult) {
        results.removeAll { $0.id == result.id }
        if var audit = currentAudit {
            audit.results.removeAll { $0.id == result.id }
            currentAudit = audit
        }
        saveResults()
    }

    // MARK: - Workflow Management

    func createWorkflow(name: String, steps: [WorkflowStep]) {
        let workflow = AttackWorkflow(
            id: UUID(),
            name: name,
            steps: steps,
            status: .pending,
            progress: AttackWorkflow.WorkflowProgress(completedSteps: 0, totalSteps: steps.count)
        )
        activeWorkflows.append(workflow)
    }

    func updateWorkflowProgress(_ workflowId: UUID, completedSteps: Int) {
        if let index = activeWorkflows.firstIndex(where: { $0.id == workflowId }) {
            activeWorkflows[index].progress.completedSteps = completedSteps

            if completedSteps >= activeWorkflows[index].steps.count {
                activeWorkflows[index].status = .completed
            }
        }
    }

    // MARK: - Statistics

    private func updateStatistics(with result: AttackResult) {
        statistics.totalAttacks += 1
        statistics.totalDuration += result.duration

        if result.success {
            statistics.successfulAttacks += 1
        } else {
            statistics.failedAttacks += 1
        }

        statistics.attacksByType[result.attackType, default: 0] += 1
        saveStatistics()
    }

    // MARK: - Persistence

    private func saveAudits() {
        if let data = try? JSONEncoder().encode(audits) {
            userDefaults.set(data, forKey: auditsKey)
        }
    }

    private func loadAudits() {
        if let data = userDefaults.data(forKey: auditsKey),
           let decodedAudits = try? JSONDecoder().decode([Audit].self, from: data) {
            audits = decodedAudits
        }
    }

    private func saveResults() {
        if let data = try? JSONEncoder().encode(results) {
            userDefaults.set(data, forKey: resultsKey)
        }
    }

    private func loadResults() {
        if let data = userDefaults.data(forKey: resultsKey),
           let decodedResults = try? JSONDecoder().decode([AttackResult].self, from: data) {
            results = decodedResults
        }
    }

    private func saveStatistics() {
        if let data = try? JSONEncoder().encode(statistics) {
            userDefaults.set(data, forKey: "statistics")
        }
    }

    private func loadStatistics() {
        if let data = userDefaults.data(forKey: "statistics"),
           let decodedStats = try? JSONDecoder().decode(AuditStatistics.self, from: data) {
            statistics = decodedStats
        }
    }

    // MARK: - ESP32 Connection

    private func setupConnections() {
        esp32Connection.isConnected = false
        esp32Connection.connectionStatus = .disconnected
    }

    func connectToESP32(ipAddress: String, port: Int) {
        esp32Connection.ipAddress = ipAddress
        esp32Connection.port = port
        esp32Connection.connectionStatus = .connecting
        webSocketService.connect(to: ipAddress, port: port)
    }

    func disconnectFromESP32() {
        webSocketService.disconnect()
        esp32Connection.isConnected = false
        esp32Connection.connectionStatus = .disconnected
    }

    func sendToESP32(message: [String: Any]) {
        webSocketService.send(message: message)
    }
}

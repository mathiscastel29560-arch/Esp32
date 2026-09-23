import Foundation

// MARK: - Audit Models

struct Audit: Identifiable, Codable {
    let id: UUID
    var name: String
    var type: AuditType
    var target: String
    var location: String
    var operatorName: String
    var dateStart: Date
    var dateEnd: Date?
    var status: AuditStatus
    var results: [AttackResult]

    enum AuditType: String, Codable, CaseIterable {
        case wifi = "WiFi"
        case ble = "Bluetooth LE"
        case subghz = "Sub-GHz"
        case nfc = "NFC"
        case combined = "Combiné"
    }

    enum AuditStatus: String, Codable {
        case pending = "En Attente"
        case inProgress = "En Cours"
        case completed = "Terminé"
        case paused = "Pause"
    }

    init(name: String, type: AuditType, target: String, location: String, operatorName: String) {
        self.id = UUID()
        self.name = name
        self.type = type
        self.target = target
        self.location = location
        self.operatorName = operatorName
        self.dateStart = Date()
        self.dateEnd = nil
        self.status = .pending
        self.results = []
    }
}

// MARK: - Attack Result Models

struct AttackResult: Identifiable, Codable {
    let id: UUID
    let attackType: String
    let target: String
    let duration: Int
    let timestamp: Date
    let success: Bool
    let message: String
    var details: [String: String]

    enum ResultStatus: String {
        case success = "success"
        case warning = "warning"
        case error = "error"
        case info = "info"
    }

    var status: ResultStatus {
        success ? .success : .error
    }

    var statusIcon: String {
        switch status {
        case .success:
            return "✅"
        case .error:
            return "❌"
        case .warning:
            return "⚠️"
        case .info:
            return "ℹ️"
        }
    }

    var statusColor: String {
        switch status {
        case .success:
            return "#2ecc71"
        case .error:
            return "#e74c3c"
        case .warning:
            return "#f39c12"
        case .info:
            return "#3498db"
        }
    }
}

// MARK: - Workflow Models

struct WorkflowStep: Identifiable, Codable {
    let id: UUID
    let stepId: String
    let type: String
    var status: StepStatus
    let parameters: [String: String]

    enum StepStatus: String, Codable {
        case pending = "pending"
        case running = "running"
        case completed = "completed"
        case failed = "failed"
    }
}

struct AttackWorkflow: Identifiable, Codable {
    let id: UUID
    var name: String
    var steps: [WorkflowStep]
    var status: WorkflowStatus
    var progress: WorkflowProgress

    enum WorkflowStatus: String, Codable {
        case pending = "En Attente"
        case running = "En Cours"
        case completed = "Terminé"
        case failed = "Échoué"
    }

    struct WorkflowProgress: Codable {
        var completedSteps: Int
        var totalSteps: Int

        var percentage: Double {
            guard totalSteps > 0 else { return 0 }
            return Double(completedSteps) / Double(totalSteps)
        }
    }
}

// MARK: - Connection Models

struct ESP32Connection {
    var isConnected: Bool = false
    var ipAddress: String = ""
    var port: Int = 8080
    var connectionStatus: ConnectionStatus = .disconnected

    enum ConnectionStatus {
        case connected
        case connecting
        case disconnected
        case error(String)
    }
}

// MARK: - Statistics

struct AuditStatistics: Codable {
    var totalAttacks: Int = 0
    var successfulAttacks: Int = 0
    var failedAttacks: Int = 0
    var totalDuration: Int = 0
    var averageDuration: Int {
        guard totalAttacks > 0 else { return 0 }
        return totalDuration / totalAttacks
    }
    var attacksByType: [String: Int] = [:]

    var successRate: Double {
        guard totalAttacks > 0 else { return 0 }
        return Double(successfulAttacks) / Double(totalAttacks)
    }
}

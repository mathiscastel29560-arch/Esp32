import Foundation
import Combine

class ESP32WebSocketService: NSObject, ObservableObject, URLSessionWebSocketDelegate {
    @Published var isConnected = false
    @Published var connectionStatus: ConnectionStatus = .disconnected
    @Published var receivedMessage: String?
    @Published var lastError: String?

    private var webSocket: URLSessionWebSocket?
    private var receiveTask: Task<Void, Never>?

    enum ConnectionStatus {
        case connected
        case connecting
        case disconnected
        case error(String)
    }

    func connect(to ipAddress: String, port: Int) {
        connectionStatus = .connecting

        let urlString = "ws://\(ipAddress):\(port)/ws"
        guard let url = URL(string: urlString) else {
            connectionStatus = .error("URL invalide: \(urlString)")
            lastError = "Adresse IP ou port invalide"
            return
        }

        let session = URLSession(configuration: .default, delegate: self, delegateQueue: .main)
        webSocket = session.webSocketTask(with: url)
        webSocket?.resume()

        // Commencer à recevoir les messages
        receiveMessages()
    }

    func disconnect() {
        connectionStatus = .disconnected
        isConnected = false
        receiveTask?.cancel()
        webSocket?.cancel(with: .goingAway, reason: nil)
        webSocket = nil
    }

    func send(message: [String: Any]) {
        guard let webSocket = webSocket,
              let jsonData = try? JSONSerialization.data(withJSONObject: message),
              let jsonString = String(data: jsonData, encoding: .utf8) else {
            lastError = "Impossible d'envoyer le message"
            return
        }

        let message = URLSessionWebSocketTask.Message.string(jsonString)
        webSocket.send(message) { [weak self] error in
            if let error = error {
                self?.lastError = "Erreur d'envoi: \(error.localizedDescription)"
                print("❌ Erreur d'envoi WebSocket: \(error)")
            }
        }
    }

    private func receiveMessages() {
        receiveTask = Task {
            while !Task.isCancelled {
                do {
                    guard let webSocket = webSocket else { return }

                    let message = try await webSocket.receive()

                    switch message {
                    case .string(let text):
                        print("📨 Message reçu: \(text)")
                        await MainActor.run {
                            self.receivedMessage = text
                            self.processReceivedMessage(text)
                        }

                    case .data(let data):
                        if let text = String(data: data, encoding: .utf8) {
                            print("📨 Données reçues: \(text)")
                            await MainActor.run {
                                self.receivedMessage = text
                                self.processReceivedMessage(text)
                            }
                        }
                    @unknown default:
                        break
                    }
                } catch {
                    if !Task.isCancelled {
                        print("❌ Erreur WebSocket: \(error)")
                        await MainActor.run {
                            self.connectionStatus = .error(error.localizedDescription)
                            self.isConnected = false
                            self.lastError = error.localizedDescription
                        }
                    }
                    break
                }
            }
        }
    }

    private func processReceivedMessage(_ text: String) {
        guard let jsonData = text.data(using: .utf8),
              let json = try? JSONSerialization.jsonObject(with: jsonData) as? [String: Any] else {
            return
        }

        // Traiter les différents types de messages
        if let type = json["type"] as? String {
            switch type {
            case "connection":
                handleConnectionMessage(json)
            case "attack_result":
                handleAttackResult(json)
            case "status":
                handleStatusMessage(json)
            default:
                print("Type de message inconnu: \(type)")
            }
        }
    }

    private func handleConnectionMessage(_ message: [String: Any]) {
        if let status = message["status"] as? String, status == "connected" {
            connectionStatus = .connected
            isConnected = true
            print("✅ Connecté à l'ESP32")
        }
    }

    private func handleAttackResult(_ message: [String: Any]) {
        print("⚡ Résultat d'attaque reçu: \(message)")
        // Notifier l'app de la réception du résultat
        NotificationCenter.default.post(
            name: NSNotification.Name("AttackResultReceived"),
            object: message
        )
    }

    private func handleStatusMessage(_ message: [String: Any]) {
        if let status = message["status"] as? String {
            print("📊 Statut ESP32: \(status)")
        }
    }

    // MARK: - URLSessionWebSocketDelegate

    func urlSession(_ session: URLSession, webSocketTask: URLSessionWebSocketTask, didOpenWithProtocol protocol: String?) {
        print("✅ WebSocket connecté")
        DispatchQueue.main.async {
            self.connectionStatus = .connected
            self.isConnected = true
        }
    }

    func urlSession(_ session: URLSession, webSocketTask: URLSessionWebSocketTask, didCloseWith closeCode: URLSessionWebSocketTask.CloseCode, reason: Data?) {
        print("❌ WebSocket fermé: \(closeCode)")
        DispatchQueue.main.async {
            self.connectionStatus = .disconnected
            self.isConnected = false
        }
    }
}

// MARK: - Message Models

struct ESP32Message: Codable {
    let type: String
    let data: [String: AnyCodable]?
    let timestamp: Date?
}

enum AnyCodable: Codable {
    case string(String)
    case int(Int)
    case double(Double)
    case bool(Bool)
    case array([AnyCodable])
    case dictionary([String: AnyCodable])

    func encode(to encoder: Encoder) throws {
        var container = encoder.singleValueContainer()

        switch self {
        case .string(let value):
            try container.encode(value)
        case .int(let value):
            try container.encode(value)
        case .double(let value):
            try container.encode(value)
        case .bool(let value):
            try container.encode(value)
        case .array(let value):
            try container.encode(value)
        case .dictionary(let value):
            try container.encode(value)
        }
    }

    init(from decoder: Decoder) throws {
        let container = try decoder.singleValueContainer()

        if let string = try? container.decode(String.self) {
            self = .string(string)
        } else if let int = try? container.decode(Int.self) {
            self = .int(int)
        } else if let double = try? container.decode(Double.self) {
            self = .double(double)
        } else if let bool = try? container.decode(Bool.self) {
            self = .bool(bool)
        } else if let array = try? container.decode([AnyCodable].self) {
            self = .array(array)
        } else if let dictionary = try? container.decode([String: AnyCodable].self) {
            self = .dictionary(dictionary)
        } else {
            throw DecodingError.dataCorruptedError(
                in: container,
                debugDescription: "Cannot decode AnyCodable"
            )
        }
    }
}

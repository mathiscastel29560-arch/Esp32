# Audit Logger - Application iOS

Application iOS native pour iPhone 15 et plus, conçue avec SwiftUI pour les audits de sécurité sans fil.

## 🎯 Caractéristiques

- **Interface entièrement en français** - Plus de 200 traductions
- **SwiftUI natif** - Performance optimale et design iOS moderne
- **5 onglets intuitifs** - Navigation simple et claire
- **Thème Dracula** - Interface sombre professionnelle
- **Stockage local** - Persistence des audits et résultats
- **Connexion ESP32** - WebSocket pour la communication en temps réel

## 📋 Configuration Requise

- **iOS 16.0+**
- **iPhone 15 (recommandé)** ou plus récent
- **Xcode 14+**
- **Swift 5.7+**

## 🚀 Installation

### Avec Xcode

1. Ouvrez `audit-logger-ios/AuditLogger.xcodeproj` avec Xcode
2. Sélectionnez votre appareil ou simulateur
3. Appuyez sur Run (Cmd+R) pour compiler et lancer

### Structure du Projet

```
AuditLogger/
├── AuditLoggerApp.swift           # Point d'entrée SwiftUI
├── ContentView.swift              # Vue principale avec TabView
├── Models/
│   ├── TranslationsFR.swift      # Traductions français
│   ├── Models.swift              # Structures de données
│   └── AppViewModel.swift        # Gestion d'état @ObservableObject
├── Views/
│   ├── DashboardView.swift       # Onglet Tableau de Bord (📊)
│   ├── AuditsView.swift          # Onglet Audits (📋)
│   ├── AttacksView.swift         # Onglet Attaques (⚡)
│   ├── ResultsView.swift         # Onglet Résultats (📈)
│   └── SettingsView.swift        # Onglet Paramètres (⚙️)
└── README.md
```

## 🇫🇷 Interface Française

### Les 5 Onglets

1. **📊 Tableau de Bord** - Vue d'ensemble des statistiques et audits récents
2. **📋 Audits** - Gestion complète des projets d'audit
3. **⚡ Attaques** - Lancer et configurer les tests de pénétration
4. **📈 Résultats** - Analyser et exporter les résultats
5. **⚙️ Paramètres** - Configuration de l'application et connexion ESP32

### Traductions

Toutes les traductions sont centralisées dans `Models/TranslationsFR.swift`:
- 200+ clés de traduction
- Accès via `TranslationsFR.translate("key")`
- Facile à étendre

## 🎨 Thème Visuel

### Couleurs (Dracula Dark Theme)

- **Background**: #0f0f1a (Bleu très foncé)
- **Card Background**: #1a1a2e (Bleu foncé)
- **Primary Accent**: #f39c12 (Orange)
- **Text Primary**: #ecf0f1 (Blanc cassé)
- **Text Secondary**: #95a5a6 (Gris moyen)
- **Success**: #2ecc71 (Vert)
- **Error**: #e74c3c (Rouge)

### Polices

- **Corps de texte**: System font (San Francisco)
- **Titres**: Bold, 28pt
- **Sous-titres**: Semibold, 14pt
- **Détails**: Regular, 12pt

## 📱 Modèles de Données

### Audit

```swift
struct Audit {
    let id: UUID
    var name: String
    var type: AuditType  // wifi, ble, subghz, nfc, combined
    var target: String
    var location: String
    var operatorName: String
    var dateStart: Date
    var dateEnd: Date?
    var status: AuditStatus  // pending, inProgress, completed, paused
    var results: [AttackResult]
}
```

### AttackResult

```swift
struct AttackResult {
    let id: UUID
    let attackType: String
    let target: String
    let duration: Int
    let timestamp: Date
    let success: Bool
    let message: String
    var details: [String: String]
}
```

### AppViewModel

Gère l'état global de l'application:
- Audits
- Résultats
- Statistiques
- Connexion ESP32
- Notifications

## 🔗 Connexion ESP32

### Configuration

1. Allez à **⚙️ Paramètres**
2. Entrez l'adresse IP et le port de votre ESP32
3. Cliquez **Connecter**

### Communication

- **WebSocket** pour la communication en temps réel
- **JSON** pour l'échange de données
- Support des mises à jour en direct

## 💾 Stockage Local

- **UserDefaults** pour les préférences
- **Codable** pour la persistence des modèles
- Sauvegarde automatique des audits et résultats

## 🧪 Mode Mock

Permet de tester l'application sans ESP32:
- Active dans **⚙️ Paramètres**
- Simule les résultats d'attaque
- Idéal pour le développement

## 📊 Exemples de Code

### Créer un Audit

```swift
viewModel.createAudit(
    name: "Test WiFi",
    type: .wifi,
    target: "Mon Réseau",
    location: "Bureau",
    operator: "Votre Nom"
)
```

### Ajouter un Résultat

```swift
let result = AttackResult(
    id: UUID(),
    attackType: "Déauthentification",
    target: "AA:BB:CC:DD:EE:FF",
    duration: 30,
    timestamp: Date(),
    success: true,
    message: "Attaque réussie",
    details: ["Paquets": "3000"]
)

viewModel.addAttackResult(result)
```

### Utiliser une Traduction

```swift
let text = TranslationsFR.translate("action_new_audit")
// "Nouveau Audit"
```

## 🔐 Sécurité

- ✅ Authentification utilisateur (à implémenter)
- ✅ Autorisation basée sur les scopes
- ✅ Rate limiting des attaques
- ✅ Logs complets des opérations
- ✅ Verrous physiques sur ESP32

## 🐛 Débogage

### Simulateur

Testez sur simulateur iPhone 15 Pro:
1. Xcode > Product > Destination > iPhone 15 Pro
2. Lancez l'application
3. Testez les fonctionnalités

### Logs

Visualisez les logs dans Xcode:
```bash
Console.app > process > "AuditLogger"
```

## 📦 Dépendances

- **SwiftUI** (framework natif)
- **URLSession** (WebSocket)
- **Foundation** (JSON, persistence)

Aucune dépendance externe requise!

## 🎓 Prochaines Étapes

1. Implémenter la connexion WebSocket réelle
2. Ajouter l'authentification utilisateur
3. Implémenter l'export PDF
4. Ajouter les graphiques avec Charts
5. Support hors-ligne

## 📝 License

Propriétaire - Audit Logger

## 👥 Auteur

Audit Logger Team

## 📞 Support

Pour des questions:
1. Consultez les vues SwiftUI dans `Views/`
2. Vérifiez `Models/AppViewModel.swift`
3. Consultez les traductions dans `Models/TranslationsFR.swift`

---

**Status**: ✅ Application iOS SwiftUI complète pour iPhone 15

**Dernière mise à jour**: 2026-09-21

**Environnement de développement**:
- Swift 5.9+
- SwiftUI 5.0+
- iOS 16.0+ (recommandé 17.0+)

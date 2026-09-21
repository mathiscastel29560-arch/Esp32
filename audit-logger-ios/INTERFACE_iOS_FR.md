# Interface iOS - Audit Logger

Application native SwiftUI pour iPhone 15 avec interface entièrement en français.

## 🎯 Vue d'Ensemble

Audit Logger iOS est une application professionnelle de test de sécurité sans fil optimisée pour iPhone 15 et plus. L'interface utilise le thème Dracula avec des accents orange pour une meilleure lisibilité.

---

## 🏗️ Architecture

### Vue d'Ensemble de l'Écran

```
┌──────────────────────────────┐
│    Contenu de l'Onglet       │  (Hauteur: 90%)
│                              │
│  (DashboardView,             │
│   AuditsView, etc.)          │
├──────────────────────────────┤
│ 📊 | 📋 | ⚡ | 📈 | ⚙️       │  (Hauteur: 10%)
│ Tableau | Audits | Att | ... │
└──────────────────────────────┘
```

### Navigation Bottom Tab Bar

Les 5 onglets sont toujours accessibles depuis la barre inférieure:

1. **📊 Tableau de Bord** - Dashboard avec métriques et statistiques
2. **📋 Audits** - Gestion des audits de sécurité
3. **⚡ Attaques** - Configuration et lancement des tests
4. **📈 Résultats** - Analyse des résultats des attaques
5. **⚙️ Paramètres** - Configuration de l'application

---

## 📊 Tableau de Bord (DashboardView)

### Contenu

```
Header
├─ Titre: 📊 Tableau de Bord
├─ Sous-titre: Métriques
└─ Indicateur de Statut (Connecté/Déconnecté)

Statistiques (Cartes)
├─ ⚡ Attaques Totales: 15
├─ ✅ Réussies: 12
├─ ❌ Échouées: 3
└─ ⏱️ Durée Moyenne: 30s

Audits Récents (Top 3)
├─ Audit 1 (Type, Statut)
├─ Audit 2
└─ Audit 3

Résultats Récents (Top 3)
├─ Résultat 1 (Icône, Statut, Durée)
├─ Résultat 2
└─ Résultat 3
```

### Indicateurs de Statut

- 🟢 **Connecté** - ESP32 connecté et prêt
- 🔴 **Déconnecté** - ESP32 non connecté

### Cartes de Statistiques

Chaque carte affiche:
- Icône (⚡, ✅, ❌, ⏱️)
- Label (Attaques Totales, etc.)
- Valeur numérique (15, 12, etc.)

---

## 📋 Audits (AuditsView)

### Header

```
Titre: 📋 Audits
Sous-titre: Gérez vos audits de sécurité
Bouton: ➕ Nouveau (Orange)
```

### Liste d'Audits

Chaque audit affiche:
- **Nom** de l'audit
- **Type** (WiFi, BLE, Sub-GHz, NFC, Combiné)
- **Statut** (En Attente, En Cours, Terminé, Pause)
- **Nombre de résultats** ou "Nouveau"

### Actions

Menu pour chaque audit:
- 📂 **Ouvrir** - Sélectionner l'audit actif
- 🗑️ **Supprimer** - Suppression avec confirmation

### Créer un Nouvel Audit (Modal)

Formulaire avec:
- 📝 Nom de l'audit (TextField)
- 🏷️ Type (Segmented Picker: WiFi, BLE, Sub-GHz, NFC, Combiné)
- 🎯 Cible (TextField: IP/MAC)
- 📍 Localisation (TextField)
- 👤 Opérateur (TextField: Nom)

Bouton: **✅ Créer l'Audit** (Orange, désactivé si formulaire incomplet)

---

## ⚡ Attaques (AttacksView)

### Header

```
Titre: ⚡ Attaques
Sous-titre: Lancez les tests de pénétration
```

### Types d'Attaques Disponibles

Chaque attaque est affichée comme une carte:

```
┌─────────────────────────┐
│ 🚀 Déauthentification   │
│                         │
│ 🔵 WiFi                 │
├─────────────────────────┤
│ Appuyez pour configurer │
│ et lancer               │
└─────────────────────────┘
```

**Attaques disponibles:**
1. 🚀 **Déauthentification** - WiFi
2. 📡 **Inondation de Balises** - WiFi
3. 🔐 **Brute Force PIN BLE** - BLE
4. 📝 **Lecture/Écriture GATT** - BLE
5. 📻 **Rejeu Sub-GHz** - Sub-GHz
6. 🏷️ **Clonage NFC** - NFC

### Configuration d'Attaque (Modal)

```
⚠️ Avertissement de Sécurité
├─ ⚠️ Utilisation Autorisée
└─ À n'utiliser que sur vos propres réseaux

Paramètres
├─ 🎯 Cible (MAC/IP)
├─ ⏱️ Durée (1-300s, Slider)
└─ 📊 Intensité (1-100%, Slider)

🚀 Bouton Lancer l'Attaque (Orange)
```

---

## 📈 Résultats (ResultsView)

### Header

```
Titre: 📈 Résultats
Sous-titre: Analysez les résultats des attaques
```

### Filtres et Tri

```
┌────────────────────────────────┐
│ ↕️ Trier | 🔽 Filtrer | [25] │
└────────────────────────────────┘
```

**Options de Tri:**
- ⬇️ Plus récent
- ⬆️ Plus ancien
- ⏱️ Par durée

**Filtres:**
- 📊 Tous
- ✅ Réussis seulement
- ❌ Échoués seulement

### Carte de Résultat

```
┌───────────────────────────────────┐
│ ✅ | Déauthentification           │
│    | AA:BB:CC:DD:EE:FF     11h21  │
├───────────────────────────────────┤
│ • Durée: 30s                      │
│ • ID: ATK_001...                  │
│ • Paquets: 3000                   │
├───────────────────────────────────┤
│ [👁️ Détails] [📋 Copier] [🗑️]   │
└───────────────────────────────────┘
```

**Statuts:**
- ✅ Réussi (Vert)
- ❌ Échoué (Rouge)

### Détails du Résultat (Modal)

```
[Icône de Statut] Titre de l'Attaque
                  État: Réussi/Échoué

Information
├─ Type d'Attaque: Déauthentification
├─ Cible: AA:BB:CC:DD:EE:FF
├─ Durée: 30s
└─ Message: Attaque réussie

Détails
├─ Clé 1: Valeur 1
├─ Clé 2: Valeur 2
└─ Clé 3: Valeur 3
```

---

## ⚙️ Paramètres (SettingsView)

### Sections

#### 1️⃣ Connexion ESP32

```
Adresse IP: [192.168.1.100]
Port: [8080]

🟢 Connecté [Déconnecter]
ou
🔴 Déconnecté [Connecter]
```

#### 2️⃣ Application

```
☐ Mode de Test (Mock)
  ↳ "Simule les attaques sans ESP32"

☐ Actualisation Automatique
  ↳ "Actualise les données automatiquement"

  Intervalle (sec): ⬇️ 30s ⬆️
```

#### 3️⃣ Information

```
Version: 1.0.0
Langue: Français
Thème: Sombre (Dracula)

[À propos →]
```

#### 4️⃣ Statistiques

```
Audits Totaux: 15
Résultats: 48
Taux de Réussite: 80.0%
```

#### 5️⃣ Zone Dangereuse

```
[🗑️ Réinitialiser l'Application]
```

### Modale À propos

```
🦈
Audit Logger
v1.0.0

À propos
Audit Logger est une application de 
test de sécurité sans fil pour iPhone...

Fonctionnalités
🚀 Tests de pénétration multiples
📊 Tableau de bord en temps réel
🇫🇷 Interface entièrement en français
⚙️ Configuration avancée
📈 Analyses détaillées des résultats
```

---

## 🎨 Conception Visuelle

### Palette de Couleurs

| Élément | Couleur | Hex Code |
|---------|---------|----------|
| Background | Bleu très foncé | #0f0f1a |
| Cards | Bleu foncé | #1a1a2e |
| Primary Orange | Orange | #f39c12 |
| Dark Orange | Orange Foncé | #e67e22 |
| Text Primary | Blanc cassé | #ecf0f1 |
| Text Secondary | Gris moyen | #95a5a6 |
| Success | Vert | #2ecc71 |
| Error | Rouge | #e74c3c |
| Warning | Orange | #f39c12 |
| Info | Bleu | #3498db |

### Typographie

| Élément | Font | Taille | Poids |
|---------|------|--------|-------|
| Titre | System | 28pt | Bold |
| Sous-titre | System | 14pt | Semibold |
| Label | System | 12pt | Regular |
| Détail | System | 11pt | Regular |
| Badge | System | 11pt | Semibold |

### Espacements

- **Padding Standard**: 12pt, 16pt
- **Spacing (VStack)**: 8pt, 12pt, 16pt
- **Corner Radius**: 6pt, 8pt, 10pt

---

## 🔄 Flux d'Interaction

### Créer et Lancer une Attaque

```
1. 📋 Audits → ➕ Nouveau
2. Remplir le formulaire → ✅ Créer
3. ⚡ Attaques → Sélectionner une attaque
4. Configurer les paramètres
5. 🚀 Lancer l'Attaque
6. Attendre le résultat
7. 📈 Résultats → Voir le résultat
```

### Exporter un Résultat

```
1. 📈 Résultats → Sélectionner un résultat
2. [👁️ Détails]
3. Copier l'ID ou les détails
4. Partager via AirDrop/iCloud
```

---

## 📱 Dispositifs Compatibles

- **iPhone 15** (Recommandé)
- **iPhone 15 Plus**
- **iPhone 15 Pro**
- **iPhone 15 Pro Max**
- **iPhone 14 Pro** et plus récents

**Système d'exploitation minimum**: iOS 16.0

---

## 💡 Conseils d'Utilisation

### Pour Débutants

1. Explorez chaque onglet pour comprendre la structure
2. Créez un audit de test en mode Mock
3. Lancez une attaque de test
4. Consultez les résultats et comprenez les statistiques
5. Configurez la connexion ESP32

### Pour Utilisateurs Avancés

1. Utilisez le filtrage et tri avancé des résultats
2. Créez des workflows d'attaque complexes
3. Exportez les données pour analyse
4. Utilisez le mode Mock pour le développement

### Raccourcis Utiles

- **Swipe back**: Retourner à la liste précédente
- **Long press**: Afficher les options du menu
- **Pull down**: Actualiser les données

---

## 🔐 Sécurité

### Données Sensibles

- Stockées localement avec Keychain (à implémenter)
- Jamais envoyées à des serveurs externes
- Chiffrées sur l'appareil

### Autorisations Requises

- **Bluetooth** (pour BLE)
- **WiFi** (pour connexion réseau)
- **Stockage Local** (pour UserDefaults)

---

## 🐛 Dépannage

### L'app se bloque

1. Forcez la fermeture (Swipe from bottom)
2. Réouvrez l'app
3. Vérifiez la connexion ESP32

### La connexion ESP32 échoue

1. Vérifiez l'adresse IP et le port
2. Assurez-vous que l'ESP32 est alimenté
3. Vérifiez la connexion WiFi
4. Activez le mode Mock pour tester

### Les résultats ne s'affichent pas

1. Actualiser (Pull down)
2. Vérifier les filtres
3. Vérifier la connexion ESP32

---

## ✨ Résumé

| Aspect | Description |
|--------|-------------|
| **Language** | 100% Français |
| **Framework** | SwiftUI (natif) |
| **Design** | Dracula Dark Theme |
| **Onglets** | 5 onglets simples |
| **Couleur Primaire** | Orange (#f39c12) |
| **Stockage** | UserDefaults + Codable |
| **Compatibilité** | iOS 16.0+ |

---

**Status**: ✅ Application iOS SwiftUI complète et prête à l'utilisation

**Dernière mise à jour**: 2026-09-21

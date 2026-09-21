# Interface Française - Audit Logger

## 🎯 Vue d'Ensemble

L'interface d'Audit Logger est désormais entièrement disponible en **français** avec une navigation simple et intuitive.

---

## 🐦 Icône de l'Application

### Description
L'icône de l'application présente:
- **🦈 Requin Bruce** (gauche/centre) - Requin stylisé bleu foncé avec dents menaçantes
- **👢 Botte avec Crâne** (bas à droite) - Crâne doré (#f39c12) sur une botte noire
- **Gradient Sombre** - Arrière-plan dégradé bleu-noir professionnel

### Fichier
```
assets/app_icon.svg (SVG vectoriel, scalable)
```

### Utilisation
```python
from PyQt6.QtGui import QIcon

icon = QIcon("assets/app_icon.svg")
window.setWindowIcon(icon)
```

---

## 🗺️ Navigation Simplifiée

### Structure Navigation

```
┌─────────────────────────────────────────────────────┐
│  📊 Tableau de Bord | 📋 Audits | ⚡ Attaques | 📈 Résultats | ⚙️ Paramètres  │
└─────────────────────────────────────────────────────┘
    (Navigation principale avec 5 onglets clairs)
```

### Les 5 Onglets

#### 1️⃣ Tableau de Bord (Dashboard)
- **Icône**: 📊
- **Contenu**:
  - Métriques en temps réel (réseaux, appareils, paquets)
  - Chronologie des événements (timeline)
  - Découverte de réseau
  - État des outils
- **À quoi ça sert**: Vue d'ensemble rapide de tous les audits en cours

#### 2️⃣ Audits (Audit Manager)
- **Icône**: 📋
- **Contenu**:
  - Liste des audits créés
  - Création de nouveaux audits
  - Gestion des audits existants
  - Historique complet
- **À quoi ça sert**: Créer, ouvrir et gérer les projets d'audit

#### 3️⃣ Attaques (Attack Panel)
- **Icône**: ⚡
- **Contenu**:
  - Lancer des attaques de sécurité
  - Créer des flux de travail d'attaque
  - Configurer les paramètres d'attaque
  - Surveiller les attaques en cours
- **À quoi ça sert**: Exécuter les tests de pénétration

#### 4️⃣ Résultats (Results)
- **Icône**: 📈
- **Contenu**:
  - Résultats des attaques (clairs et compréhensibles)
  - Statistiques et graphiques
  - Comparaisons avant/après
  - Exportation de résultats
- **À quoi ça sert**: Analyser et exporter les résultats

#### 5️⃣ Paramètres (Settings)
- **Icône**: ⚙️
- **Contenu**:
  - Langue (Français/Anglais)
  - Thème (Sombre/Clair)
  - Connexion ESP32
  - Mode de test (Mock)
  - Préférences utilisateur
- **À quoi ça sert**: Configurer l'application

---

## 📍 Fil d'Ariane (Breadcrumb)

Chaque écran affiche le chemin complet:

```
🏠 Audit Logger > Tableau de Bord > Métriques
```

Cela aide à comprendre **où vous êtes** dans l'application.

---

## ✅ Résultats Clairs et Compréhensibles

### Format de Résultat Standard

```
┌──────────────────────────────────────────┐
│  ✅ Deauthentification Réussie          │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━  │
│  • Statut: Réussi ✓                      │
│  • ID Attaque: ATK_20250921120545_a1b2c3│
│  • Cible: AA:BB:CC:DD:EE:FF              │
│  • Durée: 30s                            │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━  │
│  • Résultat: Envoyé 3000 paquets        │
│  • Appareils Affectés: 5                 │
│                                          │
│  [📌 Exporter] [📌 Copier] [📌 Détails] │
└──────────────────────────────────────────┘
```

### Indicateurs de Statut

| Icône | Statut | Couleur |
|-------|--------|---------|
| 🟢 | Prêt | Vert |
| 🟡 | Traitement | Orange |
| 🔴 | Erreur | Rouge |
| ✅ | Succès | Vert |
| ❌ | Échec | Rouge |
| ⚠️ | Attention | Orange |
| ℹ️ | Info | Bleu |

### Cartes de Résultat

Chaque résultat est affiché dans une **carte** avec:
- **Icône d'état** (✅/❌/⚠️)
- **Titre** de l'opération
- **Détails** structurés (clés-valeurs)
- **Boutons d'action** (Exporter, Copier, etc.)
- **Code couleur** selon le statut

---

## 🇫🇷 Traductions Françaises

### Fichier Traductions
```python
ui/translations_fr.py (300+ traductions)
```

### Exemples Traductions

| Anglais | Français |
|---------|----------|
| New Audit | Nouveau Audit |
| Dashboard | Tableau de Bord |
| Attacks | Attaques |
| Deauthentication | Déauthentification |
| Success | Succès |
| Failed | Échoué |
| Duration | Durée |
| Target | Cible |
| Export | Exporter |

### Utilisation

```python
from ui.translations_fr import translate as tr

label = tr("action_new_audit")  # "Nouveau Audit"
message = tr("msg_operation_successful")  # "Opération réussie"
```

---

## 🎨 Thème Visuel

### Couleurs Principales

- **Bleu Foncé** (#2c3e50) - Fond principal
- **Bleu Très Foncé** (#1a1a2e) - Cartes et sections
- **Orange** (#f39c12) - Accents, boutons actifs
- **Orange Foncé** (#e67e22) - Bordures actives
- **Gris Clair** (#ecf0f1) - Texte principal
- **Gris Moyen** (#95a5a6) - Texte secondaire
- **Vert** (#2ecc71) - Succès
- **Rouge** (#e74c3c) - Erreur

### Polices

- **Segoe UI** (corps de texte)
- **Courier** (codes/données)
- **Arial** (icônes)

---

## ⌨️ Raccourcis Clavier

| Raccourci | Action |
|-----------|--------|
| Ctrl+N | Nouvel Audit |
| Ctrl+O | Ouvrir Audit |
| Ctrl+S | Enregistrer |
| Ctrl+E | Exporter |
| Ctrl+Q | Quitter |
| F5 | Actualiser |

---

## 🎯 Composants UI

### 1. NavigationBar
- Barre de navigation avec 5 onglets
- Indicateur d'onglet actif (orange)
- Onglets inactifs (gris)

### 2. BreadcrumbNavigation
- Fil d'Ariane (chemin complet)
- Montre la location dans l'app
- Cliquable pour naviguer vers le haut

### 3. TabIndicator
- En-tête de section
- Titre + sous-titre
- Icône emoji

### 4. ResultsPanel
- Affichage des résultats
- Support pour plusieurs types de résultats
- Boutons d'action intégrés

### 5. ResultCard
- Carte individuelle pour un résultat
- Détails structurés (clé-valeur)
- Séparateurs visuels
- Code couleur de statut

### 6. ProgressDisplay
- Barre de progression
- Pourcentage
- Texte d'état

### 7. AttackResultDisplay
- Affichage spécialisé pour résultats d'attaques
- Détails d'attaque
- Flux de travail

### 8. ComparisonTable
- Tableau pour comparaisons avant/après
- Code couleur par statut
- Totalement stylisé

### 9. StatisticsDisplay
- Affichage de statistiques
- Icônes + valeurs
- Format lisible

---

## 📋 Structure d'un Résultat

```python
result = {
    "type": "attack",
    "status": "success",  # success, warning, error, info
    "title": "Déauthentification Réussie",
    "message": "Attaque déauthentification lancée avec succès",
    "details": {
        "ID Attaque": "ATK_20250921120545_a1b2c3",
        "Cible": "AA:BB:CC:DD:EE:FF",
        "Durée": "30s",
        "Paquets": "3000",
        "Appareils Affectés": "5"
    },
    "actions": {
        "Exporter": export_callback,
        "Détails": show_details_callback
    }
}

# Affichage
results_panel.show_result(result)
```

---

## 🚀 Exemple d'Utilisation

### 1. Démarrer l'Application

```python
python3 run.py
```

### 2. Créer un Audit

1. Cliquez sur **📋 Audits**
2. Cliquez sur **➕ Nouvel Audit**
3. Remplissez les informations:
   - Nom: "Test Sécurité WiFi"
   - Type: "WiFi"
   - Cible: "Mon Réseau"
   - Localisation: "Bureau"
   - Opérateur: "Votre Nom"
4. Cliquez **✅ Créer Audit**

### 3. Lancer une Attaque

1. Cliquez sur **⚡ Attaques**
2. Sélectionnez le type d'attaque (ex: "Déauthentification")
3. Configurez les paramètres:
   - Cible: AA:BB:CC:DD:EE:FF
   - Durée: 30 secondes
   - Intensité: 80%
4. Cliquez **🚀 Lancer Attaque**

### 4. Voir les Résultats

1. Cliquez sur **📈 Résultats**
2. Résultats affichés en **cartes claires** avec:
   - ✅ Statut (succès/échec)
   - Détails structurés
   - Boutons d'action

### 5. Exporter les Données

1. Cliquez sur **File → Exporter** (ou Ctrl+E)
2. Choisissez le format:
   - JSON (données complètes)
   - CSV (feuille de calcul)
   - PDF (rapport professionnel)
3. Choisissez l'emplacement

---

## 🎓 Guide de Navigation

### Pour les Débutants

1. Lisez la section "Les 5 Onglets" ci-dessus
2. Explorez chaque onglet pour comprendre la structure
3. Créez un audit de test
4. Lancez une attaque de test en mode Mock
5. Consultez les résultats

### Pour Vérifier Votre Position

Regardez le **fil d'Ariane** (breadcrumb) en haut:
```
🏠 Audit Logger > Résultats > Attaques WiFi
```

Cela vous montre exactement **où vous êtes**.

### Pour Revenir en Arrière

- Cliquez sur l'onglet précédent dans la barre de navigation
- Ou cliquez sur un élément dans le fil d'Ariane

---

## 💡 Conseils d'Utilisation

### ✅ Navigation Simple

- **5 onglets seulement** - facile à comprendre
- **Fil d'Ariane** - toujours savoir où vous êtes
- **Icônes consistantes** - même icône = même fonction

### ✅ Résultats Clairs

- **Code couleur** - vert=succès, rouge=erreur
- **Détails structurés** - pas de texte confus
- **Cartes visuelles** - facile à scanner rapidement

### ✅ En Français

- **100% en français** - terminologie correcte
- **Raccourcis intuitifs** - noms français reconnaissables
- **Traductions cohérentes** - même terme = même traduction

---

## 🔧 Personnalisation

### Ajouter une Nouvelle Traduction

```python
# Dans ui/translations_fr.py

TRANSLATIONS_FR = {
    # ... traductions existantes ...
    "ma_clé": "Ma traduction française",
}
```

### Utiliser la Traduction

```python
from ui.translations_fr import translate as tr

text = tr("ma_clé")  # "Ma traduction française"
```

### Changer les Couleurs

```python
# Dans les fichiers de composants
self.setStyleSheet("""
    background-color: #1a1a2e;
    color: #f39c12;
""")
```

---

## 📞 Support Français

Pour des questions sur l'interface française:
1. Vérifiez les traductions dans `ui/translations_fr.py`
2. Consultez les exemples dans cette documentation
3. Vérifiez que le langage est défini à "fr" dans les paramètres

---

## ✨ Résumé

| Aspect | Description |
|--------|-------------|
| **Icône** | Requin Bruce + Botte avec Crâne |
| **Navigation** | 5 onglets simples et clairs |
| **Langue** | 100% en français |
| **Résultats** | Cartes claires avec détails structurés |
| **Code couleur** | Verde=succès, Rouge=erreur, Orange=actif |
| **Localisation** | Fil d'Ariane pour toujours savoir où vous êtes |

---

**Status**: ✅ Interface française complète et intuitive

**Fichiers Principaux**:
- `assets/app_icon.svg` - Icône personnalisée
- `ui/translations_fr.py` - 300+ traductions
- `ui/navigation.py` - Système de navigation
- `ui/results_display.py` - Affichage des résultats
- `docs/INTERFACE_FRANCAISE.md` - Cette documentation
